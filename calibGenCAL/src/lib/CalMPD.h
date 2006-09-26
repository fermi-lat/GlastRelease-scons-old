#ifndef CalMPD_h
#define CalMPD_h
// $Header$
/** @file
    @author Zachary Fewtrell
*/


// LOCAL INCLUDES

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalArray.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES


// STD INCLUDES

class CalAsym;
class CIDAC2ADC;

/** \brief Represents GLAST Cal Optical gain calibration constants
    (MeV <-> CIDAC)

    contains read & write methods to various file formats 

    @author Zachary Fewtrell
*/
class CalMPD {
 public:
  CalMPD();

  void writeTXT(const std::string &filename) const;
  
  void readTXT(const std::string &filename);

  float getMPD(CalUtil::XtalIdx xtalIdx, CalUtil::DiodeNum diode) const {
    return m_mpd[diode][xtalIdx];
  }
  
  float getMPDErr(CalUtil::XtalIdx xtalIdx, CalUtil::DiodeNum diode) const {
    return m_mpdErr[diode][xtalIdx];
  }

  void setMPD(CalUtil::XtalIdx xtalIdx, CalUtil::DiodeNum diode, float val) {
    m_mpd[diode][xtalIdx] = val;
  }
  void setMPDErr(CalUtil::XtalIdx xtalIdx, CalUtil::DiodeNum diode, float val) {
    m_mpdErr[diode][xtalIdx] = val;
  }


  static const short INVALID_MPD = -5000;
  
 private:
  /// 2d vector N_MPD_PTS lograt mpdvals per xtal/mpdType
  CalUtil::CalVec<CalUtil::DiodeNum, CalUtil::CalArray<CalUtil::XtalIdx, float> > m_mpd; 
  /// corresponding error value
  CalUtil::CalVec<CalUtil::DiodeNum, CalUtil::CalArray<CalUtil::XtalIdx, float> > m_mpdErr; 

};

#endif
