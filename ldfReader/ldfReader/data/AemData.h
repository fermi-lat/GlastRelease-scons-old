#ifndef ldfReader_AemData_H
#define ldfReader_AemData_H

#include <vector>

#include "EventSummaryCommon.h"

namespace ldfReader {

    /** @class AemData
      * @brief Local storage of AEM data
      * $Header$
    */
    class AemData {
    public:

        AemData() { clear(); };
        AemData(const AemData& aem) { 
           m_summary = aem.m_summary; 
           m_lenInBytes = aem.m_lenInBytes; 
           m_exist = aem.m_exist; };
        AemData(const EventSummaryCommon &summary) { m_summary = summary; };
        ~AemData() { clear(); };

        void clear() { 
            m_summary.setSummary(0); 
            m_lenInBytes = 0; 
            m_exist = false;};

        const EventSummaryCommon& summary() const { return m_summary; };
        void initSummary(unsigned summary) { m_summary.setSummary(summary);};

       void setExist() { m_exist = true; };
       bool exist() const { return m_exist; };

        void initLength(unsigned long length) { m_lenInBytes = length; };
        unsigned long lenInBytes() const { return m_lenInBytes; };

    private:

        // Store the event sequence number for this contribution
        EventSummaryCommon m_summary;
        bool m_exist;

        unsigned long m_lenInBytes;

    };
} // end namespace
#endif
