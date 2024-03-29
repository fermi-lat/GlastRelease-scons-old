// $Header$
#ifndef Event_McIntegratingHit_H
#define Event_McIntegratingHit_H 1

#include <iostream>
#include <vector>
#include <map>
#include <utility>
#include "CLHEP/Geometry/Point3D.h"
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/SmartRefVector.h"
#include "Event/TopLevel/Definitions.h"
#include "idents/VolumeIdentifier.h"
#include "Event/Utilities/CLHEPStreams.h"
#include "Event/Utilities/IDStreams.h"
// Include all Glast container types here
//   to simplify inlude statements in algorithms
#include "GaudiKernel/ObjectVector.h"
#include "GaudiKernel/ObjectList.h"

/** @class McIntegratingHit
 * @brief Monte Carlo integrating hits such as calorimeter
 *
 * @author:      OZAKI Masanobu 
 *
 * Changes:     M.Frank 04/10/1999 : Proper use of SmartRefs and SmartRefVectors
 *              P.Binko 19/10/1999 : Proper accessors of smart references,
 *                                   Formating of ASCII output
 *              M.Ozaki 2000-12-07 : Modified for GLAST
 *              M.Ozaki 2001-01-05 : MCIntegratingHits -> McIntegratingHit
 *              T.Usher 2010-06-01 : Add ability to keep track of end responses
 * $Header$
 */

#include "Event/MonteCarlo/McParticle.h"

#include "GaudiKernel/IInterface.h"

static const CLID& CLID_McIntegratingHit = InterfaceID("McIntegratingHit", 1, 0);

namespace Event {  // NameSpace

class McIntegratingHit : virtual public ContainedObject 
{
public:
    /// This enumerative is used to aid in the fill of the m_energyArray member
    enum Particle{primary,electron,positron,overlay};
   
    enum HitType {simulation = 0x40000000,
                  overlayHit = 0x80000000 
                 };

    virtual const CLID& clID() const   { return McIntegratingHit::classID(); }
    static const CLID& classID()       { return CLID_McIntegratingHit; }

    // Define sub class to contain energy information for each diode when in a crystal volume
    class XtalEnergyDep
    {
    public:
        XtalEnergyDep() : m_totalEnergy(0.), m_directEnergy(0.), m_moment1seed(0.,0.,0.), m_moment2seed(0.,0.,0.) {}
       ~XtalEnergyDep() {}

        /// Get total deposited energy seen by this diode
        double getTotalEnergy() const {return m_totalEnergy;}

        /// Get the direct deposited energy seen by this diode
        double getDirectEnergy() const {return m_directEnergy;}
    
        /// Retrieve the energy-weighted first moments of the position
        const HepPoint3D moment1() const;
              HepPoint3D moment1();
    
        /// Retrieve the energy-weighted second moments of the position
        const HepPoint3D moment2() const;
              HepPoint3D moment2();

        // add energy/position for this diode
        void addEnergyItem(double totalEnergy, double directEnergy, const HepPoint3D& position);

        // Initialization for reading and writing 
        void initialize(double totalEnergy, double directEnergy, const HepPoint3D& mom1, const HepPoint3D& mom2)
        {
            m_totalEnergy  = totalEnergy;
            m_directEnergy = directEnergy;
            m_moment1seed  = mom1;
            m_moment2seed  = mom2;
        }

    private:
        /// total deposited energy: set automatically when m_energyInfo is modified.
        double                   m_totalEnergy;
        /// "direct" energy seen by diode
        double                   m_directEnergy;
        /// Energy-weighted_first_moments_of_the_position * number_of_energy_deposition
        HepPoint3D               m_moment1seed;
        /// Energy-weighted_second_moments_of_the_position * number_of_energy_deposition
        HepPoint3D               m_moment2seed;
    };

    /// Define the map for the deposited energy seen by each diode
    typedef std::map<std::string, XtalEnergyDep> XtalEnergyDepMap;

    /// McParticle -> deposited energy map
    // Here I needed to takeout SmartRef<McParticle> because the 
    // template library was not giving a compile error.
    typedef std::vector< std::pair<McParticle*, double> > energyDepositMap;
    /// McParticleID -> deposited energy map
    typedef std::vector< std::pair<McParticle::StdHepId, double> > energyDepositMapId;

    McIntegratingHit() : m_volumeID(),
                         m_totalEnergy(0),
                         m_moment1seed(0.,0.,0.),
                         m_moment2seed(0.,0.,0.),
                         m_packedFlags(simulation)
    {
        m_energyArray[0]=m_energyArray[1]=m_energyArray[2]=m_energyArray[3]=0;
    }

    virtual ~McIntegratingHit(){}

    /// Retrieve cell identifier
    const idents::VolumeIdentifier volumeID() const;
    /// Update cell identifier
    void setVolumeID( idents::VolumeIdentifier value );

    /// Retrieve energy
    double totalEnergy() const;
    /// Retrieve the energy-weighted first moments of the position
    const HepPoint3D moment1 () const;
          HepPoint3D moment1 ();
    /// Retrieve the energy-weighted second moments of the position
    const HepPoint3D moment2 () const;
          HepPoint3D moment2 ();

    /// Retrieve itemized energy
    const energyDepositMap& itemizedEnergy() const;
    energyDepositMap& itemizedEnergy();
    /// Retrieve itemized energy with ID
    const energyDepositMapId& itemizedEnergyId() const;
    energyDepositMapId& itemizedEnergyId();
    /// Retrieve energy from array for minimal tree
    double energyArray( Particle p ) const;
    const double* energyArray() const { return m_energyArray; };

    /// Retrieve XtalEnergyDep object
    const XtalEnergyDep& getXtalEnergyDep(const std::string& key) const;
    XtalEnergyDep& getXtalEnergyDep(const std::string& key);

    /// Direct access to map for filling
    XtalEnergyDepMap& getXtalEnergyDepMap() {return m_xtalEnergyMap;}
   
    /// Update all energyInfos
    void setEnergyItems( const energyDepositMap& value );
    void setEnergyItems( double totalEnergy, const double *energyArr,
        const HepPoint3D &moment1, const HepPoint3D &moment2);

    /// Remove all energyInfos
    void clearEnergyItems();
    
    /// Add single energyInfo to energyDepositMap
    void addEnergyItem( const double& energy, McParticle* t, const HepPoint3D& position );
    void addEnergyItem( const double& energy, SmartRef<McParticle> t, const HepPoint3D& position );
    void addEnergyItem( double energy, Particle p, const HepPoint3D& position );

    /// Retrieve primary-origin flag
    bool primaryOrigin() const;
    /// Update primary-origin flag
    void setPrimaryOrigin( bool value );

    /// Retrieve whether this hit should be digitized
    bool needDigi() const;
    /// Update whether this hit should be digitized
    void setNeedDigi( bool value );

    /// Provide access to setting/retrieving the "packed flags"
    unsigned long getPackedFlags() const {return m_packedFlags;}
    void setPackedFlags(unsigned long flags) {m_packedFlags  = flags;}
    void addPackedMask(unsigned long mask)   {m_packedFlags |= mask;}

    /// Serialize the object for reading
    virtual StreamBuffer& serialize( StreamBuffer& s );
    /// Serialize the object for writing
    virtual StreamBuffer& serialize( StreamBuffer& s ) const;
    /// Fill the ASCII output stream
    virtual std::ostream& fillStream( std::ostream& s ) const;    
       
  private:
      /// Cell identifier
      idents::VolumeIdentifier m_volumeID;
      /// Vector of Energy information that consists of deposited energy and the mother McParticle
      energyDepositMap         m_energyItem;
      /// Vector of Energy information that consists of deposited energy and the mother McParticleId
      energyDepositMapId       m_energyItemId;
      /// total deposited energy: set automatically when m_energyInfo is modified.
      double                   m_totalEnergy;
      /// Energy-weighted_first_moments_of_the_position * number_of_energy_deposition
      HepPoint3D               m_moment1seed;
      /// Energy-weighted_second_moments_of_the_position * number_of_energy_deposition
      HepPoint3D               m_moment2seed;
      /// Packed flags for particle property
      unsigned long            m_packedFlags;
      /// This is an array that holds energy contribution to the hit from the
      /// primary particle, the electron and the postitron, and the overlay contribution
      double                   m_energyArray[4];
      /// The crystal energy deposit map
      XtalEnergyDepMap         m_xtalEnergyMap;
};

/// Serialize the object for writing
inline StreamBuffer& McIntegratingHit::serialize( StreamBuffer& s ) const
{
    ContainedObject::serialize(s);
    s
      << m_volumeID
      << m_totalEnergy
      << m_moment1seed
      << m_moment2seed
//    << m_energyItem(this) // The operator<< has not implemented yet. FIX ME!!
      << m_energyItem.size();
    energyDepositMap::const_iterator it;
    for (it = m_energyItem.begin(); it != m_energyItem.end(); it++){
        s << it->first
          << it->second;
    }
    return s
      << m_packedFlags;
}


/// Serialize the object for reading
inline StreamBuffer& McIntegratingHit::serialize( StreamBuffer& s )
{
    energyDepositMap::size_type m_energyItem_size;
    ContainedObject::serialize(s);
    s
      >> m_volumeID
      >> m_totalEnergy
      >> m_moment1seed
      >> m_moment2seed
//    >> m_energyItem(this) // The operator<< has not implemented yet. FIX ME!!
      >> m_energyItem_size;
    m_energyItem.clear();
    energyDepositMap::size_type i;
    for (i = 0; i < m_energyItem_size; i++){//for (i = 0; i < m_energyItem_size; i++){ //Nast�
        SmartRef<McParticle> first;
        double               second;
        s >> first(this)
          >> second;
        m_energyItem.push_back(std::pair<McParticle*, double>(first, second));
    }
        return s
      >> m_packedFlags;
}


  

/// Fill the ASCII output stream
inline std::ostream& McIntegratingHit::fillStream( std::ostream& s ) const
{
    s << "class McCaloHitBase :"
      << "\n    Deposited Energy        = "
      << EventFloatFormat( EventFormat::width, EventFormat::precision )
      << m_totalEnergy
      << "\n    First moment (x, y, z)  = "
      << EventFloatFormat( EventFormat::width, EventFormat::precision )
      << m_moment1seed.x() / m_totalEnergy << ", "
      << EventFloatFormat( EventFormat::width, EventFormat::precision )
      << m_moment1seed.y() / m_totalEnergy << ", "
      << EventFloatFormat( EventFormat::width, EventFormat::precision )
      << m_moment1seed.z() / m_totalEnergy << " )"
      << "\n    Second moment (x, y, z) = "
      << EventFloatFormat( EventFormat::width, EventFormat::precision )
      << m_moment2seed.x() / m_totalEnergy << ", "
      << EventFloatFormat( EventFormat::width, EventFormat::precision )
      << m_moment2seed.y() / m_totalEnergy << ", "
      << EventFloatFormat( EventFormat::width, EventFormat::precision )
      << m_moment2seed.z() / m_totalEnergy << " )"
      << "\n    Volume ID               = " << m_volumeID.name();
    return s;
}


// Definition of all container types of McIntegratingHit
//template <class TYPE> class ObjectVector;
typedef ObjectVector<McIntegratingHit>     McIntegratingHitVector;
typedef ObjectVector<McIntegratingHit>     McIntegratingHitCol;
//template <class TYPE> class ObjectList;
typedef ObjectList<McIntegratingHit>       McIntegratingHitList;


/*! A small class to use the sort algorithm */
class CompareIntHits {
  public:
    bool operator()(McIntegratingHit *left, McIntegratingHit *right)
    {return left->volumeID() < right->volumeID();}

    };
}

#endif // Event_McIntegratingHit_H
