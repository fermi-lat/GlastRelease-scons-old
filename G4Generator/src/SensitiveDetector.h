//$Header$

#ifndef SensitiveDetector_h
#define SensitiveDetector_h 1

#include "G4VSensitiveDetector.hh"
#include "globals.hh"
#include <map>
#include <vector>
#include <string>

class DetectorConstruction;
class G4HCofThisEvent;
class G4Step;
/// Temp heprep stuff, to be removed
class HepRepXMLWriter;
class G4GRSSolid;

class SensitiveDetector : public G4VSensitiveDetector
{
 public:
  
  SensitiveDetector(G4String, DetectorConstruction* );
  ~SensitiveDetector();
  
  virtual void Initialize(G4HCofThisEvent*);
  virtual G4bool ProcessHits(G4Step*,G4TouchableHistory*);
  virtual void EndOfEvent(G4HCofThisEvent*);
  //void clear(){};
  
 private:
  DetectorConstruction* m_detector;
  /// Temp HepRep stuff to be removed
  HepRepXMLWriter* m_hepRepXMLWriter; 
  std::vector <std::string> m_hitID;

  // for debugging: summary of energy per logical volume
  std::map<std::string, double> m_energySummary;
};

#endif
