/**
* @class TkrNeuralNetFit
*
* @brief Class definition for the Neural Net PatRec Transient Data Object.
*
* Copied from Tracy's Link and Tree Track Fit routine.
*
* last modified 1/02
*
* @authors b. allgood and w. atwood
*
* $Header$
*/

#ifndef TKRNEURALNETFIT_H
#define TKRNEURALNETFIT_H

#include <vector>
#include "GaudiKernel/MsgStream.h"
#include "Event/Recon/TkrRecon/TkrPatCandCol.h"
#include "Event/Recon/TkrRecon/TkrFitTrack.h"
#include "TkrRecon/ITkrGeometrySvc.h"

namespace Event { //Namespace

class TkrNeuralNetFit : public TkrFitTrackCol
{
public:
    TkrNeuralNetFit() {}
    TkrNeuralNetFit(ITkrGeometrySvc* pTkrGeo, TkrClusterCol* pTkrClus, TkrPatCandCol* pTkrCand, double CalEnergy);
    ~TkrNeuralNetFit() {}
    
private:
};

}; //Namespace
#endif
