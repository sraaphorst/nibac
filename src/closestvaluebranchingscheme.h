/**
 * closestvaluebranchingscheme.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 *
 * This is an extension of RankedBranchingScheme that, when
 * given the option, picks the variable closest to a specified
 * value.
 */

#ifndef CLOSESTVALUEBRANCHINGSCHEME_H
#define CLOSESTVALUEBRANCHINGSCHEME_H

#include <map>
#include <string>
#include "common.h"
#include "nibacexception.h"
#include "rankedbranchingscheme.h"

namespace vorpal::nibac {
    // Forward declarations
    class Node;

    class ClosestValueBranchingScheme final : public RankedBranchingScheme {
    private:
        double value;

    public:
        static const double DEFAULT_VALUE;

        // By default, we use 0.5, since this is likely to perturb the problem
        // a lot, i.e. cause LP solutions to change dramatically from parent to
        // child.
        ClosestValueBranchingScheme(int, const double = DEFAULT_VALUE);

        virtual ~ClosestValueBranchingScheme();

        // There is no reason why the branching scheme should not be changeable
        // between iterations, so allow this.
        inline double getValue() const { return value; }

        inline void setValue(const double pvalue) {
            if (pvalue < 0 || pvalue > 1)
                throw IllegalParameterException("ClosestValueBranchingScheme::value", pvalue, "must be in [0,1]");
            value = pvalue;
        }

    protected:
        // This method iterates over the free variables and picks the one closest
        // to value.
        int chooseBranchingVariableIndex(Node &);
    };


    // This is a way of creating branching schemes through CommandLineProcessing.
    // If you do not wish to use CommandLineProcessing, this will be of no value to you.
    class ClosestValueBranchingSchemeCreator final : public BranchingSchemeCreator {
    private:
        int numberVariables;
        double value;

    public:
        ClosestValueBranchingSchemeCreator();

        virtual ~ClosestValueBranchingSchemeCreator();

        inline void setNumberVariables(int pnumberVariables) { numberVariables = pnumberVariables; }

        inline int getNumberVariables(void) const { return numberVariables; }

        inline void setValue(double pvalue) { value = pvalue; }

        inline double getValue(void) { return value; }

    private:
        inline std::string getBranchingSchemeName(void) override {
            return std::string("Closest value branching scheme");
        }

        std::map <std::string, std::pair<std::string, std::string>> getOptionsMap(void) override;

        bool processOptionsString(const char *) override;

        // Make create protected so that users do not accidentally call
        // this, which would result in memory leakage.
        BranchingScheme *create(void) const override;
    };
};
#endif
