/**
 * lexicographicvariableorder.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef LEXICOGRAPHICVARIABLEORDER_H
#define LEXICOGRAPHICVARIABLEORDER_H

#include "common.h"
#include "variableorder.h"

namespace vorpal::nibac {
    /**
     * This subclass of VariableOrder provides the default ordering on variables, i.e. the order sorted on index.
     */
    class LexicographicVariableOrder final : public VariableOrder {
    public:
        LexicographicVariableOrder() = default;
        virtual ~LexicographicVariableOrder() = default;

        void sort(int, const int *, int *);

        // This is just the identity function.
        inline int variableToIndex(int variable) { return variable; }

        inline int indexToVariable(int index) { return index; }
    };


    /**
     * This is a way of creating variable orders through CommandLineProcessing.
     * If you do not wish to use CommandLineProcessing, this will be of no value to you.
     */
    class LexicographicVariableOrderCreator final : public VariableOrderCreator {
    public:
        LexicographicVariableOrderCreator() = default;
        virtual ~LexicographicVariableOrderCreator() = default;

    private:
        inline std::string getVariableOrderName(void) override {
            return std::string("Lexicographic variable ordering");
        }

        std::map <std::string, std::pair<std::string, std::string>> getOptionsMap(void) override;

        bool processOptionsString(const char *) override;

        // Make create protected so that users do not accidentally call
        // this, which would result in memory leakage.
        VariableOrder *create(void) const override;
    };
};
#endif

