/**
 * block.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include <ostream>
#include <set>
#include "common.h"
#include "block.h"
#include "superduper.h"


namespace vorpal::nibac::design {
    Block::Block(int v, int k, int lexnum) {
        int *block = new int[k];
        SuperDuper::duper(v, k, lexnum, block);
        for (int i = 0; i < k; ++i)
            points.insert(block[i]);
        delete[] block;
    }


    Block::Block(int k, int *block) {
        for (int i = 0; i < k; ++i)
            points.insert(block[i]);
    }


    Block::~Block() {
    }


    const std::set<int> &Block::getPoints() const {
        return points;
    }


    std::ostream &operator<<(std::ostream &out, const Block &b) {
        auto &points = b.getPoints();
        auto beginIter = points.begin();
        auto endIter = points.end();

        out << "{";

        int counter = 0;
        auto bound = points.size() - 1;
        for (; beginIter != endIter; ++beginIter, ++counter) {
            out << *beginIter;
            if (counter < bound)
                out << ", ";
        }

        out << "}";

        return out;
    }
};