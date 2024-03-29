/**
 * @file TriRowBits.h
 * @brief header for class TriRowBits

 $Header$
 * @author Luis C. Reyes - lreyes@milkyway.gsfc.nasa.gov
*/

#ifndef TRIROWBITS_H
#define TRIROWBITS_H

#include <iostream>

#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StreamBuffer.h"
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/ObjectVector.h"
#include "GaudiKernel/IInterface.h"

#include "Event/TopLevel/Definitions.h"
#include "Event/TopLevel/EventModel.h"
#include <iostream>

/**
* @class TriRowBits
* @brief TDS for storing all the possible 3-in-a-row trigger conditions per tower
* author Luis C. Reyes   lreyes@milkyway.gsfc.nasa.gov
*/
 
//#define NUM_TWRS 16
static const unsigned NUM_TWRS=16;

static const CLID& CLID_TriRowBitsTds = InterfaceID("TriRowBitsTds", 1, 0);


namespace TriRowBitsTds{
      class TriRowBits : public DataObject{

      public:
      
        TriRowBits();
      	virtual ~TriRowBits();
	//! 3 in row combinations defined by the digi hits
      	unsigned int getDigiTriRowBits(const int tower);
	void setDigiTriRowBits(const int tower, unsigned int bitword);

	//! 3 in row combinations defined by the trigger requests (see TkrDiagnosticData)
      	unsigned int getTrgReqTriRowBits(const int tower);
	void setTrgReqTriRowBits(const int tower, unsigned int bitword);

	friend std::ostream& operator<<( std::ostream& s, const TriRowBits& obj ){return obj.fillStream(s);}
	std::ostream& fillStream( std::ostream& s ) const;

      private:

      	unsigned int m_DigiTriRowBits[NUM_TWRS];
      	unsigned int m_TrgReqTriRowBits[NUM_TWRS];

      };

      //! Initialize arrays
      inline TriRowBits::TriRowBits(){
          for(unsigned i=0; i<NUM_TWRS; i++)
	    {
	      m_DigiTriRowBits[i]=0;
	      m_TrgReqTriRowBits[i]=0;
	    }
      }

      inline TriRowBits::~TriRowBits(){
      }
     
      //! Retrieves the bitword for a given tower
      inline unsigned int TriRowBits::getDigiTriRowBits(const int tower){
              return m_DigiTriRowBits[tower];
      }

      //! Sets the value of the Bitword for a given tower
      inline void TriRowBits::setDigiTriRowBits(const int tower, unsigned int bitword){
              m_DigiTriRowBits[tower]=bitword;
      }

      //! Retrieves the bitword for a given tower
      inline unsigned int TriRowBits::getTrgReqTriRowBits(const int tower){
              return m_TrgReqTriRowBits[tower];
      }

      //! Sets the value of the Bitword for a given tower
      inline void TriRowBits::setTrgReqTriRowBits(const int tower, unsigned int bitword){
              m_TrgReqTriRowBits[tower]=bitword;
      }

      inline std::ostream& TriRowBits::fillStream( std::ostream& s ) const
	{
	  s <<"Tower   DigiTriRow   TrgReqTriRow \n";
	  for(unsigned i=0; i<NUM_TWRS; i++)
	    {
	      if(m_DigiTriRowBits[i]!=0 || m_TrgReqTriRowBits[i]!=0)
		s<<i<<" "<<m_DigiTriRowBits[i]<<" "<< m_TrgReqTriRowBits[i]<<"\n";
	    }
	  return s;
	}
}// namespace TriRowBitsTds

#endif

/** @page TriRowBitsdoc TriRowBits Documentation

@verbatim

author: Luis C. Reyes, lreyes@milkyway.gsfc.nasa.gov
date: 23 September 2004

The purpose of this code "TriRowBits.*" is to make available in TDS the output of the 3-in-a-row logic.
The logic itself takes place in "Trigger/TriggerAlg.cxx".

The idea is to look for a 3-fold coincidence between any 3 layers of a tower. Thus there are 16
possible combinations of the 3-in-a-row signal as shown below. The layers inherit the numbering
convention of the Digi Algorithm.

Combination		Coincidence of Layers
0		[y0, x0]	[y1,x1]		[y2,x2]
1		[y1, x1]	[y2,x2]		[y3,x3]
2		[y2, x2]	[y3,x3]		[y4,x4]
3		[y3, x3]	[y4,x4]		[y5,x5]
4		[y4, x4]	[y5,x5]		[y6,x6]
5		[y5, x5]	[y6,x6]		[y7,x7]
6		[y6, x6]	[y7,x7]		[y8,x8]
7		[y7, x7]	[y8,x8]		[y9,x9]
8		[y8, x8]	[y9,x9]		[y10,x10]
9		[y9, x9]	[y10,x10]	[y11,x11]
10		[y10, x10]	[y11,x11]	[y12,x12]
11		[y11, x11]	[y12,x12]	[y13,x13]
12		[y12, x12]	[y13,x13]	[y14,x14]
13		[y13, x13]	[y14,x14]	[y15,x15]
14		[y14, x14]	[y15,x15]	[y16,x16]
15		[y15, x15]	[y17,x16]	[y17,x17]

[a,b]:= a & b

When such 3-fold coincidence occurs for a given combination, the respective bit in the bitword takes 
a value of 1. The bit is equal to 0 when there is no such coincidence.

The 16 bits are arranged in the bitword from right to left.

Example:  Bitword = 2104 = 0000 1000 0011 1000

Only combinations 3, 4, 5 and 11 have a positive trigger.

Only layers 3, 4, 5, 6, 7, 11, 12, 13 have a hit

@endverbatim
*/


