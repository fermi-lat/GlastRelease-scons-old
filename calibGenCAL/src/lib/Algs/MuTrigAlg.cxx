// $Header$

/** @file
    @author fewtrell
*/

// LOCAL INCLUDES
#include "MuTrigAlg.h"
#include "../Specs/singlex16.h"
#include "../Util/RootFileAnalysis.h"
#include "../Util/CGCUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalVec.h"
#include "digiRootData/DigiEvent.h"
#include "CalUtil/SimpleCalCalib/CalPed.h"

// EXTLIB INCLUDES
#include "TGraphErrors.h"
#include "TF1.h"
#include "TStyle.h"

// STD INCLUDES
#include <set>
#include <sstream>

namespace calibGenCAL {

  using namespace std;
  using namespace CalUtil;
  using namespace singlex16;

  static const unsigned short N_MUADC_BINS     = 100;
  static const unsigned short MUADC_BINSIZE    = 30;

  MuTrigAlg::MuTrigAlg() :
    m_muThresh(FaceIdx::N_VALS, INVALID_ADC),
    m_muThreshWidth(FaceIdx::N_VALS, INVALID_ADC),
    m_ciThresh(FaceIdx::N_VALS, INVALID_ADC),
    m_ciThreshWidth(FaceIdx::N_VALS, INVALID_ADC),

    m_muThreshErr(FaceIdx::N_VALS, INVALID_ADC),
    m_muThreshWidthErr(FaceIdx::N_VALS, INVALID_ADC),
    m_ciThreshErr(FaceIdx::N_VALS, INVALID_ADC),
    m_ciThreshWidthErr(FaceIdx::N_VALS, INVALID_ADC),

    m_ciTrigSum(FaceIdx::N_VALS, vector<unsigned short>(N_CIDAC_VALS)),
    m_ciAdcN(FaceIdx::N_VALS, vector<unsigned short>(N_CIDAC_VALS)),
    m_ciADCSum(FaceIdx::N_VALS, vector<unsigned>(N_CIDAC_VALS)),
    m_delPed(FaceIdx::N_VALS, 0)
  {
    initHists();
  }

  void MuTrigAlg::fillMuonHists(const TRIG_CFG trigCfg,
                                const string &filename,
                                unsigned nEvents,
                                const CalPed &peds,
                                const bool calLOEnabled) {
    // open input root file.
    vector<string> rootFileList;
    rootFileList.push_back(filename);
    RootFileAnalysis rootFile(0,
                              &rootFileList,
                              0);

    // enable only needed branches in root file
    rootFile.getDigiChain()->SetBranchStatus("*", 0);
    rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");
    rootFile.getDigiChain()->SetBranchStatus("m_calDiagnosticCloneCol");

    nEvents = min < unsigned > (nEvents, rootFile.getEntries());
    LogStrm::get() << __FILE__ << ": Processing: " << nEvents << " events." << endl;

    //-- REUSED ARRAYS (reset after each event) --//
    // pedestal subracted LEX8 adc for each xtal face in cal
    CalVec<FaceIdx, float> adcPed;
    // array of all fle bits in cal.
    CalVec<LyrIdx, CalVec<FaceNum, bool> >
      fle(LyrIdx::N_VALS);

    CalVec<LyrIdx, CalVec<FaceNum, unsigned short> >
      hitsPerLyr(LyrIdx::N_VALS);
    // keep track of which columns are hit in each layer.
    // since we only process layers w/ 1 column hit, this value
    // will represent the only hit column in this layer
    CalVec<LyrIdx, CalVec<FaceNum, ColNum> >
      columnHit(LyrIdx::N_VALS);

    /// list of cal_lo triggers
    /// used for avoiding bias when there is no independant
    /// trigger for the cal.  in this case, another layer in the
    /// cal must be used as the independant trigger.
    set<LyrIdx> trigList;

    // Basic digi-event loop
    for (unsigned eventNum = 0; eventNum < nEvents; eventNum++) {
      if (eventNum % 1000 == 0) {
        LogStrm::get() << "Event: " << eventNum << endl;
        LogStrm::get().flush();
      }

      if (!rootFile.getEvent(eventNum)) {
        LogStrm::get() << "Warning, event " << eventNum << " not read." << endl;
        continue;
      }

      const DigiEvent *digiEvent = rootFile.getDigiEvent();
      if (!digiEvent) {
        LogStrm::get() << __FILE__ << ": Unable to read DigiEvent " << eventNum  << endl;
        continue;
      }

      //-- intialize arrays --//
      fill(adcPed.begin(), adcPed.end(), 0);
      for (LyrIdx lyrIdx; lyrIdx.isValid(); lyrIdx++) {
        fill(fle[lyrIdx].begin(), fle[lyrIdx].end(), false);
        fill(hitsPerLyr[lyrIdx].begin(), hitsPerLyr[lyrIdx].end(), 0);
        fill(columnHit[lyrIdx].begin(), columnHit[lyrIdx].end(), 0);
      }
      if (calLOEnabled)
        trigList.clear();

      ////////////////////////////////////////////////////////////
      //-- LOAD UP FLE & ADC DATA FOR ALL EVENTS INTO ARRAYS  --//
      ////////////////////////////////////////////////////////////
      //-- populate fle diagnostic info --//
      const TClonesArray *calDiagCol = digiEvent->getCalDiagnosticCol();
      TIter calDiagIter(calDiagCol);

      CalDiagnosticData  *pdiag = 0;
      while ((pdiag = (dynamic_cast<CalDiagnosticData *>(calDiagIter.Next())))) {
        //loop through each 'hit' in one event
        CalDiagnosticData &cdiag = *pdiag;    // use ref to reduce '->'
        const TwrNum twr(cdiag.tower());
        const LyrNum lyr(cdiag.layer());
        const LyrIdx lyrIdx(twr,
                      lyr);

        for (FaceNum face; face.isValid(); face++) {
          const bool trig = cdiag.low(face.val()) != 0;
          fle[lyrIdx][face] = trig;
          // keep track of how many triggered layers per
          // tower so i can avoid bias when there is only one.
          if (calLOEnabled && trig)
            trigList.insert(lyrIdx);
        }
      }

      //-- loop through each 'hit' in one event --//
      const TClonesArray *calDigiCol = digiEvent->getCalDigiCol();
      if (!calDigiCol) {
        LogStrm::get() << "no calDigiCol found for event#" << eventNum << endl;
        continue;
      }

      TIter calDigiIter(calDigiCol);

      const CalDigi      *pCalDigi = 0;

      //-- initialize adc values --
      while ((pCalDigi = dynamic_cast<CalDigi *>(calDigiIter.Next()))) {
        const     CalDigi &calDigi = *pCalDigi;    // use reference to avoid -> syntax

        // get interaction information
        const CalXtalId id(calDigi.getPackedId());
        const ColNum    col(id.getColumn());
        const LyrNum    lyr(id.getLayer());
        // only check enabled xtals
        if (!xtalEnabled(trigCfg, lyr.getGCRC(), col))
          continue;
        const TwrNum    twr(id.getTower());

        for (FaceNum face; face.isValid(); face++) {
          const unsigned short adc(calDigi.getAdcSelectedRange(LEX8.val(), (CalXtalId::XtalFace)(face.val())));

          const FaceIdx faceIdx(twr,
                          lyr,
                          col,
                          face);
          const RngIdx  rngIdx(faceIdx,
                         LEX8);

          const float ped(peds.getPed(rngIdx));
          adcPed[faceIdx] = adc - ped;

          // count hits per layer on enabled
          // xtals.
          if (adcPed[faceIdx] > 50) {
            LyrIdx lyrIdx(twr,
                          lyr);

            hitsPerLyr[lyrIdx][face]++;
            columnHit[lyrIdx][face] = col;
          }
        }
      }

      ////////////////////////////////////////////////////////////
      //-- PROCESS EVENT ARRAY & DETERMINE TRIGGER EFFICIENCY --//
      ////////////////////////////////////////////////////////////

      for (LyrIdx lyrIdx; lyrIdx.isValid(); lyrIdx++) {
        // skip if this layer is the only trigger
        // as it causes a bias
        if (calLOEnabled)
          if (trigList.size() == 1 &&
              trigList.find(lyrIdx) != trigList.end())
            continue;

        const LyrNum lyr(lyrIdx.getLyr());
        const TwrNum twr(lyrIdx.getTwr());

        for (FaceNum face; face.isValid(); face++) {
          // every column other than this column in this layer
          // must not have a 'hit'
          if (hitsPerLyr[lyrIdx][face] > 1)
            continue;

          for (ColNum col; col.isValid(); col++) {
            // only process enabled xtals for this cfg.
            if (!xtalEnabled(trigCfg, lyr.getGCRC(), col))
              continue;

            // every column other than this column in this layer
            // must not have a 'hit'
            if (hitsPerLyr[lyrIdx][face] == 1 &&
                col != columnHit[lyrIdx][face])
              continue;

            const FaceIdx faceIdx(twr,
                            lyr,
                            col,
                            face);

            m_muAdcHists[faceIdx]->Fill(adcPed[faceIdx]);
            if (fle[lyrIdx][face])
              m_muTrigHists[faceIdx]->Fill(adcPed[faceIdx]);
          }
        }
      }
    }
  }

  void MuTrigAlg::fillCIHists(const string &filename) {
    // open input root file.
    vector<string> rootFileList;
    rootFileList.push_back(filename);
    RootFileAnalysis rootFile(0,
                              &rootFileList,
                              0);

    // enable only needed branches in root file
    rootFile.getDigiChain()->SetBranchStatus("*", 0);
    rootFile.getDigiChain()->SetBranchStatus("m_calDigiCloneCol");
    rootFile.getDigiChain()->SetBranchStatus("m_calDiagnosticCloneCol");

    const unsigned nEvents(rootFile.getEntries());
    LogStrm::get() << __FILE__ << ": Processing: " << nEvents << " events." << endl;

    // array of all fle bits in cal.
    CalVec<LyrIdx, CalVec<FaceNum, bool> > fle;

    unsigned nGoodEvents = 0;
    // BEGINNING OF EVENT LOOP
    for (unsigned eventNum = 0 ; eventNum < nEvents; eventNum++) {
      if (eventNum%1000 == 0) {
        LogStrm::get() << " event " << eventNum << '\n';
        LogStrm::get().flush();
      }

      if (!rootFile.getEvent(eventNum)) {
        LogStrm::get() << "Warning, event " << eventNum << " not read." << endl;
        continue;
      }

      const DigiEvent    *digiEvent = rootFile.getDigiEvent();
      if (!digiEvent) {
        LogStrm::get() << __FILE__ << ": Unable to read DigiEvent " << eventNum  << endl;
        continue;
      }

      // determine current test setting
      const ColNum testCol      = nGoodEvents/N_PULSES_PER_XTAL;
      const unsigned testDAC    = (nGoodEvents%N_PULSES_PER_XTAL)/N_PULSES_PER_DAC;

      //-- populate fle diagnostic info --//
      const TClonesArray *calDiagCol = digiEvent->getCalDiagnosticCol();
      TIter calDiagIter(calDiagCol);

      // intialize to false
      for (LyrIdx lyrIdx; lyrIdx.isValid(); lyrIdx++)
        fill(fle[lyrIdx].begin(), fle[lyrIdx].end(), false);

      CalDiagnosticData  *pdiag(0);
      while ((pdiag = dynamic_cast<CalDiagnosticData *>(calDiagIter.Next()))) {
        //loop through each 'hit' in one event
        CalDiagnosticData &cdiag = *pdiag;    // use ref to reduce '->'
        const TwrNum twr(cdiag.tower());
        const LyrNum lyr(cdiag.layer());

        const LyrIdx lyrIdx(twr,
                      lyr);

        for (FaceNum face; face.isValid(); face++)
          fle[lyrIdx][face] = (cdiag.low(face.val()) != 0);
      }

      // Loop through each xtal interaction
      const TClonesArray *calDigiCol = digiEvent->getCalDigiCol();
      if (!calDigiCol) {
        ostringstream tmp;
        tmp << __FILE__ << ":" << __LINE__ << " "
            << "Empty calDigiCol event #" << eventNum;
        throw runtime_error(tmp.str());
      }
      TIter calDigiIter(calDigiCol);

      CalDigi            *pdig   = 0;
      const unsigned nDigis = calDigiCol->GetEntries();
      // event should have 1 hit for every xtal in each tower
      // we support any nTowers
      if (nDigis > 0 && nDigis%tXtalIdx::N_VALS == 0) {
        nGoodEvents++;
        while ((pdig = dynamic_cast<CalDigi *>(calDigiIter.Next()))) {
          //loop through each 'hit' in one event
          CalDigi &calDigi = *pdig;                  // use ref to reduce '->'

          const CalXtalId id(calDigi.getPackedId());     // get interaction information
          const ColNum col(id.getColumn());
          if (col != testCol) continue;

          const TwrNum    twr(id.getTower());
          const LyrNum    lyr(id.getLayer());

          for (FaceNum face; face.isValid(); face++) {
            const unsigned short adc(calDigi.getAdcSelectedRange(LEX8.val(), (CalXtalId::XtalFace)(face.val())));

            const FaceIdx faceIdx(twr,
                            lyr,
                            col,
                            face);

            m_ciADCSum[faceIdx][testDAC] += adc;
            m_ciAdcN[faceIdx][testDAC]++;

            if (fle[LyrIdx(twr, lyr)][face])
              m_ciTrigSum[faceIdx][testDAC]++;
          }
        }
      }
    }
  }

  void MuTrigAlg::writeCIMetavals(const string &filename) const {
    ofstream outfile(filename.c_str());

    if (!outfile.is_open())
      throw runtime_error(string("Unable to open " + filename));

    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      const TwrNum  twr(faceIdx.getTwr());
      const LyrNum  lyr(faceIdx.getLyr());
      const ColNum  col(faceIdx.getCol());
      const FaceNum face(faceIdx.getFace());

      for (unsigned dacIdx = 0;
           dacIdx < N_CIDAC_VALS;
           dacIdx++)
        outfile << " " << twr.val()
                << " " << lyr.val()
                << " " << col.val()
                << " " << face.val()
                << " " << dacIdx
                << " " << CIDAC_TEST_VALS[dacIdx]
                << " " << m_ciADCSum[faceIdx][dacIdx]
                << " " << m_ciAdcN[faceIdx][dacIdx]
                << " " << m_ciTrigSum[faceIdx][dacIdx]
                << endl;
    }
  }

  void MuTrigAlg::fitData(const CalPed &ped) {
    const auto_ptr<TF1> step(new TF1("step", "1.0/(1.0+exp(-[1]*(x-[0])))", 0, MUADC_BINSIZE*N_MUADC_BINS));

    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      const RngIdx rngIdx(faceIdx,
                    LEX8);

      float adcThresh = 0.0;

      ////////////////
      //-- CI EFF --//
      ////////////////
      // skip empty channels
      if (m_ciAdcN[faceIdx][0] != 0) {
        float adcMean[N_CIDAC_VALS],
          adcErr[N_CIDAC_VALS],
          effErr[N_CIDAC_VALS],
          eff[N_CIDAC_VALS];

        // get pedestal
        const float ciPed = (float)m_ciADCSum[faceIdx][0] /
          m_ciAdcN[faceIdx][0];

        m_delPed[faceIdx] = ciPed - ped.getPed(rngIdx);

        //calculate ped-subtracted means.
        for (unsigned dacIdx = 0; dacIdx < N_CIDAC_VALS; dacIdx++) {
          if (m_ciAdcN[faceIdx][dacIdx] == 0) {
            ostringstream tmp;
            tmp << __FILE__ << ":" << __LINE__ << " "
                << "No hits recorded for CIDAC_IDX=" << dacIdx
                << " faceIdx=" << faceIdx.val() << endl;
            throw runtime_error(tmp.str());
          }

          adcMean[dacIdx] =
            (float)m_ciADCSum[faceIdx][dacIdx] /
            m_ciAdcN[faceIdx][dacIdx] - ped.getPed(rngIdx);

          adcErr[dacIdx]  = 1.0;

          const unsigned short trig    = m_ciTrigSum[faceIdx][dacIdx];
          const unsigned short ntot    = m_ciAdcN[faceIdx][dacIdx];
          const unsigned short notrig  = ntot - trig;

          // factor used in eff calc, can't be < 1
          const unsigned short strig   = max < unsigned short > (1, trig);
          // factor used in eff calc, can't be < 1
          const unsigned short snotrig = max < unsigned short > (1, notrig);

          effErr[dacIdx] = sqrt((float)strig*notrig*notrig + snotrig*trig*trig)/(float)(ntot*ntot);

          eff[dacIdx]    = (float)trig/ntot;

          (m_ciEffHists[faceIdx])->SetBinContent(dacIdx, eff[dacIdx]);
          (m_ciEffHists[faceIdx])->SetBinError(dacIdx,   effErr[dacIdx]);
          (m_ciAdcHists[faceIdx])->SetBinContent(dacIdx, adcMean[dacIdx]);
          (m_ciAdcHists[faceIdx])->SetBinError(dacIdx,   adcErr[dacIdx]);
        }

        // find threshold center point, where efficiency > 0.5
        for (unsigned dacIdx = 0; dacIdx < N_CIDAC_VALS; dacIdx++)
          if (eff[dacIdx] > 0.5) {
            adcThresh = adcMean[dacIdx];
            break;
          }
        const auto_ptr<TGraphErrors> geff(new TGraphErrors(N_CIDAC_VALS, adcMean, eff, adcErr, effErr));

        step->SetParameters(adcThresh, 0.1);
        geff->Fit(step.get(), "QN");

        m_ciThresh[faceIdx]         = step->GetParameter(0);
        m_ciThreshWidth[faceIdx]    = step->GetParameter(1);
        m_ciThreshErr[faceIdx]      = step->GetParError(0);
        m_ciThreshWidthErr[faceIdx] = step->GetParError(1);
      }

      //////////////////
      //-- MUON EFF --//
      //////////////////
      if (m_muTrigHists[faceIdx]->GetEntries() > 0) {
        // collect only non-empty bins (requires
        // variable length vectors)
        vector<float> vADCMean,
          vADCErr,
          vEffErr,
          vEff;
        // used for passing into ROOT fitting routines
        float adcMean[N_MUADC_BINS],
          adcErr[N_MUADC_BINS],
          effErr[N_MUADC_BINS],
          eff[N_MUADC_BINS];

        const short *ymu   = (m_muAdcHists[faceIdx])->GetArray();
        const short *ytrig = (m_muTrigHists[faceIdx])->GetArray();

        adcThresh = 0.0;
        for (unsigned short adcBin = 0; adcBin < N_MUADC_BINS; adcBin++) {
          const float mu = ymu[adcBin+1];
          // simply skip empty bins
          if (mu == 0)
            continue;

          vADCMean.push_back(MUADC_BINSIZE*(adcBin+0.5));
          vADCErr.push_back(1.0);

          const float trig    = ytrig[adcBin+1];
          const float notrig  = mu-trig;
          // used in eff calc, must be >= 1.0
          const float strig   = (trig > 0.9) ? trig : 1;
          // used in eff calc, must be >= 1.0
          const float snotrig = (notrig > 0.9) ? notrig : 1;

          vEffErr.push_back(sqrt(strig*notrig*notrig + snotrig*trig*trig)/(mu*mu));
          vEff.push_back(trig/mu);
        }

        // find threshold center point, where efficiency > 0.5
        for (unsigned i = 0; i < vEff.size(); i++)
          if (vEff[i] > 0.5) {
            adcThresh = vADCMean[i];
            break;
          }

        // copy vector info into arrays
        copy(vADCMean.begin(), vADCMean.end(), adcMean);
        copy(vADCErr.begin(), vADCErr.end(), adcErr);
        copy(vEff.begin(), vEff.end(), eff);
        copy(vEffErr.begin(), vEffErr.end(), effErr);

        const auto_ptr<TGraphErrors> geff(new TGraphErrors(vADCMean.size(),
                                                     adcMean, eff, adcErr, effErr));
        step->SetParameters(adcThresh, 0.1);
        geff->Fit(step.get(), "QN");

        m_muThresh[faceIdx]         = step->GetParameter(0);
        m_muThreshWidth[faceIdx]    = step->GetParameter(1);
        m_muThreshErr[faceIdx]      = step->GetParError(0);
        m_muThreshWidthErr[faceIdx] = step->GetParError(1);
      }
    }
  }

  string MuTrigAlg::genHistName(const string &type,
                                FaceIdx faceIdx) {
    ostringstream tmp;

    tmp <<  type
        << "_" << faceIdx.toStr();
    return tmp.str();
  }

  void MuTrigAlg::writeTXT(const string &filename) const {
    ofstream outfile(filename.c_str());

    if (!outfile.is_open())
      throw runtime_error(string("Unable to open " + filename));

    // output header as comment
    outfile <<
      "; twr lyr col face muThr muThrErr muThrWid muThrWidErr [ciThr ciThrErr ciThrWid ciThrWidErr muThr/ciThr delPed]"
            << endl;

    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      const TwrNum  twr(faceIdx.getTwr());
      const LyrNum  lyr(faceIdx.getLyr());
      const ColNum  col(faceIdx.getCol());
      const FaceNum face(faceIdx.getFace());

      // skip missing channels
      if (m_muThresh[faceIdx] == INVALID_ADC)
        continue;

      outfile << " " << twr.val()
              << " " << lyr.val()
              << " " << col.val()
              << " " << face.val()
              << " " << m_muThresh[faceIdx]
              << " " << m_muThreshErr[faceIdx]
              << " " << m_muThreshWidth[faceIdx]
              << " " << m_muThreshWidthErr[faceIdx];
      // ci data is optional
      if (m_ciThresh[faceIdx] != INVALID_ADC)
        outfile << " " << m_ciThresh[faceIdx]
                << " " << m_ciThreshErr[faceIdx]
                << " " << m_ciThreshWidth[faceIdx]
                << " " << m_ciThreshWidthErr[faceIdx]
                << " " << m_muThresh[faceIdx]/m_ciThresh[faceIdx]
                << " " << m_delPed[faceIdx];

      outfile << endl;
    }
  }

  void MuTrigAlg::initHists() {
    m_muAdcHists.resize(FaceIdx::N_VALS);
    m_muTrigHists.resize(FaceIdx::N_VALS);
    m_ciAdcHists.resize(FaceIdx::N_VALS);
    m_ciEffHists.resize(FaceIdx::N_VALS);

    for (FaceIdx faceIdx; faceIdx.isValid(); faceIdx++) {
      ostringstream muAdcHistName;
      muAdcHistName << "muonAdc_" << faceIdx.val();

      m_muAdcHists[faceIdx] = new TH1S(muAdcHistName.str().c_str(),
                                       muAdcHistName.str().c_str(),
                                       N_MUADC_BINS, 0, MUADC_BINSIZE*N_MUADC_BINS);

      ostringstream muTrigHistName;
      muTrigHistName << "muonTrig_" << faceIdx.val();

      m_muTrigHists[faceIdx] = new TH1S(muTrigHistName.str().c_str(),
                                        muTrigHistName.str().c_str(),
                                        N_MUADC_BINS, 0, MUADC_BINSIZE*N_MUADC_BINS);

      ostringstream ciAdcHistName;
      ciAdcHistName << "ciAdc_" << faceIdx.val();

      m_ciAdcHists[faceIdx] = new TH1S(ciAdcHistName.str().c_str(),
                                       ciAdcHistName.str().c_str(),
                                       N_CIDAC_VALS, 0, N_CIDAC_VALS);

      ostringstream ciEffHistName;
      ciEffHistName << "ciEff_" << faceIdx.val();

      m_ciEffHists[faceIdx] = new TH1S(ciEffHistName.str().c_str(),
                                       ciEffHistName.str().c_str(),
                                       N_CIDAC_VALS, 0, N_CIDAC_VALS);
    }
  }
}; // namespace calibGenCAL

