#ifndef Event_Ebf_H
#define Event_Ebf_H

#include <iostream>

#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StreamBuffer.h"
#include "GaudiKernel/ContainedObject.h"
#include "GaudiKernel/ObjectVector.h"

#include "Event/TopLevel/Definitions.h"
#include "Event/TopLevel/EventModel.h"

/**
 * @class Ebf
 *
 * @brief TDS for storing an event in a format similar to ebf
 *
 * The data is stored as one continuos string of bytes
 * No attempt is made to verify that the data stored is correctly
 * formated ebf.
 * $Header$
 */

//extern const CLID& CLID_Ebf;

namespace EbfWriterTds{
    class Ebf : public DataObject{
    public:
        Ebf();
        Ebf(char *newData,unsigned int dataLength);
        virtual ~Ebf();

        ///Retrieve pointer to the ebf data.
        char *get(unsigned int &dataLength) const;

        ///Store the provided ebf pointer in and delete any previous ones
        void set(char *newData, unsigned int dataLength);

        virtual std::ostream& fillStream(std::ostream &s) const;
        friend std::ostream& operator << (std::ostream &s, const Ebf& obj);
    private:
        ///Pointer to the ebf data
        char *m_data;
        ///Number of bytes that are stored in data pointer
        unsigned int m_length;
    };
};
#endif
