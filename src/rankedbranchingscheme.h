/**
 * rankedbranchingscheme.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef RANKEDBRANCHINGSCHEME_H
#define RANKEDBRANCHINGSCHEME_H

#include "branchingscheme.h"
#include "common.h"

namespace vorpal::nibac {
    class Node;

    /**
     * An abstract extension of BranchingScheme.
     * This performs the basics of Margot's ranked branching scheme, implementing the rank
     * vector and branching in the forced fashion when necessarily while leaving the choice
     * arbitrary when possible. When branching can be done freely, the abstract method
     * chooseBranchingVariableIndex is called and the rank vector is modified appropriately.
     */
    class RankedBranchingScheme : public BranchingScheme {
    private:
        int v;
        int index;
        int *rankvector;

    protected:
        // The constructor now requires the number of variables in the problem
        // in order to allocate the rank vector.
        RankedBranchingScheme(int);

    public:
        virtual ~RankedBranchingScheme();

        // This method is no longer virtual, as it cannot be overridden in
        // subclasses. In this way, any overridings of it will not be executed.
        int getBranchingVariableIndex(Node &);

    protected:
        // This method performs the selection of the next variable on which to branch.
        // This must be picked from the free variables at the node but may be done
        // arbitrarily, and it is used by getBranchingVariableIndex to establish the
        // extension to the rank vector.
        virtual int chooseBranchingVariableIndex(Node &) = 0;
    };
};
#endif
