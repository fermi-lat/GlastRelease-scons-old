/** @file TkrVecPointsBuilder.h
 * @class TkrVecPointsBuilder
 *
 * @brief Provides X-Y space points from the same tower from TkrClusterCol
 *
 * @author Tracy Usher
 *
 * $Header$
 *
*/

#ifndef __TkrVecPointsBuilder_H
#define __TkrVecPointsBuilder_H 1

#include "Event/Recon/TkrRecon/TkrVecPoint.h"
#include "TkrUtil/ITkrGeometrySvc.h"
#include "TkrUtil/ITkrQueryClustersTool.h"
#include "GaudiKernel/IDataProviderSvc.h"

#include <vector>
typedef std::vector<Event::TkrVecPoint*> TkrVecPointVec;
typedef std::vector<TkrVecPointVec>      TkrVecPointVecVec;

class TkrVecPointsBuilder 
{
public:
    TkrVecPointsBuilder(bool                   mergeClusters,
                        int                    nClusToMerge,
                        int                    stripGap,
                        IDataProviderSvc*      dataSvc, 
                        ITkrGeometrySvc*       geoSvc,
                        ITkrQueryClustersTool* clusTool);

    ~TkrVecPointsBuilder();

    // provide access to the TkrVecPoints.
    const TkrVecPointVecVec& getVecPoints()              const {return m_tkrVecPointVecVec;}

    // Return number of clusters encountered
    const int                getNumClusters()            const {return m_numClusters;}

    // Return number of TkrVecPoints 
    const int                getNumTkrVecPoints()        const {return m_numVecPoints;}

    // Return number of TkrVecPoints 
    const int                getNumBiLayers()            const {return m_numBiLayersWVecPoints;}

    // Return maximum number of link combinations
    const double             getMaxNumLinkCombinations() const {return m_maxNumLinkCombinations;}
private:

    /// Merge clusters
    Event::TkrClusterVec mergeClusters(Event::TkrClusterVec& clusVec);

    /// Pointer to the merged collection of TkrClusters
    Event::TkrClusterCol* m_mergedClusters;

    /// This will keep track of all the VecPoints we will be using
    /// This is a vector of vectors, so the VecPoints are arranged 
    /// from the beginning of the possible track to the end
    TkrVecPointVecVec     m_tkrVecPointVecVec;

    /// Control parameters for merging clusters
    bool                  m_mergeClusters;
    int                   m_nClusToMerge;
    int                   m_stripGap;

    /// Keep track of the number of clusters encountered
    int               m_numClusters;

    /// Also keep track of the total number of TkrVecPoints
    int               m_numVecPoints;

    /// Keep count of the number of bilayers with TkrVecPoints
    int               m_numBiLayersWVecPoints;

    /// Finally, keep track of max possible link combinations
    double            m_maxNumLinkCombinations;

    /// Pointer to geometry service
    ITkrGeometrySvc*  m_geoSvc;
};

#endif
