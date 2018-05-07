/**
 * By Sebastian Raaphorst, 2003 - 2018.
 *
 * Stores column information for an ILP matrix.
 */

#ifndef COLUMN_H
#define COLUMN_H

#include <map>
#include "common.h"

namespace vorpal::nibac {
    class Column final {
    private:
        std::map<unsigned long, int> colinfo;

    public:
        Column();

        virtual ~Column();

        void add(unsigned long, int);

        void remove(unsigned long);

        bool intersects(const Column &, int= 1) const;
    };
};
#endif
