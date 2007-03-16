#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <stdexcept>
#include <algorithm>

#include "eventFile/LSEReader.h"
#include "eventFile/LSEWriter.h"
#include "eventFile/LSE_Context.h"
#include "eventFile/LSE_Info.h"
#include "eventFile/EBF_Data.h"

struct EvtIdx {
  std::string tag;
  unsigned startedAt;
  unsigned long long sequence;
  unsigned long long elapsed;
  unsigned long long livetime;
  double evtUTC;
  unsigned scid;
  unsigned apid;
  unsigned datagrams;
  unsigned dgmevt;
  std::string oaction;
  std::string caction;
  double dgmUTC;
  off_t fileofst;
  std::string evtfile;

  EvtIdx( const std::string& idxline ) {
    // make the text line into an input stream
    std::istringstream iss( idxline );

    // extract the components
    iss >> tag;
    iss >> startedAt;
    iss >> sequence;
    iss >> elapsed;
    iss >> livetime;
    iss >> evtUTC;
    iss >> scid;
    iss >> apid;
    iss >> datagrams;
    iss >> dgmevt;
    iss >> oaction;
    iss >> caction;
    iss >> dgmUTC;
    iss >> fileofst;
    iss >> evtfile;
  };
};

typedef std::map< std::string, eventFile::LSEReader* > lser_map;
typedef lser_map::iterator lser_iter;

struct cleanup {
  void operator() ( lser_map::value_type x ) { delete x.second; }
};

int main( int argc, char* argv[] )
{
  // get the input and output file names
  if ( argc < 4 ) {
    std::cout << "writeMerge: not enough input arguments" << std::endl;
    return 1;
  }
  std::string idxfile( argv[1] );
  std::string evtfile( argv[2] );  // this contains format specifiers
  int downlinkID = atoi( argv[3] );

  // add support for configurable output file size
  int maxEvents = -1;
  if ( argc >= 5 ) {
    maxEvents = atoi( argv[4] );
  }

  // declare the file-output object pointer
  int eventsOut = 0;
  eventFile::LSEWriter* pLSEW = NULL;

  // create a container for the chunk-evt input files
  lser_map mapLSER;

  // declare object to receive the event information
  eventFile::LSE_Context ctx;
  eventFile::EBF_Data    ebf;
  eventFile::LSE_Info::InfoType infotype;
  eventFile::LPA_Info    pinfo;
  eventFile::LCI_ACD_Info ainfo;
  eventFile::LCI_CAL_Info cinfo;
  eventFile::LCI_TKR_Info tinfo;

  // read the index file and parse the entries, retrieving the 
  // requeseted events as we go
  std::string idxline;
  std::ifstream idx( idxfile.c_str() );
  while ( getline( idx, idxline ) ) {
    
    // skip non-event records
    if ( idxline.find( "EVT:" ) != 0 ) continue;

    // make an event-index object
    EvtIdx edx( idxline );

    // get an LSEReader for the file
    lser_iter it = mapLSER.find( edx.evtfile );
    if ( it == mapLSER.end() ) {
      it = mapLSER.insert( std::pair< std::string, eventFile::LSEReader* >( edx.evtfile, new eventFile::LSEReader( edx.evtfile ) ) ).first;
    }

    // read the event at the specified location
    bool bevtread = false;
    try {
      it->second->seek( edx.fileofst );
      bevtread = it->second->read( ctx, ebf, infotype, pinfo, ainfo, cinfo, tinfo );
    } catch( std::runtime_error e ) {
      std::cout << e.what() << std::endl;
      continue;
    }
    if ( !bevtread ) {
      std::cout << "no event read from " << edx.fileofst << " of " << edx.evtfile << std::endl;
      continue;
    }

    // open an output file if necessary
    if ( !pLSEW ) {
      // create the output filename from the user-supplied template
      char ofn[512];
      snprintf( ofn, 512, evtfile.c_str(), ctx.run.startedAt, ctx.scalers.sequence );

      // open the output file
      pLSEW = new eventFile::LSEWriter( std::string( ofn ), downlinkID );
      std::cout << "created output file " << pLSEW->name() << std::endl;
    }

    // write the event to the merged file
    switch( infotype ) {
    case eventFile::LSE_Info::LPA:
      pLSEW->write( ctx, ebf, pinfo );
      break;
    case eventFile::LSE_Info::LCI_ACD:
      pLSEW->write( ctx, ebf, ainfo );
      break;
    case eventFile::LSE_Info::LCI_CAL:
      pLSEW->write( ctx, ebf, cinfo );
      break;
    case eventFile::LSE_Info::LCI_TKR:
      pLSEW->write( ctx, ebf, tinfo );
      break;
    default:
      std::cout << "unknown LSE_Info type " << infotype;
      std::cout << " at offset "            << edx.fileofst;
      std::cout << " in "                   << edx.evtfile;
      std::cout << std::endl;
      break;
    }

    // check to see if the output file is full
    if ( maxEvents > 0 && ++eventsOut >= maxEvents ) {
      // close the current file and reset the event counter
      std::cout << "wrote " << pLSEW->evtcnt() << " events to " << pLSEW->name() << std::endl;
      delete pLSEW; pLSEW = NULL;
      eventsOut = 0;
    }
  }
  std::for_each( mapLSER.begin(), mapLSER.end(), cleanup() );
  idx.close();
  if ( pLSEW ) {
    std::cout << "wrote " << pLSEW->evtcnt() << " events to " << pLSEW->name() << std::endl;
    delete pLSEW;
  }

  // all done
  return 0;
}
