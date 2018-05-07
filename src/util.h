/**
 * util.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef UTIL_H
#define UTIL_H

#include "common.h"

namespace vorpal::nibac {
    // Given a string consisting of ranges of the form:
    // 1. a ({a})
    // 2. -a ({0, ..., a})
    // 3. b- ({b, ..., end})
    // 4. a-b ({a, ..., b})
    // parse the ranges and populate the array, which consists of flags indicating
    // whether or not each element appeared in the range.
    int parseFlagsFromString(int, int *, const char *);

    // Given a comma separated list of integers, parse them and populate
    // the set with them.
    bool parseIntSetFromString(const char *, std::set<int> &);
};
#endif

