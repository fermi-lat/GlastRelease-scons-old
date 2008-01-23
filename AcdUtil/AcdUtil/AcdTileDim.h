#ifndef ACDTILEDIM_H
#define ACDTILEDIM_H

// stl headers
#include <vector>
#include <assert.h>

// Gaudi, facilities, interfaces
#include "GaudiKernel/StatusCode.h"
#include "AcdUtil/IAcdGeometrySvc.h"

// Detector & geometry 
#include "idents/AcdId.h"
#include "idents/VolumeIdentifier.h"
#include "CLHEP/Geometry/Point3D.h"
#include "CLHEP/Geometry/Transform3D.h"
#include "CLHEP/Matrix/Matrix.h"

/**
*  @class AcdTileDim
*
*  @brief This class holds the dimension information about Acd Tiles for use in the ACD reconstruction algorithms
*  
*  In these algorithms the tiles are treated as thin, flat planes.  Therefore only the tile center and the four 
*  corners are used. 
*
*  Most of the tiles are a single piece, however 10 tiles, (0-4 and 40-44) are made from two pieces, one on the top
*  of the ACD and one that runs down the side.  This makes things quite a bit more complicated.
*  
*  \author Eric Charles
*
* $Header$
*/

class AcdTileDim {

public:
    
  /// This function switches from active distance to local coords
  static StatusCode toLocalCoords(const AcdTileDim& dim, int region, const double& activeX, const double& activeY,
				  double& localX, double& localY);

public:
    
  /// Constructor: takes the tile id, the volume id, and the detector service
  AcdTileDim(const idents::AcdId& acdId, IAcdGeometrySvc& acdGeomSvc);
  
  /// trivial destructor
  ~AcdTileDim() {;}

  /// update (ie, when we get a new event)
  StatusCode update(IAcdGeometrySvc& acdGeomSvc) {
    m_acdGeomSvc = acdGeomSvc;
    m_sc = getVals();
    return m_sc;
  }

  /// direct access functions
  inline IAcdGeometrySvc& acdGeomSvc() const { return m_acdGeomSvc; }
  inline const idents::AcdId& acdId() const { return m_acdId; }
  inline StatusCode statusCode() const { return m_sc; }
  inline int nVol() const { return m_nVol; }
  inline const std::vector<double>& dim(int idx = 0) const { 
    assert(idx < m_nVol);
    return m_dim[idx]; 
  }
  inline const HepPoint3D& tileCenter(int idx =0) const { 
    assert(idx < m_nVol);
    return m_tileCenter[idx]; 
  }
  inline const HepPoint3D* corner(int idx = 0) const { 
    assert(idx < m_nVol);
    return m_corners[idx]; 
  }

  /// Local frame vectors projected into global frame
  inline const HepMatrix& localFrameVectors(int idx = 0) const {
    assert(idx < m_nVol);
    return m_localFrameVectors[idx];
  }

  inline int sharedEdge(int idx) const { 
    assert(idx < m_nVol);
    return m_shared[idx]; 
  }

  inline float sharedWidth(int idx) const { 
    assert(idx < m_nVol);
    return m_sharedWidth[idx]; 
  }

  inline const std::vector< HepPoint3D > screwHoles() const {
    return m_screwHoles;
  }

  
  /**
   * @brief get the width at a particular Y.  This is need b/c some of the tiles are trapezoids
   *
   * @param iVol which tile segment
   * @param localY the local Y coordinate where we care about the width
   * @return width (ie extent in localX) at that localY
   */
  double widthAtLocalY(int iVol, double localY) const;


  /**
   * @brief Convert a point for global to local coords
   *
   * @param global point in global frame
   * @param idx which tile segment
   * @param local point in local (ie segment) frame
   */
  void toLocal(const HepPoint3D& global, HepPoint3D& local, int idx = 0) const;

  /**
   * @brief Convert a point in local coordinates into active distances
   *
   * @param localPoint is the point in local coords
   * @param iVol which tile segment
   * @param activeX is the distance to the closest edge in localX (positive -> inside, negative -> outside)
   * @param activeY is the distance to the closest edge in localY (positive -> inside, negative -> outside)
   */
  void activeDistance(const HepPoint3D& localPoint, int iVol, double& activeX, double& activeY) const;

  /**
   * @brief get the transformation from global to local coords
   *
   * @param iVol which ribbon segment
   * @return the transformation
   */
  const HepTransform3D& transform(int iVol) const {
    return m_transform[iVol];
  }

protected:
  
  /// this function access the detector service to get the geometry information
  StatusCode getVals();  

private:  

  /// The tile id
  const idents::AcdId       m_acdId;

  /// ACD Geom Service
  IAcdGeometrySvc&          m_acdGeomSvc;

  /// This show the status of the access to the detector service, should be checked before using data
  StatusCode                m_sc;

  /// The number of volumes in this tile
  int                       m_nVol;
  
  /// Is this tile a trapezoid
  bool                      m_trapezoid;
  double                    m_xmean;
  double                    m_xdiff;

  /// the tile dimensions
  std::vector<double>       m_dim[2];

  /// the center of the tile (in global coordinates)
  HepPoint3D                m_tileCenter[2];

  /// the four corners of the tile (in global coordinates)
  HepPoint3D                m_corners[2][4];  

  /// the transformations to local coords
  HepTransform3D            m_transform[2];

  /// Local frame vectors projected into global frame
  HepMatrix                 m_localFrameVectors[2];

  /// which (if any) edges are shared between tiles:
  int                       m_shared[2];
 
  /// which face is the volume on
  //int                       m_face[2];

  /// width extra of extra volume in curved tiles
  float                     m_sharedWidth[2];

  /// Positions of the screw holes, in local coords
  std::vector< HepPoint3D > m_screwHoles;
  
};
   

#endif
