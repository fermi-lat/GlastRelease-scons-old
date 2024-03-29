#ifndef ldfReader_TKRLAYER_CXX
#define ldfReader_TKRLAYER_CXX

/** @file TkrLayer.cxx
@brief Implementation of the TkrLayer class

$Header$
*/


#include "ldfReader/data/TkrLayer.h"

namespace ldfReader {

    TkrLayer::TkrLayer() {
        clear();
    }

    TkrLayer::TkrLayer(unsigned short layerNum) {
        clear();
        m_layerNum = layerNum;
    }

    TkrLayer::~TkrLayer() {
        m_xstripsC0.clear();
        m_xstripsC1.clear();
        m_ystripsC0.clear();
        m_ystripsC1.clear();
    }

    void TkrLayer::clear() {
        m_xstripsC0.clear();
        m_xstripsC1.clear();
        m_ystripsC0.clear();
        m_ystripsC1.clear();
        m_xToT[0] = m_xToT[1] = 0;
        m_yToT[0] = m_yToT[1] = 0;
        m_layerNum = 0;
    }
}
#endif
