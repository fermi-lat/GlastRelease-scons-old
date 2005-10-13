#ifndef AsymMgr_H
#define AsymMgr_H

// LOCAL
#include "CalibItemMgr.h"

// GLAST
#include "CalUtil/CalDefs.h"
#include "CalibData/Cal/CalAsym.h"

// EXTLIB

// STD

using namespace CalDefs;
using namespace idents;

using CalibData::ValSig;

class CalCalibSvc;

/** @class AsymMgr
    @author Zachary Fewtrell
    
    \brief Manage GLAST Cal asymmetry calibration data
*/

class AsymMgr : public CalibItemMgr {
 public:
  AsymMgr();

  /// get Asymmetry calibration information for one xtal
  StatusCode getAsym(CalXtalId xtalId,
                     const vector<ValSig> *&asymLrg,
                     const vector<ValSig> *&asymSm,
                     const vector<ValSig> *&asymNSPB,
                     const vector<ValSig> *&asymPSNB,
                     const vector<float>  *&xVals);

  StatusCode evalAsymLrg(CalXtalId xtalId, 
                         double Xpos, double &asymLrg){
    if (!checkXtalId(xtalId)) return StatusCode::FAILURE;
    return evalSpline(ASYMLRG_SPLINE, xtalId, Xpos, asymLrg);
  }

  StatusCode evalPosLrg(CalXtalId xtalId, 
                        double asymLrg, double &Xpos) {
    if (!checkXtalId(xtalId)) return StatusCode::FAILURE;
    return evalSpline(INV_ASYMLRG_SPLINE, xtalId, asymLrg, Xpos);
  }

  StatusCode evalAsymSm(CalXtalId xtalId, 
                        double Xpos, double &asymSm) {
    if (!checkXtalId(xtalId)) return StatusCode::FAILURE;
    return evalSpline(ASYMSM_SPLINE, xtalId, Xpos, asymSm);
  }

  StatusCode evalPosSm(CalXtalId xtalId, 
                       double asymSm, double &Xpos) {
    if (!checkXtalId(xtalId)) return StatusCode::FAILURE;
    return evalSpline(INV_ASYMSM_SPLINE, xtalId, asymSm, Xpos);
  }

  StatusCode evalAsymNSPB(CalXtalId xtalId, 
                          double Xpos, double &asymNSPB) {
    if (!checkXtalId(xtalId)) return StatusCode::FAILURE;
    return evalSpline(ASYMNSPB_SPLINE, xtalId, Xpos, asymNSPB);
  }

  StatusCode evalPosNSPB(CalXtalId xtalId, 
                         double asymNSPB, double &Xpos) {
    if (!checkXtalId(xtalId)) return StatusCode::FAILURE;
    return evalSpline(INV_ASYMNSPB_SPLINE, xtalId, asymNSPB, Xpos);
  }

  StatusCode evalAsymPSNB(CalXtalId xtalId, 
                          double Xpos, double &asymPSNB) {
    if (!checkXtalId(xtalId)) return StatusCode::FAILURE;
    return evalSpline(ASYMPSNB_SPLINE, xtalId, Xpos, asymPSNB);
  }

  StatusCode evalPosPSNB(CalXtalId xtalId, 
	                     double asymPSNB, double &Xpos) {
    if (!checkXtalId(xtalId)) return StatusCode::FAILURE;
    return evalSpline(INV_ASYMPSNB_SPLINE, xtalId, asymPSNB, Xpos);
  }


 private:

  /// List of each type of spline for asym calib_type
  typedef enum {
    ASYMLRG_SPLINE,
    INV_ASYMLRG_SPLINE,
    ASYMSM_SPLINE,
    INV_ASYMSM_SPLINE,
    ASYMNSPB_SPLINE,
    INV_ASYMNSPB_SPLINE,
    ASYMPSNB_SPLINE,
    INV_ASYMPSNB_SPLINE,
    N_SPLINE_TYPES
  } SPLINE_TYPE;

  StatusCode genLocalStore();

  bool checkXtalId(CalXtalId xtalId);

  LATWideIndex genIdx(CalXtalId xtalId) {return XtalIdx(xtalId);}

  StatusCode loadIdealVals();
  /// Store ideal (fake) vals for large diode asym (used when db is down)
  vector<ValSig> m_idealAsymLrg;
  /// Store ideal (fake) vals for small diode asym (used when db is down)
  vector<ValSig> m_idealAsymSm;
  /// Store ideal (fake) vals for NegSmall diode PosBig diode asym (used when db is down)
  vector<ValSig> m_idealAsymNSPB;
  /// Store ideal (fake) vals for PosSmall diode NegBig diode asym (used when db is down)
  vector<ValSig> m_idealAsymPSNB;
  
  vector<float> m_idealXVals;

  

};
#endif
