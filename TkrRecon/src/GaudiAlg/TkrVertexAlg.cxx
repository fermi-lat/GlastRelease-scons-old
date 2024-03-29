
/** 
 * @class TkrVertexAlg
 *
 * @brief TkrRecon Gaudi Algorithm for controlling the search for and fitting
 *        of all possible vertices from the collection of fit tracks. 
 *        Gaudi Tools are used to implement a particular type of vertex algorithm, 
 *        and this algorithm controls their creation and use. 
 *        The algorithm depends upon input from the track finding and fitting
 *        stages of TkrRecon. Results are output to the TDS class TkrVertex
 *        Algorithm has two modes: First Pass and Iteration
 *
 * 03-27-2003 
 *
 * Adapted and augmented from vertex finding code in Atwood/Hernando code
 *
 * @author The Tracking Software Group
 *
 * File and Version Information:
 *      $Header$
 */

#include "GaudiKernel/IToolSvc.h"

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/DataObject.h"

#include "Event/TopLevel/EventModel.h"
#include "Event/Recon/TkrRecon/TkrTrack.h"

#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"

#include "src/Vertex/IVtxBaseTool.h"

#include "gui/DisplayControl.h"
#include "GuiSvc/IGuiSvc.h"
#include "gui/GuiMgr.h"

class TkrVertexAlg : public Algorithm
{
public:
    // Standard Gaudi Algorithm constructor format
    TkrVertexAlg(const std::string& name, ISvcLocator* pSvcLocator); 
    virtual ~TkrVertexAlg() {}

    // The thee phases in the life of a Gaudi Algorithm
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();
    
private:
    // Type of vertexing algorithm to run
    std::string   m_vertexToolName;

    // Yet another fine tool from Sears
    IVtxBaseTool* m_vertexTool;
};

// Used by Gaudi for identifying this algorithm
//static const AlgFactory<TkrVertexAlg>  Factory;
//const IAlgFactory& TkrVertexAlgFactory = Factory;
DECLARE_ALGORITHM_FACTORY(TkrVertexAlg);

// Standard Gaudi Constructor format
TkrVertexAlg::TkrVertexAlg(const std::string& name, ISvcLocator* pSvcLocator) :
Algorithm(name, pSvcLocator)  
{ 
    declareProperty("VertexerType", m_vertexToolName = "ComboVtxTool");
}


StatusCode TkrVertexAlg::initialize()
{
    // Purpose and Method: Initialization method for the vertexing algorithm
    // Inputs:  None
    // Outputs:  StatusCode upon completetion
    // Dependencies: None
    // Restrictions and Caveats:  None

    MsgStream log(msgSvc(), name());

    setProperties();
    
    log << MSG::DEBUG; 
    if (log.isActive() ) {
        log << "Initializing TkrVertexAlg";
    }
    log <<endreq;

    StatusCode sc = toolSvc()->retrieveTool(m_vertexToolName, m_vertexTool);

    if (sc.isFailure()) {
        log << MSG::ERROR << "Cannot initialize vertexing tool: " << m_vertexToolName << endreq;
        return sc;
    }

    return StatusCode::SUCCESS;
}


StatusCode TkrVertexAlg::execute()
{
    // Purpose and Method: Method called for each event
    // Inputs:  None
    // Outputs:  StatusCode upon completetion
    // Dependencies: The value of m_VertexerType which determines exactly which 
    //               vertexing tool is used. 
    // Restrictions and Caveats:  None

    StatusCode sc = StatusCode::SUCCESS;
  
    MsgStream log(msgSvc(), name());
  
    // Recover the collection of Fit tracks
    Event::TkrTrackCol* pTkrTracks = SmartDataPtr<Event::TkrTrackCol>(eventSvc(),EventModel::TkrRecon::TkrTrackCol);
    //std::cout << "TkrVertexAlg::execute: " << pTkrTracks->size() << " tracks to vertex" << std::endl;
    
    if(pTkrTracks==0 || pTkrTracks->size()==0) return sc;
    // Retrieve the information on vertices
    SmartDataPtr<Event::TkrVertexCol> pVtxCol(eventSvc(), EventModel::TkrRecon::TkrVertexCol);

    // If vertices already exist then we need to delete them
    if (pVtxCol)
    {
        //Loop through and delete all TkrVertex contained objects in the collection
        int colSize = pVtxCol->size();
        while(colSize--)
        {
            pVtxCol->pop_back();
        }
    }
    else  
    {
        //Create a new object container
        pVtxCol = new Event::TkrVertexCol(); 

        //Register the vertex collection object in the TDS
        sc = eventSvc()->registerObject("/Event/TkrRecon/TkrVertexCol",pVtxCol);
    }

    // If we have fit tracks then proceed with the vertexing
    if(sc.isSuccess() && pTkrTracks->size() > 0)
    {
        // This tells the tool to perform the vertexing
        sc = m_vertexTool->findVtxs();

        log << MSG::DEBUG;
        if (log.isActive()) log << pVtxCol->size() << " vertices found ";
        log << endreq;
    }
  
    return sc;
}


StatusCode TkrVertexAlg::finalize()
{   
    return StatusCode::SUCCESS;
}


