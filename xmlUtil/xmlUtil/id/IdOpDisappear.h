// $Header$

#ifndef XMLUTIL_IDOPDISAPPEAR_H
#define XMLUTIL_IDOPDISAPPEAR_H

#include "xmlUtil/id/IdOperation.h"

namespace xmlUtil {
  //! Simplest possible operation, taking any id to null
  class IdOpDisappear : public IdOperation {
  public:
    IdOpDisappear(DOM_Element) {}
    ~IdOpDisappear() {}
    NamedId * convert(const NamedId& inputId) {return 0;}
  };
}    


#endif

