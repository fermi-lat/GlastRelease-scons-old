/** 
* @file SurfaceMuons.cxx
* @brief declaration and definition of SurfaceMuons
*
*  $Header$
*/
#include "Spectrum.h"
#include "SpectrumFactory.h"

#include "CLHEP/Random/RandFlat.h"
#include <string>
#include <utility>
#include <algorithm>
#include <map>

/** 
* \class SurfaceMuons
*
* \brief Spectrum representing cosmic ray muon flux at the Earth's surface
* \author T. Burnett
* 
* $Header$
*/
//

class SurfaceMuons : public Spectrum
{
public:
    /** @brief ctot
    @param paramstring string from xml. If present, assume is the range of costheta to generate
    */
    SurfaceMuons(const std::string& paramstring);

    /// flux integrated over energy, will be multipled by solidAngle to get a rate/m^2
    double flux (double time ) const { return m_flux/solidAngle();}

    double solidAngle()const{return 2*M_PI*fabs(m_cosmax-m_cosmin);}
    
    /** @brief sample a single particle energy from the spectrum
    @param current time (ignored)
    @return the energy in GeV, or MeV if the source was specified as such
    */
    double energy( double time);

    
    /** @brief return solid angle pair (costh, phi) for the given energy
    @param energy the energy previously generated
    */
    virtual std::pair<double,double> dir(double energy);
    
    
    virtual std::string title() const{return "SuraceMuons";}
    virtual const char * particleName() const;
    inline  const char * nameOf() const {return "SurfaceMuons";}
    
    
private:

  // option to choose which energy spectrum is implemented.
  // 0 (default): use the spectrum as a function of E*cos(theta)
  // 1: use an analytic form, see analyticSpectrum()
  int m_option;


  double analyticSpectrum(double time) const;

  // local function that approximates the spectrum as a function of E*cos(theta)
    double spectrum(double e);

    /**
    This is the integral spectrum as a map where the key is the integral spectrum and the
    value the associated energy. This makes is easy to invert it by using the STL algorithm lower_bound.
  */
    std::map<double,double> m_ispec; // integral spectrum, for sampling
    double m_flux;
    double m_index;
    double m_emax;
    double m_cosmin, m_cosmax;
    double m_costh;
    double m_total;
    
};


static SpectrumFactory<SurfaceMuons> factory;
const ISpectrumFactory& SurfaceMuonsFactory = factory;

SurfaceMuons::SurfaceMuons(const std::string& paramstring)
: m_index(2.71)
, m_emax(1000)
{
    //Purpose: Initializes parameters during construction
    
    std::vector<float> params;
    
    parseParamList(paramstring,params);

    // determine integral flux for specified costheta range

    m_cosmin = (params.size()>0)? params[0]: 0.0;
    m_cosmax = (params.size()>1)? params[1]: 1.0;

    m_option = (int) (params.size()>2 ? params[2]: 0.);

    if(m_option == 0) {
      m_flux = 70. * 2*M_PI * fabs(pow(m_cosmax,3) - pow(m_cosmin,3))/3;

      // create integral table of the flux function, as a map of
      // energy and e*flux(e), with logarithmic energies
      int n=100;
      double  emin=1;
        
      for( int i=0; i< n; ++i){
        double 
	  e = pow(10, 0.025*i),
	  f= e*spectrum(e);
        
        m_ispec[m_total +=f] =e;
      }
    }
}


const char* SurfaceMuons::particleName()const
{
/// purpose: return a point to the particle name, either mu+ or mu-
    static const char * pnames[] = {"mu+", "mu-"};
    static double 
        charge_ratio = 1.2,  // from many measurements.
        plus_fraction=1/(1+charge_ratio);
    return RandFlat::shoot()>plus_fraction? pnames[0]:pnames[1];
}
  
static inline double cube(double x){return x*x*x;}

/// sample a single particle energy from the spectrum: assume called first
double SurfaceMuons::energy( double time )
{
    using namespace std;
    // first choose the angle for the dir function, assuming cos**2 distribution
    static double third=1.0/3.0;
    double r = RandFlat::shoot();
    m_costh = pow(r*cube(m_cosmax)+(1-r)*cube(m_cosmin), third);

    if(m_option == 1) return analyticSpectrum(time);

    // select an energy*costh from the distribution

    double fraction = RandFlat::shoot();
     map<double,double>::const_iterator 
         element  = m_ispec.lower_bound(fraction*m_total),
         prev = element;
         prev--;
     if( element!=m_ispec.end() ){
         double energy = prev->second;
         // interpolate if not last one
         double 
             deltaI = fraction-prev->first/m_total,
             deltaE = element->second -energy,
             correct = deltaE*deltaI;
         return (energy+deltaI)/m_costh; 
    }
    return m_emax/m_costh;
}

  
std::pair<double,double> SurfaceMuons::dir(double)
{
    // purpose: return the pair; note uses the previous value for cos(theta)
    return std::make_pair(m_costh, RandFlat::shoot(2*M_PI));
}

static inline double sqr(double x){return x*x;}

double SurfaceMuons::spectrum(double ecth)
{
    // purpose: spectrum as a function of E*cos(theta)
    // returns value differential in E, according to stuff in particle properties

    double 
        atmos = 1/(1+1.1*ecth/115.) + 0.054/(1+1.1*ecth/850.),
        cutoff = ecth<30? exp(-sqr(::log10(ecth/30.))/0.55) : 1.0;


    return pow(ecth, -2.71)*atmos*cutoff;
}

double SurfaceMuons::analyticSpectrum(double time) const 
{
  // analytic form is based on following energy spectrum
  //  0.003 flat (0.2 GeV < E < 1. GeV)
  //  0.003 * E^(-1.1) (1 GeV < E < 4 GeV)
  //  0.006 * E^(-1.6) (4 GeV < E < 10 GeV)
  //  0.038 * E^(-2.6) (10 GeV < E < 200 GeV)
  // only generated events with energy from 0.2 GeV to 200 GeV

  // ratio used later in determining in which energy range the event is going
  // to be generated
  double norm = 0.0024 + 0.00388 + 0.00184 + 0.000592;
  double ratio[] = {0.0024/norm, 0.00388/norm, 0.00184/norm, 0.000592/norm};
  double energy;

  static HepRandom* randomEng = HepRandom::getTheGenerator();

  // determine the energy range of the event to be generated
  float range = randomEng->flat();

  // use transformation method to generate random numbers
  float rand = randomEng->flat();

  if(range <= ratio[0]) {  // 0.2 GeV < E < 1 GeV
    energy = 0.2 + 0.8 * rand;
  }
  else if(range <= ratio[0] + ratio[1]) {  // 1 GeV < E < 4 GeV
    static double normE0 = 1. - pow(4., -0.1);
    energy = pow(1. - normE0 * rand, -10.);
  }
  else if(range <= ratio[0] + ratio[1] + ratio[2]) {  // 4 GeV < E < 10 GeV
    static double normE1 = pow(4., -0.6) - pow(10., -0.6);
    static double temp = pow(4., -0.6);
    energy = pow(temp - normE1 * rand, -5./3.);
  }
  else { // 10 GeV < E < 200 GeV
    static double normE2 = pow(10., -1.6) - pow(200., -1.6);
    static double temp = pow(10., -1.6);
    energy = pow(temp - normE2 * rand, -1./1.6);
  }

  return energy;
}
