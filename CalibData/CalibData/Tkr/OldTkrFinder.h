//$Header$
#ifndef CalibData_OldTkrFinder_h
#define CalibData_OldTkrFinder_h

/** @class OldTkrFinder

   Utility class which knows how to find the index of the
   correct set of TKR calibration constants given id and (sometimes) 
   FE chip number.  Note convention for tower numbering follows
   (Ritz, Nordby) LAT-SS-00035-03-D1.  For the standard 4x4 array,
   this yields

                     12   13   14   15  | r3
                                        |
                      8    9   10   11  | r2
                                        |           ^
                      4    5    6    7  | r1        |
                                        |           |
                      0    1    2    3  | r0        
                                        |           Y
                     -------------------+
                     c0   c1   c2   c3              X -->

                            

   @author J. Bogart

*/
#include "idents/TkrId.h"

namespace CalibData {
  class OldTkrFinder {
  public: 
    /**
       Constructor. Input defines 
     */
    OldTkrFinder(unsigned nTowerRow, unsigned nTowerCol, unsigned nTray = 19)
      : m_towerRow(nTowerRow), 
        m_towerCol(nTowerCol), 
        m_unilayer(2*nTray)
    {init();}

    ~OldTkrFinder() {}


    unsigned findIx(unsigned towerRow, unsigned towerCol, unsigned tray,
                    bool top)  const; 

    unsigned findIx(const idents::TkrId& id) const;

    unsigned getSize() const {return m_unilayer*m_tower;} 

    unsigned getNTowerRow() const {return m_towerRow;}
    unsigned getNTowerCol() const {return m_towerCol;}

    unsigned getNUnilayer() const {return m_unilayer;}
    //    unsigned getNChip() const {return m_feChip;}


    bool equals(const OldTkrFinder& other) const;

  private:
    OldTkrFinder() : m_towerRow(4), m_towerCol(4), m_unilayer(38)
                   {init();}  // uses all defaults
    unsigned m_towerRow;
    unsigned m_towerCol;
    unsigned m_tower;
    unsigned m_unilayer;
    //    unsigned m_feChip;
    void     init();  // computes m_tower, m_c0, m_c1
    //    unsigned m_dacCol;

    //    unsigned m_c0;
    //    unsigned m_c1;

  };
}    



#endif
