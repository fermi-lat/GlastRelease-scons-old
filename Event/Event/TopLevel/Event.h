#ifndef Event_EVENT_H
#define Event_EVENT_H 1

#include <iostream>
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StreamBuffer.h"
#include "GaudiKernel/DataObject.h"
#include "Event/Utilities/TimeStamp.h"
#include "Event/TopLevel/Definitions.h"

//static const CLID& CLID_Event = InterfaceID("Event", 2, 0);
//The following number is currently hardwired into Gaudi (v12r0)!!
static const CLID& CLID_Event = 110;

/** @class EventHeader
* @brief Essential header information of the event.
* It can be identified by "/Event" on the TDS.
*
* It contains:
* - run number
* - event number
* - time stamp
* - trigger word
*
* $Header$
*/
namespace Event{

class EventHeader : public DataObject                                                {
    
public:
    /// default constructor: flag all fields invalic
    EventHeader()
        : DataObject(), m_event(0), m_run(-1), m_time(-1),  m_trigger((unsigned int)-1),
        m_triggerWordTwo((unsigned int)-1) { }
    
    virtual ~EventHeader() { }
    
    /// Retrieve reference to class definition structure
    virtual const CLID& clID() const { return EventHeader::classID(); }
    static const CLID& classID() { return CLID_Event; }
    
    /// Retrieve event number
    unsigned long event () const                                        { return m_event; }
    /// Update event number
    void setEvent (unsigned long value)                                { m_event = value; }
    
    /// Retrieve run number
    long run () const                                            { return m_run; }
    /// Update run number
    void setRun (long value)                                    { m_run = value; }
    
    /// Retrieve reference to event time stamp
    const TimeStamp& time () const                              { return m_time; }
    /// Update reference to event time stamp
    void setTime (const TimeStamp& value)                      { m_time = value; }

    /// Retrieve trigger word
    unsigned int trigger()const                                { return m_trigger;}
    /// update trigger word
    void setTrigger(unsigned int value)                         {m_trigger = value;}

    /// Retrieve second trigger word
    unsigned int triggerWordTwo() const                         {return m_triggerWordTwo; }
    /// update second trigger word
    void setTriggerWordTwo(unsigned int value)                  { m_triggerWordTwo = value; }
    
 
    /// Retrieve live time
    double livetime()const                                { return m_livetime;}
    /// update live time
    void setLivetime(double value)                         {m_livetime = value;}
    
    /// Serialize the object for writing
    virtual StreamBuffer& serialize( StreamBuffer& s ) const;
    /// Serialize the object for reading
    virtual StreamBuffer& serialize( StreamBuffer& s );
    
    /// Output operator (ASCII)
    friend std::ostream& operator<< ( std::ostream& s, const EventHeader& obj )        {
        return obj.fillStream(s);
    }
    /// Fill the output stream (ASCII)
    virtual std::ostream& fillStream( std::ostream& s ) const;
    
private:
    /// Event number
    unsigned long       m_event;
    /// Run number
    long                m_run;
    /// Time stamp: use special class to encapsulate type
    TimeStamp           m_time;
    /// trigger word: note that a class is available to use for this (THB: do we need this?)
    unsigned int        m_trigger;
    /// store additional bits, now that we're saving the trigger engines for both GEM and GLT
    unsigned int        m_triggerWordTwo;

    /// live time
    double              m_livetime;
};


//
// Inline code must be outside the class definition
//


/// Serialize the object for writing
inline StreamBuffer& EventHeader::serialize( StreamBuffer& s ) const                 {
    DataObject::serialize(s);
    // HMK The member variables are not filled yet
    return s;
    //<< m_event
    //<< m_run
    //<< m_time;
}


/// Serialize the object for reading
inline StreamBuffer& EventHeader::serialize( StreamBuffer& s )                       {
    DataObject::serialize(s);
    // HMK The member variables are not filled yet
    return s;
    //>> m_event
    //>> m_run
    //>> m_time;
}


/// Fill the output stream (ASCII)
inline std::ostream& EventHeader::fillStream( std::ostream& s ) const                {
    return s
        << "class Event :"
        << "\n    Event number = "
        << EventField( EventFormat::field12 )
        << m_event
        << "\n    Run number   = "
        << EventField( EventFormat::field12 )
        << m_run
        << "\n    Time         = " << m_time
        << std::setbase(16) << "\n    Trigger      = " << m_trigger
        << "\n    TriggerWordTwo      = " << m_triggerWordTwo;
}
} // namespace Event
#endif    // Event_EVENT_H
