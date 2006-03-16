#ifndef ldfReader_LATDATA_CXX
#define ldfReader_LATDATA_CXX

/** @file LatData.cxx
@brief Implementation of the LatData class

$Header$
*/

#include "ldfReader/data/LatData.h"

namespace ldfReader {

    LatData* LatData::m_instance = 0;


    LatData::LatData() {
        clearTowers(); 
    }

    LatData* LatData::instance() {
        if (m_instance == 0) {
            m_instance = new LatData();
        }
        return m_instance;
    }

    void LatData::print() const {
      const lsfData::RunInfo run = m_metaEvent.run();
      const lsfData::DatagramInfo dgm = m_metaEvent.datagram();
      const lsfData::GemScalers scalers = m_metaEvent.scalers();
      const lsfData::Time time = m_metaEvent.time();

      std::cout << "==========================================" 
                << std::endl << std::endl << "Event " 
                << scalers.sequence() << " context:" << std::endl;

      std::cout << time << scalers << run << dgm << std::endl
                << "Event " << scalers.sequence() << " info:" << std::endl
                << "---------------" << std::endl
                << "LSE_Info:  timeTicks = 0x" << std::uppercase << std::hex 
                << std::setfill('0') << std::setw(8) 
                << time.timeTicks() << " (" << std::dec 
                << time.timeTicks()
                << ")\nLSE_Info:  " << time.timeHack() << std::endl;
      const lsfData::Configuration *cfg = m_metaEvent.configuration();
      if (cfg) {
          const lsfData::LpaConfiguration* lpa = cfg->castToLpaConfig();
          if (lpa) {
          std::cout << "LPA_Info: softwareKey = 0x" << std::hex
                    << lpa->softwareKey() << std::endl
                    << "LPA_Info: hardwareKey = 0x" << lpa->hardwareKey() 
                    << std::dec << std::endl;
          }
      }

  }


    TowerData* LatData::getTower(unsigned int id) {
        if (m_towerMap.find(id) != m_towerMap.end()) {
            return m_towerMap[id];
        }
        return 0;
    }

    AcdDigi* LatData::getAcd(const char *name) {
        if (m_acdCol.find(name) != m_acdCol.end()) {
            return m_acdCol[name];
        }
        return 0;
    }

//   const AcdDigi* LatData::getAcd(const char *name) const {
//        if (m_acdCol.find(name) != m_acdCol.end()) {
//            return m_acdCol[name];
//        }
//        return 0;
//    }

    void LatData::clearTowers()
    {
        std::map<unsigned int, TowerData*>::iterator towerIter = m_towerMap.begin();
        while(towerIter != m_towerMap.end())
        {
            TowerData* tower = (towerIter++)->second;
            delete tower;
        }
        m_towerMap.clear();

        std::map<const char *, AcdDigi*>::iterator acdIter = m_acdCol.begin();
        while(acdIter != m_acdCol.end()) {
            AcdDigi *digi = (acdIter++)->second;
            delete digi;
        }
        m_acdCol.clear();
        m_gem.clear();
        m_osw.clear();
        m_aem.clear();
        m_flags = 0;
        m_eventSize = 0;
        m_metaEvent.clear();
        m_ccsds.clear();
        m_eventId = 0;
    }


    bool LatData::eventSeqConsistent() const {
    //Purpose and Method:  Checks that the event sequence number is consistent
    // accross all contributions.  If not false is returned... 
    // Assumes m_eventId for any errors messages

        unsigned long firstEvtSeq=0;
        bool foundFirst = false;

        if(getGem().exist()) {
            firstEvtSeq = getGem().summary().eventNumber();
            foundFirst = true;
        }

        if ( (getAem().exist()) && !foundFirst) {
            firstEvtSeq = getAem().summary().eventNumber(); 
            foundFirst = true;
        } else if ( (getAem().exist()) && (firstEvtSeq != getAem().summary().eventNumber()) ) {
            printf("AEM does not match event Seq %lu \n", getAem().summary().eventSequence());
            return false;
        }

        if ( (getOsw().exist()) && !foundFirst ){
            firstEvtSeq = getOsw().summary().eventNumber();
            foundFirst = true;
        } else if ( (getOsw().exist()) && (firstEvtSeq != getOsw().summary().eventNumber()) ){
            printf("OSW does not match event Seq %lu \n", getOsw().summary().eventSequence());
            return false;
        }

        std::map<unsigned int, TowerData*>::const_iterator towerIter = m_towerMap.begin();
        while(towerIter != m_towerMap.end())
        {
            TowerData* tower = (towerIter++)->second;
            const TemData tem = tower->getTem();
            if ( (tem.exist()) && !foundFirst) {
                firstEvtSeq = tem.summary().eventNumber();
                foundFirst = true;
            } else if ( (tem.exist()) && (firstEvtSeq != tem.summary().eventNumber())) {
                printf("TEM does not match event Seq %lu \n", tem.summary().eventSequence());
                return false;
            }
        }
        // all ok
        return true;
    }


    unsigned LatData::checkPacketError() {
        // Returns true if OR of all is TRUE
        // Returns false if OR of all packetError words is FALSE
        // Assumes m_eventId has been set for any error messages

        unsigned orAll = 0;

        if(getGem().exist()) {
            orAll |= getGem().packetError();
        }

        if (getAem().exist()) {
            orAll |= getAem().packetError();
         }

        if ( getOsw().exist()) {
            orAll |= getOsw().packetError();
         }

        std::map<unsigned int, TowerData*>::const_iterator towerIter = m_towerMap.begin();
        while(towerIter != m_towerMap.end())
        {
            TowerData* tower = (towerIter++)->second;
            const TemData tem = tower->getTem();
            if (tem.exist()){
                orAll |= tem.packetError();
            }
        }
        if (orAll != 0) {
            printf("Setting Packet Error Flag, Event: %u\n", m_eventId);
            setPacketErrorFlag();
        }
        return (orAll);
    }

    unsigned LatData::checkTemError() {
        // Assumes m_eventId has been set for any error messages

        unsigned orAll = 0;
        unsigned int temOrAll = 0;

        std::map<unsigned int, TowerData*>::const_iterator towerIter = m_towerMap.begin();
        while(towerIter != m_towerMap.end())
        {
            TowerData* tower = (towerIter++)->second;
            const TemData tem = tower->getTem();
            if (tem.exist()) {
                temOrAll |= tem.summary().error();
                //tem.summary().print();
                //printf("tem error %d\n", tem.summary().error());
                orAll |= tem.summary().error();
                //printf("orAll = %d\n", orAll);
            }
        }
        if ( getOsw().exist()) {
            orAll |= getOsw().summary().error();
            if (temOrAll != getOsw().summary().error()) 
                printf("OSW error summary bit does not match OR of all error bits across all TEM contributions, %d event Num: %u\n", m_eventId);
        }
        if (orAll != 0) {
            printf("Event Summary Error Flag Set, Event:  %u\n", m_eventId);
            setTemErrorFlag();
        }
        return (orAll);
    }


    unsigned LatData::checkTrgParityError() {
        // Assumes m_eventId has been set for any error messages

        unsigned orAll = 0;

        if(getGem().exist()) 
            orAll |= getGem().summary().trgParityError();

        if (getAem().exist()) 
            orAll |= getAem().summary().trgParityError();

        unsigned int temOrAll = 0;
        std::map<unsigned int, TowerData*>::const_iterator towerIter = m_towerMap.begin();
        while(towerIter != m_towerMap.end())
        {
            TowerData* tower = (towerIter++)->second;
            const TemData tem = tower->getTem();
            if (tem.exist()) {
                temOrAll |= tem.summary().trgParityError();
                //tem.summary().print();
                //printf("tem error %d\n", tem.summary().trgParityError());
                orAll |= tem.summary().trgParityError();
                //printf("orAll = %d\n", orAll);
            }
        }
        if ( getOsw().exist()) {
            orAll |= getOsw().summary().trgParityError();
            if (temOrAll != getOsw().summary().trgParityError()) 
                printf("OSW error summary bit does not match OR of all error bits across all TEM contributions, %d event Number: %u\n", getOsw().summary().trgParityError(), m_eventId);
        }
        if (orAll != 0) {
            printf("Trg Parity Error Flag Set, Event:  %u\n", m_eventId);
            setTrgParityErrorFlag();
        }
        return (orAll);
    }

}

#endif
