/**
 * variableorder.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef VARIABLEORDER_H
#define VARIABLEORDER_H

#include "common.h"
#include <map>
#include <string>

namespace vorpal::nibac {
    class CommandLineProcessing;

    /**
     * This file provides an abstract class to induce an ordering on a set of variables.
     * This is required to establish initial variable orderings when using the Margot B&C
     * scheme (variable orderings are irrelevant in the regular vanilla B&C). The implication
     * of variable orderings are that they impact the structure of the Schreier-Sims scheme
     * table, which can affect the runtime of the problem by, for example changing the effort
     * required to down / reverse-down and changing which variables are selected by minimum
     * index branching.
     *
     * All ordering classes should extend this one. Then, program authors who wish to use an
     * ordering function should register the ordering function with the static member register,
     * which will call the parent and populate the map of VariableOrder. Using the register function
     * allows the programmer to determine which ordering functions are available, how they should be
     * designated via the command line, and also allows various configuation (i.e. the public register
     * method in the subclass could contain some user information).
     */
    class VariableOrder {
    protected:
        VariableOrder() = default;

    public:
        virtual ~VariableOrder() = default;

        // Given a list of indicated size, sort it according to the ordering, storing
        // it in the third parameter. This is useful for functionality like preparing
        // the initial base and is not currently used by NIBAC.
        virtual void sort(int, const int *, int *) = 0;

        // Given a variable, return its index in the order.
        virtual int variableToIndex(int) = 0;

        // Given an index, return which variable appears in that position in the order.
        virtual int indexToVariable(int) = 0;
    };


    /**
     * This is a way of creating solution managers through CommandLineProcessing.
     * If you do not wish to use CommandLineProcessing, this will be of no value to you.
     */
    class VariableOrderCreator {
        // Friend declarations.
        friend class CommandLineProcessing;

    protected:
        VariableOrderCreator() = default;

    public:
        virtual ~VariableOrderCreator() = default;

    protected:
        virtual std::string getVariableOrderName() = 0;

        virtual std::map<std::string, std::pair<std::string, std::string> > getOptionsMap() = 0;

        virtual bool processOptionsString(const char *) = 0;

        // Make create protected so that users do not accidentally call
        // this, which would result in memory leakage.
        virtual VariableOrder *create() const = 0;
    };
};
#endif
