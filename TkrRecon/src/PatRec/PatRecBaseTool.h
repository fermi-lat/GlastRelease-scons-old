#ifndef PATRECBASETOOL_H
#define PATRECBASETOOL_H

#include "TkrUtil/ITkrGeometrySvc.h"
#include "TkrUtil/ITkrQueryClustersTool.h"
#include "TkrRecon/PatRec/ITkrFindTrackTool.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IDataProviderSvc.h"

/**
 * @class PatRecBaseTool
 * @brief Base class for the concrete pattern recognition tools.
 * @author GLAST Tracker Software group
 * $Header$
 */
class PatRecBaseTool : public AlgTool, virtual public ITkrFindTrackTool 
{
 public:
  // Constructor
  PatRecBaseTool( const std::string& type, const std::string& name, 
		  const IInterface* parent);
  // Standard Destructor
  virtual ~PatRecBaseTool() {;}
  
  ///Implementation of the method provided by the base class AlgTool.
  virtual StatusCode initialize();


 protected:

  /// Pointer to the local Tracker geometry service
  ITkrGeometrySvc*       m_tkrGeom;

  /// Pointer to the local FailureMode service
  ITkrFailureModeSvc*    m_tkrFail;

  /// Event Service member directly useable by concrete classes.
  IDataProviderSvc*      m_dataSvc;

  /// Query Clusters tool
  ITkrQueryClustersTool* m_clusTool;

};
#endif

