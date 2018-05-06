// NIBACException.cpp
//
// By Sebastian Raaphorst, 2006.
//
// $Author$
// $Date$

#include "common.h"
#include "nibacexception.h"
#include <ostream>


NIBACException::NIBACException()
{
}


std::ostream &operator<<(std::ostream &out, const NIBACException &e)
{
  out << e.what();
  return out;
}

