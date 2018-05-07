/**
 * block.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 *
 * A convenience class that uses a set of integers to represent a collection of points.
 */

#ifndef BLOCK_H
#define BLOCK_H

#include <set>
#include <ostream>
#include "common.h"

namespace vorpal::nibac::design {
    class Block final {
    private:
        std::set<int> points;

    public:
        // Given v, k, and a lex number, create the block.
        Block(int, int, int);

        // Given k and an array of size k, create the block.
        Block(int, int *);

        // Destructor
        ~Block();

        // Accessor to the points.
        const std::set<int> &getPoints() const;
    };


    // Printing routine.
    std::ostream &operator<<(std::ostream &, const Block &);
};

#endif
