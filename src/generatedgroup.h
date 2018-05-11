/**
 * generatedgroup.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef GENERATEDGROUP_H
#define GENERATEDGROUP_H

#include "common.h"
#include "formulation.h"
#include "group.h"

namespace vorpal::nibac {
    /**
     *  A group that is not as easily determined as others; e.g. not the symmetric group.
     *  Instead, this is a group that is generated by a number of permutations. This is an
     *  abstract class that contains the functionality needed to calculate symmetry groups.
     *  The Schreier-Sims scheme is an example of a concrete subclass of this class.
     */
    class GeneratedGroup : public Group {
    public:
        GeneratedGroup() = default;
        virtual ~GeneratedGroup() = default;

        virtual void enter(int *) = 0;

        // Default technique to calculate a symmetry group from an ILP, i.e. the nauty technique.
        void createSymmetryGroup(Formulation &);

        // Naive algorithm for finding the symmetry group of an ILP.
        void findSymmetryGroup1(Formulation &);

        // Backtracking / partitioning algorithm for finding the symmetry group of an ILP.
        void findSymmetryGroup2(Formulation &);

        // nauty algorithm for finding the symmetry group of an ILP.
        void findSymmetryGroup3(Formulation &);

    private:
        // Given a size of a base set n, generate the first (identity) permutation of S_n.
        static void firstSnPerm(int, int *);

        // Given a size of a base set n, a permutation, and a workspace of size n, get the
        // next permutation (in lex. order). This method returns false when there are no
        // more permutations to iterate through.
        static bool nextSnPerm(int, int *, int *);
    };
};

#endif
