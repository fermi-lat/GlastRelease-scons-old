
#ifndef __TKRDISPLAYALG_H
#define __TKRDISPLAYALG_H 1

#include "GaudiKernel/Algorithm.h"
#include "TkrRecon/Cluster/TkrClusters.h"
#include "TkrRecon/Track/SiRecObjs.h"


//----------------------------------------------
//
//   TkrDisplayAlg
//
//	 Tracker Geometry Service. Used to keep track of the 
//   particular tracker geometry in use
//----------------------------------------------
//             Tracy Usher, SLAC, 2/28/01
//----------------------------------------------
//##########################################################
class TkrDisplayAlg : public Algorithm
//##########################################################
{
public:
	//! Constructor of this form must be provided
	TkrDisplayAlg(const std::string& name, ISvcLocator* pSvcLocator); 
	virtual ~TkrDisplayAlg() {}
	//! mandatory
	StatusCode initialize();
	//! mandatory
	StatusCode execute();
	//! mandatory
	StatusCode finalize();

protected:

	StatusCode retrieve();

private:

	TkrClusters* m_TkrClusters;
	SiRecObjs*   m_SiRecObjs;
};
      
#endif
