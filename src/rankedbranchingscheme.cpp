/**
 * rankedbranchingscheme.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include "common.h"
#include "rankedbranchingscheme.h"
#include "nibacexception.h"
#include "node.h"
#include <set>

namespace vorpal::nibac {
    RankedBranchingScheme::RankedBranchingScheme(int pv)
            : v(pv), index(0) {
        rankvector = new int[v];
        for (int i = 0; i < v; ++i)
            rankvector[i] = v;
    }


    RankedBranchingScheme::~RankedBranchingScheme() {
        delete[] rankvector;
    }


    int RankedBranchingScheme::getBranchingVariableIndex(Node &n) {
        // Iterate over the free variables and determine the one with minimum value in
        // the rank vector.
        int min = v;
        int variable = -1;
        std::set<int> &freevars = n.getFreeVariables();
        std::set<int>::iterator beginIter = freevars.begin();
        std::set<int>::iterator endIter = freevars.end();
        for (; beginIter != endIter; ++beginIter)
            if (rankvector[*beginIter] < min) {
                variable = *beginIter;
                min = rankvector[variable];
            }

        if (variable > 0)
            return variable;

        // Otherwise, we can branch arbitrarily.
        variable = chooseBranchingVariableIndex(n);
        if (variable == -1 && !n.getFreeVariables().empty())
            throw UnexpectedResultException("Branching scheme chose no variable, but free variables exist");
        if (n.getFreeVariables().find(variable) == n.getFreeVariables().end())
            throw UnexpectedResultException("Branching scheme chose a free variable not in the node's free list");

        // Modify the rank vector and the index.
        assert(index < v);
        assert(rankvector[variable] == v);
        rankvector[variable] = index;
        ++index;

        return variable;
    }
};