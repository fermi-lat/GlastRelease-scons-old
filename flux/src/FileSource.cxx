/**
 * @file FileSource.cxx
 * @brief Read in the incident particle properties from a file.
 * @author J. Chiang
 *
 * $Header$
 */

#include <cstdlib>
#include <fstream>
#include <map>

#include "facilities/Util.h"

#include "FileSource.h"

namespace {
   void readLines(std::string inputFile, 
                  std::vector<std::string> &lines,
                  const std::string &skip) {
      facilities::Util::expandEnvVar(&inputFile);
      std::ifstream file(inputFile.c_str());
      lines.clear();
      std::string line;
      while (std::getline(file, line, '\n')) {
         if (line != "" && line != " "             //skip (most) blank lines 
             && line.find_first_of(skip) != 0) {   //and commented lines
            lines.push_back(line);
         }
      }
   }
}

FileSource::FileSource(const std::string & params) 
   : m_launchDirection(0), m_launchPoint(0), m_currentLine(0),
     m_backOffDistance(2000) {
   std::map<std::string, std::string> pars;
   facilities::Util::keyValueTokenize(params, ", ", pars);
   
   std::string input_file = pars["input_file"];
   ::readLines(input_file, m_inputLines, "#");
   m_interval = 1./std::atof(pars["rate"].c_str());
   if (pars.count("backoff_distance")) {
      m_backOffDistance = std::atof(pars["backoff_distance"].c_str());
   }
}

float FileSource::operator() (float xi) {
   (void)(xi);
   return energy(0);
}

double FileSource::energy(double time) {
   (void)(time);
   if (m_currentLine < m_inputLines.size()) {
//      parseCurrentLine();
      m_currentLine++;
   } else {
      m_interval = 3e8; /// @todo Find a better way to handle the EOF.
   }
   return m_energy;
}

void FileSource::parseCurrentLine() {
   std::vector<std::string> tokens;
   facilities::Util::stringTokenize(m_inputLines.at(m_currentLine), 
                                    " \t", tokens);
   m_particleName = tokens.at(0);
   m_energy = std::atof(tokens.at(1).c_str());

// Starting point of incident particle in instrument coordinates.
   double x = std::atof(tokens.at(2).c_str());
   double y = std::atof(tokens.at(3).c_str());
   double z = std::atof(tokens.at(4).c_str());

// Direction cosines of incident particle.
   double cosx = std::atof(tokens.at(5).c_str());
   double cosy = std::atof(tokens.at(6).c_str());
   double cosz = std::atof(tokens.at(7).c_str());

   HepVector3D dir(cosx, cosy, cosz);
   m_launchDirection->setDir(dir);

   HepPoint3D starting_pt(x, y, z);
   m_launchPoint->setPoint(starting_pt - m_backOffDistance*dir);
}

const char * FileSource::particleName() const {
   return m_particleName.c_str();
}

double FileSource::interval(double time) {
   (void)(time);
   return m_interval;
}

LaunchDirection * FileSource::launchDirection() {
   return m_launchDirection;
}

LaunchPoint * FileSource::launchPoint() {
   return m_launchPoint;
}

void FileSource::FileLaunchDir::execute(double KE, double time) {
   (void)(KE);
   m_glastToGalactic = GPS::instance()->transformGlastToGalactic(time);
}

const HepVector3D & FileSource::FileLaunchDir::dir() const {
   return m_dir;
}

std::string FileSource::FileLaunchDir::title() const {
   return "FileSource";
}

const HepVector3D & FileSource::FileLaunchDir::skyDirection() const {
   static HepVector3D my_dir;
   my_dir = m_glastToGalactic*m_dir;
   return my_dir;
}

const HepPoint3D & FileSource::FileLaunchPoint::point() const {
   return m_pt;
}

std::string FileSource::FileLaunchPoint::title() const {
   return "FileLaunchPoint";
}
