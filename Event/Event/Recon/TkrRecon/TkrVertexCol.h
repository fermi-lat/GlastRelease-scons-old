//
//------------------------------------------------------------------------
//
// Class definition for TkrVertexCol
//
// Transient Data Object for the vertices found from the collection of fit
// tracks.
//
// Tracy Usher March 1, 2002
//
//------------------------------------------------------------------------
//

#ifndef TkrVertexCol_H
#define TkrVertexCol_H

#include <vector>
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/DataObject.h"
#include "GlastEvent/Recon/TkrRecon/TkrVertex.h"
/** 
* @class TkrVertexCol
*
* @brief TDS Container class for keeping a list of found TkrVertex objects
*
* @author The Tracking Software Group
*
* $Header$
*/


extern const CLID& CLID_TkrVertexCol;

namespace TkrRecon { //Namespace

class TkrVertexCol : public DataObject
{
public:
    TkrVertexCol() {m_Vertices.clear();}
   ~TkrVertexCol();

    //Provide ability to output some information about self...
    void writeOut(MsgStream& log) const;

	//! GAUDI members to be use by the converters
	static const CLID&  classID()           {return CLID_TkrVertexCol;}
	virtual const CLID& clID()        const {return classID();}

    //How many reconstructed tracks are there?
    int                 getNumVertices() const {return m_Vertices.size();}

    //Access to tracks through an iterator
    TkrVertexVecPtr     getVertexPtr()         {return m_Vertices.begin();}

    //Access to tracks by index
    TkrVertex*          getVertex(int idx)     {return m_Vertices[idx];}

    //Add tracks to the list
    void                addVertex(TkrVertex* vertex);

private:
    TkrVertexVec m_Vertices;
};

}; //Namespace

#endif