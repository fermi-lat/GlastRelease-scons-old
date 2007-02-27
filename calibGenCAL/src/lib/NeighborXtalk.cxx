// $Header$

/** @file
    @author fewtrell
 */

// LOCAL INCLUDES
#include "NeighborXtalk.h"
#include "SplineUtil.h"

// GLAST INCLUDES
#include "CalUtil/CalDefs.h"
#include "CalUtil/CalVec.h"

// EXTLIB INCLUDES
#include "TFile.h"
#include "TNtuple.h"

// STD INCLUDES
#include <fstream>
#include <sstream>

using namespace std;
using namespace CalUtil;
using namespace SplineUtil;

const SplineUtil::Polyline *NeighborXtalk::getPts(CalUtil::RngIdx dest,
                                                  CalUtil::RngIdx source
) const
{
  XtalkMap::const_iterator xtalkIt = m_xtalkMap.find(dest);


  if (xtalkIt == m_xtalkMap.end())
    return 0;

  ChannelSplineMap::const_iterator it =
    xtalkIt->second.find(source);

  if (it == xtalkIt->second.end())
    return 0;

  return &(it->second);
}

SplineUtil::Polyline *NeighborXtalk::getPts(CalUtil::RngIdx dest,
                                            CalUtil::RngIdx source)
{
  XtalkMap::iterator xtalkIt = m_xtalkMap.find(dest);


  if (xtalkIt == m_xtalkMap.end())
    return 0;

  ChannelSplineMap::iterator it =
    xtalkIt->second.find(source);

  if (it == m_xtalkMap[dest].end())
    return 0;

  return &(it->second);
}

void NeighborXtalk::writeTXT(const std::string &filename) const {
  ofstream outfile(filename.c_str());

  if (!outfile.is_open()) {
    ostringstream tmp;
    tmp << __FILE__ << ":" << __LINE__ << " "
        << "ERROR! unable to open txtFile='" << filename << "'";
    throw runtime_error(tmp.str());
  }

  // output header info as comment
  outfile << "; destRngIdx srcRngIdx dac adc" << endl;

  outfile.precision(2);
  outfile.setf(ios_base::fixed);

  // loop through each destination channel
  for (RngIdx destIdx; destIdx.isValid(); destIdx++) {
    XtalkMap::const_iterator xtalkIt = m_xtalkMap.find(destIdx);
    if (xtalkIt == m_xtalkMap.end())
      continue;

    const ChannelSplineMap &destMap = xtalkIt->second;

    // loop through each registered source channel
    for (ChannelSplineMap::const_iterator it = destMap.begin();
         it != destMap.end();
         it++) {
      RngIdx srcIdx = it->first;
      const  Polyline &pLine = it->second;

      // loop through each point in polyline
      for (Polyline::const_iterator it = pLine.begin();
           it != pLine.end();
           it++) {
        const float &dac = it->first;
        const float &adc = it->second;

        outfile << destIdx.val() << " "
                << srcIdx.val() << " "
                << dac << " "
                << adc << " "
                << endl;
      }
    }
  }
}

void NeighborXtalk::setPoint(CalUtil::RngIdx dest,
                             CalUtil::RngIdx source,
                             float dac,
                             float adc) {
  // find all cross talk entries for given 'destination' channel
  XtalkMap::iterator xtalkIt = m_xtalkMap.find(dest);


  // create new destination map if needed
  if (xtalkIt == m_xtalkMap.end())
    xtalkIt = m_xtalkMap.insert(XtalkMap::value_type(dest, ChannelSplineMap())).first;

  // find curve for given source, destination pair.
  ChannelSplineMap::iterator chanIt =
    xtalkIt->second.find(source);

  // create new spline curve if needed
  if (chanIt == xtalkIt->second.end())
    chanIt = xtalkIt->second.insert(ChannelSplineMap::value_type(source, Polyline())).first;

  chanIt->second.push_back(Point2D(dac, adc));
}


void NeighborXtalk::writeTuple(const std::string &filename) const {
  // open new root file
  TFile rootFile(filename.c_str(),
	             "RECREATE",
				 "GLAST Cal Neighboring Crystal Cross-talk splines");
  
  // loop through each destination channel
  for (RngIdx destIdx; destIdx.isValid(); destIdx++) {
    XtalkMap::const_iterator xtalkIt = m_xtalkMap.find(destIdx);
    if (xtalkIt == m_xtalkMap.end())
      continue;

    const ChannelSplineMap &destMap = xtalkIt->second;

    // loop through each registered source channel
    for (ChannelSplineMap::const_iterator it = destMap.begin();
         it != destMap.end();
         it++) {
      RngIdx srcIdx = it->first;
      const  Polyline &pLine = it->second;
      
	  string tuple_name("neighbor_xtalk" + destIdx.toStr() + "_from_" +
		                srcIdx.toStr());

	  TNtuple *tuple = new TNtuple(tuple_name.c_str(),
		                         tuple_name.c_str(),
								 "dac:adc");

	  float tuple_data[2];

      // loop through each point in polyline
      for (Polyline::const_iterator it = pLine.begin();
           it != pLine.end();
           it++) {
        tuple_data[0] = it->first;
        tuple_data[1] = it->second;

        tuple->Fill(tuple_data);
      }
    }
  }

  rootFile.Write();
  rootFile.Close();
}
