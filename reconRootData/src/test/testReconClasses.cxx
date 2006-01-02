
#include <reconRootData/ReconEvent.h>
#include <commonRootData/RootDataUtil.h>
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom.h"
#include "TCollection.h"
#include "TVector3.h"
#include <iostream>

/** @file testReconClasses.cxx
* @brief This defines a test routine for the Reconstruction ROOT classes.
*
* This program creates a new Recon ROOT file, and the opens it up again
* for reading.  The contents are checked against the values known to be stored
* during the original writing of the file.
* The contents of the file are printed to the screen.
* The program returns 0 if the test passed.
* If failure, the program returns -1.
*
* $Header$
*/
const UInt_t RUN_NUM = 1;
Float_t RAND_NUM;

int checkReconEvent(ReconEvent *evt, UInt_t ievent) {
    
    if (evt->getRunId() != RUN_NUM) {
        std::cout << "Run Id is wrong: " << evt->getRunId() << std::endl;
        return -1;
    }
    if (evt->getEventId() != ievent) {
        std::cout << "Event Id is wrong: " << evt->getEventId() << std::endl;
        return -1;
    }

    return 0;
    
}

/// Read in the ROOT file just generated via the write method
int read(char* fileName, int numEvents) {
    TFile *f = new TFile(fileName, "READ");
    TTree *t = (TTree*)f->Get("Recon");
    ReconEvent *evt = 0;
    t->SetBranchAddress("ReconEvent", &evt);

    std::cout << "Opened the ROOT file for reading" << std::endl;

    UInt_t ievent;
    for (ievent = 0; ievent < (UInt_t) numEvents; ievent++) {
        
        t->GetEvent(ievent);
        std::cout << "ReconEvent ievent = " << ievent << std::endl;
        evt->Print() ;
        
        if (checkReconEvent(evt, ievent) < 0)
         { return -1 ; }
        
        // prepare data
        AcdRecon * acd = evt->getAcdRecon() ;
        CalRecon * cal = evt->getCalRecon() ;
        TkrRecon * tkr = evt->getTkrRecon() ;
        if ((!acd)||(!cal)||(!tkr))
         { return -1 ; }
        acd->Print() ;
        cal->Print();
        tkr->Print();
        AcdRecon acdRef ;
        CalRecon calRef ;
        TkrRecon tkrRef ;
         
        // usual tests
        acdRef.Fake(ievent,RAND_NUM) ;
        if (!acd->CompareInRange(acdRef))
         { return -1 ; }
        calRef.Fake(ievent,RAND_NUM) ;
        if (!cal->CompareInRange(calRef))
         { return -1 ; }
        tkrRef.Fake(ievent,RAND_NUM) ;
        if (!tkr->CompareInRange(tkrRef))
         { return -1 ; }
         
        // opposite tests
        std::cout<<"===== on purpose errors ====="<<std::endl ;
        acdRef.Clear() ;
        acdRef.Fake(ievent,RAND_NUM*2.) ;
        if (acd->CompareInRange(acdRef))
         { return -1 ; }
        calRef.Clear() ;
        calRef.Fake(ievent,RAND_NUM*2.) ;
        if (cal->CompareInRange(calRef))
         { return -1 ; }
        tkrRef.Clear() ;
        tkrRef.Fake(ievent,RAND_NUM*2.) ;
        if (tkr->CompareInRange(tkrRef))
         { return -1 ; }
        std::cout<<"============================="<<std::endl ;
        
        evt->Clear();
    }

    f->Close();
    delete f;

    return 0;
}

/// Create a new ROOT file
int write(char* fileName, int numEvents) {
    
    Int_t buffer = 64000 ;
    Int_t splitLevel = 1 ;

    TFile * f =  new TFile(fileName,"RECREATE") ;
    TTree * t = new TTree("Recon","Recon") ;
    ReconEvent * ev = new ReconEvent() ;
    t->Branch("ReconEvent", "ReconEvent", &ev, buffer, splitLevel);
    std::cout << "Created new ROOT file" << std::endl;

    TRandom randGen ;
    RAND_NUM = randGen.Rndm() ;
    
    Int_t ievent ;
    for (ievent = 0; ievent < numEvents; ievent++) {

        AcdRecon * acdRec = new AcdRecon() ;
        acdRec->Fake(ievent,RAND_NUM) ;
        
        CalRecon * calRec = new CalRecon() ;
        calRec->Fake(ievent,RAND_NUM) ;

        TkrRecon * tkrRec = new TkrRecon();
        tkrRec->Fake(ievent,RAND_NUM) ;        

        ev->initialize(ievent, RUN_NUM, tkrRec, calRec, acdRec);
        ev->initEventFlags(1) ;
        
        t->Fill();
        ev->Clear();
    }

    std::cout << "Filled ROOT file with " << numEvents << " events" << std::endl;
    delete ev;

    f->Write();
    f->Close();
    delete f;
    return(0);
}


/// Main program
/// Return 0 for success.
/// Returns -1 for failure.
int main(int argc, char **argv) {
    
    char *fileName = "recon.root";
    int n = 1;
    int numEvents = 10;
    if (argc > 1) {
        fileName = argv[n++];
    } 
    if (argc > 2) {
        numEvents = atoi(argv[n++]);
    } 

    int sc = 0 ;
    try 
     {
      sc = write(fileName, numEvents) ;
      sc = read(fileName, numEvents) ;
     }
    catch (...)
     {
      std::cout<<"AN UNKNOWN EXCEPTION HAS BEEN RAISED"<<std::endl ;
      sc = 1 ;
     }
     
     
    if (sc == 0) {
        std::cout << "RECON ROOT file writing and reading succeeded!" << std::endl;
    } else {
        std::cout << "FAILED recon writing and reading" << std::endl;
    }

    return(sc);
}
