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
        TemData(const TemData& osw) { m_summary = osw.m_summary; };
        TemData(const EventSummaryCommon& summary) {m_summary = summary; };
        ~TemData() { clear(); };

        void clear() { m_summary.setSummary(0); };

        const EventSummaryCommon& summary() const { return m_summary; };
        void initSummary(unsigned summary) { m_summary.setSummary(summary);};

    private:

        // Store the event sequence number for this contribution
        EventSummaryCommon m_summary;

    };
} // end namespace
#endif
