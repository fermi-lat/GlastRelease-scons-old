//      $Header$
//
// Description:
//      TkrQueryClusters is a container for Tkr clusters, and has the methods
//      for making the clusters from hits, and for accessing the clusters for various kinds of information.
//
// Author(s):
//      Tracy Usher     
//      Leon Rochester     



#include "TkrRecon/Cluster/TkrQueryClusters.h"

//------------  Operations ---------------------------

Point TkrQueryClusters::meanHit(TkrCluster::view v, int iplane)
{
    // Purpose and Method: Returns the mean position of all clusters in a layer
    // Inputs:  view and plane number
    // Outputs:  mean position of all the clusters in the layer
    // Dependencies: None
    // Restrictions and Caveats:  None
	
	Point Pini(0.,0.,0);
	
	int nhits = m_pClus->nHits(v,iplane);
	if (nhits == 0) return Pini;
	
	std::vector<TkrCluster*> AuxList = m_pClus->getHits(v,iplane);
	for (int ihit=0; ihit<nhits; ihit++){
		Pini += AuxList[ihit]->position();	
	}
	Point Pini2(Pini.x()/nhits,Pini.y()/nhits,Pini.z()/nhits);
	return Pini2;
}

Point TkrQueryClusters::meanHitInside(TkrCluster::view v, int iplane, double inRadius,
								 Point Pcenter)
{
    // Purpose and Method: Returns mean position of hits
    //    within a distance of a point in the measured dimension,
    //    and no more than one tower away
    // Inputs:  view and plane number, radius and center
    // Outputs:  mean position of clusters satisfying criterion
    // Dependencies: None
    // Restrictions and Caveats:  None
	
	Point P(0.,0.,0);
	std::vector<TkrCluster*> AuxList = m_pClus->getHits(v,iplane);
	int nhits = AuxList.size();
	if (nhits == 0) return P;
	
	double nsum = 0.;
	double xsum = 0.;
	double ysum = 0.;
	double zsum = 0.;
	
	for (int ihit=0; ihit<nhits; ihit++)
    {
		P = AuxList[ihit]->position();
		
        double hitRadius = fabs(P.x() - Pcenter.x());
        double twrRadius = fabs(P.y() - Pcenter.y());
		
		if      (v == TkrCluster::Y) 
        {
            hitRadius = fabs(P.y() - Pcenter.y());
            twrRadius = fabs(P.x() - Pcenter.x());
        }
        else if (v != TkrCluster::X) 
        {
            hitRadius = (P-Pcenter).mag();
            twrRadius = 0.;
        }
		
        // Check that hit is close and within one tower
        if (hitRadius < inRadius && twrRadius < 1.1 * s_towerPitch) 
        {
			nsum += 1.;
			xsum += P.x();
			ysum += P.y();
			zsum += P.z();
		}
	}
	
    if (nsum > 0.) P = Point(xsum/nsum, ysum/nsum, zsum/nsum);
	
    return P;
}

Point TkrQueryClusters::nearestHitOutside(TkrCluster::view v, int iplane, 
									 double inRadius, Point Pcenter, int& id)
{
    // Purpose and Method: returns the position of the closest cluster
    //    outside of a given distance from a point in the measured direction,
    //    and in the same or adjacent tower in the other direction.
    // Inputs:  view and plane, center and distance
    // Outputs:  Position of nearest cluster
    // Dependencies: None
    // Restrictions and Caveats:  None
	
	Point Pnear(0.,0.,0.);
	id = -1;
	
	int nhits = m_pClus->nHits(v,iplane);
	if (nhits == 0) return Pnear;
	
	std::vector<TkrCluster*> AuxList;
	AuxList = m_pClus->getHits(v,iplane);
	
	double minRadius = inRadius;
	double maxRadius = 1e6;
	Point Pini(0.,0.,0.);
	for (int ihit = 0; ihit< nhits; ihit++) 
    {
        if (AuxList[ihit]->hitFlagged()) continue;
		
		Pini = AuxList[ihit]->position();
		
		
        double hitRadius = fabs(Pini.x() - Pcenter.x());
        double twrRadius = fabs(Pini.y() - Pcenter.y());
		
		if      (v == TkrCluster::Y) 
        {
            hitRadius = fabs(Pini.y() - Pcenter.y());
            twrRadius = fabs(Pini.x() - Pcenter.x());
        }
        else if (v != TkrCluster::X) 
        {
            hitRadius = (Pini-Pcenter).mag();
            twrRadius = 0.;
        }
        
        if ( hitRadius >= minRadius && hitRadius < maxRadius && twrRadius < 1.1*s_towerPitch) 
        {
			maxRadius = hitRadius;
			Pnear     = Pini;
			id        = AuxList[ihit]->id();
		}
	}
	return Pnear;
}

int TkrQueryClusters::numberOfHitsNear( int iPlane, double inRadius, Point& x0)
{
	// Purpose and Method: counts the number of hits in a bilayer 
	//       within a square of side 2*inRadius
	// Inputs:  plane number, distance, central point
	// Outputs:  the number of hits that satisfy the criteria
	// Dependencies: None
	// Restrictions and Caveats:  None
	
    return numberOfHitsNear(iPlane, inRadius, inRadius, x0);
}

int TkrQueryClusters::numberOfHitsNear( int iPlane, double dX, double dY, Point& x0)
{
	// Purpose and Method: counts the number of hits in a bilayer 
	//      within a rectangle of sides 2*dX, 2*dY
	// Inputs:  plane number, dx, dy, central point
	// Outputs:  the number of hits that satisfy the criteria
	// Dependencies: None
	// Restrictions and Caveats:  None
	
    int numHits = 0;
	
    //Look for hits in the X view of desired layer
    std::vector<TkrCluster*> clusterList = m_pClus->getHits(TkrCluster::X, iPlane);
    int nHitsInPlane = clusterList.size();
	
    while(nHitsInPlane--)
    {
        double hitDiffX = x0.x() - clusterList[nHitsInPlane]->position().x();
        double hitDiffY = x0.y() - clusterList[nHitsInPlane]->position().y();
		
        if (fabs(hitDiffX < dX) && fabs(hitDiffY) < s_towerPitch) numHits++;
    }
	
    // Look for hits in the Y view of desired layer
    clusterList = m_pClus->getHits(TkrCluster::Y, iPlane);
    nHitsInPlane = m_pClus->nHits();
	
    while(nHitsInPlane--)
    {
        double hitDiffX = x0.x() - clusterList[nHitsInPlane]->position().x();
        double hitDiffY = x0.y() - clusterList[nHitsInPlane]->position().y();
		
        if (fabs(hitDiffX) < s_towerPitch && fabs(hitDiffY) < dY) numHits++;
    }
	
    return numHits;
}

int TkrQueryClusters::numberOfHitsNear( TkrCluster::view v, int iPlane, double inRadius, Point& x0)
{
    // Purpose and Method: counts the number of hits within a distance "inRadius" in the 
    //     measurement direction, and within one tower in the other direction
    // Inputs:  plane number, dx, dy, central point
    // Outputs:  the number of hits that satisfy the criteria
    // Dependencies: None
    // Restrictions and Caveats:  None
	
    int numHits = 0;
	
    // Look for hits in the desired view of the given layer
    std::vector<TkrCluster*> clusterList = m_pClus->getHits(v, iPlane);
    int nHitsInPlane = clusterList.size();
	
    while(nHitsInPlane--)
    {
        double hitDiffV = v == TkrCluster::X 
			? x0.x() - clusterList[nHitsInPlane]->position().x()
			: x0.y() - clusterList[nHitsInPlane]->position().y();
        double hitDiffO = v == TkrCluster::X 
			? x0.y() - clusterList[nHitsInPlane]->position().y()
			: x0.x() - clusterList[nHitsInPlane]->position().x();
		
        if (fabs(hitDiffV) < inRadius && fabs(hitDiffO) < s_towerPitch) numHits++;
    }
	
    return numHits;
}

double TkrQueryClusters::s_towerPitch;


