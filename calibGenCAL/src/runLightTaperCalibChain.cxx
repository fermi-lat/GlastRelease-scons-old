#include <fstream>
#include <string>
#include "lightTaperCalib.h"

void parseFileNames(std::vector<std::string>& fileNames, 
		    const std::string& line) 
{
  std::string::size_type pos = 0;
  for( ; ; ) {

    string::size_type i = line.find(' ', pos);
    if(i != string::npos) {
      fileNames.push_back(line.substr(pos, i-pos));
    }
    else {

      std::string lastFile = line.substr(pos);

      //make sure it is a root file name
      if(lastFile.find("root") != string::npos) {
	fileNames.push_back(lastFile);
	break;
      }
      else {
	break;
      }

    }

    pos = i + 1;
  }
}

int main(int argn, char** argc) {

  std::ifstream inputFile;

  if(argn > 1) {
    inputFile.open(argc[1]);
  }
  else {
    inputFile.open("../src/lightTaperCalibChain_option.dat");
  }

  // first line is a list of recon root files, separated by " " 
  std::string line;
  getline(inputFile, line);

  std::vector<std::string> digiFileNames;

  parseFileNames(digiFileNames, line);

  TChain* digiChain = new TChain("Digi");

  for(std::vector<std::string>::const_iterator itr = digiFileNames.begin();
      itr != digiFileNames.end(); ++itr) {

    std::cout << "digi file: " << *itr << endl;

    digiChain->Add(itr->c_str());

  }

  // second line is a list of recon root files, separated by " "
  getline(inputFile, line);

  std::vector<std::string> reconFileNames;

  parseFileNames(reconFileNames, line);

  TChain* reconChain = new TChain("Recon");

  for(std::vector<std::string>::const_iterator itr = reconFileNames.begin();
      itr != reconFileNames.end(); ++itr) {

    std::cout << "recon file: " << *itr << endl;

    reconChain->Add(itr->c_str());

  }

  std::string txtFileName;
  inputFile >> txtFileName;

  std::string rootFileName;
  inputFile >> rootFileName;

  lightTaperCalib calib;

  calib.readPedestal("/nfs/farm/g/glast/u03/EM2003/rootFiles/v3r3p2/muon_ver/calib/muped__031005015905_031005040315.txt");

  calib.genLightTaper(digiChain, reconChain, txtFileName.c_str(), 
		      rootFileName.c_str());

}
