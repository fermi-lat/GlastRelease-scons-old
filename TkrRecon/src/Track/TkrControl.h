
#ifndef TKRCONTROL_H
#define TKRCONTROL_H

// Temporary (I think...)
#include "src/TrackFit/KalFitTrack/GFcontrol.h"

//############################################
class TkrControl
//############################################
{
public:

    // Define as a singleton object
    static TkrControl* GetPtr();

    // Retrieve values
    const inline int    GetMaxCandidates()      {return m_MaxCandidates;     }
    const inline int    GetMinTermHitCount()    {return m_MinTermHitCount;   }
    const inline double GetFEneParticle()       {return m_FEneParticle;      }
    const inline double GetSigmaCut()           {return m_SigmaCut;          }
    const inline double GetMinEnergy()          {return m_MinEnergy;         }
    const inline int    GetMaxConsecutiveGaps() {return m_MaxConsecutiveGaps;}
    const inline int    GetMinSegmentHits()     {return m_MinSegmentHits;    }
    const inline double GetMaxChisqCut()        {return m_MaxChiSqCut;       }
    const inline double GetIniErrSlope()        {return m_IniErrorSlope;     }
    const inline double GetIniErrPosition()     {return m_IniErrorPosition;  }
    const inline bool   GetPlaneEnergies()      {return m_PlaneEnergies;     }

    // Allow for control variables to be set at initialization
    void SetMaxCandidates(  int    maxCand)   {m_MaxCandidates      = maxCand;   GFcontrol::maxCandidates     = maxCand;   }
    void SetMinTermHitCount(int    minCount)  {m_MinTermHitCount    = minCount;  GFcontrol::minTermHitCount    = minCount; }
    void SetFEneParticle(   double enePart)   {m_FEneParticle       = enePart;   GFcontrol::FEneParticle       = enePart;  }
    void SetSigmaCut(       double sigmaCut)  {m_SigmaCut           = sigmaCut;  GFcontrol::sigmaCut           = sigmaCut; }
    void SetMinEnergy(      double minEnergy) {m_MinEnergy          = minEnergy; GFcontrol::minEnergy          = minEnergy;}
    void SetMaxConsGaps(    int    maxGaps)   {m_MaxConsecutiveGaps = maxGaps;   GFcontrol::maxConsecutiveGaps = maxGaps;  }
    void SetMinSegmentHits( int    minHits)   {m_MinSegmentHits     = minHits;   GFcontrol::minSegmentHits     = minHits;  }
    void SetMaxChisqCut(    double maxChi)    {m_MaxChiSqCut        = maxChi;    GFcontrol::maxChisqCut        = maxChi;   }
    void SetIniErrSlope(    double errSlp)    {m_IniErrorSlope      = errSlp;    GFcontrol::iniErrorSlope      = errSlp;   }
    void SetIniErrPos(      double errPos)    {m_IniErrorPosition   = errPos;    GFcontrol::iniErrorPosition   = errPos;   }
    void SetPlaneEnergies(  bool   enePlane)  {m_PlaneEnergies      = enePlane;  GFcontrol::planeEnergies      = enePlane; }

private:
    // Constructor and destructor are private
    TkrControl();
   ~TkrControl() {}

    // Pointer to the singleton object
    static TkrControl* m_this;

    // Data members
    int    m_MaxCandidates;
    int    m_MinTermHitCount;

    double m_FEneParticle;

    double m_SigmaCut;
    double m_MinEnergy;

    int    m_MaxConsecutiveGaps;
    int    m_MinSegmentHits;
    double m_MaxChiSqCut;
    double m_IniErrorSlope;
    double m_IniErrorPosition;

    bool   m_PlaneEnergies;
};

#endif
