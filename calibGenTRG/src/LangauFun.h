#ifndef LangauFun_h
#define LangauFun_h

// $Header$

// LOCAL INCLUDES

// GLAST INCLUDES

// EXTLIB INCLUDES
#include "Rtypes.h"

// STD INCLUDES
#include <memory>

class TNtuple;
class TH1;
class TF1;

  
  /** @file
      @author Zach Fewtrell, Alexandre Chekhtman
  */

  /** @brief Singleton-like class provides pointers to ROOT TF1 functions which
      fit with gaussian-convolved landau function.

      Fit parameters:
      - par[0] = Width (scale) parameter of Landau density
      - par[1] = Most Probable Value (MP, location) parameter of Landau density
      - par[2] = Total area (integral -inf to inf, normalization constant)
      - par[3] = Width (sigma) of convoluted Gaussian function
      - par[4] = Height of background model which is level below the MPV and decays to zero after the MPV

      In the Landau distribution (represented by the CERNLIB approximation),
      the maximum is located at x=-0.22278298 with the location parameter= 0.
      This shift is corrected within this function, so that the actual
      maximum is identical to the MP parameter.
  */

  class LangauFun {
  public:
    /// retrieve gaussian convolved landau fuction with limits & initial values appropriate for LE CIDAC scale
    static TF1 &     getLangauDAC();

  };

#endif
