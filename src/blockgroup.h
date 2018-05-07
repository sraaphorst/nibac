/**
 * blockgroup.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 *
 * This is a subgroup of SchreierSimsGroup, which, given the relevant parameters for
 * a t-(v, k, l) packing / covering / design, uses the action of the generators
 * {(1 2), (1 3), ..., (1 v)} of Sym(v) in order to create a Schreier-Sims
 * representation of the corresponding isomorphic subgroup of Sym(vCk) (v choose k)
 * that acts on the variables representing the k-sets of the design.
 *
 * TODO: Change this to (1 2), (1 2 3 ... v) for speed improvement.
 */

#ifndef BLOCKGROUP_H
#define BLOCKGROUP_H

#include <set>
#include "common.h"
#include "schreiersimsgroup.h"

namespace vorpal::nibac::design {
    class BlockGroup final : public SchreierSimsGroup {
    public:
        BlockGroup(int, int, int, int * = 0);

        BlockGroup(int, int, int, std::set<int> &, int * = 0);

#ifdef NOTWORKING
        // This constructor uses the reduced group. It requires t and
        // takes no hole.
        BlockGroup(int, int, int, int, int* = 0);
#endif

        virtual ~BlockGroup();

    protected:
        virtual void vertexPermutationToBlockPermutation(int, int, int, int *, int *);

        virtual void initializeGroup(int, int, int, std::set<int> &);

#ifdef NOTWORKING
        // TODO: Investigate this.
        // *** This was a very nice idea, but it does not work. Check the generation of
        // STS(13), of which the last two are isomorphic via the long cycle:
        // (0 1 11 9 4 2 8 5 12 6 10 7 3) to see this. ***

        // If the canonical fixings were performed, this creates the highly reduced
        // permutation group. Note that there cannot be any hole in this case.
        // The required parameters are t, v, k, lambda.
        virtual void initializeReducedGroup(int, int, int, int);
#endif
    };
};

#endif
