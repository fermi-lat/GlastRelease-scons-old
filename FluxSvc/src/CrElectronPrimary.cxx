/**
 * CrElectronPrimary:
 * The primary particle generator for primary cosmic-ray electrons.
 *
 * Ver 1.0 on 2001-04-18 by Masanobu Ozaki <ozaki@astro.isas.ac.jp>
 *    separate the primary component from the all-in-one code.
 * 2001-04-26 comments are added by T. Mizuno
 *
 * $Header$
 */

#include <math.h>

// Geant4
//#include "Randomize.hh"		// in source/global/HEPRandom/include/
//#include "SystemOfUnits.h"	// in source/global/management/include/
// CLHEP
#include <CLHEP/Random/RandomEngine.h>
#include <CLHEP/Random/RandGeneral.h>
#include <CLHEP/Random/JamesRandom.h>

#include "CrElectronPrimary.h"


typedef  double G4double;


// private function definitions.
namespace {
  const G4double pi    = 3.14159265358979323846264339;
  const G4double restE = 5.11e-4; // rest energy of electron in [GeV]
  // lower and higher energy limit of primary electron in units of GeV
  const G4double lowE_primary  = 1.0;
  const G4double highE_primary = 100.0;

  // The following value were computed from the model in advance.
  // This should be changed when the COR or force-field potential changes.
  // Currently it is computed for 4.44 [GV] and 1100 [MV], respectively.
  // The integral should be done from lowE_* to highE_*.
  // FIX ME!!
  const G4double INTEGRAL_primary = 4.3643; // [m**-2 s**-1 Sr**-1]


  // gives v/c as a function of kinetic Energy
  inline G4double beta(G4double E /* GeV */)
  {
#if 0	// if E ~ restE
    return sqrt(1 - pow(E/restE+1, -2));
#else	// if E >> restE
    return 1.0;
#endif
  }


  // gives rigidity (GV) as a function of kinetic Energy
  inline G4double rigidity(G4double E /* GeV */)
  {
#if 0	// if E ~ restE
    return sqrt(pow(E + restE, 2) - pow(restE, 2));
#else	// if E >> restE
    return E;
#endif
  }


  // gives kinetic energy (GeV) as a function of rigidity
  inline G4double energy(G4double rigidity /* GV */)
  {
#if 0	// if energy ~ restE
    return sqrt(pow(rigidity, 2) + pow(restE, 2)) - restE;
#else	// if energy >> restE
    return rigidity;
#endif
  }



  //=====================================================================
  /**
   * Generate a random distribution of primary cosmic ray electrons:
   * j(E) = mod_spec(E, phi) * beta(E) * geomag_cut(E, CutOff)
   *   mod_spec(E, phi) = org_spec(E + phi * 1e-3) * ((E+restE)**2 - restE**2) / ((E+restE+phi*1e-3)**2 - restE**2)
   *   org_spec(E) = A * rigidity(E)**-a
   *     A = 7.23e-1
   *     a = 3.33
   *   rigidity(E) = sqrt((E+restE)**2 - restE**2)
   *   beta(E) = sqrt(1 - (E/restE+1)**-2)
   *   geomag_cut(E, CutOff) = 1/(1 + (rigidity(E)/CutOff)**-12.0)
   *     CutOff = 4.44 for Theta_M = 0.735 and altitude = 50km (balloon experiment)
   *     phi = 540, 1100 [MV] for Solar minimum, maximum
   *   E: [GeV]
   *   j: [MeV**-1 m**-2 s**-1 Sr**-1]
   *
   * References:
   *  org_spec: Komori, Y. et al. 1999, Proc. of Dai-Kikyu (large balloon) Sympo. Heisei-11yr, 33-36  (Fig. 2)
   *  mod_spec: Gleeson, L. J. and Axford, W. I. 1968, ApJ, 154, 1011-1026 (Eq. 11)
   *  geomag_cut formula: an eyeball fitting function of the ratio of BESS98 and AMS98 proton spectra.
   *  CutOff: calculated as (Rc/GV) = 14.9 * (1+h/R)^-2 * (cos(theta_M))^4,
   *          where h gives altitude from earth surface, R means an radius of earth,
   *          and theta_M is geomagnetic lattitude. See references below.
   *          "Handbook of space astronomy and astrophysics" 2nd edition, p225 
   *            (Zombeck, 1990, Cambridge University Press)
   *          "High Energy Astrophysics" 2nd edition, p325-330
   *            (M. S. Longair, 1992, Cambridge University Press)
   */

  const G4double A_primary = 7.23e-1;	// Normalization factor of the original spec.
  const G4double a_primary = 3.33;	// Differential spectral index


  // Geomagnetic cut off factor of primary cosmic rays
  inline G4double geomag_cut(G4double E /* GeV */, G4double cor /* GV */)
  {
    return 1./(1 + pow(rigidity(E)/cor, -12.0));
  }


  // Unmodulated cosmic ray spectrum outside the Solar system
  inline G4double org_spec(G4double E /* GeV */)
  {
    return A_primary * pow(rigidity(E), -a_primary);
  }


  // Force-field approximation of the Solar modulation
  inline G4double mod_spec(G4double E /* GeV */, G4double phi /* MV */)
  {
    return org_spec(E + phi * 1e-3) * (pow(E+restE, 2) - pow(restE, 2))
      / (pow(E+restE+phi*1e-3, 2) - pow(restE, 2));
  }


  // The property function that the primary component should obey.
  inline G4double primaryCRspec(G4double E /* GeV */, G4double cor /* GV */, G4double phi /* MV */)
  {
    return mod_spec(E, phi) * geomag_cut(E, cor) * beta(E);
  }


  // envelope function in lower energy
  // If we express rigidity as R, energy as E, cutoff rigidity as Rc,
  // and cutoff energy as Ec, geomagnetic cutoff function
  // is enveloped as follows.
  //=============================================================
  // 1/(1+(R/Rc)^-12.0) < (R/Rc)^12.0 < C * E^(a_envelope) / Rc^12.0
  //=============================================================
  // Here, C and a_primary are determined as
  //  R(lowE_primary)^12.0 = C*(lowE_primary)^(a_envelope)
  // and a_envelope is determined as
  //  (Rc/Rc)^12.0 = C * Ec^(a_envelope) / Rc^12.0
  inline G4double primaryCRenvelope1(G4double E /* GeV */, G4double cor /* GV */, G4double phi /* MV */)
  {
    G4double a_envelope =
      12.0*log(cor/rigidity(lowE_primary))/log(energy(cor));
    G4double A_envelope =
      pow(rigidity(lowE_primary),12.0)/pow(lowE_primary,a_envelope);
    return A_primary * A_envelope * pow(cor, -12.0) *
      pow(E, a_envelope - a_primary);
  }


  // integrated envelope function in lower energy
  inline G4double primaryCRenvelope1_integral(G4double E /* GeV */, G4double cor /* GV */, G4double phi /* MV */)
  {
    G4double a_envelope =
      12.0*log(cor/rigidity(lowE_primary))/log(energy(cor));
    G4double A_envelope =
      pow(rigidity(lowE_primary),12.0)/pow(lowE_primary,a_envelope);
    return A_primary * A_envelope * pow(cor, -12.0) *
      1./(a_envelope - a_primary + 1) * pow(E, a_envelope - a_primary + 1);
  }


  // inverse function of integrated envelope function in lower energy
  inline G4double primaryCRenvelope1_integral_rev(G4double value, G4double cor /* GV */, G4double phi /* MV */)
  {
    G4double a_envelope =
      12.0*log(cor/rigidity(lowE_primary))/log(energy(cor));
    G4double A_envelope =
      pow(rigidity(lowE_primary),12.0)/pow(lowE_primary,a_envelope);
    return pow( (a_envelope-a_primary+1) / (A_primary * A_envelope * pow(cor, -12.0))
                * value, 1./(a_envelope-a_primary+1));
  }


  // envelope function in higher energy
  inline G4double primaryCRenvelope2(G4double E /* GeV */, G4double cor /* GV */, G4double phi /* MV */)
  {
    return A_primary * pow(E, -a_primary);
  }


  // integrated envelope function in higher energy
  inline G4double primaryCRenvelope2_integral(G4double E /* GeV */, G4double cor /* GV */, G4double phi /* MV */)
  {
    return A_primary * pow(E, -a_primary+1) / (-a_primary+1);
  }


  // inverse function of integrated envelope function in higher energy
  inline G4double primaryCRenvelope2_integral_rev(G4double value, G4double cor /* GV */, G4double phi /* MV */)
  {
    return pow(value * (-a_primary+1) / A_primary, 1./(-a_primary+1));
  }


  // The rundam number generator for the primary component.
  G4double primaryCRenergy(HepRandomEngine* engine, G4double cor, G4double solarPotential)
  {
    G4double rand_min_1 = primaryCRenvelope1_integral(lowE_primary, cor, solarPotential);
    G4double rand_max_1 = primaryCRenvelope1_integral(energy(cor), cor, solarPotential);
    G4double rand_min_2 = primaryCRenvelope2_integral(energy(cor), cor, solarPotential);
    G4double rand_max_2 = primaryCRenvelope2_integral(highE_primary, cor, solarPotential);

    //G4cerr << "rand_min_1: " << rand_min_1 << ", rand_max_1: " << rand_max_1 << ", ";
    //     << "rand_min_2: " << rand_min_2 << ", rand_max_2: " << rand_max_2 << G4endl;

    G4double envelope1_area = rand_max_1 - rand_min_1;
    G4double envelope2_area = rand_max_2 - rand_min_2;

    double r, E;

    while (1){
      if (engine->flat() <= envelope1_area / (envelope1_area + envelope2_area)){
        // envelope in lower energy
        r = engine->flat() * (rand_max_1 - rand_min_1) + rand_min_1;
	//G4cerr << "r: " << r << ", ";
        E = primaryCRenvelope1_integral_rev(r, cor, solarPotential);
	// G4cerr << "E: " << E << ", primarySpec: " << primaryCRspec(E, cor, solarPotential) << ", ";
	// G4cerr << "primaryEnvelope1: " << primaryCRenvelope1(E, cor, solarPotential) << G4endl;
        if (engine->flat() <= primaryCRspec(E, cor, solarPotential) / primaryCRenvelope1(E, cor, solarPotential))
          break;
      } else {
        // envelope in higher energy
        r = engine->flat() * (rand_max_2 - rand_min_2) + rand_min_2;
	// G4cerr << "r: " << r << ", ";
        E = primaryCRenvelope2_integral_rev(r, cor, solarPotential);
	// G4cerr << "E: " << E << ", primarySpec: " << primaryCRspec(E, cor, solarPotential) << ", ";
	// G4cerr << "primaryEnvelope2: " << primaryCRenvelope2(E, cor, solarPotential) << G4endl;
        if (engine->flat() <= primaryCRspec(E, cor, solarPotential) / primaryCRenvelope2(E, cor, solarPotential))
          break;
      }
    }
    return E;//THB * GeV;
  }
} // End of noname-namespace: private function definitions.


//
//
//

CrElectronPrimary::CrElectronPrimary()
{
  ;
}


CrElectronPrimary::~CrElectronPrimary()
{
  ;
}


std::pair<double,double> CrElectronPrimary::dir(double energy, HepRandomEngine* engine) const
  // return: cos(zenith_angle) and azimuth [rad]
  // The downward has plus sign in cos(zenith_angle),
  // and azimuth = 0 from the east, +pi/2 from the north.
{
  // Assuming isotropic from the upper (i.e. sky side) hemisphere.
  // With the cosine correction, the zenith-angle distribution should
  // be sin(theta).

  double  zenith_angle = acos(engine->flat());//THB * rad;
  double  azimuth      = engine->flat() * 2 * pi;

  return std::pair<double,double>(cos(zenith_angle), azimuth); //THB / rad);
}



double CrElectronPrimary::energySrc(HepRandomEngine* engine) const
{
  return primaryCRenergy(engine, m_cutOffRigidity, m_solarWindPotential);
}


double CrElectronPrimary::flux() const
{
  /*****
  // Integrated over the upper (sky-side) hemisphere.
  return  2 * pi * INTEGRAL_primary;  // [m**-2 s**-1]
  *****/
  // Integrated over the upper (sky-side) hemisphere.
  return  0.5 * INTEGRAL_primary;  // [m**-2 s**-1 Sr**-1]
}


double CrElectronPrimary::solidAngle() const
{
  return  2 * pi;
}


const char* CrElectronPrimary::particleName() const
{
  return "e-";
}


float CrElectronPrimary::operator()(float r)
{
  HepJamesRandom  engine;
  engine.setSeed(r * 900000000);
  // 900000000 comes from HepJamesRandom::setSeed function's comment...

  return (float)energySrc(&engine);
}


double CrElectronPrimary::calculate_rate(double old_rate)
{
  return  old_rate;
}


float CrElectronPrimary::flux(float latitude, float longitude) const
{
  return  flux();
}


float CrElectronPrimary::flux(std::pair<double,double> coords) const
{
  return  flux();
}


std::string CrElectronPrimary::title() const
{
  return  "CrElectronPrimary";
}


float CrElectronPrimary::fraction(float energy)
// This function doesn't work in this class... :-(
{
  return  0;
}


std::pair<float,float> CrElectronPrimary::dir(float energy) const
{
  HepJamesRandom  engine;
  std::pair<double,double>  d = dir(energy, &engine);
  return  std::pair<float,float>(d.first, d.second);
}

