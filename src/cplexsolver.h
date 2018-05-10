/**
 * cplexsolver.h
 *
 * By Sebastian Raaphorst, 2013 - 2018.
 */

#ifndef CPLEXSOLVER_H
#define CPLEXSOLVER_H

#include <ilcplex/ilocplex.h>
#include <limits.h>
#include <vector>
#include <map>
#include "common.h"
#include "formulation.h"
#include "bac.h"
#include "node.h"
#include "lpsolver.h"

namespace vorpal::nibac {
    /**
     * An interface between the generic BAC framework and CPLEX 7.
     * This is a concrete implementation of LPSolver.
     * @see LPSolver
     */
    class CPLEXSolver final : public LPSolver {
    private:
        static CPLEXSolver _cplexsolver;

        // Data structure to hold the relevant information for a formulation.
        // This organization may seem profoundly ridiculous, but since IloCplex::solveRelaxation
        // crashes on repeated calls for MIPs, and IloNumVarArray::setType does not exist despite
        // documentation to the contrary, I can find no way to alternate between solving LP relaxations
        // and solving ILPs using CPLEX without maintaining two sets of variables (one Int, one Float)
        // and otherwise identical models and algorithms.
        struct CPLEXInfo {
            std::map<int, IloRange* > fixingsL;
            std::map<int, IloRange* > fixingsI;
            IloNumVarArray *varsL;
            IloNumVarArray *varsI;
            IloModel *modelL;
            IloModel *modelI;
            IloCplex *cplexL;
            IloCplex *cplexI;
        };

        struct ConstraintInfo {
            IloRange *rangeL;
            IloRange *rangeI;
        };

        // The CPLEX environment object.
        IloEnv env;

    protected:
        CPLEXSolver();

        virtual ~CPLEXSolver();

    public:
        void setupFormulation(Formulation &) override;

        void cleanupFormulation(Formulation &) override;

        void setObjectiveFunction(Formulation &, std::vector<int> &, int = INT_MIN, int = INT_MAX) override;

        void addConstraint(Formulation &, Constraint *) override;

        void removeConstraint(Formulation &, Constraint *) override;

        void addCut(Formulation &, Constraint *) override;

        void removeCut(Formulation &, Constraint *) override;

        void fixVariable(Formulation &, int, int) override;

        void unfixVariable(Formulation &, int) override;

        int solveNode(BAC &, Node &, bool= false) override;

        void *createConstraint(Formulation &, std::vector<int> &, std::vector<int> &, int, int);

        void deleteConstraint(void *);

        bool exportModel(Formulation &, const char *);

#ifdef DEBUG
        void printVariables(Formulation&);
#endif
    };
};
#endif
