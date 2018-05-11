/**
 * bacoptions.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include "common.h"
#include "bacoptions.h"
#include "bac.h"
#include "branchingscheme.h"
#include "cutproducer.h"
#include "solutionmanager.h"

namespace vorpal::nibac {
    // Static declarations
    const int    BACOptions::BB_DEPTH_DEFAULT = INT_MAX;
    const int    BACOptions::CP_MIN_NUMBER_OF_CUTS_DEFAULT = 5;
    const double BACOptions::CP_MIN_VIOLATIONL_DEFAULT = 0.3;
    const double BACOptions::CP_MIN_VIOLATIONU_DEFAULT = 0.6;
    const double BACOptions::CP_VIOLATION_TOLERANCEL_DEFAULT = 0.3;
    const double BACOptions::CP_VIOLATION_TOLERANCEU_DEFAULT = 0.6;
    const double BACOptions::CP_ACTIVITY_TOLERANCE_DEFAULT = 0.1;
    const int    BACOptions::BB_LBOUND_DEFAULT = INT_MIN;
    const int    BACOptions::BB_UBOUND_DEFAULT = INT_MAX;
    const bool   BACOptions::keepcutsDefault = true;


    BACOptions::BACOptions()
            : BB_DEPTH(BB_DEPTH_DEFAULT),
              CP_MIN_NUMBER_OF_CUTS(CP_MIN_NUMBER_OF_CUTS_DEFAULT),
              CP_MIN_VIOLATIONL(CP_MIN_VIOLATIONL_DEFAULT),
              CP_MIN_VIOLATIONU(CP_MIN_VIOLATIONU_DEFAULT),
              CP_VIOLATION_TOLERANCEL(CP_VIOLATION_TOLERANCEL_DEFAULT),
              CP_VIOLATION_TOLERANCEU(CP_VIOLATION_TOLERANCEU_DEFAULT),
              CP_ACTIVITY_TOLERANCE(CP_ACTIVITY_TOLERANCE_DEFAULT),
              BB_LBOUND(BB_LBOUND_DEFAULT),
              BB_UBOUND(BB_UBOUND_DEFAULT),
              branchingScheme(nullptr),
              solutionManager(nullptr),
              keepcuts(keepcutsDefault),
              manualFixingsFlag(false),
              exportFileName(nullptr) {
    }


    BACOptions::~BACOptions() {
    }


    bool BACOptions::removeCutProducer(CutProducer *cutProducer) {
        for (std::vector<CutProducer *>::iterator iter = cutProducers.begin();
             iter != cutProducers.end();
             ++iter)
            if (*iter == cutProducer) {
                // We have found the cut producer, so remove it.
                cutProducers.erase(iter);
                return true;
            }

        // The cut producer was not found.
        return false;
    }


    bool BACOptions::removeInitial0Fixing(int variable) {
        std::set<int>::iterator iter = initial0Fixings.find(variable);
        if (iter != initial0Fixings.end()) {
            initial0Fixings.erase(iter);
            return true;
        }
        return false;
    }


    bool BACOptions::removeInitial1Fixing(int variable) {
        std::set<int>::iterator iter = initial1Fixings.find(variable);
        if (iter != initial1Fixings.end()) {
            initial1Fixings.erase(iter);
            return true;
        }
        return false;
    }
};