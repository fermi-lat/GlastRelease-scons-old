/**
 * @class TkrNeuralNetFitTool
 *
 * @brief Implements a Gaudi Tool for performing a track fit. This version uses 
 *        candidate tracks from the Neural Net Pattern Recognition which are then fit
 *        with KalFitTrack 
 *
 * @author The Tracking Software Group
 *
 * $Header$
 */

#ifndef TKRCOMBOFITTOOL_H
#define TKRCOMBOFITTOOL_H
/*
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/DataSvc.h"
#include "TkrRecon/Track/ITkrFitTool.h"
#include "TkrUtil/ITkrGeometrySvc.h"
#include "TkrUtil/ITkrFailureModeSvc.h"

class TkrNeuralNetFitTool : public AlgTool, virtual public ITkrFitTool
{
public:
    /// Standard Gaudi Tool interface constructor
    TkrNeuralNetFitTool(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~TkrNeuralNetFitTool() {}

    /// @brief Method to fit a single candidate track. Will retrieve any extra info 
    ///        needed from the TDS, then create and use a new KalFitTrack object to 
    ///        fit the track via a Kalman Filter. Successfully fit tracks are then 
    ///        added to the collection in the TDS.
    StatusCode doTrackFit(Event::TkrPatCand* patCand);

private:
    /// Pointer to the local Tracker geometry service
    ITkrGeometrySvc* pTkrGeoSvc;
    /// Pointer to failure mode service
    ITkrFailureModeSvc* pTkrFail;

    /// Pointer to the Gaudi data provider service
    DataSvc*        pDataSvc;
};
*/
#endif
