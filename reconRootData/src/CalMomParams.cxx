
// File and Version information:
// $Header: 
//
//  Implementation file of Root clone for CalMomParams
//  
// Authors:
//
//    Luca Baldini
//    Johan Bregeon
//

#include <reconRootData/CalMomParams.h>
#include <commonRootData/RootDataUtil.h>
#include "Riostream.h"

ClassImp(CalMomParams)

CalMomParams::CalMomParams
( double energy, double eneError,
  TVector3 centroid,
  double cntdxx, double cntdxy, double cntdxz, double cntdyy, double cntdyz, double cntdzz,
  TVector3 axis,
  double axsdxx, double axsdxy, double axsdxz, double axsdyy, double axsdyz, double axsdzz,
  Int_t numIterations, Int_t numCoreXtals, Int_t numXtals,
  Double_t transRms, Double_t longRms, Double_t longRmsAsym, Double_t longSkewness,
  Double_t coreEnergyFrac, Double_t fullLength, Double_t dEdxSpread )
{
    init
    ( energy,eneError,
      centroid,cntdxx,cntdxy,cntdxz,cntdyy,cntdyz,cntdzz,
      axis,axsdxx,axsdxy,axsdxz,axsdyy,axsdyz,axsdzz,
      numIterations,numCoreXtals,numXtals,
      transRms,longRms,longRmsAsym,longSkewness,coreEnergyFrac,fullLength,dEdxSpread ) ;
}

void CalMomParams::Clear( Option_t * )
{
    init
    ( 0.,0.,
      TVector3(0.,0.,0.),0.,0.,0.,0.,0.,0.,
      TVector3(0.,0.,0.),0.,0.,0.,0.,0.,0.,
      0,0,0,
      0.,0.,0.,0.,0.,0.,0.) ;
}

void CalMomParams::init
( Double_t energy, Double_t eneError,
  TVector3 centroid,
  Double_t cntdxx, Double_t cntdxy, Double_t cntdxz,
  Double_t cntdyy, Double_t cntdyz, Double_t cntdzz,
  TVector3 axis,
  Double_t axsdxx, Double_t axsdxy, Double_t axsdxz,
  Double_t axsdyy, Double_t axsdyz, Double_t axsdzz,
  Int_t numIterations, Int_t numCoreXtals, Int_t numXtals,
  Double_t transRms, Double_t longRms, Double_t longRmsAsym, Double_t longSkewness,
  Double_t coreEnergyFrac, Double_t fullLength, Double_t dEdxSpread )
{
    setEnergy(energy);
    setEnergyErr(eneError);
    setCentroid(centroid);
    setxPosxPos(cntdxx);
    setxPosyPos(cntdxy);
    setxPoszPos(cntdxz);
    setyPosyPos(cntdyy);
    setyPoszPos(cntdyz);
    setzPoszPos(cntdzz);
    setAxis(axis);
    setxDirxDir(axsdxx);
    setxDiryDir(axsdxy);
    setxDirzDir(axsdxz);
    setyDiryDir(axsdyy);
    setyDirzDir(axsdyz);
    setzDirzDir(axsdzz);
    m_numIterations = numIterations;
    m_numCoreXtals = numCoreXtals;
    m_numXtals = numXtals;
    m_transRms = transRms;
    m_longRms = longRms;
    m_longRmsAsym = longRmsAsym;
    m_longSkewness = longSkewness;
    m_coreEnergyFrac = coreEnergyFrac;
    m_fullLength = fullLength;
    m_dEdxSpread = dEdxSpread;
}

Double_t CalMomParams::getElongation() const
{
  if ( getTransRms() > 0.){
    return getLongRms()/getTransRms();
  }
  return -1.;
}

Double_t CalMomParams::getdEdxAverage() const
{
  if ( getFullLength() > 0. ) {
    return getEnergy()/getFullLength();
  }
  return -1.;
}

void CalMomParams::Print( Option_t * ) const
{
  // Call the base class method...
  CalParams::Print();
  // ... then print the additional stuff.
  std::cout << "\n" <<
    "Number of iterations = " << getNumIterations() << "\n" << 
    getNumCoreXtals() << "/" << getNumXtals() <<
    " xtal(s) used for the final centroid/axis\n" << 
    "Transverse RMS = " << getTransRms() << " mm\n" <<
    "Longitudinal RMS = " << getLongRms() << " mm\n" <<
    "Longitudinal RMS asymmetry = " << getLongRmsAsym() << "\n" <<
    "Longitudinal skewness = " << getLongSkewness() << "\n" <<
    "Elongation = " << getElongation() << "\n" <<
    "Core energy fraction = " << getCoreEnergyFrac() << "\n" <<
    "Cluster full length = " << getFullLength() << " X0\n" <<
    "Average dE/dx = " << getdEdxAverage() << " MeV/X0 (+- " << getdEdxSpread() << ")" <<
    std::endl;
}

// dummy data, just for tests
void CalMomParams::Fake( Int_t /* ievent */, UInt_t /* rank */, Float_t /* randNum */ )
 {
    init
    ( 1.,2.,
      TVector3(3.,4.,5.),6.,7.,8.,9.,10.,11.,
      TVector3(12.,13.,14.),15.,16.,17.,18.,19.,20.,
      21,21,21,
      22.,23.,24.,25.,26.,27.,28.) ;
 }

Bool_t CalMomParams::CompareInRange( const CalMomParams & cp, const std::string & name ) const {
    
    Bool_t result = true ;
    /// To be implemented, based on the base class.
    std::cout << "CalMomParams::CompareInRange() not implemented, yet." << std::endl;
    return result ;
        
}
