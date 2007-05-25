#ifndef GCRCalibAlg_h
#define GCRCalibAlg_h

// $Header$

/** @file
    @author Zach Fewtrell
*/

// LOCAL INCLUDES

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalArray.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <vector>
#include <string>
#include <map>
#include <set>

class TVector3;
class GcrSelectedXtal;
class GcrSelectEvent;
class CalDigi;
class DigiEvent;

class SimpleIniFile;

namespace calibGenCAL {

  /** \brief Algorithm-type class for generating GLAST Cal optical Calibrations for 
      GCR events
  */

  class CalPed;
  class CIDAC2ADC;
  class GCRHists;
  class CalMPD;

  class GCRCalibAlg {
  public:
    /// \param cfgPath (optional) ini type filename for loading optional parameters.  set to "" for defautls (supports env var expansion)
    GCRCalibAlg(const std::string &cfgPath="");

    /// populate histograms from digi root event file
    /// \nEvents max # events to loop through
    void fillHists(const unsigned nEventsMax,
                   const std::vector<std::string> &digiRootFileList,
                   const std::vector<std::string> &gcrSelectRootFileList,
                   const CalPed &peds,
                   const CIDAC2ADC &dac2adc,
                   GCRHists &gcrHists);



  private:

    /// load parameters from cfg file.
    void readCfg(std::string cfgPath);

    /// perform cuts based on GCRSelect info
    /// fill list of xtals for digi processing
    void     processGcrEvent();

    /// process single GCRSelect hit
    void     processGcrHit(const GcrSelectedXtal &gcrXtal);

    /// fill histograms w/ digi info from hit selected
    /// by process GCR event
    void     processDigiEvent();

    /// process single CalDigi hit
    void     processDigiHit(const CalDigi &calDigi);

    /// cut xtal hit based on track entry & exit face.
    /// return true for good hit.
    bool     crossedFaceCut(const unsigned crossedFaces) const;
    /// cut hit based on incident track angle
    /// \note use orthogonalAngleCut && longitudinalAngleCut
    /// variables.
    bool     angleCut(const TVector3 &pathVec,
                      const CalUtil::DirNum dir) const;

    /// cut hit based on hit distancefrom xtal face
    /// \note use xtalEndCut variable
    bool     posCut(const CalUtil::XtalIdx xtalIdx,
                    const TVector3 &entry,
                    const TVector3 &exit) const;

    /// store cfg & status data pertinent to current algorithm run
    struct AlgData {
      AlgData() {
        clear();
      }

      /// reset all member values
      void clear() {
        nEventsAttempted = 0;
        nEventsRead      = 0;
        nGcrHits         = 0;
        nHitsXface       = 0;
        nHitsAngle       = 0;
        nHitsPos         = 0;
        nFills.fill(0);
        calPed           = 0;
        dac2adc          = 0;
      }

      void summarizeAlg(ostream &ostrm) const;

      /// number of events attempt to read from root file
      unsigned                                       nEventsAttempted;
      /// number of events sucessfully read from root file
      unsigned                                       nEventsRead;
      /// number of GcrSelectedXtal hits processed
      unsigned                                       nGcrHits;
      /// number of Hits pass the whichFacesCrossed cut
      unsigned                                       nHitsXface;
      /// number of hits pass the incident angle cut
      unsigned                                       nHitsAngle;
      /// number of hits pass the distance xtal pos cut
      unsigned                                       nHitsPos;
      /// number of histogram fills.
      CalUtil::CalArray<CalUtil::DiodeNum, unsigned> nFills;

      /// adc pedestals for use by algorithm
      const CalPed *                                 calPed;

      /// cidac2adc for use by alg
      const CIDAC2ADC *                              dac2adc;

      GCRHists *                               gcrHists;
    }     algData;

    /// store data pertinent to current event
    struct EventData {
      EventData() {
        clear();
      }

      /// reset all member values
      void clear() {
        eventNum       = 0;
        next();
      }

      /// clear out obj in prep for next event
      /// leave vars that need to persiste from
      /// event to event
      void next() {
        digiEvent      = 0;
        gcrSelectEvent = 0;
        finalHitMap.clear();
        inferredZ = 0;
      }

      /// current event index
      unsigned eventNum;

      /// pointer to current digiEvent leaf
      const DigiEvent *digiEvent;

      /// pointer to current gcrEvent leaf
      GcrSelectEvent * gcrSelectEvent;

      /// list of xtals which passed GCR cuts
      typedef std::map < CalUtil::XtalIdx, 
                         const GcrSelectedXtal * > FinalHitMap;

      typedef FinalHitMap::iterator  FinalHitMapIter;

      FinalHitMap      finalHitMap;

      unsigned short inferredZ;

    } eventData;

    /// enable hit cut based on path entry & exit face on xtal
    bool  cutCrossedFaces;
    /// enable hit cut based on track angle
    bool  cutTrackAngle;
    /// enable hit cut based on longitudinal hit position.
    bool  cutLongitudinalPos;

    /// sin of max incident angle component orthogonal to xtal length.
    float maxOrthogonalSin;
    /// sin of max incident angle component along xtal length.
    float maxLongitudinalSin;
    /// ignore tracks which come w/ in some distance from xtal end
    float mmCutFromEnd;
    /// directly based on mmCutFromEnd
    float mmCutFromCtr;

    /// list of particles 'of interest' for storage in histograms
    std::set<int> zList;
  };


}; // namespace calibGenCAL
#endif
