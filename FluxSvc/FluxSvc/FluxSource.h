/** @file FluxSource.h
    @brief FluxSource declaration
    */
#ifndef FluxSource_h
#define FluxSource_h 1

#include "FluxSvc/EventSource.h"
#include "CLHEP/Geometry/Point3D.h"
#include "CLHEP/Geometry/Vector3D.h"

// forward declarations
class DOM_Element;
class ISpectrum;

// 
/** @class FluxSource
    @brief class which manages to compute flux from various particle source configurations
    $Header$
*/
class FluxSource : public EventSource  
{
public:      
    /**  constructor
      @param xelem The xml description for this source
      */
    FluxSource ( const DOM_Element& xelem );
#if 0 // are these needed anywhere?
    FluxSource(double aFlux, ISpectrum* aSpec,  double l, double b);
    FluxSource ( ISpectrum* aSpec = 0, double aFlux = 0 );
#endif

    ///    destructor
    virtual ~FluxSource();

    ///    generate an event 
    virtual FluxSource* event(double time);

    ///    full-length title description of this EventSource.
    virtual std::string fullTitle () const;

    ///    brief title description (for display) for this event source
    virtual std::string displayTitle () const;

    virtual double flux(double time)const; // calculate flux for attached spectrum

    virtual double rate(double time)const; // calculate rate for attached spectrum

    /// return a title describing the spectrum and angles
    std::string title()const;

    /// print facility
    void  printOn ( std::ostream&  ) {}

    /// set spectrum, with optional parameter to set the maximum energy?
    void spectrum(ISpectrum* s, double emax=-1);

    ISpectrum* spectrum() const{ return m_spectrum; }

    double interval(double ){return m_interval;}

    //! Denotes what Energy Units the energy
    //! of incoming particles are in
    enum EnergyScale { 
        MeV,        //! MeV
        GeV         //! GeV
    } m_energyscale;

    virtual int eventNumber()const;

    double energy()const { return m_energy;}
    const HepVector3D& launchDir()const {return m_correctedDir;}
    const HepPoint3D&  launchPoint()const { return m_launchPoint;}

private:

    // forward declaration of classes that handle the lauch direction
    class LaunchDirection;  // base class
    class RandomDirection;  // choose randomly from range 
    class SourceDirection;  // choose from an external source class

    // forward declaration of classes that handle launch point
    class LaunchPoint;  // base class
    class RandomPoint; // random strategy
    class FixedPoint;  // fixed, or pencil
    class Patch;  // a box


    LaunchPoint* m_launch_pt; // pointer to actual point stategy: must be set
    LaunchDirection* m_launch_dir;

    ISpectrum*         m_spectrum;	    // spectrum to generate

    double m_energy;
    double m_interval;
    // associated with a specific launch

    /// result of strategy
    HepVector3D m_launchDir;

    ///direction after being corrected for the "tilt" angles.
    HepVector3D m_correctedDir;

    HepPoint3D  m_launchPoint;

    double calculateInterval (double time);

    ///interval function to be used by non-spectrum sources
    double explicitInterval (double time);
    ///    getLaunch - compute launch point, direction, & energy
    virtual void computeLaunch (double time=0);


};
#endif
