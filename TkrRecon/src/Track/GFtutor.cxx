
#include "TkrRecon/Track/GFtutor.h"

//-------- GFtutor --------------------------------------

TkrClusters* GFtutor::_DATA = 0;// Sets by TrackerRecon - made accesible to all GF-objects

bool GFtutor::CUT_veto = false;
bool GFtutor::CONTROL_connectGFpair = false;

ITkrGeometrySvc* GFtutor::pTrackerGeo = 0;

int     GFtutor::m_numPlanes = 0;
double	GFtutor::m_trayWidth = 0;
double	GFtutor::m_trayGap   = 0;

double	GFtutor::m_siStripPitch = 0;
double	GFtutor::m_siThickness  = 0;
double	GFtutor::m_siResolution = 0;

//----------------- Static function ----------------------
//########################################################
void GFtutor::load(TkrClusters* scl, ITkrGeometrySvc* pTrkGeo)			 
//########################################################
{

	GFtutor::pTrackerGeo = pTrkGeo;
	
	GFtutor::_DATA = scl;

	GFtutor::CUT_veto = true;
	GFtutor::CONTROL_connectGFpair = true;
	
	GFtutor::m_numPlanes = pTrackerGeo->numPlanes();
	
	GFtutor::m_trayWidth = pTrackerGeo->trayWidth();
	GFtutor::m_trayGap   = pTrackerGeo->trayHeight();

	GFtutor::m_siStripPitch = pTrackerGeo->siStripPitch();
	GFtutor::m_siThickness  = pTrackerGeo->siThickness();
	GFtutor::m_siResolution = pTrackerGeo->siResolution();

}
//------------- protected --------------------------------
int GFtutor::okClusterSize(TkrCluster::view axis, int indexhit, 
						   double slope)			 
//########################################################
{
    int icluster = 0;
    
    int size = (int) GFtutor::_DATA->size(axis,indexhit);
    
    double distance = GFtutor::siThickness()*fabs(slope)/
		GFtutor::siStripPitch();
    distance = distance - 1.;
    int idistance = (int) distance;
    if (idistance < 1) idistance = 1;
    
    if (size < idistance) icluster = 0;
    else if (size == idistance) icluster = 1;
    else if (size > idistance) icluster = 2;
    
    if (icluster == 0 && size >=2 && idistance >=2) icluster = 1;
    
    return icluster;
}

//#########################################################################
bool GFtutor::neighbourTowers(int itower, int jtower)
//#########################################################################
{
    bool ok = false;
    
    int kxtower = (int) itower/10;
    int kytower = itower - 10*itower;
    
    int kx0tower = (int) jtower/10;
    int ky0tower = jtower - 10*kx0tower;
    
    if (kxtower >= kx0tower-1 && kxtower <= kx0tower+1) ok = true;	
    if (kytower >= ky0tower-1 && kytower <= ky0tower+1) ok = true; 
    
    // problem detected on the ID hit - not used for the momemt - 05/16/99 JAH
    if (itower < 10 || jtower < 10) ok =true;
    
    return ok;
}
