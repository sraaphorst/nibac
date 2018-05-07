/**
 * solutionmanager.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef SOLUTIONMANAGER_H
#define SOLUTIONMANAGER_H

#include <map>
#include <string>
#include "common.h"

namespace vorpal::nibac {
    class Node;
    class CommandLineProcessing;

    /**
     * An interface for managing solutions.
     */
    class SolutionManager {
    protected:
        SolutionManager() = default;

    public:
        virtual ~SolutionManager() = default;

        virtual void newSolution(Node &) = 0;
    };


    /**
     * This is a way of creating solution managers through CommandLineProcessing.
     * If you do not wish to use CommandLineProcessing, this will be of no value to you.
     */
    class SolutionManagerCreator {
        // Friend declarations.
        friend class CommandLineProcessing;

    protected:
        SolutionManagerCreator() = default;
        virtual ~SolutionManagerCreator() = default;

        virtual std::string getSolutionManagerName(void) = 0;

        virtual std::map<std::string, std::pair<std::string, std::string> > getOptionsMap(void) = 0;

        virtual bool processOptionsString(const char *) = 0;

        // Make create protected so that users do not accidentally call
        // this, which would result in memory leakage.
        virtual SolutionManager *create(void) const = 0;
    };
};
#endif
