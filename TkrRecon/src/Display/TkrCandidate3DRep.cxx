#include "TkrRecon/Display/TkrCandidate3DRep.h"

//------------------------------------------------------------------------------
/// Algorithm parameters which can be set at run time must be declared.
/// This should be done in the constructor.

const char col3d_blue[]       = "blue";
const char col3d_violet[]     = "violet";
const char col3d_turquoise[]  = "turquoise";
const char col3d_orange[]     = "orange";
const char col3d_maroon[]     = "maroon";
const char col3d_aquamarine[] = "aquamarine";

const char* p3dColors[] = {col3d_blue,   col3d_violet, col3d_turquoise,
                           col3d_orange, col3d_maroon, col3d_aquamarine};

//#############################################################################
TkrCandidate3DRep::TkrCandidate3DRep(TkrCandidates** ppCands, ITkrGeometrySvc* pTkrGeometry)
//#############################################################################
{
	ppTkrCandidates = ppCands;
    pTkrGeo         = pTkrGeometry;
}
//-------------------- private ----------------------
//##############################################
void TkrCandidate3DRep::update()
//##############################################
{
    TkrCandidates*    pTkrCandidates = *ppTkrCandidates;

    //Zero out the pointer so we don't accidentally try to draw the event
    *ppTkrCandidates = 0;

	//Now see if we can do the drawing
	if (pTkrCandidates)
	{
        int numCandTracks = pTkrCandidates->getNumCands();
        int colorIdx      = 0;

        gui::DisplayRep* pDisplay = this;

        while(numCandTracks--)
        {
            TkrPatCand* pTkrCand = pTkrCandidates->getTrack(numCandTracks);

            //Put a marker at the start of the candidate
            Point  strtPoint = pTkrCand->position();
            double x         = strtPoint.x();
            double y         = strtPoint.y();
            double zCur      = strtPoint.z() + 10.;
            double zPrev     = strtPoint.z() + 100.;

	        setColor(p3dColors[colorIdx]);
            markerAt(strtPoint);

            int numHits             = pTkrCand->numPatCandHits();
            CandHitVectorPtr hitPtr = pTkrCand->getCandHitPtr();

            while(numHits--)
            {
                TkrPatCandHit* pHitCand = &(*hitPtr++);
                Point          hitCoord = pHitCand->Position();

                if (pHitCand->View() == TkrCluster::X) x = hitCoord.x();
                else                                   y = hitCoord.y();

                zCur = hitCoord.z();

                if (abs(zPrev-zCur) < 0.25)
                {
                    Point newPoint(x,y,0.5*(zCur+zPrev));
                    moveTo(strtPoint);
                    lineTo(newPoint);

                    strtPoint = newPoint;
                }

                zPrev = zCur;
            }

            colorIdx = (colorIdx + 1) % 6;
        }



	    setColor("blue");
    }

    return;
}

