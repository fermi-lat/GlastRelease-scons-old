#ifndef MONTECARLOFILLER_H
#define MONTECARLOFILLER_H
#include <vector>
#include <string>

#include "HepRepSvc/IFiller.h"

class IGlastDetSvc;
class IDataProviderSvc;
class IParticlePropertySvc;

/** 
 *  @class MonteCarloFiller
 *
 *  @brief 
 *
 *  @author R.Giannitrapani, M.Frailis
 */

class MonteCarloFiller: public IFiller{
  
 public:
  MonteCarloFiller(IGlastDetSvc* gsvc,
                   IDataProviderSvc* dpsvc,
                   IParticlePropertySvc* ppsvc):
    m_gdsvc(gsvc),m_dpsvc(dpsvc),m_ppsvc(ppsvc){};

  /// This method init the type tree
  virtual void buildTypes ();
  /// This method fill the instance tree, using the string vector to decide
  /// which subinstances to fill
  virtual void fillInstances (std::vector<std::string>&);

  bool hasType(std::vector<std::string>& list, std::string type); 

 private:
  IGlastDetSvc* m_gdsvc;
  IDataProviderSvc* m_dpsvc;
  IParticlePropertySvc* m_ppsvc;
};

#endif //MONTECARLOFILLER_H
