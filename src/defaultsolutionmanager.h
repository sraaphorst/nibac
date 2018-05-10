/**
 * defaultsolutionmanager.h
 *
 * By Sebastian Raaphorst 2003 - 2018.
 */

#ifndef DEFAULTSOLUTIONMANAGER_H
#define DEFAULTSOLUTIONMANAGER_H

#include <map>
#include <string>
#include <vector>
#include "common.h"
#include "node.h"
#include "solutionmanager.h"

namespace vorpal::nibac {
    /**
     * A default implementation of a solution manager that manages families of solutions using a vector.
     */
    class DefaultSolutionManager : public SolutionManager {
    protected:
        double bestsoln;
        std::vector< std::vector< int > * > solutions;
        Formulation::ProblemType ptype;
        Formulation::SolutionType stype;
        bool generateall;

    public:
        DefaultSolutionManager(Formulation::ProblemType, Formulation::SolutionType);

        virtual ~DefaultSolutionManager();

        virtual void newSolution(Node &);

        virtual std::vector< std::vector< int> * > &getSolutions();

    protected:
        virtual void clearVector();
    };


    // This is a way of creating solution managers through CommandLineProcessing.
    // If you do not wish to use CommandLineProcessing, this will be of no value to you.
    class DefaultSolutionManagerCreator : public SolutionManagerCreator {
    private:
        Formulation::ProblemType problemType;
        Formulation::SolutionType solutionType;

    public:
        DefaultSolutionManagerCreator();

        virtual ~DefaultSolutionManagerCreator();

        inline void setProblemType(Formulation::ProblemType pproblemType) {
            problemType = pproblemType;
        }

        inline Formulation::ProblemType getProblemType(void) { return problemType; }

        inline void setSolutionType(Formulation::SolutionType psolutionType) {
            solutionType = psolutionType;
        }

        inline Formulation::SolutionType getSolutionType(void) { return solutionType; }

    protected:
        inline virtual std::string getSolutionManagerName(void) {
            return std::string("Default solution manager");
        }

        virtual std::map <std::string, std::pair<std::string, std::string>> getOptionsMap(void);

        virtual bool processOptionsString(const char *);

        // Make create protected so that users do not accidentally call
        // this, which would result in memory leakage.
        virtual SolutionManager *create(void) const;
    };
};
#endif
