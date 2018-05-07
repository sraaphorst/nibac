/**
 * glpksolver.h
 *
 * By Sebastian Raaphorst, 2013 - 2018.
 */

#ifndef GLPKSOLVER_H
#define GLPKSOLVER_H

#include <limits.h>

extern "C" {
#include <glpk.h>
}

#include <vector>
#include "lpsolver.h"

namespace vorpal::nibac {
    // Class forwards
    class BAC;
    class Constraint;
    class Formulation;
    class Node;

    /**
     * An interface between the generic BAC framework and the free GLPK. This is a concrete implementation of LPSolver.
     */
    class GLPKSolver final : public LPSolver {
    private:
        static GLPKSolver _glpksolver;

        // Data structure to populate the Formulation::data field.
        struct LPXInfo {
            LPX *lp;
            int offset;
        };

    protected:
        GLPKSolver() = default;
        virtual ~GLPKSolver() = default;

    public:
        virtual void setupFormulation(Formulation &);

        virtual void cleanupFormulation(Formulation &);

        virtual void setObjectiveFunction(Formulation &, std::vector<int> &, int= INT_MIN, int= INT_MAX);

        virtual void addConstraint(Formulation &, Constraint *);

        virtual void removeConstraint(Formulation &, Constraint *);

        virtual void addCut(Formulation &, Constraint *);

        virtual void removeCut(Formulation &, Constraint *);

        virtual void fixVariable(Formulation &, int, int);

        virtual void unfixVariable(Formulation &, int);

        virtual int solveNode(BAC &, Node &, bool= false);

        void *createConstraint(Formulation &, std::vector<int> &, std::vector<int> &, int, int);

        void deleteConstraint(void *);

        bool exportModel(Formulation &, const char *);

#ifdef DEBUG
        void printVariables(Formulation&);
#endif
    };
};

#endif
