/**
 * node.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include <map>
#include <set>
#include <list>
#include "node.h"
#include "bac.h"
#include "bacoptions.h"
#include "constraint.h"
#include "formulation.h"
#include "lpsolver.h"
#include "variableorder.h"

namespace vorpal::nibac {
// While we only care about the branching variables for the majority of operations, the partial
// solution vector must contain room for all variables.
    Node::Node(BAC &pbac,
               Formulation &pformulation,
               Group *pgroup,
               int pnumberVariables,
               int pnumberBranchingVariables,
               const std::set<int> *fixed0,
               const std::set<int> *fixed1)
            :
#ifdef NODEGROUPS
            rootNodeFlag(true),
#endif
            group(pgroup),
            bac(pbac),
            formulation(pformulation),
            processedFlag(false),
            depth(0),
            branchVariableIndex(-1),
            branchVariableValue(-1),
            branchingVariableIndex(-1),
            nextBranchingVariableValue(1),
            possiblyMaximalFlag(true),
            ancestorsCanonicalFlag(true),
            isCanonicalFlag(true),
            testCanonicityFlag(true),
            numberVariables(pnumberVariables),
            numberBranchingVariables(pnumberBranchingVariables),
            numberFixedVariables(0),
            number0FixedVariables(0),
            numberLPSolves(0),
            solutionValue(0),
            solutionVariableArray(new double[pnumberBranchingVariables]),
            partialSolutionArray(new short int[pnumberVariables]) {
        // Indicate that every variable is free in the partial solution.
        for (int i = 0; i < numberVariables; ++i)
            partialSolutionArray[i] = -1;

        // Get the variable order.
        VariableOrder *variableOrder = bac.getOptions().getVariableOrder();

        // Populate the free list with all variables, and the lookup maps.
        for (int i = 0; i < numberBranchingVariables; ++i) {
            freeVariables.insert(i);
            freeVariableToIndex[i] = variableOrder->variableToIndex(i);
            indexToFreeVariable[i] = variableOrder->indexToVariable(i);
        }

        // If there are any variables we want to fix to 1, fix them.
        // NOTE that it is tremendously important that we perform the
        // 1-fixing before the 0-fixing for Margot's algorithms. Otherwise,
        // we will never properly calculate orbits in stabilizers!
        if (fixed1)
            for (std::set<int>::const_iterator iter = fixed1->begin();
                 iter != fixed1->end();
                 ++iter)
                bac.fixVariableTo1(*this, *iter, true);

        // If there are any variables we want to fix to 0, fix them.
        if (fixed0)
            for (std::set<int>::const_iterator iter = fixed0->begin();
                 iter != fixed0->end();
                 ++iter)
                bac.fixVariableTo0(*this, *iter, true);
    }


// Note that, in the constructor, we don't modify the number of fixed variables
// or the number of 0 fixed variables. This will be taken care of in the call
// to fixVariableTo0 or fixVariableTo1.
    Node::Node(Node *parent,
               int pbranchVariableIndex,
               int pbranchVariableValue)
            :
#ifdef NODEGROUPS
            rootNodeFlag(false),
#endif
            bac(parent->bac),
            formulation(parent->formulation),
            processedFlag(false),
            depth(parent->depth + 1),
            branchVariableIndex(pbranchVariableIndex),
            branchVariableValue(pbranchVariableValue),
            branchingVariableIndex(-1),
            nextBranchingVariableValue(1),
            possiblyMaximalFlag(pbranchVariableValue ? true : parent->possiblyMaximalFlag),
            ancestorsCanonicalFlag(parent->ancestorsCanonicalFlag),
            testCanonicityFlag(parent->testCanonicityFlag),
            numberVariables(parent->numberVariables),
            numberBranchingVariables(parent->numberBranchingVariables),
            numberFixedVariables(parent->numberFixedVariables),
            number0FixedVariables(parent->number0FixedVariables),
            numberLPSolves(0),
            solutionValue(0),
            solutionVariableArray(new double[parent->numberBranchingVariables]),
            partialSolutionArray(new short int[parent->numberVariables]),
            freeVariables(parent->freeVariables),
            freeVariableToIndex(parent->freeVariableToIndex),
            indexToFreeVariable(parent->indexToFreeVariable) {
        // Determine if this node is, by default, canonical.
        // If the parent is canonical and we branched on 0, then we are naturally canonical.
        isCanonicalFlag = parent->isCanonicalFlag && branchVariableValue == 0;

        // Copy the partial solution from the parent.
        memcpy(partialSolutionArray,
               parent->partialSolutionArray,
               sizeof(short int) * numberVariables);

        // Set the symmetry group for this node.
#ifdef NODEGROUPS
        bac.getOptions().getStatistics().getGroupCopyTimer().start();
        group = parent->group->makeCopy();
        bac.getOptions().getStatistics().getGroupCopyTimer().stop();
#else
        group = parent->group;
#endif

        // Adjustments of the numberFixedVariables, number0FixedVariables, etc. done here,
        // since setting one variable might force others to be set and we can't predict this
        // in the node without knowing which algorithm we are using.
        if (branchVariableValue == 1)
            bac.fixVariableTo1(*this, branchVariableIndex);
        else
            bac.fixVariableTo0(*this, branchVariableIndex);
    }


    Node::~Node() {
        // Unfix all the fixed variables.
        for (std::set<int>::iterator iter = fixedVariables.begin();
             iter != fixedVariables.end();
             ++iter)
            formulation.unfixVariable(*iter);

        // We need to remove all cuts at the node from the formulation.
        for (std::set<Constraint *>::iterator iter = cuts.begin();
             iter != cuts.end();
             ++iter) {
            Constraint *cut = *iter;
            formulation.removeCut(cut);
            delete cut;
        }
        cuts.clear();

        if (bac.getOptions().keepCuts()) {
            // Add back the constraints that we removed from the
            // branch rooted at this node.
            for (std::set<Constraint *>::iterator iter = removedCuts.begin();
                 iter != removedCuts.end();
                 ++iter)
                formulation.addCut(*iter);
        }

        // Delete the partial solution.
        if (partialSolutionArray)
            delete[] partialSolutionArray;

        // Delete the array of solution variables.
        if (solutionVariableArray)
            delete[] solutionVariableArray;

#ifdef NODEGROUPS
        if (!rootNodeFlag)
          // Destroy the symmetry group.
          delete group;
#endif
    }


    void Node::cleanup(void) {
        if (solutionVariableArray)
            delete[] solutionVariableArray;
        solutionVariableArray = 0;
    }


    int Node::getLowestFreeVariableIndex() {
        assert((freeVariables.empty() && indexToFreeVariable.empty())
               || (!freeVariables.empty() && !indexToFreeVariable.empty()));
        return (freeVariables.empty() ? -1 : (*(indexToFreeVariable.begin())).second);
    }


    void Node::addCut(Constraint *cut) {
        cuts.insert(cut);
        formulation.addCut(cut);
    }


    void Node::removeCut(Constraint *cut) {
        // If it exists in the list of constraints, remove it from there.
        std::set<Constraint *>::iterator iter = cuts.find(cut);
        if (iter != cuts.end()) {
            cuts.erase(iter);
            formulation.removeCut(cut);
            delete cut;
        } else {
            // This constraint is either global to the formulation or
            // defined in a parent node, so we want to restore it when
            // we backtrack beyond this node if keepConstraints is set.
            // Otherwise, we simply remove it and delete it.
            if (bac.getOptions().keepCuts()) {
                removedCuts.insert(cut);
                formulation.removeCut(cut);
            } else {
                formulation.removeCut(cut);
                delete cut;
            }
        }
    }


    void Node::setSolutionToPartial() {
        for (int i = 0; i < numberBranchingVariables; ++i)
            solutionVariableArray[i] = partialSolutionArray[i];
    }
};