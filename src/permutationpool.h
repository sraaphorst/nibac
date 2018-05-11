/**
 * permutationpool.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef PERMUTATIONPOOL_H
#define PERMUTATIONPOOL_H

#include "common.h"

namespace vorpal::nibac {
    /** This class basically stores a pool of pre-allocated permutations.
     * The reason for this is to avoid continuously allocating memory for integer
     * arrays using new and delete, which is highly inefficient. In this way, we
     * can simply reuse old permutations. The pool grows as needed.
     */
    class PermutationPool final {
    private:
        // Static factory.
        static PermutationPool *staticpool;

#ifndef NOPERMPOOL
        int **permpool;
        int freetop;
        int poolsize;
        double increasefactor;
#endif

        // Size of permutations.
        int permsize;

#ifdef NOPERMPOOL
        PermutationPool(int);
#else

        PermutationPool(int, int, double);

#endif

        virtual ~PermutationPool();

    public:
#ifndef NOPERMPOOL
        static void createPool(int, int = 10000, double = 0.5);
#else
        static void createPool(int);
#endif

        static void deletePool();

        static PermutationPool *getPool();

#ifdef NOPERMPOOL
        inline int *newPermutation() { return new int[permsize]; }
#else

        int *newPermutation();

#endif

#ifdef NOPERMPOOL
        inline void freePermutation(int *p) { delete[] p; }
#else

        void freePermutation(int *);

#endif
    };
};
#endif
