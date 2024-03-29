#ifndef MuonMPDAlg_h
#define MuonMPDAlg_h
// $Header$

/** @file
    @author Zachary Fewtrell
*/

// LOCAL INCLUDES
#include "src/lib/Util/TwrHodoscope.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES
#include "TCanvas.h"
#include "TGraph.h"
#include "TF1.h"
#include "TH2S.h"

// STD INCLUDES
#include <iostream>

class DigiEvent;

namespace CalUtil {
  class CalPed;
  class CIDAC2ADC;
  class CalAsym;
  class CalMPD;
}

namespace calibGenCAL {

  class MPDHists;

  /** \brief Algorithm class generates CalMPD calibration data from digi ROOT
      event files

      @author Zachary Fewtrell
  */
  class MuonMPDAlg {
  public:
	  MuonMPDAlg(const CalUtil::CalPed &ped,
               const CalUtil::CIDAC2ADC &dac2adc,
               const CalUtil::CalAsym &asym,
               MPDHists &mpdHists);

    /// populate histograms from digi root event file
    void        fillHists(const unsigned nEntries,
                          const std::vector<std::string> &rootFileList);

  private:
    /// process a single event for histogram fill
    void        processEvent(const DigiEvent &digiEvent);

    /// process a single tower's data in single event for
    /// histogram fill
    void        processTower(TwrHodoscope &hscope);

    /// hodoscopic event cut for X direction crystals
    bool        passCutX(const TwrHodoscope &hscope);

    /// hodoscopic event cut for Y direction crystals
    bool        passCutY(const TwrHodoscope &hscope);

    class AlgData {
    private:
      void init() {
        nXEvents = 0;
        nYEvents = 0;
        nXtals   = 0;
      }

    public:

		AlgData(const CalUtil::CalAsym &asym);

      unsigned nXEvents;
      unsigned nYEvents;
      unsigned nXtals;

      TCanvas  canvas;
      TGraph   graph;
      TF1      lineFunc;
      TH2S     viewHist;

	  const    CalUtil::CalAsym &calAsym;
    } algData;

    class EventData {
    private:
      /// reset all member variables
      void init()
      {
        eventNum = 0;
        next();
      }

    public:
		EventData(const CalUtil::CalPed &ped,
                const CalUtil::CIDAC2ADC &dac2adc) :
        hscopes(CalUtil::TwrNum::N_VALS, TwrHodoscope(ped, dac2adc))
      {
        init();
      }

      /// rest all member variables that do not retain data
      /// from one event to next.
      void next() {
        // clear all hodoscopes
        for (CalUtil::TwrNum twr; twr.isValid(); twr++)
          hscopes[twr].clear();
      }

      /// need one hodo scope per tower
      CalUtil::CalVec<CalUtil::TwrNum, TwrHodoscope> hscopes;

      unsigned eventNum;
    } eventData;

    MPDHists &m_mpdHists;
  };

}; // namespace calibGenCAL
#endif
