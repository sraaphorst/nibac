/**
 * formulation.cpp
 *
 * By Sebastian Raaphorst 2003 - 2018.
 */

#include <vector>
#include <map>
#include "common.h"
#include "formulation.h"
#include "column.h"
#include "constraint.h"
#include "lpsolver.h"

namespace vorpal::nibac {
    Formulation::Formulation(ProblemType pproblemType,
                             SolutionType psolutionType,
                             int pnumberVariables,
                             int pnonBranchingIndex)
            : problemType(pproblemType),
              solutionType(psolutionType),
              numberVariables(pnumberVariables),
              nonBranchingIndex(pnonBranchingIndex) {
        LPSolver::getInstance()->setupFormulation(*this);
        columns.resize(numberVariables);
    }


    Formulation::~Formulation() {
        // Delete any remaining constraints. This MUST be done
        // before the formulation is cleaned up, because constraints
        // might depend on the model and data LP-specific structures.
        std::map<unsigned long, Constraint *>::iterator iter;
        while (constraints.size() > 0) {
            iter = constraints.begin();
            Constraint *constraint = (*iter).second;
            constraints.erase(iter);
            LPSolver::getInstance()->removeConstraint(*this, constraint);
            delete constraint;
        }

        // We clean up the formulation last of all, freeing any
        // LP-solver specific data structures (model and data
        // variables).
        LPSolver::getInstance()->cleanupFormulation(*this);
    }


    void Formulation::setObjectiveFunction(std::vector<int> &coefficients, int lowerBound, int upperBound) {
        objectiveFunction = coefficients;
        LPSolver::getInstance()->setObjectiveFunction(*this, coefficients, lowerBound, upperBound);
    }


    void Formulation::addConstraint(Constraint *constraint) {
        // Add it to our list...
        constraints[constraint->getID()] = constraint;

        // ...and to our LP-solver model
        LPSolver::getInstance()->addConstraint(*this, constraint);

        // Add the ID to the column information.
        std::vector<int> &positions = constraint->getPositions();
        std::vector<int> &coefficients = constraint->getCoefficients();
        std::vector<int>::iterator pbeginIter = positions.begin();
        std::vector<int>::iterator pendIter = positions.end();
        std::vector<int>::iterator cbeginIter = coefficients.begin();
        std::vector<int>::iterator cendIter = coefficients.end();
        unsigned long id = constraint->getID();

        for (; pbeginIter != pendIter; ++pbeginIter, ++cbeginIter)
            columns[*pbeginIter].add(id, *cbeginIter);
    }


    void Formulation::removeConstraint(Constraint *constraint) {
        // Remove it from our list...
        constraints.erase(constraint->getID());

        // ...and from our LP-solver model
        LPSolver::getInstance()->removeConstraint(*this, constraint);

        // ...and remove the ID from the relevant columns.
        std::vector<int> &positions = constraint->getPositions();
        unsigned long id = constraint->getID();

        for (std::vector<int>::iterator iter = positions.begin();
             iter != positions.end();
             ++iter)
            columns[*iter].remove(id);
    }


    void Formulation::addCut(Constraint *cut) {
        // Add it to our list...
        cuts[cut->getID()] = cut;

        // ...and to our LP-solver model
        LPSolver::getInstance()->addCut(*this, cut);
    }


    void Formulation::removeCut(Constraint *cut) {
        // Remove it from our list...
        cuts.erase(cut->getID());

        // ...and from our LP-solver model
        LPSolver::getInstance()->removeCut(*this, cut);
    }


    void Formulation::fixVariable(int variable, int value) {
        // Fix it in the map...
        fixings[variable] = value;

        // ...and in the LP.
        LPSolver::getInstance()->fixVariable(*this, variable, value);
    }


    void Formulation::unfixVariable(int variable) {
        // Remove it from the map...
        fixings.erase(variable);

        // ...and from the LP.
        LPSolver::getInstance()->unfixVariable(*this, variable);
    }


// TODO: What if the formulation has multiple bounds? We've allowed this, and it won't work, i.e lower AND upper.
// We could ensure that lower are fixed as well.
    int Formulation::getBCoefficient(int index) {
        return (constraints[index])->getUpperBound();
    }


    int Formulation::getMatrixCoefficient(int row, int column) {
        // Check to see if constraint row is defined on variable column
        Constraint *constraint = constraints[row];
        std::vector<int> &constraintPositions = constraint->getPositions();

        // Search through the positions
        int position = 0;
        for (std::vector<int>::iterator iter = constraintPositions.begin();
             iter != constraintPositions.end();
             ++iter, ++position)
            if (*iter == column)
                return (constraint->getCoefficients())[position];
        return 0;
    }


    bool Formulation::exportModel(const char *filename) {
        return LPSolver::getInstance()->exportModel(*this, filename);
    }


    double Formulation::evaluateObjectiveFunction(double *variableValues) {
        double total = 0;
        int i = 0;
        for (std::vector<int>::iterator iter = objectiveFunction.begin();
             iter != objectiveFunction.end();
             ++iter, ++i)
            total += (*iter) * variableValues[i];
        return total;
    }


    bool Formulation::checkPartialFeasibility(short int *variableValues) {
        std::map<unsigned long, Constraint *>::iterator bconsIter = constraints.begin();
        std::map<unsigned long, Constraint *>::iterator econsIter = constraints.end();
        std::vector<int>::iterator bposIter, eposIter, bcoefIter, ecoefIter;
        Constraint *constraint;
        int maxv, minv;

        for (; bconsIter != econsIter; ++bconsIter) {
            constraint = (*bconsIter).second;

            // For each constraint, we determine the maximum and minimum value positions
            // for the coefficient / variable portion.
            std::vector<int> &positions = constraint->getPositions();
            bposIter = positions.begin();
            eposIter = positions.end();

            std::vector<int> &coefficients = constraint->getCoefficients();
            bcoefIter = coefficients.begin();
            ecoefIter = coefficients.end();

            minv = 0;
            maxv = 0;

            for (; bposIter != eposIter; ++bposIter, ++bcoefIter) {
                minv += (variableValues[*bposIter] == -1 ? (*bcoefIter < 0 ? *bcoefIter : 0) : *bcoefIter *
                                                                                               variableValues[*bposIter]);
                maxv += (variableValues[*bposIter] == -1 ? (*bcoefIter > 0 ? *bcoefIter : 0) : *bcoefIter *
                                                                                               variableValues[*bposIter]);
            }

            // Determine if it is impositionsible to satisfy the constraint.
            if (minv > constraint->getUpperBound() || maxv < constraint->getLowerBound())
                return false;
        }

        // All constraints were valid.
        return true;
    }
};