#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/Algorithm.h"

#include "Event/TopLevel/Event.h"
#include "Event/TopLevel/MCEvent.h"
#include "Event/TopLevel/EventModel.h"
#include "Event/MonteCarlo/McParticle.h"
#include "Event/MonteCarlo/McIntegratingHit.h"
#include "Event/MonteCarlo/McPositionHit.h"
#include "Event/Utilities/TimeStamp.h"


#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TDirectory.h"
#include "TObjArray.h"
#include "TCollection.h"  // Declares TIter

#include "mcRootData/McEvent.h"

#include "facilities/Util.h"
#include "commonData.h"
#include "RootIo/IRootIoSvc.h"

// ADDED FOR THE FILE HEADERS DEMO
#include "src/FileHeadersTool.h"

#include <map>
#include <string>

/** @class mcRootReaderAlg
 * @brief Reads Monte Carlo data from a persistent ROOT file and stores the
 * the data in the TDS.
 *
 * @author Heather Kelly
 * $Header$
 */

class mcRootReaderAlg : public Algorithm
{	
public:
    
    mcRootReaderAlg(const std::string& name, ISvcLocator* pSvcLocator);
    
    /// Handles setup by opening ROOT file in read mode and creating a new TTree
    StatusCode initialize();
    
    /// Orchastrates reading from ROOT file and storing the data on the TDS 
    /// for each event
    StatusCode execute();
    
    /// Closes the ROOT file and cleans up
    StatusCode finalize();
    
    
private:
    
    /// Retrieves event and run Id from the ROOT file and stores them on the TDS
    StatusCode readMcEvent();
    
    /// Retrieves McParticles from the ROOT file and stores them on the TDS
    StatusCode readMcParticles();
    
    /// Retrieves McPositionHits from the ROOT file and stores them on the TDS
    StatusCode readMcPositionHits();
    
    /// Retrieves McIntegratingHits from the ROOT file and stores them on the
    /// TDS
    StatusCode readMcIntegratingHits();
    
    /// Converts from ROOT's VolumeIdentifier to idents::VolumeIdentifier 
    void convertVolumeId(VolumeIdentifier rootVolId, 
        idents::VolumeIdentifier &tdsVolId);
    
    /// Closes the ROOT file
    void close();
    
    /// ROOT file pointer
    TFile *m_mcFile;
    /// ROOT chain
    TChain *m_mcTree;
    /// Top-level Monte Carlo ROOT object
    McEvent *m_mcEvt;
    /// name of the input ROOT file
    std::string m_fileName;
    /// Array of file names for TChain
    StringArrayProperty m_fileList;
    /// name of the Monte Carlo TTree stored in the ROOT file
    std::string m_treeName;
    /// Number of Events in the input ROOT TTree
    int m_numEvents;

    commonData m_common;

    /// Keep track of MC particles as we retrieve them from the ROOT file
    /// The id is the unique id assigned by ROOT for every TObject.
    std::map<UInt_t, Event::McParticle*> m_particleMap;

    IRootIoSvc*   m_rootIoSvc;
  

    // ADDED FOR THE FILE HEADERS DEMO
    IFileHeadersTool * m_headersTool ;
};

static const AlgFactory<mcRootReaderAlg>  Factory;
const IAlgFactory& mcRootReaderAlgFactory = Factory;


mcRootReaderAlg::mcRootReaderAlg(const std::string& name, 
                                 ISvcLocator* pSvcLocator) 
                                 : Algorithm(name, pSvcLocator)
{
    // Input pararmeters that may be set via the jobOptions file
    // Input ROOT file name
    declareProperty("mcRootFile",m_fileName="");
    StringArrayProperty initList;
    std::vector<std::string> initVec;
    initVec.push_back("mc.root");
    initList.setValue(initVec);
    declareProperty("mcRootFileList", m_fileList=initList);
    // ROOT TTree name
    declareProperty("mcTreeName", m_treeName="Mc");
    
    initVec.clear();
    m_particleMap.clear();
}

StatusCode mcRootReaderAlg::initialize()
{
    // Purpose and Method:  Called once before the run begins.
    //	  This method opens a new ROOT file and prepares for reading.
    
    StatusCode sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());
    
    // ADDED FOR THE FILE HEADERS DEMO
    StatusCode headersSc = toolSvc()->retrieveTool("FileHeadersTool",m_headersTool) ;
    if (headersSc.isFailure()) {
        log<<MSG::WARNING << "Failed to retreive headers tool" << endreq;
    }
    
    // Use the Job options service to set the Algorithm's parameters
    // This will retrieve parameters set in the job options file
    setProperties();
     
    if ( service("RootIoSvc", m_rootIoSvc, true).isFailure() ){
        log << MSG::INFO << "Couldn't find the RootIoSvc!" << endreq;
        log << MSG::INFO << "Event loop will not terminate gracefully" << endreq;
        m_rootIoSvc = 0;
        //return StatusCode::FAILURE;
    }   

    facilities::Util::expandEnvVar(&m_fileName);
    
    // Save the current directory for the ntuple writer service
    TDirectory *saveDir = gDirectory;	
    m_mcTree = new TChain(m_treeName.c_str());

    std::string emptyStr("");
    if (m_fileName.compare(emptyStr) != 0) {
	  TFile f(m_fileName.c_str());
      if (!f.IsOpen()) {
        log << MSG::ERROR << "ROOT file " << m_fileName.c_str() 
            << " could not be opened for reading." << endreq;
        return StatusCode::FAILURE;
      }
	  f.Close();
	  m_mcTree->Add(m_fileName.c_str());
          log << MSG::INFO << "Opened file: " << m_fileName.c_str() << endreq;
    } else {
      const std::vector<std::string> fileList = m_fileList.value( );
      std::vector<std::string>::const_iterator it;
      std::vector<std::string>::const_iterator itend = fileList.end( );
      for (it = fileList.begin(); it != itend; it++) {
        std::string theFile = (*it);
		TFile f(theFile.c_str());
        if (!f.IsOpen()) {
          log << MSG::ERROR << "ROOT file " << theFile.c_str() 
              << " could not be opened for reading." << endreq;
          return StatusCode::FAILURE;
        }
		f.Close();
		m_mcTree->Add(theFile.c_str());
                log << MSG::INFO << "Opened file: " << theFile.c_str() << endreq;
      }
    }
    
    m_mcEvt = 0;
    m_mcTree->SetBranchAddress("McEvent", &m_mcEvt);
    m_common.m_mcEvt = m_mcEvt;
    
    m_numEvents = m_mcTree->GetEntries();

    if (m_rootIoSvc) {
        m_rootIoSvc->setRootEvtMax(m_numEvents);
	if (!m_mcTree->GetIndex()) m_mcTree->BuildIndex("m_runId", "m_eventId");
        m_rootIoSvc->registerRootTree(m_mcTree);
    }
     
    saveDir->cd();
    return sc;
    
}

StatusCode mcRootReaderAlg::execute()
{
    // Purpose and Method:  Called once per event.  This method calls
    //	 the appropriate methods to read data from the ROOT file and store
    //	 data on the TDS.
    
    MsgStream log(msgSvc(), name());
    StatusCode sc = StatusCode::SUCCESS;
    
    if (m_mcEvt) m_mcEvt->Clear();
   
    if (!m_mcTree) {
      log << MSG::WARNING << "MC Root unreadable" << endreq;
      return sc;
    }

	static Int_t evtId = 0;
	int readInd, numBytes;
	if (evtId==0)m_mcTree->SetBranchAddress("McEvent", &m_mcEvt);
	std::pair<int,int> runEventPair = (m_rootIoSvc) ? m_rootIoSvc->runEventPair() : std::pair<int,int>(-1,-1);

	if ((m_rootIoSvc) &&  (m_rootIoSvc->index() >= 0)) {
		readInd = m_rootIoSvc->index();
	} else if ((m_rootIoSvc) && (runEventPair.first != -1) && (runEventPair.second != -1)) {
		int run = runEventPair.first;
		int evt = runEventPair.second;
		readInd = m_mcTree->GetEntryNumberWithIndex(run, evt);
	} else {
		readInd = evtId;
	}

	if (readInd >= m_numEvents) {
            log << MSG::WARNING << "Requested index is out of bounds - no MC data loaded" << endreq;
            return StatusCode::SUCCESS;
	}
	else {
		log << MSG::INFO << "Requested index: " << readInd << endreq;
	}

    // ADDED FOR THE FILE HEADERS DEMO
    m_mcTree->LoadTree(readInd);
    m_headersTool->readConstMcHeader(m_mcTree->GetFile()) ;
    
	numBytes = m_mcTree->GetEntry(readInd); 
	if ((numBytes <= 0) || (!m_mcEvt)) {
            log << MSG::WARNING << "Failed to Load Mc Event" << endreq;
            return StatusCode::SUCCESS;
	}

    sc = readMcEvent();
    if (sc.isFailure()) {
        log << MSG::ERROR << "Failed to read top level McEvent" << endreq;
        return sc;
    }
    
    m_particleMap.clear();
    
    Event::MCEvent* mcEventObj = 
        SmartDataPtr<Event::MCEvent>(eventSvc(), EventModel::MC::Event);
    if (!mcEventObj) {
        log << "Failed to retrieve McEvent" << endreq;
        return sc;
    }
    
    sc = readMcParticles();
    if (sc.isFailure()) return sc;
    
    sc = readMcPositionHits();
    if (sc.isFailure()) return sc;
    
    sc = readMcIntegratingHits();
    if (sc.isFailure()) return sc;
    
	evtId = readInd+1;
     
    return sc;
}

StatusCode mcRootReaderAlg::readMcEvent() {
    // Purpose and Method:  Retrieves the event and run ids from the ROOT file
    //	  Stores them on the TDS in the EventHeader
    // Input:  ROOT McEvent object
    // TDS Output:  EventModel::EventHeader
    
    MsgStream log(msgSvc(), name());
    
    StatusCode sc = StatusCode::SUCCESS;
    
    // Retrieve the Event data for this event
    SmartDataPtr<Event::EventHeader> evt(eventSvc(), EventModel::EventHeader);
    if (!evt) return sc;
    
    unsigned int eventIdTds = evt->event();
    unsigned int runIdTds = evt->run();
    
    unsigned int eventIdRoot = m_mcEvt->getEventId();
    unsigned int runIdRoot = m_mcEvt->getRunId();
    int sourceIdRoot = m_mcEvt->getSourceId();
    unsigned int sequenceRoot = m_mcEvt->getSequence();
	TimeStamp timeTds(m_mcEvt->getTimeStamp());
    
    // Check to see if the event and run ids have already been set.
    if (eventIdTds != eventIdRoot) evt->setEvent(eventIdRoot);
    if (runIdTds != runIdRoot) evt->setRun(runIdRoot);
    
    SmartDataPtr<Event::MCEvent> mcEvt(eventSvc(), EventModel::MC::Event);
    if (!mcEvt) return sc;
    mcEvt->initialize(runIdRoot, sourceIdRoot, sequenceRoot, timeTds);
    
    return sc;
}

StatusCode mcRootReaderAlg::readMcParticles() {
    // Purpose and Method:  Retrieve the McParticle collection from the ROOT
    //	  file and fill the TDS McParticle collection
    // Input:  ROOT McParticle collection
    // TDS Output:  EventModel::EVENT::McParticleCol
    
    MsgStream log(msgSvc(), name());
    
    StatusCode sc = StatusCode::SUCCESS;
    
    TObjArray *particles = m_mcEvt->getMcParticleCol();
    if (!particles) return sc;
    TIter partIter(particles);
    
    // create the TDS location for the McParticle Collection
    Event::McParticleCol* pTdsCol = new Event::McParticleCol;
    sc = eventSvc()->registerObject(EventModel::MC::McParticleCol, pTdsCol);
    if (sc.isFailure()) {
        log << "Failed to register McParticle Collection" << endreq;
        return sc;
    }
    
    McParticle *pRoot;
    // Create the map of ROOT unique ids and TDS McParticle objects
    while ((pRoot = (McParticle*)partIter.Next()) != 0) {
        Event::McParticle *pTds = new Event::McParticle();
        m_particleMap[pRoot->GetUniqueID()] = pTds;
    }
    
    // reset the iterator to the beginning of the list of McParticles
    partIter.Reset();
    
    // Now that the map is available, we initialize all of the TDS McParticles
    while ((pRoot = (McParticle*)partIter.Next())!=0) {
        
        Event::McParticle *pTds = m_particleMap[pRoot->GetUniqueID()];
        
        int idTds = pRoot->getParticleId();
        
        unsigned int statusBitsTds = pRoot->getStatusFlags();
        
        TLorentzVector initialMomRoot = pRoot->getInitialFourMomentum();
        
        HepLorentzVector initialMomTds(initialMomRoot.X(), initialMomRoot.Y(),
            initialMomRoot.Z(), initialMomRoot.T());
        
        TLorentzVector finalMomRoot = pRoot->getFinalFourMomentum();
        HepLorentzVector finalMomTds(finalMomRoot.X(), finalMomRoot.Y(), 
            finalMomRoot.Z(), finalMomRoot.T());
        
        TVector3 initPosRoot = pRoot->getInitialPosition();
        HepPoint3D initPosTds(initPosRoot.X(), 
            initPosRoot.Y(), initPosRoot.Z());
        
        TVector3 finalPosRoot = pRoot->getFinalPosition();
        HepPoint3D finalPosTds(finalPosRoot.X(), 
            finalPosRoot.Y(), finalPosRoot.Z());
        
        const McParticle *momRoot = pRoot->getMother();
        
        Event::McParticle *momTds = 0;
        
        if (momRoot != 0) {
            // The case where this is the primary particle
            if (momRoot->GetUniqueID() == pRoot->GetUniqueID()) {
                momTds = pTds;
            } else if (m_particleMap.find(momRoot->GetUniqueID()) != m_particleMap.end()) {
                momTds = m_particleMap[momRoot->GetUniqueID()];
            } else {
                log << MSG::INFO << "Failed to find the McParticle in the"
                    << " map!!" << endreq;
            }
        } else {
            static bool warn = false;
            if (!warn) {
                log << MSG::WARNING << "Cannot read in mother McParticles using "
                    << "\nSetting all McParticle mother pointers to"
                    << " point at themselves" << endreq;
                warn = true;
            }
            momTds = pTds;
        }

        std::string processTdsStr(pRoot->getProcess().Data());
        // Setup the TDS version fo the McParticle
        pTds->init(momTds, idTds, statusBitsTds, initialMomTds, 
                    finalMomTds, initPosTds, finalPosTds, processTdsStr );
        
        // Check version < ROOT 3.04.02, and do not attempt to read in daughters
        static bool warn = false;
        int ver = m_mcTree->GetCurrentFile()->GetVersion();
       //// if (!warn) {
        if (ver < 30402) {
            log << MSG::WARNING << "Cannot read in TRef with ROOT 3.02.07"
                " McParticle daughter list is unavailable" << endreq;
            warn = true;
        }
        
        
        // Process the list of daughters
        const TRefArray daughterArr = pRoot->getDaughterList();
        const McParticle *daughterPartRoot;
        int iPart;
        for (iPart = 0; iPart < daughterArr.GetEntries(); iPart++) {
			// Retrieving the daughter directly - rather than through my copy of TRefArray
            //daughterPartRoot = (McParticle*)daughterArr.At(iPart);
            daughterPartRoot = pRoot->getDaughter(iPart);
            if (!daughterPartRoot) {
                static bool warn = false;
                if (!warn) {
                    log << MSG::WARNING << "Cannot read in TRef "
                        " McParticle daughter list is unavailable" << endreq;
                    warn = true;
                }
            } else {
                if (m_particleMap.find(daughterPartRoot->GetUniqueID()) != m_particleMap.end()) {
                    Event::McParticle* partTds = m_particleMap[daughterPartRoot->GetUniqueID()];
                    pTds->addDaughter(partTds);
                } else {
                    log << MSG::WARNING << "failed to find McParticle daughter in the"
                        << " map!" << endreq;
                }
            }
            
        }
        

        // Add the TDS McParticle to the TDS collection of McParticles
        pTdsCol->push_back(pTds);
        
    }
    
    return sc;
}

StatusCode mcRootReaderAlg::readMcPositionHits() {
    // Purpose and Method:  Retrieve the McPositionHit collection from the ROOT
    //	file and fill the TDS McPositionHit collection.
    // Input:  ROOT McPositionHit collectoin
    // TDS Output:  EventModel::MC::McPositionHitCol
    
    MsgStream log(msgSvc(), name());
    StatusCode sc = StatusCode::SUCCESS;
    
    const TObjArray *posHits = m_mcEvt->getMcPositionHitCol();
    if (!posHits) return sc;
    
    // this test and alternate return needed to feed to G4Generator, which wants to register its own collection
    if(  posHits->GetLast() ==-1)  return sc;

    TIter hitIter(posHits);
    
    // create the TDS location for the McParticle Collection
    Event::McPositionHitVector* pTdsCol = new Event::McPositionHitVector;
    sc = eventSvc()->registerObject(EventModel::MC::McPositionHitCol, pTdsCol);
    if (sc.isFailure()) {
        log << "Failed to register McPositionHit Collection" << endreq;
        return sc;
    }
    
    const McPositionHit *posHitRoot;
    while ((posHitRoot = (McPositionHit*)hitIter.Next())!=0) {
        
        Event::McPositionHit *posHitTds = new Event::McPositionHit();
        
        VolumeIdentifier volIdRoot = posHitRoot->getVolumeId();
        idents::VolumeIdentifier volIdTds;
        convertVolumeId(volIdRoot, volIdTds);
        
        TVector3 entryRoot = posHitRoot->getEntryPosition();
        HepPoint3D entryTds(entryRoot.X(), entryRoot.Y(), entryRoot.Z());
        
        TVector3 exitRoot = posHitRoot->getExitPosition();
        HepPoint3D exitTds(exitRoot.X(), exitRoot.Y(), exitRoot.Z());
        
        TVector3 globalEntryRoot = posHitRoot->getGlobalEntryPosition();
        HepPoint3D globalEntryTds(globalEntryRoot.X(), globalEntryRoot.Y(), globalEntryRoot.Z());
        
        TVector3 globalExitRoot = posHitRoot->getGlobalExitPosition();
        HepPoint3D globalExitTds(globalExitRoot.X(), globalExitRoot.Y(), globalExitRoot.Z());
        
        double edepTds= posHitRoot->getDepositedEnergy();
        
        double epartTds = posHitRoot->getParticleEnergy();
        posHitTds->setParticleEnergy(epartTds);
        
        double tofTds = posHitRoot->getTimeOfFlight();
        posHitTds->setTimeOfFlight(tofTds);
        
        const McParticle* mcPartRoot = posHitRoot->getMcParticle();
        Event::McParticle *mcPartTds = 0;
        if (mcPartRoot != 0) {
            mcPartTds = m_particleMap[mcPartRoot->GetUniqueID()];
            posHitTds->setMcParticle(mcPartTds);
        }
        
        const McParticle *originRoot = posHitRoot->getOriginMcParticle();
        Event::McParticle *originTds = 0;
        if (originRoot != 0) {
            originTds = m_particleMap[originRoot->GetUniqueID()];
            posHitTds->setOriginMcParticle(originTds);
        }
        
        Event::McParticle::StdHepId particleIdTds = posHitRoot->getMcParticleId();
        Event::McParticle::StdHepId originIdTds = posHitRoot->getOriginMcParticleId();
        
        // setup the TDS McPositionHit
        posHitTds->init(edepTds, volIdTds, entryTds, exitTds, globalEntryTds, globalExitTds);
        posHitTds->setMcParticleId(particleIdTds);
        posHitTds->setOriginMcParticleId(originIdTds);
        
        // add the McPositionHit to the TDS collection of McPositionHits
        pTdsCol->push_back(posHitTds);
    }
    
    return sc;
}

StatusCode mcRootReaderAlg::readMcIntegratingHits() {
    // Purpose and Method:  Retrieve the McIntegratingHit collection from the
    //	   ROOT file and fill the TDS McIntegratingHit collection.
    // Input:  ROOT McIntegratingHit collection
    // TDS Output:  EventModel::EVENT::McIntegratingHitCol
    
    MsgStream log(msgSvc(), name());
    StatusCode sc = StatusCode::SUCCESS;
    
    const TObjArray *intHits = m_mcEvt->getMcIntegratingHitCol();
    if (!intHits) return sc;

    // this test and alternate return needed to feed to G4Generator, which wants to register its own collection
    if(  intHits->GetLast() ==-1)  return sc;

    TIter hitIter(intHits);
    
    // create the TDS location for the McParticle Collection
    Event::McIntegratingHitVector* pTdsCol = new Event::McIntegratingHitVector;
    sc = eventSvc()->registerObject(EventModel::MC::McIntegratingHitCol, pTdsCol);
    if (sc.isFailure()) {
        log << "Failed to register McIntegratingHit" << endreq;
        return sc;
    }
    
    McIntegratingHit *intHitRoot;
    while ((intHitRoot = (McIntegratingHit*)hitIter.Next())!=0) {
        
        Event::McIntegratingHit *intHitTds = new Event::McIntegratingHit();
        
        const VolumeIdentifier idRoot = intHitRoot->getVolumeId();
        idents::VolumeIdentifier idTds;
        convertVolumeId(idRoot, idTds);
        
        intHitTds->setVolumeID(idTds);
        
        //	  const McIntegratingHit::energyDepositMap mcPartMapRoot = intHitRoot->getItemizedEnergy();
        //	  McIntegratingHit::energyDepositMap::const_iterator rootMapIt;
        //	  log << MSG::DEBUG << "EnergyMap size: " << mcPartMapRoot.size() << endreq;
        // Can't seem to read this back in due to TRef problem in Root 3.02.03
        /*
        for (rootMapIt = mcPartMapRoot.begin(); rootMapIt != mcPartMapRoot.end(); rootMapIt++){
        McParticle* mcPartRoot = rootMapIt->first;
        Event::McParticle *mcPartTds = m_particleMap[mcPartRoot->GetUniqueID()];
        double e = rootMapIt->second;
        TVector3 posRoot = mcPartRoot->getFinalPosition();
        HepPoint3D posTds(posRoot.X(), posRoot.Y(), posRoot.Z());
        intHitTds->addEnergyItem(e, mcPartTds, posTds);
        }
        */
        
        double totalEnergyRoot = intHitRoot->getTotalEnergy();
        const TVector3 moment1Root = intHitRoot->getMoment1();
        HepPoint3D moment1Tds(moment1Root.X(), moment1Root.Y(), moment1Root.Z());
        const TVector3 moment2Root = intHitRoot->getMoment2();
        HepPoint3D moment2Tds(moment2Root.X(), moment2Root.Y(), moment2Root.Z());
        
        double energyArr[3] = { intHitRoot->getMcParticleEnergy(McIntegratingHit::PRIMARY),
            intHitRoot->getMcParticleEnergy(McIntegratingHit::ELECTRON),
            intHitRoot->getMcParticleEnergy(McIntegratingHit::POSITRON) };
        
        intHitTds->setEnergyItems(totalEnergyRoot, energyArr, moment1Tds, moment2Tds);
        
        // Add the TDS McIntegratingHit to the TDS McIntegratingHit collection
        pTdsCol->push_back(intHitTds);
    }
    
    return sc;
}

void mcRootReaderAlg::convertVolumeId(VolumeIdentifier rootVolId, 
                                      idents::VolumeIdentifier& tdsVolId) 
{
    // Purpose and Method:  We must store the volume ids as two 32 bit UInt_t
    //	   in the ROOT class.  The idents::VolumeIdentifier class stores the
    //	   data in one 64 bit word.  We must convert from the two 32 bit words
    //	   into the 64 bit word.  We perform the conversion by iterating over
    //	   all of the ids in the ROOT VolumeIdentifier and appending them to
    //	   the TDS idents::VolumeIdentifier.
    // Input:  ROOT VolumeIdentifier
    // Ouput:  idents::VolumeIdentifier
    
    int index;
    for (index = 0; index < rootVolId.size(); index++) {
        tdsVolId.append(rootVolId.operator [](index));
    }
    
}


void mcRootReaderAlg::close() 
{
    // Purpose and Method:  Writes the ROOT file at the end of the run.
    //	  The TObject::kOverWrite parameter is used in the Write method
    //	  since ROOT will periodically write to the ROOT file when the bufSize
    //	  is filled.  Writing would create 2 copies of the same tree to be
    //	  stored in the ROOT file, if we did not specify kOverwrite.
    
    //TDirectory *saveDir = gDirectory;
    //m_mcFile->cd();
    //m_mcFile->Close();
    //saveDir->cd();
    if (m_mcTree) delete m_mcTree;
}

StatusCode mcRootReaderAlg::finalize()
{
    close();
    
    StatusCode sc = StatusCode::SUCCESS;
    setFinalized();
    return sc;
}
