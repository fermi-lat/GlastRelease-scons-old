
/** 
* @class TkrReconAlg
*
* @brief TkrRecon Gaudi Algorithm 
*        Main algorithm for driving the tracker reconstruction. 
*        Operates in two modes:
*        1) First pass - does the full reconstruction including clustering and track finding
*        2) Iteration - allows a second (or more) pass for refitting tracks and vertexing
*
* 03-27-2003 
*
*
* @author The Tracking Software Group
*
* File and Version Information:
*      $Header$
*/


#include <vector>

#include "Event/TopLevel/Event.h"
#include "Event/TopLevel/EventModel.h"
#include "Event/Recon/TkrRecon/TkrCluster.h"
#include "Event/Recon/TkrRecon/TkrTree.h"
#include "Event/Recon/TkrRecon/TkrVertex.h"
#include "LdfEvent/EventSummaryData.h"
#include "TkrRecon/Services/TkrInitSvc.h"

#include "TkrUtil/ITkrGhostTool.h"

#include "GaudiKernel/Algorithm.h" 
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/IToolSvc.h"

#include "Utilities/TkrException.h"
#include <exception>
#include <sstream>
#include <iomanip>
#include <map>

namespace {
    std::string doubleToString(double x) {
        std::ostringstream oStr;
        oStr.str("");
        oStr << std::setprecision(17)
            << std::setw(25) 
            << std::setiosflags(std::ios::scientific) 
            << x;
        return oStr.str();
    }

    const int exceptionTestTypes = 10;
}

// little class to hold the event error information
class TkrErrorRecord 
{
public:
    TkrErrorRecord (int run, int event, double time, std::string errorString) :
      m_run(run), m_event(event), m_time(time), m_errorString(errorString) {}
      virtual ~TkrErrorRecord() {}
      int         getRun()         {return m_run;}
      int         getEvent()       {return m_event;}
      double      getLastTime()    {return m_time;}
      std::string getErrorString() {return m_errorString;}
private:
    int    m_run;
    int    m_event;
    double m_time;
    std::string m_errorString;
};

typedef std::vector<TkrErrorRecord*> errorVec;
typedef errorVec::iterator errorIter;
typedef std::map<std::string, int> errorMap;
typedef errorMap::const_iterator errorMapIter;

// Class defintion...
class TkrReconAlg : public Algorithm
{
public:

    // Standard Gaudi Algorithm constructor format
    TkrReconAlg(const std::string& name, ISvcLocator* pSvcLocator); 
    virtual ~TkrReconAlg() 
    {
        errorIter iter;
        for (iter=m_errorArray.begin(); iter!=m_errorArray.end();++iter) {
            delete *iter;
        }
        m_errorArray.clear();
        m_stage = "";
    }

    // The thee phases in the life of a Gaudi Algorithm
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();

private:

    enum {SKIPALL=0, CLUSTERING, FILTERING, PATREC, FITTING, VERTEXING };
    StatusCode handleError(std::string errorString);
    int         m_errorCount;
    bool        m_saveBadEvents;
    errorMap    m_errCountMap;
    bool        m_suppressClusterSizePrintout;

    // Input parameter which determines the type of reconstruction to run
    std::string m_TrackerReconType;

    // Pointers to the four main TkrRecon Gaudi Algorithms
    Algorithm*  m_TkrClusterAlg;
    Algorithm*  m_TkrFilterAlg;
    Algorithm*  m_TkrFindAlg;
    Algorithm*  m_TkrTrackFitAlg;
    Algorithm*  m_TkrVertexAlg;
    Algorithm*  m_TkrDisplayAlg; // LSR

    Event::EventHeader* m_header;
    errorVec m_errorArray;
    double   m_lastTime;

    int m_eventCount;
    bool m_testExceptions;
    bool m_printExceptions;
    std::string m_stage;
    int m_lastStage;
    int m_firstStage;
    bool m_checkGhosts;

    // Simple event filter
    int m_maxClusters;

    // this is because 2 copies of TkrReconAlg are instantiated: "FirstPass" and "Iteration"
    static bool s_failed;
    static bool s_saveBadEvents;
    static int  s_run;
    static int  s_event;
    static bool s_ghostClusterDone;
    static bool s_ghostTrackDone;
    static bool s_ghostVertexDone;

    ITkrGhostTool* m_ghostTool;
};

bool TkrReconAlg::s_failed = false;
bool TkrReconAlg::s_saveBadEvents = true;
int  TkrReconAlg::s_run = -1;
int  TkrReconAlg::s_event = -1;
bool TkrReconAlg::s_ghostClusterDone = false;
bool TkrReconAlg::s_ghostTrackDone   = false;
bool TkrReconAlg::s_ghostVertexDone  = false;

// Definitions for use within Gaudi
//static const AlgFactory<TkrReconAlg>  Factory;
//const IAlgFactory& TkrReconAlgFactory = Factory;
DECLARE_ALGORITHM_FACTORY(TkrReconAlg);

// Algorithm constructor
TkrReconAlg::TkrReconAlg(const std::string& name, ISvcLocator* pSvcLocator) :
Algorithm(name, pSvcLocator) 
{
    // Variable to select reconstruction type
    declareProperty("TrackerReconType", m_TrackerReconType="TreeBased");
    // the following are for testing and debugging the code
    // defaults are set correctly for standard use

    // If true, try very hard not to crash
    declareProperty("saveBadEvents", m_saveBadEvents=true);
    // turn on some bad things to see if exception handling is working
    declareProperty("testExceptions", m_testExceptions=false);
    // force exceptions to be printed
    declareProperty("printExceptions", m_printExceptions= false);
    // control flag to suppress parts of the recon:
    // 0 = skip all, 1 = do clustering, 2 = and filtering, 3 = and patrec, 4 = and fit, >=5 = and vertex
    declareProperty("lastStage", m_lastStage=100);
    // <=1 = start with clustering, 2 = filtering, 3 = patrec, 4 = fit, 5 = vertex, >5 skip everything
    declareProperty("firstStage", m_firstStage=0);
    // This will abort reconstruction if too many clusters found
    declareProperty("maxAllowedClusters", m_maxClusters=2000);
    // Suppress individual printout of cluster sizes
    declareProperty("suppressClusterSizePrintout", m_suppressClusterSizePrintout = true);
    declareProperty("checkGhosts" , m_checkGhosts = true);
}

// Initialization method
StatusCode TkrReconAlg::initialize()
{
    // Purpose and Method: Overall Tracker Reconstruction Initialization
    // Inputs:  None
    // Outputs:  StatusCode upon completetion
    // Dependencies: Value of m_TrackerReconType determining the type of recon to run
    // Restrictions and Caveats:  None

    MsgStream log(msgSvc(), name());
    StatusCode sc = StatusCode::SUCCESS;

    log << MSG::INFO << "TkrReconAlg Initialization";
    if( (sc=setProperties()).isFailure()) log << " didn't work!";
    log << endreq;

    m_errorCount = 0;
    m_lastTime   = 0.0;
    m_eventCount = -1;

    if(name()!="Iteration") {
        s_saveBadEvents = m_saveBadEvents;
    }

    // Initialization will depend on whether this is initial or iteration pass version
    // If first pass then we do full reconstruction
    // Start with all pointers null

    m_TkrClusterAlg  = 0;
    m_TkrFilterAlg   = 0;
    m_TkrFindAlg     = 0;
    m_TkrTrackFitAlg = 0;
    m_TkrVertexAlg   = 0;

    if (name() == "Iteration")
    {
        // No Clustering algorithm on iteration

        // Filtering algorithm
        if( createSubAlgorithm("TkrFilterAlg", "TkrFilterIter", m_TkrFilterAlg).isFailure() ) 
        {
            log << MSG::ERROR << " could not open TkrFilterAlg/TkrFilterIter " << endreq;
            return StatusCode::FAILURE;
        }

        // No Track finding algorithm on iteration
        if( createSubAlgorithm("TkrFindAlg", "TkrFindIter", m_TkrFindAlg).isFailure() ) 
        {
            log << MSG::ERROR << " could not open TkrFindAlg/TkrFindIter " << endreq;
            return StatusCode::FAILURE;
        }

        // Track Fitting algorithm
        if( createSubAlgorithm("TkrTrackFitAlg", "TkrFitIter", m_TkrTrackFitAlg).isFailure() ) 
        {
            log << MSG::ERROR << " could not open TkrTrackFitAlg/TkrFitIter " << endreq;
            return StatusCode::FAILURE;
        }

        // Vertex finding and fitting algorithm
        if( createSubAlgorithm("TkrVertexAlg", "TkrVertexIter", m_TkrVertexAlg).isFailure() ) 
        {
            log << MSG::ERROR << " could not open TkrVertexAlg/TkrVertexIter " << endreq;
            return StatusCode::FAILURE;
        }
    }
    else if (name() == "TkrCluster")
    {
        // Clustering algorithm
        if( createSubAlgorithm("TkrClusterAlg", "TkrPreCluster", m_TkrClusterAlg).isFailure() ) 
        {
            log << MSG::ERROR << " could not open TkrClusterAlg/TkrPreCluster " << endreq;
            return StatusCode::FAILURE;
        }
    }

    else
    {
        // Clustering algorithm
        if( createSubAlgorithm("TkrClusterAlg", "TkrClusFirst", m_TkrClusterAlg).isFailure() ) 
        {
            log << MSG::ERROR << " could not open TkrClusterAlg/TkrClusFirst " << endreq;
            return StatusCode::FAILURE;
        }

        // Filtering algorithm
        if( createSubAlgorithm("TkrFilterAlg", "TkrFilterFirst", m_TkrFilterAlg).isFailure() ) 
        {
            log << MSG::ERROR << " could not open TkrFilterAlg/TkrFilterFirst " << endreq;
            return StatusCode::FAILURE;
        }

        // Track finding algorithm
        if( createSubAlgorithm("TkrFindAlg", "TkrFindFirst", m_TkrFindAlg).isFailure() ) 
        {
            log << MSG::ERROR << " could not open TkrFindAlg/TkrFindFirst " << endreq;
            return StatusCode::FAILURE;
        }

        // Set the property controlling the type of track finding to perform
        m_TkrFindAlg->setProperty("TrackFindType", m_TrackerReconType);

        // Track Fitting algorithm
        if( createSubAlgorithm("TkrTrackFitAlg", "TkrFitFirst", m_TkrTrackFitAlg).isFailure() ) 
        {
            log << MSG::ERROR << " could not open TkrTrackFitAlg/TkrFitFirst " << endreq;
            return StatusCode::FAILURE;
        }

        // Vertex finding and fitting algorithm
        if( createSubAlgorithm("TkrVertexAlg", "TkrVertexFirst", m_TkrVertexAlg).isFailure() ) 
        {
            log << MSG::ERROR << " could not open TkrVertexAlg/TkrVertexFirst " << endreq;
            return StatusCode::FAILURE;
        }

        
        // Display algorithm (if GuiSvc is present) // LSR
        if( createSubAlgorithm("TkrDisplayAlg", "TkrDisplayAlg", m_TkrDisplayAlg).isFailure() ) 
        {
        log << MSG::ERROR << " could not open TkrDisplayAlg " << endreq;
        return StatusCode::FAILURE;
        }
        m_TkrDisplayAlg->setProperty("TrackerReconType", m_TrackerReconType);
        
    }

    // Set the property controlling the type of track fitting to perform
    if(m_TkrTrackFitAlg) m_TkrTrackFitAlg->setProperty("TrackFitType", m_TrackerReconType);

    // Ghost Tool
    sc = toolSvc()->retrieveTool("TkrGhostTool", m_ghostTool);
    if(sc.isFailure()) {
        log << MSG::ERROR << "TkrGhostTool not found!" << endreq;
        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
}

StatusCode TkrReconAlg::execute()
{
    // Purpose and Method: Overall Tracker Reconstruction execution method (called every event)
    // Inputs:  None
    // Outputs:  StatusCode upon completetion
    // Dependencies: None
    // Restrictions and Caveats:  None
    MsgStream log(msgSvc(), name());
    log << MSG::DEBUG;
    bool doDebug = log.isActive();
    log << endreq;

    StatusCode sc = StatusCode::SUCCESS;

    m_header = SmartDataPtr<Event::EventHeader>(eventSvc(), EventModel::EventHeader);
    bool newEvent;
    if(!m_header) {
        log << MSG::ERROR << "Event header not found!" << endreq;
        return StatusCode::FAILURE;
    } else {
        // check if new event, if so, reset s_failed flag
        int run   = m_header->run();
        int event = m_header->event();
       
        newEvent = (run!=s_run||event!=s_event);
        if(newEvent) {
            if (doDebug) {

               log << MSG::DEBUG;
               log << "------- Tkr Recon of new Event  " 
                                                                   << run << ":" << event << " (" 
                                                                   << m_eventCount << ") --------";
               log << endreq;
            }


            m_eventCount++;
            s_failed = false;
            s_run   = run;
            s_event = event;
            s_ghostClusterDone = false;
            s_ghostTrackDone   = false;
            s_ghostVertexDone  = false;
        }
    }

    if(doDebug&&newEvent) {
        log << MSG::DEBUG << "------- " << name() << "-------" << endreq;
    }
    newEvent = false;

    // recon has failed at a previous stage... just return;
    if(s_failed) return StatusCode::SUCCESS;

    std::string stageFailed = "Stage failed";

    try {
        // Call clustering if in first pass mode
        m_stage = "TkrClusterAlg";
        if (m_TkrClusterAlg && m_lastStage>=CLUSTERING && m_firstStage<=CLUSTERING) {
            sc = m_TkrClusterAlg->execute();
            if (sc.isFailure())
            {
                return handleError(stageFailed);
            }
            // Check number of clusters returned
            int numClusters = 0;
            Event::TkrClusterCol* clusterCol = SmartDataPtr<Event::TkrClusterCol>(eventSvc(),EventModel::TkrRecon::TkrClusterCol);
            if(clusterCol) {
                numClusters = clusterCol->size();
            }
            if(doDebug) 
                log << MSG::DEBUG << numClusters << " TkrClusters found" << endreq;

            m_stage = "ClusterSize";
            if (numClusters > m_maxClusters)
            {
                std::stringstream errorStream;
                errorStream << numClusters << " clusters found, max allowed is " 
                    << m_maxClusters ;
                return handleError(errorStream.str());
            }
        }

        if(!s_ghostClusterDone&&m_ghostTool&&m_checkGhosts) {
            m_stage = "GhostCheck - clusters";
            s_ghostClusterDone = true;
            if ((sc=m_ghostTool->flagSingles()).isFailure() ||
                (sc=m_ghostTool->flagEarlyHits()).isFailure())
            {
                return handleError(stageFailed);
            }
        }

        m_stage = "TestExceptions";
        if (m_testExceptions && m_eventCount%exceptionTestTypes==3) sc = StatusCode::FAILURE;

        if (sc.isFailure())
        {
            return handleError(stageFailed);
        }

        // Call track filter stage
        m_stage = "TkrFilterAlg";
        if(m_TkrFilterAlg && m_lastStage>=FILTERING && m_firstStage<=FILTERING) {
            sc = m_TkrFilterAlg->execute();
            if (sc.isFailure())
            {
                return handleError(stageFailed);
            }
        }

        // Call track finding if in first pass mode
        m_stage = "TkrFindAlg";
//        if (m_TkrFindAlg && m_lastStage>=PATREC && m_firstStage<=PATREC) {
        if (m_TkrFindAlg && m_lastStage>=PATREC && m_firstStage<=FITTING) {
            log << MSG::DEBUG << "about to call Tkr finding" << endreq;
            sc = m_TkrFindAlg->execute();
            if (sc.isFailure())
            {
                return handleError(stageFailed);
            }

            // Check number of tracks returned
            if(doDebug) {
                Event::TkrTrackCol* trackCol = SmartDataPtr<Event::TkrTrackCol>(eventSvc(),
                    EventModel::TkrRecon::TkrTrackCol);
                int numTracks = 0;
                if(trackCol) numTracks = trackCol->size();
                trackCol = SmartDataPtr<Event::TkrTrackCol>(eventSvc(),
                    EventModel::TkrRecon::TkrCRTrackCol);
                int numCRTracks = 0;
                if(trackCol) numCRTracks = trackCol->size();
                // output number of tracks on debug
                log << MSG::DEBUG << numTracks << " Normal, " 
                    << numCRTracks << " CR TkrTracks found" << endreq ;
            }
        }

        // Check for ghosts
        Event::TkrTrackCol* trackCol = SmartDataPtr<Event::TkrTrackCol>(eventSvc(),EventModel::TkrRecon::TkrTrackCol);
        if(m_TkrFindAlg&&!s_ghostTrackDone&&m_ghostTool&&trackCol&&m_checkGhosts) {
            m_stage = "GhostCheck - tracks";
            sc = m_ghostTool->flagEarlyTracks(); 
            s_ghostTrackDone = true;
            if (sc.isFailure())
            {
                return handleError(stageFailed);
            }
        }

        // Call track fit
        m_stage = "TkrTrackFitAlg";
        if(m_TkrTrackFitAlg && m_lastStage>=FITTING && m_firstStage<=FITTING) {
            sc = m_TkrTrackFitAlg->execute();
            if (sc.isFailure())
            {
                return handleError(stageFailed);
            }

            // Check number of tracks again
            if(doDebug) {
                Event::TkrTrackCol* trackCol = SmartDataPtr<Event::TkrTrackCol>(eventSvc(),
                    EventModel::TkrRecon::TkrTrackCol);
                int numTracks = 0;
                if(trackCol) numTracks = trackCol->size();
                trackCol = SmartDataPtr<Event::TkrTrackCol>(eventSvc(),
                    EventModel::TkrRecon::TkrCRTrackCol);
                int numCRTracks = 0;
                if(trackCol) numCRTracks = trackCol->size();
                log << MSG::DEBUG << numTracks << " Normal, " 
                    << numCRTracks << " TkrTracks remain after fitting" ;
                log << endreq;
            }
        }

        int numVtxs = 0;

        // Call vertexing
        m_stage = "TkrVertexAlg";
        if(m_TkrVertexAlg && m_lastStage>=VERTEXING && m_firstStage<=VERTEXING) {
            sc = m_TkrVertexAlg->execute();
            if (name()=="Iteration" && m_testExceptions && m_eventCount%exceptionTestTypes==4) sc = StatusCode::FAILURE;
            if (sc.isFailure())
            {
                return handleError(stageFailed);
            }
            if (doDebug) {
                Event::TkrVertexCol* vtxCol = SmartDataPtr<Event::TkrVertexCol>(eventSvc(),EventModel::TkrRecon::TkrVertexCol);
                if(vtxCol) numVtxs = vtxCol->size();
                // Check number of vertices returned
                log << numVtxs << " TkrVertex's found" << endreq;
            }
        }

        // Check for ghosts
        Event::TkrVertexCol* vtxCol = SmartDataPtr<Event::TkrVertexCol>(eventSvc(),EventModel::TkrRecon::TkrVertexCol);
        if(m_TkrVertexAlg&&m_ghostTool&&!s_ghostVertexDone&&numVtxs>0&&vtxCol&&m_checkGhosts) {
            m_stage = "GhostCheck - vertices";
            sc = m_ghostTool->flagEarlyVertices();
            s_ghostVertexDone = true;
            if (sc.isFailure())
            {
                return handleError(stageFailed);
            }
        }

        // throw some exceptions to test the logging
        m_stage = "TestExceptions";
        if (m_testExceptions) {
            if(m_eventCount%exceptionTestTypes==1) {
                throw TkrException("Testing TkrException ");
            }

            if(m_eventCount%exceptionTestTypes==2) {
                int i = 0;
                int j = 1;
                int k = j/i;
                k++;
            } else if(m_eventCount%exceptionTestTypes==5) {
                int* pInt = 0;
                int x = *pInt;
                log << MSG::INFO << "Test x = " << x << endreq;
            }
        }

    }catch( TkrException& e ){
        return handleError(e.what());

    }catch( std::exception& e) {
        return handleError(e.what());

    }catch(...){
        return handleError("Unknown Exception");
    }

    m_lastTime = m_header->time();

    log << MSG::DEBUG;
    log << "-------   Finish of Tkr Recon stage " << name() << "  --------";
    log << endreq;

    return sc;
}

StatusCode TkrReconAlg::handleError(std::string errorString) 
{
    MsgStream log(msgSvc(), name());

    m_header->setTkrReconError(); 

    SmartDataPtr<LdfEvent::EventSummaryData> summaryTds(eventSvc(), "/Event/EventSummary"); 
    if (!summaryTds) {
        LdfEvent::EventSummaryData *evtSumTds = new LdfEvent::EventSummaryData();
        evtSumTds->setTkrReconBit();
        StatusCode sc = eventSvc()->registerObject("/Event/EventSummary", evtSumTds);
        if( sc.isFailure() ) {
            log << MSG::ERROR << "could not register /Event/EventSummary " << endreq;
            // this is really bad, abort run...
            return sc;
        }
    } else {
        summaryTds->setTkrReconBit();
    }

    ++m_errorCount;
    int run     = m_header->run();
    int event   = m_header->event();

    errorString = m_stage+": "+errorString;

    m_errCountMap[m_stage]++;

    TkrErrorRecord* errorRec = new TkrErrorRecord(run, event, m_lastTime, errorString);
    if (!m_printExceptions) { log << MSG::DEBUG;}
    else                    { log << MSG::WARNING;}
    if (log.isActive()) {
        log << endreq << "**********************" << endreq;
        log << "Run " << run << " Event " << event 
            << " Time " << m_lastTime << endreq;
        log  << "   " << errorString << endreq;
        log << "**********************" << endreq;
    }
    log << endreq;

    if(m_stage!="ClusterSize"||!m_suppressClusterSizePrintout ) m_errorArray.push_back(errorRec);

    // Clean out Trees, tracks and vertices in the TDS so we don't confuse downstream algorithms
    // Start with the Trees
    SmartDataPtr<Event::TkrTreeCol> treeCol(eventSvc(),EventModel::TkrRecon::TkrTreeCol);
    if (treeCol) 
    {
        while(treeCol->size()) treeCol->pop_back();
    }

    // Now the tracks
    SmartDataPtr<Event::TkrTrackCol> 
        trackCol(eventSvc(),EventModel::TkrRecon::TkrTrackCol);
    if (trackCol) 
    {
        while(trackCol->size()) trackCol->pop_back();
    }
    
    // and the CR tracks!
    trackCol = SmartDataPtr<Event::TkrTrackCol>(eventSvc(),EventModel::TkrRecon::TkrCRTrackCol);
    if (trackCol) 
    {
        while(trackCol->size()) trackCol->pop_back();
    }

    // Now get rid of the  vertices
    SmartDataPtr<Event::TkrVertexCol> 
        vertexCol(eventSvc(), EventModel::TkrRecon::TkrVertexCol);
    if (vertexCol)
    {
        while(vertexCol->size()) vertexCol->pop_back();
    }
    // and unflag the clusters
    // Retrieve the pointer to the reconstructed clusters
    SmartDataPtr<Event::TkrClusterCol> clusterCol(
        eventSvc(),EventModel::TkrRecon::TkrClusterCol);
    if(clusterCol) 
    {
        // maybe we'll try this at some point
        // for now, it seems to cause problems downstream
        //if(m_stage=="TkrClusterAlg") {
        //    while(clusterCol->size()) clusterCol->pop_back();
        //} else {
        int num_hits = clusterCol->size();
        for(int i=0; i<num_hits; i++) (*clusterCol)[i]->unflag();
        //}
    }

    StatusCode sc = StatusCode::FAILURE;
    if(s_saveBadEvents) sc = StatusCode::SUCCESS;
    s_failed   = true;
    m_lastTime = m_header->time();
    return sc;
}

StatusCode TkrReconAlg::finalize()
{   
    MsgStream log(msgSvc(), name());
    StatusCode sc = StatusCode::SUCCESS;

    if (s_saveBadEvents) {
        log << MSG::INFO << endreq;
        log << MSG::INFO << "====>> " << m_errorCount 
            << (m_errorCount==1 ? " event" : " events") 
            << " rejected from further TkrRecon analysis " << endreq;
        log << MSG::INFO << endreq;
        if(m_errCountMap.size()>0){
            log << MSG::INFO << "Errors by stage: " << endreq;
            errorMapIter mapIter;
            for(mapIter=m_errCountMap.begin(); mapIter!=m_errCountMap.end();++mapIter) {
                log << MSG::INFO << mapIter->first << ": " << mapIter->second << endreq;
            }
            log << MSG::INFO << endreq;
            log << MSG::INFO << "Individual error records follow" << endreq;
            if(m_suppressClusterSizePrintout) log << MSG::INFO << 
                "    (ClusterSize errors are suppressed)"  << endreq;
            log << MSG::INFO << endreq;
        }
    }
    errorIter iter;
    for (iter=m_errorArray.begin(); iter!=m_errorArray.end();++iter) {
        TkrErrorRecord* pError = *iter;
        std::stringstream errorStream;
        errorStream << "Run " << pError->getRun() << " Event " << pError->getEvent() 
            << " Time " << doubleToString(pError->getLastTime());
        log << MSG::INFO << errorStream.str() << endreq;
        log << MSG::INFO << "   " << pError->getErrorString() << endreq;
    }
    return sc;
}
