// $Header$

#ifndef GlastDetectorManager_h
#define GlastDetectorManager_h
#ifdef WIN32 // for G4 
#include <float.h>
#endif

#include "G4LogicalVolume.hh"
#include "G4VSensitiveDetector.hh"

#include "DisplayManager.h"

#include "idents/VolumeIdentifier.h"
#include "DetectorConstruction.h"

#include <map>

class Instrument;
class SiDetector;
class CsIDetector;
class Scintillator;
class DiodeDetector;
class GenericDet;

/**
A single class that manages the GlastDetector hierarchy in the G4 environment:
- Reads the xml description file and instantiates them
- connects each GlastDetector object with a special interface object to convert G4Step information
- When called with a G4Step, construcs the ID from the touchable history, then looks up the 
  corresponding object

  */
class GlastDetectorManager : public G4VSensitiveDetector {
public:
    
    //! constructor called with pointer to DetectorConstruction, for map of (partial) ids
    //! needed for constructing the from the list of physical volumes in the touchable history
    GlastDetectorManager( DetectorConstruction* det);
    
    //! initialize clears things 
    virtual void Initialize(G4HCofThisEvent*);

    //! G4 passes in each step in a sensitive volume
    virtual G4bool ProcessHits(G4Step* aStep ,G4TouchableHistory*);

    //! End of event will finish digitization
    virtual void EndOfEvent(G4HCofThisEvent*);
    
    idents::VolumeIdentifier constructId(G4Step * aStep);
    
    //! Called from DetectorConstruction to set the sensitive detector propery
    void process(G4LogicalVolume*);
    
    //! used in initialization
    void addSiDetector(SiDetector* si);
    void addCsIDetector(CsIDetector* csi);
    void addDiodeDetector(DiodeDetector* diode);
    void addACDtile(Scintillator* acd);
private:
    Instrument* m_instrument;
    DetectorConstruction::IdMap* m_idMap;
    
    typedef std::map<idents::VolumeIdentifier::int64, GenericDet*> DetectorMap;
    DetectorMap m_detMap;
    // for debugging: summary of energy per logical volume
    std::map<std::string, double> m_energySummary;
    
    //! keep track of detectors
    typedef std::map<idents::VolumeIdentifier, unsigned int> DetectorList;
    typedef std::map<idents::VolumeIdentifier, double>DetectorEnergyTotal;
    DetectorList m_detectorList;
    DetectorEnergyTotal m_detectorEnergy;
    
};

#endif