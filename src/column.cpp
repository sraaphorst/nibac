/**
 * column.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include <map>
#include "common.h"
#include "column.h"

namespace vorpal::nibac {
    Column::Column() {
    }


    Column::~Column() {
    }


    void Column::add(unsigned long pos, int coeff) {
        colinfo[pos] = coeff;
    }


    void Column::remove(unsigned long pos) {
        colinfo.erase(pos);
    }


    bool Column::intersects(const Column &other, int coeff) const {
        // We iterate over the sorted keys to determine if they share anything in common.
        std::map<unsigned long, int>::const_iterator beginIter1 = colinfo.begin();
        std::map<unsigned long, int>::const_iterator endIter1 = colinfo.end();
        std::map<unsigned long, int>::const_iterator beginIter2 = other.colinfo.begin();
        std::map<unsigned long, int>::const_iterator endIter2 = other.colinfo.end();

        while (beginIter1 != endIter1 && beginIter2 != endIter2)
            if ((*beginIter1).first == (*beginIter2).first) {
                if ((*beginIter1).second == (*beginIter2).second)
                    return true;
                ++beginIter1;
                ++beginIter2;
            } else if ((*beginIter1).first < (*beginIter2).first)
                ++beginIter1;
            else
                ++beginIter2;

        return false;
    }
};