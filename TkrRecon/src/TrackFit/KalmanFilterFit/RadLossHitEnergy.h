/**
 * @class RadLossHitEnergy
 *
 * @brief Implementation of a class used for assigning energy to a hit during the fit process
 *        This version is intended to be used with normal track fits
 *
 * @author Tracy Usher
 *
 * $Header$
 */

#ifndef RadLossHitEnergy_h
#define RadLossHitEnergy_h

#include "src/TrackFit/KalmanFilterFit/IFitHitEnergy.h"
#include "src/Track/TkrControl.h"

class RadLossHitEnergy : public IFitHitEnergy
{
public:

    // Constructor needs the matrices that transform state vector, covariance matrix
    RadLossHitEnergy();
    ~RadLossHitEnergy() {};

    double initialHitEnergy(const Event::TkrPatCandHit& candHit, const double trkEnergy);
    double updateHitEnergy(const double curEnergy, const double radLen);
    double getHitEnergy(const double energy) {return energy;}

private:
    TkrControl* m_control;
};


#endif