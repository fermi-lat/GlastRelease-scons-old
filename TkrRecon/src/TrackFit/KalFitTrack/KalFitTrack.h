//----------------------------------------
//
//      Kalman Filter for Glast
//
//      Original due to Jose Hernando-Angle circa 1997-1999
//      Re-written to combine both X and Y projections (2001)
//
//      W. B. Atwood, UCSC, Nov., 2001  
//      
//----------------------------------------

#ifndef __KalFitTrack_H
#define __KalFitTrack_H 1

#include <vector>
#include "GaudiKernel/MsgStream.h"
#include "Event/Recon/TkrRecon/TkrFitTrack.h"
#include "Event/Recon/TkrRecon/TkrPatCandHit.h"
#include "Event/Recon/TkrRecon/TkrCluster.h"
#include "TkrRecon/ITkrGeometrySvc.h"

namespace Event {

class KalFitTrack: public TkrFitTrack
{    
public:
    KalFitTrack(TkrClusterCol* clusters, ITkrGeometrySvc* geo, int layer, int tower, double sigmaCut, double energy, const Ray& testRay);
   ~KalFitTrack() {}

    // Hit Finding & Fitting
    void          findHits();
    void          doFit();
    void          addMeasHit(const TkrPatCandHit& candHit);
    
    /// Access 
    inline Point  getPosAtZ(double deltaZ)   const{return m_x0+deltaZ*m_dir;} 
    inline Vector getDirection()             const{return m_dir;}
    inline double getStartEnergy()           const{return m_energy0;}
    inline int    numSegmentPoints()         const{return m_numSegmentPoints;}
    inline double chiSquareSegment(double penaltyGap = 0.)  
                                             const{return m_chisqSegment + penaltyGap*getNumGaps();}
    inline int    getNumXHits()              const{return m_nxHits;}
    inline int    getNumYHits()              const{return m_nyHits;}
    inline double getKalEnergyError()        const{return m_KalEnergyErr;}

    /// Access errors at track start
    double        getErrorXPosition()      const;
    double        getErrorXSlope()         const;
    double        getErrorYPosition()      const;
    double        getErrorYSlope()         const;

    /// Access to derived information on kinks
    double        getKink(int iplane)      const;
    double        getKinkNorma(int iplane) const;
        
    // Operations
    void          flagAllHits(int iflag=1);
    void          unFlagAllHits();
    void          unFlagHit(int num);
    int           compareFits(KalFitTrack& ktrack);

    enum          Status {EMPTY, FOUND, CRACK}; 
    void          setStatus(Status status) {m_status = status;}
    Status        status() const           {return m_status;}
    
private:	
    // Utilities
    void          ini();
    double        computeQuality() const;
    void          clear();
    TkrFitPar     guessParameters();
    TkrFitHit     generateFirstFitHit(TkrFitPar pars);
    void          finish();
    void          filterStep(int iplane);
    int           okClusterSize(TkrCluster::view axis, int indexhit, double slope);	
       
    // Finds the next hit layer using particle propagator
    TkrFitPlane   projectedKPlane(TkrFitPlane previous, int klayer, double& arc_min, TkrFitHit::TYPE type = TkrFitHit::FIT);

    // returns next TkrFitPlane - finds hit along the way 
    Status        nextKPlane(const TkrFitPlane& previous, int kplane, TkrFitPlane& next, TkrFitHit::TYPE typ = TkrFitHit::FIT); 
    
    // Selecting the Hit
    double        sigmaFoundHit(const TkrFitPlane& previous, const TkrFitPlane& next, int& indexhit, double& radius); // returns also indexhit and radius
    void          incorporateFoundHit(TkrFitPlane& next, int indexhit); // modifies next
    bool          foundHit(int& indexhit, double& min_Dist, double max_Dist, 
                           double error, const Point& CenterX, const Point& nearHit);
    
    // access to the Step Plane 
    TkrFitPlane   firstKPlane() const;
    TkrFitPlane   lastKPlane() const;
    TkrFitPlane   previousKPlane() const;
    TkrFitPlane   originalKPlane() const;

    // Energy Part
    void          eneDetermination();
    
    // segment Part: First protion that influences direction
    int           computeNumSegmentPoints(TkrFitHit::TYPE typ = TkrFitHit::SMOOTH);
    double        computeChiSqSegment(int nhits, TkrFitHit::TYPE typ = TkrFitHit::SMOOTH);
     
    // Input Data
    const Ray m_ray; 
   
    // These are copies of what is in TkrBase
    double m_energy0;
    Point  m_x0;
    Vector m_dir;
    
    // Status
    Status m_status;
    bool   m_alive;

    // axis information
    TkrCluster::view m_axis;

    //KalTrack data
    int    m_iLayer;
    int    m_iTower;
    int    m_numSegmentPoints;
    double m_chisqSegment;
    double m_sigma;
    int    m_nxHits;
    int    m_nyHits;
    double m_KalEnergyErr;

    //Pointers to clusters and geoemtry
    Event::TkrClusterCol* m_clusters;
    ITkrGeometrySvc*      m_tkrGeo;
};

};

#endif
