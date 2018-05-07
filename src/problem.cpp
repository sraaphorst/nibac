/**
 * problem.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifdef TIMESYMMETRYALGORITHMS
#include <iostream>
#endif

#include <vector>
#include "common.h"
#include "problem.h"
#include "bac.h"
#include "bacoptions.h"
#include "branchingscheme.h"
#include "cutproducer.h"
#include "formulation.h"
#include "group.h"
#include "margotbac.h"
#include "margotbacoptions.h"

#ifdef TIMESYMMETRYALGORITHMS
#include "schreiersimsgroup.h"
#endif

#include "solutionmanager.h"
#include "variableorder.h"

namespace vorpal::nibac {
    // Static initializaters.
    const int Problem::FREE = 0;
    const int Problem::FIXEDTO0 = -1;
    const int Problem::FIXEDTO1 = 1;


    Problem::Problem(BACOptions &poptions,
                     Formulation::ProblemType pproblemType,
                     Formulation::SolutionType psolutionType,
                     int pnumberVariables,
                     int pnumberBranchingVariables)
            : options(poptions),
              problemType(pproblemType),
              solutionType(psolutionType),
              numberVariables(pnumberVariables),
              numberBranchingVariables(pnumberBranchingVariables < 0 ? pnumberVariables : pnumberBranchingVariables),
              fixingFlags(new int[pnumberVariables]),
              formulation(pproblemType, psolutionType, numberVariables, numberBranchingVariables),
              group(0) {
        // Initialize all variables to free.
        for (int i = 0; i < numberVariables; ++i)
            fixingFlags[i] = FREE;
    }


    Problem::~Problem() {
        // Free the memory for the fixingFlags array.
        delete[] fixingFlags;
    }


    void Problem::initialize() {
    }


    void Problem::setupFixingFlags(void) {
        // If manual fixing has been specified, this is handled here. Otherwise, the programmer
        // may specify fixings of their choice by setting up the fixingFlags array in
        // determineFixingFlags.
        if (options.getManualFixings()) {
            for (std::set<int>::iterator iter = options.getInitial0Fixings().begin();
                 iter != options.getInitial0Fixings().end();
                 ++iter)
                fixingFlags[*iter] = FIXEDTO0;

            for (std::set<int>::iterator iter = options.getInitial1Fixings().begin();
                 iter != options.getInitial1Fixings().end();
                 ++iter)
                fixingFlags[*iter] = FIXEDTO1;
        } else
            determineFixingFlags(fixingFlags);
    }


    void Problem::determineFixingFlags(int *flags) {
    }


    void Problem::establishInitialBase(int *base) {
        VariableOrder *variableOrder = options.getVariableOrder();
        if (!variableOrder)
            throw IllegalOperationException("No VariableOrder configured in the BACOptions");

        int *tmpArray = new int[numberVariables];

        // Based on the fixingFlags, we establish an initial base for the Schreier-Sims group
        // that will be efficient for the downing. This base will have the structure:
        // F_1 F_0 F
        int index = 0;
        int number1Fixings = 0;
        int number0Fixings = 0;
        int numberFree = 0;

        // Begin with the 1-fixings.
        for (int i = 0; i < numberVariables; ++i)
            if (fixingFlags[i] == FIXEDTO1) {
                tmpArray[index] = i;
                ++index;
                ++number1Fixings;
            }
        variableOrder->sort(number1Fixings, tmpArray, base);

        // Proceed to the 0-fixings.
        for (int i = 0; i < numberVariables; ++i)
            if (fixingFlags[i] == FIXEDTO0) {
                tmpArray[index] = i;
                ++index;
                ++number0Fixings;
            }
        variableOrder->sort(number0Fixings, tmpArray + number1Fixings, base + number1Fixings);

        // Finish with the free variables.
        for (int i = 0; i < numberVariables; ++i)
            if (fixingFlags[i] == FREE) {
                tmpArray[index] = i;
                ++index;
                ++numberFree;
            }
        variableOrder->sort(numberFree, tmpArray + number1Fixings + number0Fixings,
                            base + number1Fixings + number0Fixings);

        delete[] tmpArray;
    }


    void Problem::constructSymmetryGroup(void) {
        group = 0;
    }


    void Problem::solve(void) {
        // Perform any necessary remaining initialization that isn't handled by the
        // Problem constructor and by the algorithms.
        initialize();

        Statistics &statistics = options.getStatistics();

        // *** FORMULATION ***
        // The formulation timer measures the time to construct the
        // formulation and set up the fixings.
        statistics.getFormulationTimer().start();
        constructFormulation();
        setupFixingFlags();

        for (int i = 0; i < numberVariables; ++i)
            if (fixingFlags[i] == FIXEDTO1)
                options.getInitial1Fixings().insert(i);
            else if (fixingFlags[i] == FIXEDTO0)
                options.getInitial0Fixings().insert(i);
        statistics.getFormulationTimer().stop();

        // *** EXPORT ***
        // Export the problem, if necessary.
        if (options.getExportFileName()) {
            std::cerr << "Exporting model.\n";
            std::cerr << "Exporting to " << options.getExportFileName() << "\n";
            std::cerr << "Calling function.\n";
            bool flag = formulation.exportModel(options.getExportFileName());
            std::cerr << "Exported.\n";
            if (!flag)
                std::cerr << "Could not export model to " << options.getExportFileName() << std::endl;
        }

        // *** SYMMETRY GROUP ***
        statistics.getSymmetryGroupTimer().start();
        constructSymmetryGroup();
        statistics.getSymmetryGroupTimer().stop();

        // *** SOLVE ***
        // Now create the B&C. We use a standard BAC if a symmetry group is not specified, and
        // if one is, then we use a MargotBAC.
        BAC *bac;
        if (!group)
            bac = new BAC(formulation, options);
        else
            bac = new MargotBAC(formulation, *group, (MargotBACOptions &) options);

        // We now solve.
        bac->solve();

        // *** PROCESS SOLUTIONS ***
        // Process the solutions.
        processSolutions();

        // *** CLEANUP ***
        delete bac;
    }


    void Problem::processSolutions() {
    }
};