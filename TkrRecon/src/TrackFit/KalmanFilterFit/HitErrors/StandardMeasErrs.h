/**
 * @class StandardMeasErrs
 *
 * @brief This class assigns errors to the measured coordinates in a given plane
 *        StandardMeasErrs implements a version which corrects the error in the measured direction by
 *        the incoming track slope. It includes protection to prevent the error from becoming too small. 
 *
 * @author Tracy Usher (editor) taken from code authored by Bill Atwood
 *
 * $Header$
 */

#ifndef StandardMeasErrs_h
#define StandardMeasErrs_h

#include "IComputeMeasErrors.h"
#include "src/Track/TkrControl.h"
#include "TkrUtil/ITkrGeometrySvc.h"

class StandardMeasErrs : public IComputeMeasErrors
{
public:

    // Constructor needs the matrices that transform state vector, covariance matrix
    StandardMeasErrs(ITkrGeometrySvc* tkrGeo);
   ~StandardMeasErrs() {};


    Event::TkrFitMatrix computeMeasErrs(const Event::TkrFitPar& newPars, 
                                        const Event::TkrFitMatrix& oldCovMat, 
                                        const Event::TkrCluster& cluster);

private:
    ITkrGeometrySvc* m_tkrGeo;
    TkrControl*      m_control;
};


#endif