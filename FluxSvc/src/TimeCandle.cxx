// $Header$


#include "TimeCandle.h"

#include "FluxException.h" // for FATAL_MACRO
#include <utility>
#include <strstream>
#include <cmath>
#include "SpectrumFactory.h"

static SpectrumFactory<TimeCandle> factory;
const ISpectrumFactory& TimeCandleFactory = factory;

TimeCandle::TimeCandle()
:m_name("TimeTick")
,m_T0(30.)
{}//default constructor
TimeCandle::TimeCandle(const std::string& params)
:m_name("TimeTick")
,m_T0(parseParamList(params,0))
{}



std::string TimeCandle::title()const
{
    std::strstream s;
    s << particleName() << '(' << 1 << " GeV";
    s << ")" << '\0';
    std::string t(s.str()); s.freeze(false);
    return t;
}

float
TimeCandle::operator()(float f)const
{
    return 1.;
}

const char*
TimeCandle::particleName() const
{
    return m_name.c_str();
}

float TimeCandle::parseParamList(std::string input, int index)
{
    std::vector<float> output;
    int i=0;
    for(;!input.empty() && i!=std::string::npos;){
        float f = ::atof( input.c_str() );
        output.push_back(f);
        i=input.find_first_of(",");
        input= input.substr(i+1);
    } 
    return output[index];
}