#ifndef _GlastDigi_CalDigiAlg_H
#define _GlastDigi_CalDigiAlg_H 1


// Include files
#include "CalUtil/ICalFailureModeSvc.h"
#include "CalXtalResponse/IXtalADCTool.h"
#include "GaudiKernel/Algorithm.h"
#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"
#include <vector>
#include <map>
// MC classes
#include "Event/MonteCarlo/McIntegratingHit.h"

/** @class CalDigiAlg
* @brief Algorithm to convert from McIntegratingHit objects into 
* CalDigi objects and store them in the TDS. Combines contributions from
* Xtal segments and accounts for light taper along the length of the Xtals.
* Energies are converted to adc values after pedestal subtraction, and the 
* appropriate gain range is identified.
*
* Author:  A.Chekhtman
*
*/

using namespace std;

class CalDigiAlg : public Algorithm {

public:

    CalDigiAlg(const string& name, ISvcLocator* pSvcLocator); 

    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();

    //! pair of signals per Xtal. For SignalMap.
    /** @class XtalSignal
    * @brief nested class of CalDigiAlg to separately hold the energy deposits in the crystal
    * and the diodes.Vector of diodes holds all 4 per crystal.
    *
    * Author:  A.Chekhtman
    *
    */

protected:
    StatusCode fillSignalEnergies();
    StatusCode createDigis();

private:

    /// names for identifier fields
    enum {fLATObjects, fTowerY, fTowerX, fTowerObjects, fLayer,
        fMeasure, fCALXtal,fCellCmp, fSegment};

    /// local cache for constants defined in xml files
    /// x tower number
    int m_xNum;  
    /// y tower number
    int m_yNum;  
    /// detModel identifier for CAL
    int m_eTowerCal;  
    /// detModel identifier for LAT Towers
    int m_eLatTowers;
    /// number of layers (ie in z)
    int m_CalNLayer;  
    // number of Xtals per layer
    int m_nCsIPerLayer;  

    /// map to contain the McIntegratingHit vs XtaliD relational table
    typedef map< idents::CalXtalId,  vector< const Event::McIntegratingHit*> > PreDigiMap;

    /// map to contain the McIntegratingHit vs XtaliD relational table
    PreDigiMap m_idMcIntPreDigi;   

    /// map to contain the McIntegratingHit vs XtaliD relational table
    multimap< idents::CalXtalId, Event::McIntegratingHit* > m_idMcInt;   

    /// pointer to failure mode service
    ICalFailureModeSvc* m_FailSvc;

    /// type of readout range: BEST or ALL
    string m_rangeType;
    /// name of Tool for calculating light taper
    string m_xtalADCToolName;
    /// pointer to actual tool for converting energy to ADC
    IXtalADCTool* m_xtalADCTool;
};



#endif // _GlastDigi_CalDigiAlg_H

