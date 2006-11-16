/** @file InterleaveAlg.cxx

@brief declaration and definition of the class InterleaveAlg

$Header$

*/

#include "InterleaveAlg.h"

// this service manages livetime calculations
#include "Trigger/ILivetimeSvc.h"

// TDS class declarations: input data, and McParticle tree
#include "Event/MonteCarlo/McParticle.h"
#include "Event/TopLevel/Event.h"
#include "Event/TopLevel/EventModel.h"
#include "Event/TopLevel/MCEvent.h"

// access to the tuple
#include "ntupleWriterSvc/INTupleWriterSvc.h"

// ROOT includes
#include "TTree.h"
#include "TFile.h"
#include "TLeaf.h"

// Gaudi system includes
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/SmartRefVector.h"

#include "facilities/Util.h"

#include <cassert>
#include <vector>
#include <iomanip>
#include <fstream>
#include <string>

//------------------------------------------------------------------------

static const AlgFactory<InterleaveAlg>  Factory;
const IAlgFactory& InterleaveAlgFactory = Factory;

namespace {
    // For static function access to the Gaudi-generated instance
    InterleaveAlg* instance(0);

    std::vector<std::string> sourcenames;
    std::map<std::string, BackgroundSelection*> selectormap;
    double defaultRate(500);
}


void InterleaveAlg::defineSource(const std::string& name){ sourcenames.push_back(name);}

void InterleaveAlg::currentSource(const std::string& name)
{
    if( instance==0) return; // not set up yet
    BackgroundSelection* selector = selectormap[name];
    instance->setSelector( selector );
}

double InterleaveAlg::downlinkRate(const std::string& name)
{
    if( instance==0) return defaultRate; // just to get started 
    double ret(0);
    if(name.empty()){
        for(std::map<std::string, BackgroundSelection*>::const_iterator it(selectormap.begin()); it!=selectormap.end();++it){
            ret += it->second->downlinkRate();
        }

    } else{
        ret = selectormap[name]->downlinkRate();
    }
    return ret;
}
double InterleaveAlg::triggerRate(const std::string& name)
{
    double ret(0);
    if(name.empty()){
        for(std::map<std::string, BackgroundSelection*>::const_iterator it(selectormap.begin()); it!=selectormap.end();++it){
            ret += it->second->triggerRate();
        }

    } else{
        ret = selectormap[name]->triggerRate();
    }
    return ret;
}

//------------------------------------------------------------------------
//! ctor
InterleaveAlg::InterleaveAlg(const std::string& name, ISvcLocator* pSvcLocator)
:Algorithm(name, pSvcLocator)
, m_selector(0)
, m_count(0),   m_meritTuple(0), m_runLeaf(0)

{
    if( instance!=0 ) throw std::invalid_argument("InterleaveAlg: only one instance allowed!");

    // declare properties with setProperties calls
    declareProperty("RootFile",     m_rootFile="");
    declareProperty("TreeName",     m_treeName="MeritTuple");
    declareProperty("DisableList",  m_disableList);
    declareProperty("MapName",      m_mapName="interleave_map");

    // initialize the disable list, which can be added to, or replaced in the JO
    std::vector<std::string> tlist;
    tlist.push_back("EvtElapsedTime");
    tlist.push_back("EvtLiveTime");
    tlist.push_back("Pt*");
    tlist.push_back("FT1*");
    tlist.push_back("CT*");
    m_disableList = tlist;

}
//------------------------------------------------------------------------
//! set parameters and attach to various perhaps useful services.
StatusCode InterleaveAlg::initialize(){
    StatusCode  sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());

    // Use the Job options service to set the Algorithm's parameters
    setProperties();

    // get a pointer to RootTupleSvc
    sc = service("RootTupleSvc", m_rootTupleSvc);       
    if( sc.isFailure() ) {
        log << MSG::ERROR << "failed to get the RootTupleSvc" << endreq;
        return sc;
    }
    m_rootTupleSvc->addItem(m_mapName.value(), "run",    &m_run);
    m_rootTupleSvc->addItem(m_mapName.value(), "event",  &m_event);
    m_rootTupleSvc->addItem(m_mapName.value(), "irun",   &m_irun);
    m_rootTupleSvc->addItem(m_mapName.value(), "ievent", &m_ievent);


    sc = service("LivetimeSvc", m_LivetimeSvc);
    if( sc.isFailure() ) {
        log << MSG::ERROR << "failed to get the LivetimeSvc" << endreq;
        return sc;
    }

    if( m_meritTuple==0){
        void * ptr= 0;
        m_rootTupleSvc->getItem(m_treeName.value().c_str(),"", ptr);
        if( ptr==0){
            log << MSG::ERROR << "Could not find the merit tuple" << endreq;
            return StatusCode::FAILURE;
        }
        m_meritTuple = static_cast<TTree*>(ptr);
    }
    // initialize the background selection
    try {
        std::string file(m_rootFile.value());
        if( !file.empty()){
            facilities::Util::expandEnvVar(&file);
            log << MSG::INFO << "Using xml or root file " << file << " for interleave." << endreq;
            for( std::vector<std::string>::const_iterator it(sourcenames.begin()); it!=sourcenames.end();++it){
                const std::string& name=*it;
                log << MSG::INFO << "setting up tuple key "<<  name << endreq;
                selectormap[name]= new BackgroundSelection(name, file, m_disableList, m_meritTuple);
            }
            log << endreq;
            // initialized, can use the statics now.
            instance = this;

        }else{
            log << MSG::INFO<< "No file specified for interleave" << endreq;
            return sc;
        }

    }catch( const std::exception& e){
        log << MSG::ERROR << e.what() << endreq;
#if 0
        log << MSG::WARNING << "Continuing without background" << endreq;
            return sc;
#else
        return StatusCode::FAILURE;
#endif
    }

    log << MSG::INFO << "initialized OK: initial downlink rate to merge is " << downlinkRate() << " Hz"<< endreq;
    m_LivetimeSvc->setTriggerRate(triggerRate());

    return sc;
}

//------------------------------------------------------------------------
//! process an event
StatusCode InterleaveAlg::execute()
{
    StatusCode  sc = StatusCode::SUCCESS;

    if( m_selector==0) return sc;
    MsgStream   log( msgSvc(), name() );

    // check that the TDS has an appropriate pseudo-background 

    SmartDataPtr<Event::McParticleCol> particles(eventSvc(), EventModel::MC::McParticleCol);
    
    if( m_runLeaf==0){
        if( (m_runLeaf = m_meritTuple->GetLeaf("EvtRun"))==0){
            log << MSG::ERROR << "EvtRun leaf not found in the tuple" << endreq;
            return StatusCode::FAILURE;
        }
        if( (m_eventLeaf = m_meritTuple->GetLeaf("EvtEventId"))==0){
            log << MSG::ERROR << "EvtEventId leaf not found in the tuple" << endreq;
            return StatusCode::FAILURE;
        }
    }

    if (0==particles) {
        log << MSG::ERROR << "No MC particles!" << endreq;
        return StatusCode::FAILURE;
    }   

    const Event::McParticle& primary = **particles->begin();
    double ke = primary.initialFourMomentum().e()-primary.initialFourMomentum().m();
    log << MSG::DEBUG << "Primary particle energy: " << ke << endreq;

    if( ke>1. ){
        setFilterPassed(false); // since this is on a branch, and we want the sequence to fail
        return sc; // not a flagged sampled_background 
    }
    ++m_count;


    // let the livetime service know about the current trigger rate,
    // and set the current accumulated live time in the header
    m_LivetimeSvc->setTriggerRate( triggerRate() );
    SmartDataPtr<Event::EventHeader>   header(eventSvc(),    EventModel::EventHeader);
    header->setLivetime( m_LivetimeSvc->livetime());

    // ask for a tree corresponding to our current position: it will set all the tuple

    m_selector->selectEvent();

    // overwrite the event info
    copyEventInfo();
    
    // finally flag that we want to add this event to the output tuple
    m_rootTupleSvc->storeRowFlag( m_treeName.value(), true);

    return sc;
}

//------------------------------------------------------------------------
//! clean up, summarize
StatusCode InterleaveAlg::finalize(){
    StatusCode  sc = StatusCode::SUCCESS;
    if( m_selector==0) return sc;
    MsgStream log(msgSvc(), name());

    log << MSG::INFO << "Inserted "<< m_count << " sampled background events." << endreq; 
    delete m_selector;
    return sc;
}

//------------------------------------------------------------------------
namespace {
    //! Utility to set a scalar value in a ROOT tree. 
    //! Assume that the type is known!
    template <typename  Type> 
        void setLeafValue(TLeaf* leaf, Type newvalue)
    {
        Type& rval = *static_cast<Type*>(leaf->GetValuePointer());
        rval = newvalue;
    }

}
//------------------------------------------------------------------------
void InterleaveAlg::copyEventInfo()
{
    
    static TLeaf *  timeLeaf=0,* liveLeaf=0, *sourceLeaf=0;
    if( timeLeaf==0){
        timeLeaf =  m_meritTuple->GetLeaf("EvtElapsedTime");
        liveLeaf =  m_meritTuple->GetLeaf("EvtLiveTime");
        sourceLeaf =m_meritTuple->GetLeaf("McSourceId");
        assert(timeLeaf!=0);
    }
    SmartDataPtr<Event::EventHeader>   header(eventSvc(),    EventModel::EventHeader);
    // these types *must* correspond with those in EvtValsTool, which this code replaces for interleaved events
    float EvtRun           = header->run();
    float EvtEventId       = header->event();
    double EvtElapsedTime  = header->time();
    float EvtLiveTime      = header->livetime();

    // load the map entries and save the row
    m_run   =  header->run();
    m_event =  header->event();
    m_irun  =  static_cast<int>(m_runLeaf->GetValue());
    m_ievent=  static_cast<int>( m_eventLeaf->GetValue());
    m_rootTupleSvc->saveRow(m_mapName.value());

    // these have to be done here, since there is no algorithm 
    setLeafValue(m_runLeaf,     EvtRun);
    setLeafValue(m_eventLeaf,   EvtEventId);
    setLeafValue(timeLeaf,      EvtElapsedTime);
    setLeafValue(liveLeaf,      EvtLiveTime);

    //  make the source id negative; make zero -1 by offset. (2's complement)
    if( sourceLeaf==0 ) return; // testing mode
    float & sourceid = *static_cast<float*>(sourceLeaf->GetValuePointer());
    sourceid=-1-sourceid;
}


