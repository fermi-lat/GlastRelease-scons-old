#ifndef RootFileAnalysis_h
#define RootFileAnalysis_h 1

// LOCAL INCLUDES

// GLAST INCLUDES
#include "digiRootData/DigiEvent.h"

// SET TO '1' TO ENABLE RECON/MC
#define CGC_USE_RECON 0
#define CGC_USE_MC    0

#if CGC_USE_RECON
#include "reconRootData/ReconEvent.h"
#endif
#if CGC_USE_MC
#include "mcRootData/McEvent.h"
#endif

// EXTLIB INCLUDES
#include "TChain.h"

// STD INCLUDES
#include <string>
#include <vector>
#include <iostream>

/** \brief Makes a VCR for GLAST root event files

  RootFileAnalysis contains the following features...

  1) ability to read in one or more GLAST digi, recon and/or mc root files.
  2) ability to step/rewind through events in those files
*/

using namespace std;

class RootFileAnalysis {
  public :
    RootFileAnalysis(const vector<string> &mcFilenames,
                     const vector<string> &digiFilenames,
                     const vector<string> &recFilenames,
                     ostream &ostr = cout);
  ~RootFileAnalysis();

  /// Manually set next event to be read.  fseek() of sorts
  void startWithEvt(Int_t nEvt) { m_startEvt = nEvt; };
  /// Rewind file back to event 0.
  void rewind() { m_startEvt = 0; };

  /// returns total number of events in all open files
  UInt_t getEntries() const;
  /// Retrieve pointers to given event #.
  UInt_t getEvent(UInt_t ievt);

 protected:
#if CGC_USE_MC
  /// Chains store event data for all files
  TChain      m_mcChain;
  /// Pointer to current McEvent
  McEvent     *m_mcEvt;
  /// true if MC data stream is enabled
  bool m_mcEnabled;
  /// list of filenames for mc data
  vector<string> m_mcFilenames;
#endif

  /// Chains store event data for all files
  TChain      m_digiChain;
  /// pointer to current DigiEvent
  DigiEvent   *m_digiEvt;
  /// true if Digi data stream is enabled
  bool m_digiEnabled;
  /// list of input files for digi data
  vector<string> m_digiFilenames;

#if CGC_USE_RECON
  /// Chains store event data for all files
  TCain       m_recChain;
  /// pointer to current ReconEvent
  ReconEvent  *m_recEvt;
  /// true if Recon data stream is enabled
  bool m_recEnabled;
  /// list of input file for recon data
  vector<string> m_recFilenames;
#endif


  /// helpful list of all 3 TChains
  TObjArray   m_chainArr;

  /// current event number
  Int_t m_startEvt;

  /// Zeros out all member vars, does NOT free memory, for use in constructor
  void zeroMembers();

  ostream &m_ostrm;

};

#endif
