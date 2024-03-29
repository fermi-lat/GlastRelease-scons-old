// File and Version information:
// $Header$
//
//  Implementation file of CalCluster and CalClusterCol classes
//  
// Authors:
//
//    Luca Baldini
//    Johan Bregeon
//

#include "Event/Recon/CalRecon/CalCluster.h"

void Event::CalCluster::initialize(const CalXtalsParams& xtalsParams,
                                   const CalMSTreeParams& mstParams,
                                   const CalFitParams& fitParams,
                                   const CalMomParams& momParams,
                                   const CalClassParams& classParams)
{
  m_xtalsParams = xtalsParams;
  m_mstParams   = mstParams;
  m_fitParams   = fitParams;
  m_momParams   = momParams;
  m_classParams = classParams;
}

void Event::CalCluster::iniCluster()
{
  m_producerName = "Not set";
  m_statusBits   = 0;
  m_xtalsParams  = CalXtalsParams(); 
  m_mstParams    = CalMSTreeParams();
  m_fitParams    = CalFitParams();
  m_momParams    = CalMomParams();
  m_classParams  = CalClassParams();
}

void Event::CalCluster::writeOut(MsgStream& stream) const
{
  std::stringstream s;
  fillStream(s);
  stream << s.str();
}

std::ostream& Event::CalCluster::fillStream(std::ostream& s) const
{
  s <<
    "Producer name: " << m_producerName << "\n" <<
    "Status bits: " << m_statusBits << "\n" <<
    "----------------------------------------------------\n" <<
    "-------- Generic xtal collection properties --------\n" << 
    "----------------------------------------------------\n" << m_xtalsParams << "\n" <<
    "----------------------------------------------------\n" <<
    "---------- Output from the MST clustering ----------\n" << 
    "----------------------------------------------------\n" << m_mstParams << "\n" <<
    "----------------------------------------------------\n" <<
    "----------- Output from the fitting tool -----------\n" << 
    "----------------------------------------------------\n" << m_fitParams << "\n" <<
    "----------------------------------------------------\n" <<
    "--------- Output from the moments analysis ---------\n" << 
    "----------------------------------------------------\n" << m_momParams << "\n" <<
    "----------------------------------------------------\n" <<
    "---------- Output from the classification ----------\n" << 
    "----------------------------------------------------\n" << m_classParams <<
    "------------------------------------------------------";
  return s; 
}
