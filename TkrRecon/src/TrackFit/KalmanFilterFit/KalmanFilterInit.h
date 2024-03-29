/**
 * @class KalmanFilterInit
 *
 * @brief Definition of a Kalman Filter transport matrix
 *
 * @author Tracy Usher
 *
 * $Header$
 */

#ifndef KalmanFilterInit_h
#define KalmanFilterInit_h

#include <vector>

class StdTransportMatrix;
class StdProjectionMatrix;
class ThreeDProjectionMatrix;
class StdProcNoiseMatrix;
class NoProcNoiseMatrix;

class KalmanFilterInit
{
public:

    // Constructor 
    KalmanFilterInit(std::vector<double>& zCoords, std::vector<int> projection, std::vector<double> energy);
   ~KalmanFilterInit() {};

    void init(StdTransportMatrix& tMat) const;
    void init(StdProjectionMatrix& tMat) const;
    void init(ThreeDProjectionMatrix& tMat) const;
    void init(StdProcNoiseMatrix& tMat) const;
    void init(NoProcNoiseMatrix& tMat) const;

private:
    std::vector<double> m_zCoords;
    std::vector<int>    m_projection;
    std::vector<double> m_energy;
};


#endif
