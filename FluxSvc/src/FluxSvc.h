/** 
* @file FluxSvc.h
* @brief definition of the class FluxSvc
*
*  $Header$
*/

#ifndef _H_FluxSvc_
#define _H_FluxSvc_
/** 
* \class FluxSvc
*
* \brief Service that implements the IFluxSvc interface, to return an IFlux object.
*  FluxSvc handles the creation and interfacing with Flux objects.  
* \author Toby Burnett tburnett@u.washington.edu
* 
* $Header$
*/

// includes
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IRunable.h"
#include "GaudiKernel/Property.h"

#include "FluxSvc/IFluxSvc.h"
#include <list>

//forward declarations
template <class TYPE> class SvcFactory;
class IFlux;  // interface
class FluxMgr;  // actual manager
class IParticlePropertySvc; 
class IAppMgrUI;


class FluxSvc : 
    virtual public Service, 
    virtual public IFluxSvc,
    virtual public IRunable
{  
public:
    
    /// return pointer to a flux object
    StatusCode source(std::string name, IFlux*&);
    
    /// return a list of possible names
    std::list<std::string> fluxNames()const;
    
    /// add a new SpectrumFactory
    virtual void addFactory(std::string name, const ISpectrumFactory* factory );
        
    /// pass a specific amount of time
    virtual void pass ( double t);
    
    /// create a set of display windows using rootplot.
    void rootDisplay(std::vector<const char*> arguments);
    
    ///return the pointer to the current IFlux object
    IFlux* currentFlux();
    
    /// name of the flux
    std::string fluxName()const;
    
    /// set the glast tilt angles.
    void setOrientation(std::pair<double,double> ang);

    /// get the angular values of the satellite
    std::pair<double,double> getOrientation();
    
    ///this transforms glast-local (cartesian) vectors into galactic (cartesian) vectors
    HepRotation transformGlastToGalactic(double time)const;
    
    /// get the current satellite location
    std::pair<double,double> location();

    /// return a string which uniquely identifies the source
    std::string uniqueIDString()const;

    ///0=NONE, 1=UPDOWN, 2=SLEWING, 3=ONEPERORBIT
    void setRockType(int rockType);

    ///this should return the source file names, along with the contained sources.
    std::vector<std::pair< std::string ,std::list<std::string> > > sourceOriginList() const;
   

    /// for the IRunnable interfce
    virtual StatusCode run();

    //------------------------------------------------------------------
    //  stuff required by a Service
    
    /// perform initializations for this service. 
    virtual StatusCode initialize ();
    
    /// perform the finalization, as required for a service.
    virtual StatusCode finalize ();
    
    /// Query interface
    virtual StatusCode queryInterface( const IID& riid, void** ppvUnknown );
    
protected: 
    
    /// Standard Constructor
    FluxSvc ( const std::string& name, ISvcLocator* al );
    
    /// destructor
    virtual ~FluxSvc ();
    
private:
    
    IParticlePropertySvc* m_partSvc;
    
    /// Allow SvcFactory to instantiate the service.
    friend class SvcFactory<FluxSvc>;
    
    FluxMgr * m_fluxMgr;
    /// the user-defined list of acceptable XML sources (from JobOptions.txt)
    //std::string m_source_lib;
    std::vector<std::string> m_source_lib;
    /// the default XML file name (from JobOptions.txt)
    std::string m_source_lib_default;
    /// set dtd to use.
    std::string m_dtd_file;
    /// the "current" flux object
    IFlux* m_currentFlux;

    /// Reference to application manager UI
    IAppMgrUI*    m_appMgrUI;
    IntegerProperty m_evtMax;

    // starting and ending times for orbital simulation
    DoubleProperty m_startTime;
    DoubleProperty m_endTime;
};


#endif // _H_FluxSvc