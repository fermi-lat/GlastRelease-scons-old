//$Header$
#ifndef MootCnvSvc_h
#define MootCnvSvc_h  1

/// Include files
#include <vector>
#include <string>
#include <cstring>

class IDataProviderSvc;

#include "GaudiKernel/Service.h"
#include "MootSvc/IMootSvc.h"
// #include "CalibSvc/IInstrumentName.h"

#include "GaudiKernel/IIncidentListener.h"

/// Forward and external declarations
template <class TYPE> class SvcFactory;
class ISvcLocator;
class MsgStream;

namespace MOOT {
  class MoodConnection;
  //   class MootQuery;  already declared in IMootSvc
}

/** @class MootSvc
    Implements IMootSvc interface, providing access to Moot data.
    See also CalibData/Moot for definition of data structures
    
    @author J. Bogart
*/
class MootSvc :  public Service, 
                 virtual public IIncidentListener,
                 virtual public IMootSvc
{
  /// Only factories can access protected constructors
  friend class SvcFactory<MootSvc>;

 public:

  // Reimplemented from IMootSv

  /// Filter config routines
  /**
     Get info for all active filters (union over all modes)
   */
  unsigned getActiveFilters(std::vector<CalibData::MootFilterCfg>&
                            filters);

  /**
     Get info for  active filters for mode @a acqMode
   */
  unsigned getActiveFilters(std::vector<CalibData::MootFilterCfg>&
                            filters, unsigned acqMode);

  /**
     Get info and handler name for filter specified by mode and handler id
   */
  CalibData::MootFilterCfg* 
  getActiveFilter(unsigned acqMode, unsigned handlerId, 
                  std::string& handlerName);

  /// Return last LATC master key seen in data
  virtual unsigned getHardwareKey();

  /// See enum definitions for source (TDS or job options) and items
  /// (scid, etc.) in IMootSvc.h.
  /// Note if Moot config key is set in job options, all other items
  /// (scid, start time, hw key) are derived from this key, hence are
  ///  also considered to be set from job options.  
  /// Similarly if both start time and scid are set in job options, other
  /// items may be derived, so all are considered to be set from jo
  virtual MOOT::InfoSrc getInfoItemSrc(MOOT::InfoItem item);

  /// Return Moot config key for current acquisition
  /// Return 0 if unknown
  virtual unsigned getMootConfigKey();

  /// Return absolute path for parameter source file of specified class.
  /// If none return empty string.
  std::string getMootParmPath(const std::string& cl, unsigned& hw);

  /// Return parm with GEM info.  Does not necessarily include ROI
  virtual const CalibData::MootParm* getGemParm(unsigned& hw);

  /// Return parm containing ROI registers (not necessarily
  /// other GEM registers though)
  virtual const CalibData::MootParm* getRoiParm(unsigned& hw);


  /// Return MootParm structure for parameter source file of specified class.
  /// If none return blank structure.
  virtual const CalibData::MootParm* getMootParm(const std::string& cl, 
                                                 unsigned& hw);

  // Return pointer to Moot parameter collection.  Also set output
  // arg. hw to current hw key
  virtual const CalibData::MootParmCol* getMootParmCol(unsigned& hw);




  /// Return index in MootParmCol of specified class
  virtual int latcParmIx(const std::string& parmClass) const;

  virtual MOOT::MootQuery* getConnection() const {return m_q;}

  /// Return true if MootSvc has been turned off via job option 
  virtual bool noMoot() const {return m_noMoot;}


  // Reimplemented from IInterface

  virtual StatusCode queryInterface( const InterfaceID& riid, 
				     void** ppvInterface );  

  virtual StatusCode initialize();
  virtual StatusCode finalize();

 protected:

  MootSvc(const std::string& name, ISvcLocator* svc );
  virtual ~MootSvc();

 public:

  // For internal CalibSvc use, but not necessarily MootSvc class
  // Not part of IMootSvc interface
  typedef std::pair<std::string, std::string> ParmPrecinct;


 private:

  /// Routine which willbe called when a new incident has occurred
  virtual void handle(const Incident&);

  MOOT::MootQuery* makeConnection(bool verbose=true);
  void closeConnection();
 
  StatusCode getPrecincts();  // fills m_parmPrecincts

  /// Given param class name, find associated precinct
  std::string findPrecinct(const std::string& pclass);

  StatusCode updateMootParmCol();
  StatusCode updateFswEvtInfo();  // keys, started_at, scid

  /// Handles for metadata access
  MOOT::MootQuery*    m_q;
  MOOT::MoodConnection* m_c;

  /// MOOT archive path.  
  std::string   m_archive;  

  std::vector<ParmPrecinct> m_parmPrecincts;

  IDataProviderSvc* m_eventSvc;

  unsigned          m_hw; // latc master from most recent event or from Config
  unsigned          m_mootConfigKey; // For current run
  unsigned          m_startTime;     // use this to look up moot config
  unsigned          m_scid;          // use this to look up moot config
  unsigned          m_countdown; // decide when to close open MySQL connection
  int               m_nEvent;

  bool  m_useEventKeys;                       // job options
  bool  m_verbose;                            // controls MoodConnection
  bool  m_lookUpStartTime;                    // true if we get it from event
  bool  m_lookUpScid;                         // true if we get it from event
  bool  m_fixedConfig;                        // diagnostic
  bool  m_noMoot;                             // if true, just go away
  bool  m_exitOnFatal;                         // True by default

  CalibData::MootParmCol*  m_mootParmCol;
};
#endif   

