/**
 * nibacexception.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include "common.h"
#include "nibacexception.h"
#include <ostream>

namespace vorpal::nibac {
    std::ostream &operator<<(std::ostream &out, const NIBACException &e) {
      out << e.what();
      return out;
    }
}
