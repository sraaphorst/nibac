/**
 * randomvariableorder.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef RANDOMVARIABLEORDER_H
#define RANDOMVARIABLEORDER_H

#include <map>
#include <string>
#include "common.h"
#include "variableorder.h"

namespace vorpal::nibac {
    class VariableOrder;

    /** This subclass of VariableOrder provides a completely random ordering on the
     * variables it is given. It requires that the RNG consisting of random() in cstdlib
     * be seeded using srandom() for the desired unpredictability, or by calling
     * initializeRNG below.
     */
    class RandomVariableOrder final : public VariableOrder {
    private:
        // We use arrays to establish an order on the variables.
        int numberVariables;
        int *indexList;
        int *variableList;

    public:
        // Static method to initialize the RNG, if users do not wish to do so manually.
        static void initializeRNG();

        RandomVariableOrder(int);
        virtual ~RandomVariableOrder();

        void sort(int, const int *, int *);

        inline int variableToIndex(int variable) { return indexList[variable]; }

        inline int indexToVariable(int index) { return variableList[index]; }
    };


    /**
     * This is a way of creating variable orders through CommandLineProcessing.
     * If you do not wish to use CommandLineProcessing, this will be of no value to you.
     */
    class RandomVariableOrderCreator final : public VariableOrderCreator {
    protected:
        int numberVariables;

    public:
        RandomVariableOrderCreator(const int);
        virtual ~RandomVariableOrderCreator() = default;

        inline void setNumberVariables(int pnumberVariables) { numberVariables = pnumberVariables; }

        inline int getNumberVariables() const { return numberVariables; }

    private:
        inline std::string getVariableOrderName() override {
            return std::string("Random variable ordering");
        }

        std::map <std::string, std::pair<std::string, std::string>> getOptionsMap() override;

        bool processOptionsString(const char *) override;

        // Make create protected so that users do not accidentally call
        // this, which would result in memory leakage.
        VariableOrder *create() const override;
    };
};
#endif

