
//$Header$

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/IDetDataSvc.h"

#include "CalibData/CalibTime.h"
#include "CalibData/CalibModel.h"
#include "CalibData/Tkr/BadStrips.h"
#include "CalibData/Tkr/TkrSplitsCalib.h"
#include "CalibData/Tkr/TkrTot.h"

#include "TkrUtil/ITkrBadStripsSvcCalib.h"
#include "TkrUtil/ITkrFailureModeSvcCalib.h"
#include "TkrUtil/ITkrSplitsSvc.h"
#include "TkrUtil/ITkrToTSvc.h"

#include <string>


/**
@file TkrCalibAlg.cxx Algorithm to organize the updating the calibration data

  TkrCalibAlg is an algorithm which
  accesses calibration data in what is expected to be a standard manner to inform
  the services of the appearance of a new calibration. 
*/

/** 
@class TkrCalibAlg
  Algorithm that handles updating the calibrations
*/
class TkrCalibAlg : public Algorithm {
    
public:
    TkrCalibAlg(const std::string& name, ISvcLocator* pSvcLocator);    
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();
    
private:
    
    /// pointer to data provider
    IDataProviderSvc*   m_pCalibDataSvc;
    /// Handle to the IDetDataSvc interface of the CalibDataSvc
    IDetDataSvc*        m_detDataSvc;
    /// pointer to bad strips service
    ITkrBadStripsSvcCalib*   m_pTkrBadStripsSvc;
    /// pointer to failure mode service
    ITkrFailureModeSvcCalib* m_pTkrFailureModeSvc;
    /// pointer to splits service
    ITkrSplitsSvc*           m_pTkrSplitsSvc;
    /// pointer to splits service
    ITkrToTSvc*              m_pTkrToTSvc;
    /// serial number of the hot strips calibration
    int m_serHot;
    /// serial number of the dead strips calibration
    int m_serDead;
    /// serial number of the splits calibration
    int m_serSplits;
    /// serial number of the charge injection calibration
    int m_serInjection;
    /// serial number of the ToT muon calibration
    int m_serMuons;

    /// flavor of calibration required ("ideal" means: do nothing)
    std::string m_flavor;
    // these for individual calib types, overrides the above.
    /// flavor of the dead strips calibration
    std::string m_deadStripsFlavor;
    /// flavor of the hot strips calibration
    std::string m_hotStripsFlavor;
    /// flavor of the splits calibration
    std::string m_splitsFlavor;
    /// flavor of the charge injection calibration
    std::string m_injectionFlavor;
    /// flavor of the ToT muon calibration
    std::string m_muonFlavor;

};

/// Instantiation of a static factory to create instances of this algorithm
static const AlgFactory<TkrCalibAlg> Factory;
const IAlgFactory& TkrCalibAlgFactory = Factory;


TkrCalibAlg::TkrCalibAlg(const std::string&  name, 
                         ISvcLocator*        pSvcLocator )
                         : Algorithm(name, pSvcLocator), m_pCalibDataSvc(0), 
                         m_pTkrBadStripsSvc(0), m_pTkrFailureModeSvc(0),
                         m_serHot(-1), m_serDead(-1)
{
    declareProperty("calibFlavor",           m_flavor           = "ideal");
    declareProperty("deadStripsCalibFlavor", m_deadStripsFlavor = "notSet");
    declareProperty("hotStripsCalibFlavor",  m_hotStripsFlavor  = "notSet");
    declareProperty("splitsCalibFlavor",     m_splitsFlavor     = "notSet");
    declareProperty("chargeInjectionCalibFlavor",  
                                             m_injectionFlavor  = "notSet");
    declareProperty("muonCalibFlavor",       m_muonFlavor       = "notSet");
}

StatusCode TkrCalibAlg::initialize() 
{
    StatusCode sc;
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "Initialize()" << endreq;
    
    // So far don't have any properties, but in case we do some day..
    setProperties();
    
    sc = service("CalibDataSvc", m_pCalibDataSvc, true);
    if ( !sc.isSuccess() ) {
        log << MSG::ERROR 
            << "Could not get IDataProviderSvc interface of CalibDataSvc" 
            << endreq;
        return sc;
    }

    // Query the IDetDataSvc interface of the calib data service
    sc = m_pCalibDataSvc->queryInterface(IID_IDetDataSvc, 
                                (void**) &m_detDataSvc);
    if ( !sc.isSuccess() ) {
        log << MSG::ERROR 
	    << "Could not query IDetDataSvc interface of CalibDataSvc" 
	    << endreq;
        return sc;
    } else {
        log << MSG::DEBUG 
	    << "Retrieved IDetDataSvc interface of CalibDataSvc" 
	    << endreq;
    }
    
    sc = service("TkrBadStripsSvc", m_pTkrBadStripsSvc, true);
    if ( !sc.isSuccess()) {
        log << MSG::ERROR 
            << "Could not get TkrBadStripsSvc" 
            << endreq;
        return sc;
    }
    
    sc = service("TkrFailureModeSvc", m_pTkrFailureModeSvc, true);
    if ( !sc.isSuccess() ) {
        log << MSG::ERROR 
            << "Could not get TkrFailureModeSvc" 
            << endreq;
        return sc;
    }
        
    sc = service("TkrSplitsSvc", m_pTkrSplitsSvc, true);
    if ( !sc.isSuccess() ) {
        log << MSG::ERROR 
            << "Could not get TkrSplitsSvc" 
            << endreq;
        return sc;
    }

    sc = service("TkrToTSvc", m_pTkrToTSvc, true);
    if ( !sc.isSuccess() ) {
        log << MSG::ERROR 
            << "Could not get TkrToTSvc" 
            << endreq;
        return sc;
    }

    // Get properties from the JobOptionsSvc
    sc = setProperties();

    // go through the individual flavors... 
    // set them equal to the overall flavor unless they've been set

    if (m_deadStripsFlavor=="notSet") m_deadStripsFlavor = m_flavor;
    if (m_hotStripsFlavor=="notSet")  m_hotStripsFlavor  = m_flavor;
    if (m_splitsFlavor=="notSet")     m_splitsFlavor     = m_flavor;
    if (m_injectionFlavor=="notSet")  m_injectionFlavor  = m_flavor;
    if (m_muonFlavor=="notSet")       m_muonFlavor       = m_flavor;

    return StatusCode::SUCCESS;   
}


StatusCode TkrCalibAlg::execute( ) {
    
    MsgStream log(msgSvc(), name());
    
    bool updateStripsNow = false;

    // check the dead channels

    //    SmartDataPtr<CalibData::BadStrips> pDead(m_pCalibDataSvc, "");
    CalibData::BadStrips* pDead = 0; //  = SmartDataPtr<CalibData::BadStrips>(m_pCalibDataSvc, "");
    if(m_deadStripsFlavor!="ideal" && m_deadStripsFlavor!="") {
        std::string fullDeadPath = "/Calib/TKR_DeadChan/"+m_deadStripsFlavor;
        pDead = SmartDataPtr<CalibData::BadStrips>(m_pCalibDataSvc, fullDeadPath);
        if (!pDead) {
            log << MSG::ERROR 
                << "Failed access to Dead strips via smart ptr" << endreq;
            return StatusCode::FAILURE;
        }

        m_pCalibDataSvc->updateObject((CalibData::BadStrips *)pDead);
        if (!pDead) {
            log << MSG::ERROR 
                << "Update of dead strips failed" << endreq;
            return StatusCode::FAILURE;
        }

        int newSerNo = pDead->getSerNo();
        if (newSerNo!=m_serDead) {
            log << MSG::INFO << "deadStrips serial number changed..." 
                << endreq;
            m_serDead = newSerNo;
            updateStripsNow = true;
            log << MSG::INFO << "Retrieved with path " << fullDeadPath << endreq
                << "Serial #" <<  pDead->getSerNo() << endreq; 
            log << MSG::INFO << "Vstart: " <<  (pDead->validSince()).hours()
                << "  Vend: " << (pDead->validTill()).hours() << endreq;
            log << MSG::INFO << "Bad type: " << pDead->getBadType() 
                << " has " << pDead->getBadTowerCount() << " bad towers " << endreq;				
        }
    }

    // now the hot channels

    //    SmartDataPtr<CalibData::BadStrips>pHot(m_pCalibDataSvc, "");
    CalibData::BadStrips* pHot = 0; // = SmartDataPtr<CalibData::BadStrips>(m_pCalibDataSvc, "");
    if(m_hotStripsFlavor!="ideal" && m_hotStripsFlavor!="") {
        std::string fullHotPath = "/Calib/TKR_HotChan/"+m_hotStripsFlavor;
        pHot = SmartDataPtr<CalibData::BadStrips>(m_pCalibDataSvc, fullHotPath);
        if (!pHot) {
            log << MSG::ERROR 
                << "Failed access to Hot strips via smart ptr" << endreq;
            return StatusCode::FAILURE;
        }

        m_pCalibDataSvc->updateObject((CalibData::BadStrips *)pHot);
        if (!pHot) {
            log << MSG::ERROR 
                << "Update of Hot strips failed" << endreq;
            return StatusCode::FAILURE;
        }

        int newSerNo = pHot->getSerNo();
        if (newSerNo!=m_serHot) {
            log << MSG::INFO << "hotStrips serial number changed..." 
                << endreq;
            m_serHot = newSerNo;
            updateStripsNow = true;
            log << MSG::INFO << "Retrieved with path " << fullHotPath << endreq
                << "Serial #" <<  pHot->getSerNo() << endreq; 
            log << MSG::INFO << "Vstart: " <<  (pHot->validSince()).hours()
                << "  Vend: " << (pHot->validTill()).hours() << endreq;
            log << MSG::INFO << "Bad type: " << pHot->getBadType() 
                << " has " << pHot->getBadTowerCount() << " bad towers " << endreq;
        }
    }

    // just update dead and Hot, no harm if one of them hasn't changed
    if (updateStripsNow) {
        log << MSG::INFO <<" about to update constants" << endreq;
        m_pTkrBadStripsSvc->update(pDead, pHot);
        m_pTkrFailureModeSvc->update(pDead, pHot);
    }

    // now the splits
    if(m_splitsFlavor!="ideal" && m_splitsFlavor!="") {

        std::string fullSplitsPath = CalibData::TKR_Splits + "/" + m_splitsFlavor;
        DataObject* pObject;
        m_pCalibDataSvc->retrieveObject(fullSplitsPath, pObject);
        CalibData::TkrSplitsCalib* pSplits = 0;
        pSplits = dynamic_cast<CalibData::TkrSplitsCalib*> (pObject);
        if (!pSplits) {
            log << MSG::ERROR 
                << "Failed access to splits via smart ptr" << endreq;
            return StatusCode::FAILURE;
        }
        m_pCalibDataSvc->updateObject(pObject);
        pSplits = dynamic_cast<CalibData::TkrSplitsCalib*> (pObject);
        if (!pSplits) {
            log << MSG::ERROR 
                << "Update of splits failed" << endreq;
            return StatusCode::FAILURE;
        }

        int newSerNo = pSplits->getSerNo();
        if (newSerNo!=m_serSplits) {
            log << MSG::INFO << "splits serial number changed..." 
                << endreq;
            m_serSplits = newSerNo;
            updateStripsNow = true;
            log << MSG::INFO << "Retrieved with path " << fullSplitsPath << endreq
                << "Serial #" <<  pSplits->getSerNo() << endreq; 
            log << MSG::INFO << "Vstart: " <<  (pSplits->validSince()).hours()
                << "  Vend: " << (pSplits->validTill()).hours() << endreq;

           // last thing, pass pointer to TkrSplitsSvc
           m_pTkrSplitsSvc->update(pSplits);
        }
    }

    // now charge injection
    if(m_injectionFlavor!="ideal" && m_injectionFlavor!="") {

        std::string fullToTPath = CalibData::TKR_TOTSignal + "/" + m_injectionFlavor;
        DataObject* pObject;
        m_pCalibDataSvc->retrieveObject(fullToTPath, pObject);
        CalibData::TkrTotCol* pToT = 0;
        pToT = dynamic_cast<CalibData::TkrTotCol*> (pObject);
        if (!pToT) {
            log << MSG::ERROR 
                << "Failed access to charge-injection via smart ptr" << endreq;
            return StatusCode::FAILURE;
        }
        m_pCalibDataSvc->updateObject(pObject);
        pToT = dynamic_cast<CalibData::TkrTotCol*> (pObject);
        if (!pToT) {
            log << MSG::ERROR 
                << "Update of ToT charge-injection constants failed" << endreq;
            return StatusCode::FAILURE;
        }

        int newSerNo = pToT->getSerNo();
        if (newSerNo!=m_serInjection) {
            log << MSG::INFO << "charge-injection serial number changed..." 
                << endreq;
            m_serInjection = newSerNo;
            log << MSG::INFO << "Retrieved with path " << fullToTPath << endreq
                << "Serial #" <<  pToT->getSerNo() << endreq; 
            log << MSG::INFO << "Vstart: " <<  (pToT->validSince()).hours()
                << "  Vend: " << (pToT->validTill()).hours() << endreq;

           // last thing, pass pointer to TkrSplitsSvc
           m_pTkrToTSvc->update(pToT);
        }
    }
    // now muon calibration
    if(m_muonFlavor!="ideal" && m_muonFlavor!="") {
        // muon code here!
    }

    return StatusCode::SUCCESS;
}

StatusCode TkrCalibAlg::finalize( ) {
    
    MsgStream log(msgSvc(), name());
    log << MSG::INFO 
        << "          Finalize TkrCalibAlg "
        << endreq;
    
    return StatusCode::SUCCESS;
}
