#ifndef G4MEDIA_H
#define G4MEDIA_H
#include <vector>
#include <string>
#include "GlastSvc/GlastDetSvc/IMedia.h"

/**
 * @class G4Media
 * 
 * @brief Utility class to instantiate the Geant4 material table 
 *
 * This class instanciates the Geant4 material table from the xml
 * representation; this is done via the visitors mechanism provided by
 * detModel. To hide the concrete implementation of detModel, this is done with
 * methods provided by the abstracts GlastSvc interfaces. The G4Media implements
 * one of these interfaces.
 *
 * @author R.Giannitrapani
 * 
 * $Header$ 
 */
class G4Media : public IMedia {
public:

    /** Create a new simple material
        @param name of the material
        @param type type of the material
        @param params Parameters vector 
                      - (z,a) for element
                      - (z,a,density) for material
        @param symbol The symbol (if applicable)
    */
    virtual void addMaterial(std::string name, 
                             MediaType type,
                             const DoubleVector& params, std::string symbol);

    /** Create a new composite material out of a list of materials name
        @param name of the material
        @param type type of the composite (natoms or fractional)
        @param density The density of the composite
        @param components The list of components names
        @param qty The vector of quantities (it is interpreted as int or 
                   double depending on type)
    */
    virtual void addComposite(std::string name, 
                              CompositeType type,
                              double density,
                              const StringVector& components, 
                              const DoubleVector& qty);
    

};
#endif
