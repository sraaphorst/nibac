/**
 * branchingscheme.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 *
 * The abstract implementation of a branching scheme.
 * An instance of this class is supplied to the nodestack.
 * It is then used to select branching variables. This,
 * however, cannot be done arbitrarily in conjunction with
 * 0-fixing; in this case, branching must be done consistently
 * throughout the tree. Otherwise, we may miss branches of
 * computation that lead to valid results.
 */

#ifndef BRANCHINGSCHEME_H
#define BRANCHINGSCHEME_H

#include <map>
#include <string>
#include "common.h"

namespace vorpal::nibac {
// Forward declarations
    class Node;


    class BranchingScheme {
    protected:
        BranchingScheme();

    public:
        virtual ~BranchingScheme();

        // This is the method that determines, given a node, on which variable we
        // should branch. It returns the variable index, or -1 if there is no
        // possible variable.
        virtual int getBranchingVariableIndex(Node &) = 0;
    };


    // Class forward for BranchingSchemeCreator.
    class CommandLineProcessing;

    // This is a way of creating solution managers through CommandLineProcessing.
    // If you do not wish to use CommandLineProcessing, this will be of no value to you.
    class BranchingSchemeCreator {
        // Friend declarations.
        friend class CommandLineProcessing;

    protected:
        BranchingSchemeCreator();

        BranchingSchemeCreator(int p_id);

        virtual ~BranchingSchemeCreator();

        virtual std::string getBranchingSchemeName(void) = 0;

        virtual std::map<std::string, std::pair<std::string, std::string> > getOptionsMap(void) = 0;

        virtual bool processOptionsString(const char *) = 0;

        // Make create protected so that users do not accidentally call
        // this, which would result in memory leakage.
        virtual BranchingScheme *create(void) const = 0;
    };
};
#endif
