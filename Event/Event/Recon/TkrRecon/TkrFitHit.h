
#ifndef _TkrFitHit_H
#define _TkrFitHit_H 1

#include "Event/Recon/TkrRecon/TkrFitPar.h"
#include "Event/Recon/TkrRecon/TkrFitMatrix.h"

/** 
* @class TkrFitHit
*
* @brief Contains the Track Parameters, Track Covariance matrix and parameters type
*
* @author Bill Atwood
*
* $Header$
*/

namespace Event { //Namespace

class TkrFitHit
{   // Class to link a parameter vector and a covariance matrix together
    // at a particular location.  The hits types are given by the
    // enumerated member TYPE. 
    
public:
    enum TYPE {MEAS, FIT, PRED, SMOOTH, UNKNOWN};

    TkrFitHit() 
        : m_type(UNKNOWN) 
    {}

    TkrFitHit(TYPE t, const TkrFitPar& p, const TkrFitMatrix& m)
        :m_type(t),m_par(p),m_cov(m) 
    {}
    TkrFitHit (const TkrFitHit& right)
        : m_type(right.m_type), m_par(right.m_par), m_cov(right.m_cov)
    {}
    
    TkrFitHit changeType(TYPE type);
    
    inline const TYPE& getType()        const {return m_type;}
    inline const TkrFitPar& getPar()    const {return m_par;}
    inline const TkrFitMatrix& getCov() const {return m_cov;}
    
private:
    
    TYPE         m_type;
    TkrFitPar    m_par;
    TkrFitMatrix m_cov;
};

}; //Namespace


#endif

