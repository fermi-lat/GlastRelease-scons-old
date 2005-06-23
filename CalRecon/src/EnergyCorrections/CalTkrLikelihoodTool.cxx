#include "CalLikelihoodTool.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "Event/TopLevel/EventModel.h"
#include "Event/Digi/TkrDigi.h"

/**   
* @class CalTkrLikelihoodTool
*
* Algorithm for correction of energy degradation in the tracker by correlating
* the energy in the CAL with the number of hit TKR strips.  
*
*
*/

class CalTkrLikelihoodTool : public CalLikelihoodTool 
{
public:
    //! constructor
    CalTkrLikelihoodTool(const std::string& type, 
                         const std::string& name, 
                         const IInterface* parent);
    virtual ~CalTkrLikelihoodTool(){}
    
    //! Tracker energy degradation correction using number of TKR hit strips
    /*! This method uses the correlation between the energy \"lost\" in the 
    * tracker and the energy deposited in the calorimeter.
    * We used the Monte Carlo simulation of the LAT to determine this correlation
    * at several energies, from 50 MeV up to 1 GeV, and angles from 0 to 32\deg. 
    * For one particular incident energy and angle, the bidimensionnal
    * distribution of  the  number of hit strips and the energy deposited in the 
    * CAL can be characterised by the 1D distribution:
    * \f[ E_{CAL} + \alpha TkrHits \f]
    * where  \f$\alpha\f$ is been optimised so as to obtain the narrowest such
    * distribution, normalised to a probability and with its MPV at the incident
    * energy.
    * These distributions can be used to defined a probability density function.
    * The reconstructed energy for a given event then becomes the one maximising
    * the probability, for a reconstruced direction, CAL raw energy,...
    *
    * \par The method takes 4 arguments:
    * \param eTotal Total energy measured in the calorimeter in MeV
    * \param nHits  Total number of hit strips in the CAL
    * \param vertex[3] reconstructed vertex position  
    * \param dir[3] reconstructed direction
    *
    *\return Corrected energy in MeV
    *
    *\warning needs TKR reconstruction
    *
    *\author
    */
           
    Event::CalCorToolResult* doEnergyCorr(Event::CalCluster*, Event::TkrVertex* );
};

#include <GaudiKernel/DeclareFactoryEntries.h>
DECLARE_TOOL_FACTORY(CalTkrLikelihoodTool) ;

CalTkrLikelihoodTool::CalTkrLikelihoodTool( const std::string& type, 
                                            const std::string& name, 
                                            const IInterface* parent)
                                          : CalLikelihoodTool(type,name,parent)
{
    // declare base interface for all consecutive concrete classes
    declareInterface<ICalEnergyCorr>(this);
    declareProperty("dataFile",
                    m_dataFile="$(CALRECONROOT)/xml/CalTkrLikelihood.data");
};

Event::CalCorToolResult* CalTkrLikelihoodTool::doEnergyCorr(Event::CalCluster* cluster, Event::TkrVertex* vertex)
//Purpose and method:
//
//   This function performs:
//   The main actions are:
//      - check wheter the event meets basic requirements (CUTS)
//      - calculate energy by TKR correction method using LikelihoodTool 
// 
// TDS input: CalCluster
// TDS output: CalClusters
{
    MsgStream log(msgSvc(), "CalTkrLikelihoodTool::doEnergyCorr");
    Event::CalCorToolResult* corResult = 0;

    // if reconstructed tracker data doesn't exist or number of tracks is 0:
    if (vertex == 0)
    {
        log << MSG::DEBUG << "Ending doEnergyCorr: No TKR Reconstruction" 
            << endreq;
        return corResult;
    }

    const Vector& trackDirection = vertex->getDirection();
    const Point&  trackPosition  = vertex->getPosition();
    // CUTS
    // this checks whether a set of PDF parameters exist for this event's
    // energy, direction and point of impact.
  
    // Energy:
    if( cluster->getCalParams().getEnergy() < 0. ) 
    {
      log << MSG::DEBUG << "Ending doEnergyCorr: "
                           "CAL Energy below Method Minimum"
          << endreq;
        return corResult;
    }

    if( cluster->getCalParams().getEnergy() > 2900. ) 
    {
       log << MSG::DEBUG << "Ending doEnergyCorr: "
                            "CAL Energy above Method Maximum"
           << endreq;
        return corResult;
    }
  
    // direction: slope must be above \f$cos(32\circ)$\f
    if( fabs(trackDirection.z()) < .8480481 )
    { 
        log << MSG::DEBUG << "Ending doEnergyCorr: Slope is too Small."
            << endreq;
        return corResult; 
    }


    int vertexPos = findTkrVertex(trackPosition);
    if( vertexPos < 0 || vertexPos > 15 ) 
    { 
        log << MSG::DEBUG << "Ending doEnergyCorr: "
                             "Vertex is out of Method Reach." 
        << endreq;
        return corResult; 
    }

    double geometricCut= findGeometricCut(trackPosition, trackDirection, 
                                          cluster);
    if( geometricCut<.15 )
    {
        return corResult;
    }
    // CALCULATE
    //    - get number of hits in TKR
    int nHits= 0;
    if( hasCorrelation() )
    {
      Event::TkrDigiCol *tkrDigiData =
                  SmartDataPtr<Event::TkrDigiCol>(m_dataSvc, 
                                                  EventModel::Digi::TkrDigiCol);

      for ( Event::TkrDigiCol::iterator it= tkrDigiData->begin(); 
            it!=tkrDigiData->end(); ++it )
        nHits+= (*it)->getNumHits();
    }


    setEventPDFdata(vertexPos+((geometricCut>.35)+(geometricCut>.6))*16);
    setEventPDFparameters(fabs(trackDirection.z()),
                          cluster->getCalParams().getEnergy(),
                          nHits);
    log << MSG::VERBOSE 
        << "PDF Index: " << vertexPos+((geometricCut>.35)+(geometricCut>.6))*16 
        << endreq
        << "Parameters: " << fabs(trackDirection.z()) << ", " 
        << cluster->getCalParams().getEnergy() << ", " << nHits << endreq;
    
    corResult= calculateEvent( 50., 3000., cluster, log );

    log << MSG::DEBUG << "Ending doEnergyCorr: Reconstruction Done" << endreq;
    return corResult;
}

