#ifndef ldfReader_TemData_H
#define ldfReader_TemData_H

#include <vector>

#include "EventSummaryCommon.h"

namespace ldfReader {

    /** @class TemData
      * @brief Local storage of TEM data
      * $Header$
    */
    class TemData {
    public:

        TemData() { clear(); };
        TemData(const TemData& tem) { 
            clear();
            m_summary = tem.m_summary; 
            m_lenInBytes = tem.m_lenInBytes; 
            m_calEnd = tem.m_calEnd;
            m_tkrEnd = tem.m_tkrEnd;
            m_exist = tem.m_exist; };
        TemData(const EventSummaryCommon& summary) {clear(); m_summary = summary; };
        ~TemData() { clear(); };

        void clear() { 
            m_summary.setSummary(0); 
            m_lenInBytes = 0; 
            m_exist = false; 
            m_packetError = 0;
            m_calEnd = 0;
            m_tkrEnd = 0;
        };

        void print() const { 
             if (!exist()) {
                 printf("No TEM\n");  
                 return;
             }
             printf("TemData:\n");
             printf("contribution length = %lu Bytes\n", m_lenInBytes);
             m_summary.print();
             printf("\n");
        };

        const EventSummaryCommon& summary() const { return m_summary; };
        void initSummary(unsigned summary) { m_summary.setSummary(summary);};

       void setExist() { m_exist = true; };
       bool exist() const { return m_exist; };

        void initLength(unsigned long length) { m_lenInBytes = length; };
        unsigned long lenInBytes() const { return m_lenInBytes; };

        void initPacketError(unsigned packetError) { m_packetError=packetError; };
        unsigned packetError() const { return m_packetError; };

        void calEnd(unsigned long i) { m_calEnd = i; };
        void tkrEnd(unsigned long i) { m_tkrEnd = i; }; 
 
        unsigned long calEnd() const { return m_calEnd; };
        unsigned long tkrEnd() const { return m_tkrEnd; };

    private:

        // Store the event sequence number for this contribution
        EventSummaryCommon m_summary;

        bool m_exist;

        unsigned m_packetError;

        unsigned long m_lenInBytes;

        unsigned long m_calEnd, m_tkrEnd;

    };
} // end namespace
#endif
