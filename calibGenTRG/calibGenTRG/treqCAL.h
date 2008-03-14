/** 
*@class treqCAL
*
*@brief Root Tree Ananlysis treqCAL as a root tree analysis
*
* This class is for Trigger Root Tree Ananlysis of the time structure of a run
*
* Version 0.1 24-August-2006 Martin Kocian Creation
*/

#ifndef TREQCAL_H
#define TREQCAL_H 

#if !defined(__CINT__)
// Need these includes if we wish to compile this code
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TMath.h>
#include <TStyle.h>
#include <TH2.h>
#include <TProfile.h>
#include "calibGenTRG/RootTreeAnalysis.h"
#else  // for interactive use
#include <TH1D.h>
class RootTreeAnalysis;
#endif 

#include  <iostream> 
#include  <iomanip> 
#include  <fstream> 

class TestReport;

class treqCAL : public RootTreeAnalysis {

public :

  treqCAL(); 

  virtual ~treqCAL() {;}

  void inithistos();
  void setParameters(int tkrdelay,int caldelay);
  void writeoutresults(const char* reportname, const char* filename);
  void fit(TH1D* histo,double& mean, double &emean, double& sigma, double& esigma, int& nevents);
  void getMean(TH1D* histo,double& mean, double &emean, double& sigma, double& esigma, int& nevents);
  void Go(Long64_t numEvents);
  void markError(char*, TestReport*);
  
 private: 
  char m_filename[256]; 
  int m_gid;
  double m_starttime;
  int m_nev;
  int m_status;
  int m_tkrdelay,m_caldelay;
  double m_clmean,m_clsigma;
  double m_chmean,m_chsigma;
  TH1D *m_clallevents, *m_challevents, *m_clintersect, *m_chintersect, *m_clsingletower, *m_chsingletower;
  TH1D *m_clbytower[16];
  TH1D *m_chbytower[16];
  TH2D *m_clvsenergy, *m_chvsenergy;
  
  string m_testnr; 
 


  ClassDef(treqCAL,1) 
    
};

#endif
