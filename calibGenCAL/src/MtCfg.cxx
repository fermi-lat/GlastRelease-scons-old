// LOCAL INCLUDES
#include "MtCfg.h"
#include "CalDefs.h"
#include "CGCUtil.h"

// GLAST INCLUDES
#include "facilities/Util.h"

// EXTLIB INCLUDES

// STD INCLUDES
#include <sstream>
#include <iomanip>

using namespace CGCUtil;

const string MtCfg::TEST_INFO("TEST_INFO");
const string MtCfg::PATHS("PATHS");
const string MtCfg::GENERAL("GENERAL");

void MtCfg::readCfgFile(const string& path) {
  clear();

  xmlBase::IFile ifile(path.c_str());
  using facilities::Util;
  
  // TEST INFO
  timestamp  = ifile.getString(TEST_INFO.c_str(), "TIMESTAMP");
  instrument = ifile.getString(TEST_INFO.c_str(), "INSTRUMENT");
  Util::expandEnvVar(&timestamp);
  Util::expandEnvVar(&instrument);

  string tmpStr = ifile.getString(TEST_INFO.c_str(), "TOWER_BAY");
  Util::expandEnvVar(&tmpStr);
  twrBay = Util::stringToInt(tmpStr);
  CAL_LO_enabled = ifile.getBool(TEST_INFO.c_str(), "CAL_LO_TRIGGER_ENABLED") != 0;
  
  dacVals    = ifile.getIntVector(TEST_INFO.c_str(), "DAC_SETTINGS");
  nPulsesPerDAC  = ifile.getInt(TEST_INFO.c_str(), "N_PULSES_PER_DAC");

  // PATHS
  outputDir = ifile.getString(PATHS.c_str(), "OUTPUT_FOLDER");
  Util::expandEnvVar(&outputDir);

  dtdFile   = ifile.getString(PATHS.c_str(), "DTD_FILE");
  Util::expandEnvVar(&dtdFile);
  outputXMLPath = ifile.getString(PATHS.c_str(), "XMLPATH");
  Util::expandEnvVar(&outputXMLPath);
  outputTXTPath = ifile.getString(PATHS.c_str(), "TXTPATH");
  Util::expandEnvVar(&outputTXTPath);
  
  rootFileA = ifile.getString(PATHS.c_str(), "ROOTFILE_A");
  Util::expandEnvVar(&rootFileA);
  rootFileB = ifile.getString(PATHS.c_str(), "ROOTFILE_B");
  Util::expandEnvVar(&rootFileB);
  rootFileCI = ifile.getString(PATHS.c_str(), "ROOTFILE_CI");
  Util::expandEnvVar(&rootFileCI);

  pedFileTXT      = ifile.getString(PATHS.c_str(), "PEDFILE_TXT");
  Util::expandEnvVar(&pedFileTXT);

  histFile = ifile.getString(PATHS.c_str(), "HISTFILE");
  Util::expandEnvVar(&histFile);

  logfile = ifile.getString(PATHS.c_str(), "LOGFILE");
  Util::expandEnvVar(&logfile);

  // ridiculous comparison elminates cast warning in msvc
  genXML      = ifile.getBool(GENERAL.c_str(), "GENERATE_XML") != 0;
  genTXT      = ifile.getBool(GENERAL.c_str(), "GENERATE_TXT") != 0;
  genLogfile  = ifile.getBool(GENERAL.c_str(), "GENERATE_LOGFILE") != 0;
  genHistfile = ifile.getBool(GENERAL.c_str(), "GENERATE_HISTFILE") != 0;

  // Geneate derived config quantities.
  nDACs          = dacVals.size();
  nPulsesPerXtal = nPulsesPerDAC * nDACs;
  nPulsesPerRun  = ColNum::N_VALS*nPulsesPerXtal;

  baseFilename = rootFileA;
  path_remove_dir(baseFilename);
  path_remove_ext(baseFilename);
  vector<string> tokens;
  string delim("_");
  tokenize_str(baseFilename,tokens,delim); 
  string moduleName = "FMxxx";
  if ((tokens[1]).size() == 5 && ((tokens[1]).substr(0,2))== string("FM")) moduleName=tokens[1];
  if ((tokens[2]).size() == 5 && ((tokens[2]).substr(0,2)) == string("FM")) moduleName=tokens[2];

  string runID = tokens[0];
  if (runID.substr(0,12) == string("digitization")) runID = tokens[1];

  // Auto-generate output filenames
  string twrBayStr; //shared by all output filenames
  {
    ostringstream tmp;
    tmp << 'T' << setw(2) << setfill('0') << twrBay;
    twrBayStr = tmp.str();
  }
  if (outputXMLPath.length() == 0)
	outputXMLPath = outputDir + runID+ "_" + twrBayStr + "_" + moduleName + "_" + "cal_bias.xml";
  if (outputTXTPath.length() == 0)
    outputTXTPath = outputDir + "muTrig." + baseFilename +'.' + twrBayStr + ".txt";
  if (logfile.length() == 0)
    logfile = outputDir + "muTrig_logfile." + baseFilename + '.' + twrBayStr + ".txt";
  if (histFile.length() == 0)
    histFile = outputDir + "muTrigEff." + baseFilename + '.' + twrBayStr + ".root";

  if (pedFileTXT.length() == 0)
    pedFileTXT = outputDir + "mc_peds." + baseFilename + '.' + twrBayStr + ".txt";

  // setup output stream
  // add cout by default
  ostrm.getostreams().push_back(&cout);
  // add user Req logfile
  if (genLogfile) {
    logstr.open(logfile.c_str());
    ostrm.getostreams().push_back(&logstr);
  }
}

void MtCfg::clear() {  
}

void MtCfg::summarize() {
}
