#ifndef TKRMAKECLUSTERTABLE_H
#define TKRMAKECLUSTERTABLE_H 

/** 
* @class TkrMakeClusterTable
*
* @brief generates the clusters when invoked
*
* @author Leon Rochester
*
* $Header$
*/

#include "Event/Recon/TkrRecon/TkrCluster.h"
#include "Event/Recon/TkrRecon/TkrCluster.h"
#include "Event/Digi/TkrDigi.h"
#include "Event/TopLevel/EventModel.h"
#include "Event/TopLevel/Event.h"
#include "Event/RelTable/RelTable.h"
#include "Event/RelTable/Relation.h"
#include "Event/MonteCarlo/McPositionHit.h"
#include "TkrUtil/ITkrGeometrySvc.h"

class TkrMakeClusterTable
{
    typedef Event::Relation<Event::TkrDigi,Event::McPositionHit> relDigiType;
    typedef Event::Relation<Event::TkrCluster,Event::McPositionHit> relCluType;

public:
    /// default constructor: passes pointers to services and classes, 
    /// and makes the table
    
    
    TkrMakeClusterTable(const Event::TkrClusterCol* pClus,
        const Event::TkrDigiCol* pDigi, 
        ObjectList<relDigiType>* pRelTab,
        Event::RelTable<Event::TkrCluster, Event::McPositionHit>* pClRelTab,
        ITkrGeometrySvc* tkrGeom);
    
    ~TkrMakeClusterTable() { }

    int digiOrder(const Event::TkrCluster* pClust);

private:
    ITkrGeometrySvc* m_tkrGeom;
    
};

#endif // TkrMakeClusterTable
