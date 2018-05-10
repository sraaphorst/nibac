/**
 * node.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef NODE_H
#define NODE_H

#include <map>
#include <set>
#include "common.h"
#include "formulation.h"
#include "group.h"

namespace vorpal::nibac {
    class NodeStack;
    class BAC;

    /**
     * Represents all data in a single node of the ILP branch and cut tree.
     */
    class Node final {
        // Friend class declarations.
        friend class NodeStack;

    private:
#ifdef NODEGROUPS
        // If this is the root node, we do not destroy its symmetry group upon backtracking.
        bool rootNodeFlag;
#endif

        // The group associated with this node. This can be null.
        Group *group;

        // The BAC algorithm to which this problem belongs.
        BAC &bac;

        // Formulation of the problem.
        Formulation &formulation;

        // Has this node been processed by the B&C yet?
        bool processedFlag;

        // The depth of the node in the search tree.
        int depth;

        // Information as to how we got here: the variable which
        // was branched upon and the value chosen.
        int branchVariableIndex;
        int branchVariableValue;

        // Information on the children of this node: the variable
        // on which to branch, and the next value to try.
        int branchingVariableIndex;
        int nextBranchingVariableValue;

        // Can this node be maximal?
        bool possiblyMaximalFlag;

        // This is MargotBAC specific, but we need it for turning
        // isomorphism testing on and off. This flag indicates whether
        // all ancestor nodes have been tested for isomorphism. If they
        // have, we can use a much more efficient canonicity test.
        bool ancestorsCanonicalFlag;

        // This is also MargotBAC specific, but we need it in the same way
        // as we do the above. This flag indicates whether or not this node
        // is canonical. If we branch on 0, this node is automatically
        // marked as being canonical if the parent is canonical. The root node
        // is always canonical (if not, there's a problem).
        bool isCanonicalFlag;

        // This is a flag that is used in dynamic canonicity testing. If, for a given
        // node, we don't have enough fixings or some other criterion is not satisfied,
        // this flag can be turned off and we will not proceed to further check this node
        // or its descendents for canonicity unless explicitly told to do so by canonicity
        // flags in MargotBACOptions.
        bool testCanonicityFlag;

        // Information about the number of variables fixed at this node.
        // NOTE: Apart from managing the partial solution, we do not need to
        //       know anything about non-branching variables, so this information
        //       is used only internally.
        int numberVariables;
        int numberBranchingVariables;
        int numberFixedVariables;
        int number0FixedVariables;

        // Information about the solving of the LP relaxation.
        int numberLPSolves;
        double solutionValue;
        double *solutionVariableArray;

        // The partial solution up to this point.
        // -1 indicates a free variable.
        // 0 indicates a variable fixed to 0.
        // 1 indicates a variable fixed to 1.
        // Information is needed in this format for quick feasibility
        // testing, and will prevent the need for us to reconstruct this
        // array for each node using the fixed and free lists.
        short int *partialSolutionArray;

        // The cuts associated with this node.
        std::set<Constraint *> cuts;

        // The cuts that we removed from this node, which should
        // be readded when we backtrack to the parent node.
        std::set<Constraint *> removedCuts;

        // The free variables at this node.
        std::set<int> freeVariables;

        // The variables that we fixed at this node.
        std::set<int> fixedVariables;

        // A lookup and reverse lookup map that determines the order on the free variables
        // at the node. freeVariableToIndex maps a variable to its index in the ordering, and
        // indexToFreeVariable performs the reverse. Note that the index will not be contiguous,
        // i.e. it will correspond to the original ordering according to the variable order
        // that is chosen and will not be modified apart from removing fixed entries from the maps.
        std::map<int, int> freeVariableToIndex;
        std::map<int, int> indexToFreeVariable;

    public:
        Node(BAC &, Formulation &, Group *, int, int, const std::set<int> * = 0, const std::set<int> * = 0);

        Node(Node *, int, int);

        virtual ~Node();

        void cleanup();

        // Get the number of LPs solved at this node, or report an attempt to solve one.
        inline int getNumberLPsSolved() const { return numberLPSolves; }

        inline void reportLPSolved() { ++numberLPSolves; }

        inline void setSolutionValue(double psolutionValue) { solutionValue = psolutionValue; }

        inline double getSolutionValue() const { return solutionValue; }

        inline int getDepth() const { return depth; }

        inline int getBranchVariableIndex() const { return branchVariableIndex; }

        inline int getBranchVariableValue() const { return branchVariableValue; }

        inline bool possiblyMaximal() const { return possiblyMaximalFlag; }

        inline bool ancestorsCanonical() const { return ancestorsCanonicalFlag; }

        inline void setAncestorsCanonical(bool pancestorsCanonicalFlag) {
            ancestorsCanonicalFlag = pancestorsCanonicalFlag;
        }

        inline bool isCanonical() const { return isCanonicalFlag; }

        inline void setCanonical(bool pisCanonicalFlag) { isCanonicalFlag = pisCanonicalFlag; }

        inline bool shouldTestCanonicity() const { return testCanonicityFlag; }

        inline void setTestCanonicity(bool ptestCanonicityFlag) { testCanonicityFlag = ptestCanonicityFlag; }

        // As mentioned above, we only consider BRANCHING variables here.
        inline int getNumberBranchingVariables() const { return numberBranchingVariables; }

        // TODO: Do we need this?
        inline void setNumberFixedVariables(int pnumberFixedVariables) { numberFixedVariables = pnumberFixedVariables; }

        inline int getNumberFixedVariables() const { return numberFixedVariables; }

        // TODO: Do we need this?
        inline void
        setNumber0FixedVariables(int pnumber0FixedVariables) { number0FixedVariables = pnumber0FixedVariables; }

        inline int getNumber0FixedVariables() const { return number0FixedVariables; }

        inline double *getSolutionVariableArray() { return solutionVariableArray; }

        inline short int *getPartialSolutionArray() { return partialSolutionArray; }

        // TODO: These should probably be const!
        inline std::set<int> &getFreeVariables() { return freeVariables; }

        inline std::set<int> &getFixedVariables() { return fixedVariables; }

        // Fix a variable in the most basic way.
        inline void fixVariable(int variable, int value) {
            freeVariables.erase(variable);
            fixedVariables.insert(variable);
            ++numberFixedVariables;
            if (value == 0)
                ++number0FixedVariables;
            partialSolutionArray[variable] = value;
            formulation.fixVariable(variable, value);

            // Remove the entries from the lookup and reverse lookup maps.
            std::map<int, int>::iterator iter = freeVariableToIndex.find(variable);
            assert(iter != freeVariableToIndex.end());
            assert(indexToFreeVariable.find((*iter).second) != indexToFreeVariable.end());
            indexToFreeVariable.erase((*iter).second);
            freeVariableToIndex.erase(iter);
        }

        // Return the variable of free index of lowest value with respect to the chosen ordering.
        int getLowestFreeVariableIndex(void);

        void addCut(Constraint *);
        void removeCut(Constraint *);

        // Set the solution value to the partial solution.
        void setSolutionToPartial(void);

        inline Group *getSymmetryGroup() { return group; }
    };
};
#endif
