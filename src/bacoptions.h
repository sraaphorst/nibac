/**
 * bacoptions.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 *
 * A simplification of specifying options for the branch-and-cut algorithm.
 * Comprises options with sensible default values.
 * This saves us from having to provide everything to the constructors for BAC and MargotBAC.
 */

#ifndef BACOPTIONS_H
#define BACOPTIONS_H

#include <iostream>
#include <set>
#include <vector>
#include "common.h"
#include "branchingscheme.h"
#include "cutproducer.h"
#include "nibacexception.h"
#include "solutionmanager.h"
#include "statistics.h"
#include "variableorder.h"

namespace vorpal::nibac {
    class BACOptions {
    protected:
        // Depth to which we process the tree using our algorithms; when we reach
        // depth, we allow CPLEX's pure ILP solving to take over.
        int BB_DEPTH;
        static const int BB_DEPTH_DEFAULT;

        // The minimum number of cuts we need to generate in a single iteration of
        // the cutting plane algorithm in order to continue.
        int CP_MIN_NUMBER_OF_CUTS;
        static const int CP_MIN_NUMBER_OF_CUTS_DEFAULT;

        // Upper and lower bounds on the minimum violation.
        double CP_MIN_VIOLATIONL;
        static const double CP_MIN_VIOLATIONL_DEFAULT;
        double CP_MIN_VIOLATIONU;
        static const double CP_MIN_VIOLATIONU_DEFAULT;

        // Upper and lower bounds on the violation tolerance.
        // Default: BAC::CP_DEFAULT_VIOLATION_TOLERANCEL/U
        double CP_VIOLATION_TOLERANCEL;
        static const double CP_VIOLATION_TOLERANCEL_DEFAULT;
        double CP_VIOLATION_TOLERANCEU;
        static const double CP_VIOLATION_TOLERANCEU_DEFAULT;

        // For a cut ax <= b, a cut is considered inactive in a node if
        // b - ax > activityTolerance, i.e. the cut is within activityTolerance
        // away from being violated.
        double CP_ACTIVITY_TOLERANCE;
        static const double CP_ACTIVITY_TOLERANCE_DEFAULT;

        // Upper and lower bounds on the optimal solution.
        int BB_LBOUND;
        static const int BB_LBOUND_DEFAULT;
        int BB_UBOUND;
        static const int BB_UBOUND_DEFAULT;

        // The cut producers used for the problem.
        std::vector<CutProducer *> cutProducers;

        // The branching scheme used for the B&C.
        // This MUST be set or BAC will throw an exception.
        BranchingScheme *branchingScheme;

        // Solution manager reported to by the B&C.
        // This MUST be set or BAC will throw an exception.
        SolutionManager *solutionManager;

        // The variable order used by the problem.
        // This MUST be set or BAC will throw an exception.
        VariableOrder *variableOrder;

        // Statistics for the problem.
        Statistics statistics;

        // If fixings are performed manually by the user, this flag will be set to true.
        // This should be checked by the programmer to avoid specific fixings.
        bool manualFixingsFlag;

        // The variables fixed to 0 and 1 by preprocessing.
        std::set<int> initial0Fixings;
        std::set<int> initial1Fixings;

        // When constraints are no longer active, we remove them from the node and
        // its branches. However, we assume that they might become active in a sibling,
        // so we readd them when we backtrack. However, storing all these inactive cuts
        // might not be feasible with regards to memory for very large problems, so we
        // provide a flag which indicates to the program that we simply want to throw
        // away these cuts instead of keeping them. The default is to keep them.
        bool keepcuts;
        static const bool keepcutsDefault;

        // A filename to which we want to export the ILP. If null, as in the default,
        // we never export.
        const char *exportFileName;

    public:
        BACOptions();

        virtual ~BACOptions();

        // Controls the depth of processing, as outlined above.
        inline int getDepth(void) const { return BB_DEPTH; }

        inline void setDepth(int pBB_DEPTH) {
            if (pBB_DEPTH < 0)
                throw IllegalParameterException("BACOptions::Depth", pBB_DEPTH, "Depth must be nonnegative");
            BB_DEPTH = pBB_DEPTH;
        }

        inline static int getDepthDefault(void) { return BB_DEPTH_DEFAULT; }

        // The minimum number of cuts for the cutting plane, as outlined above.
        inline int getMinimumNumberOfCuts(void) const { return CP_MIN_NUMBER_OF_CUTS; }

        inline void setMinimumNumberOfCuts(int pCP_MIN_NUMBER_OF_CUTS) {
            if (pCP_MIN_NUMBER_OF_CUTS < 0)
                throw IllegalParameterException("BACOptions::MinimumNumberOfCuts", pCP_MIN_NUMBER_OF_CUTS,
                                                "MinimumNumberOfCuts must be nonnegative");
            CP_MIN_NUMBER_OF_CUTS = pCP_MIN_NUMBER_OF_CUTS;
        }

        inline static int getMinimumNumberOfCutsDefault(void) { return CP_MIN_NUMBER_OF_CUTS_DEFAULT; }

        // The lower bound on the minimum violation, as outlined above.
        inline double getMinimumViolationL(void) const { return CP_MIN_VIOLATIONL; }

        inline void setMinimumViolationL(double pCP_MIN_VIOLATIONL) {
            if (pCP_MIN_VIOLATIONL < 0)
                throw IllegalParameterException("BACOptions::MinimumViolationL", pCP_MIN_VIOLATIONL,
                                                "MinimumViolationL must be nonnegative");
            CP_MIN_VIOLATIONL = pCP_MIN_VIOLATIONL;
        }

        inline static double getMinimumViolationLDefault(void) { return CP_MIN_VIOLATIONL_DEFAULT; }

        // The upper bound on the minimum violation, as outlined above.
        inline double getMinimumViolationU(void) const { return CP_MIN_VIOLATIONU; }

        inline void setMinimumViolationU(double pCP_MIN_VIOLATIONU) {
            if (pCP_MIN_VIOLATIONU < 0)
                throw IllegalParameterException("BACOptions::MinimumViolationU", pCP_MIN_VIOLATIONU,
                                                "MinimumViolationU must be nonnegative");
            CP_MIN_VIOLATIONU = pCP_MIN_VIOLATIONU;
        }

        inline static double getMinimumViolationUDefault(void) { return CP_MIN_VIOLATIONU_DEFAULT; }

        // The lower bound on the violation tolerance, as outlined above.
        inline double getViolationToleranceL(void) const { return CP_VIOLATION_TOLERANCEL; }

        inline void setViolationToleranceL(double pCP_VIOLATION_TOLERANCEL) {
            if (pCP_VIOLATION_TOLERANCEL < 0)
                throw IllegalParameterException("BACOptions::ViolationToleranceL", pCP_VIOLATION_TOLERANCEL,
                                                "ViolationToleranceL must be nonnegative");
            CP_VIOLATION_TOLERANCEL = pCP_VIOLATION_TOLERANCEL;
        }

        inline static double getViolationToleranceLDefault(void) { return CP_VIOLATION_TOLERANCEL_DEFAULT; }

        // The upper bound on the violation tolerance, as outlined above.
        inline double getViolationToleranceU(void) const { return CP_VIOLATION_TOLERANCEU; }

        inline void setViolationToleranceU(double pCP_VIOLATION_TOLERANCEU) {
            if (pCP_VIOLATION_TOLERANCEU < 0)
                throw IllegalParameterException("BACOptions::ViolationToleranceU", pCP_VIOLATION_TOLERANCEU,
                                                "ViolationToleranceU must be nonnegative");
            CP_VIOLATION_TOLERANCEU = pCP_VIOLATION_TOLERANCEU;
        }

        inline static double getViolationToleranceUDefault(void) { return CP_VIOLATION_TOLERANCEU_DEFAULT; }

        // The activity tolerance, as outlined above.
        inline double getActivityTolerance(void) const { return CP_ACTIVITY_TOLERANCE; }

        inline void setActivityTolerance(double pCP_ACTIVITY_TOLERANCE) {
            if (pCP_ACTIVITY_TOLERANCE < 0)
                throw IllegalParameterException("BACOptions::ActiviyTolerance", pCP_ACTIVITY_TOLERANCE,
                                                "ActivityTolerance must be nonnegative");
            CP_ACTIVITY_TOLERANCE = pCP_ACTIVITY_TOLERANCE;
        }

        inline static double getActivityToleranceDefault(void) { return CP_ACTIVITY_TOLERANCE_DEFAULT; }

        // The lower bound on the solution.
        inline int getLowerBound(void) const { return BB_LBOUND; }

        inline void setLowerBound(int pBB_LBOUND) { BB_LBOUND = pBB_LBOUND; }

        inline static int getLowerBoundDefault(void) { return BB_LBOUND_DEFAULT; }

        // The upper bound on the solution.
        inline int getUpperBound(void) const { return BB_UBOUND; }

        inline void setUpperBound(int pBB_UBOUND) { BB_UBOUND = pBB_UBOUND; }

        inline static int getUpperBoundDefault(void) { return BB_UBOUND_DEFAULT; }

        // Accessor to the vector of cut producers. We can only retrieve this way, not add.
        inline const std::vector<CutProducer *> &getCutProducers(void) { return cutProducers; }

        // Add a CutProducer to the list of producers for this class.
        inline void addCutProducer(CutProducer *cutProducer) {
            cutProducers.push_back(cutProducer);
        }

        // Remove the specified cut producer from the collection. If no such cut producer
        // exists, returns false.
        bool removeCutProducer(CutProducer *);

        // The branching scheme.
        inline BranchingScheme *getBranchingScheme(void) { return branchingScheme; }

        inline void setBranchingScheme(BranchingScheme *pbranchingScheme) { branchingScheme = pbranchingScheme; }

        // The solution manager for the problem.
        inline SolutionManager *getSolutionManager(void) { return solutionManager; }

        inline void setSolutionManager(SolutionManager *psolutionManager) { solutionManager = psolutionManager; }

        // The variable order for the problem.
        inline VariableOrder *getVariableOrder(void) { return variableOrder; }

        inline void setVariableOrder(VariableOrder *pvariableOrder) { variableOrder = pvariableOrder; }

        // Accessor for the statistics.
        inline Statistics &getStatistics(void) { return statistics; }

        // Accessor for manual fixings.
        inline bool getManualFixings(void) const { return manualFixingsFlag; }

        inline void setManualFixings(bool pmanualFixingsFlag) { manualFixingsFlag = pmanualFixingsFlag; }

        // Methods to manage the initial 0 fixings.
        inline std::set<int> &getInitial0Fixings(void) { return initial0Fixings; }

        inline void addInitial0Fixing(int variable) { initial0Fixings.insert(variable); }

        bool removeInitial0Fixing(int);

        // Methods to manage the initial 1 fixings.
        inline std::set<int> &getInitial1Fixings(void) { return initial1Fixings; }

        inline void addInitial1Fixing(int variable) { initial1Fixings.insert(variable); }

        bool removeInitial1Fixing(int);

        // Do we want to keep the cuts for backtracking?
        inline bool keepCuts(void) const { return keepcuts; }

        inline void keepCuts(bool pkeepcuts) { keepcuts = pkeepcuts; }

        inline static bool keepCutsDefault(void) { return keepcutsDefault; }

        // The export file. Set to null to not export.
        inline const char *getExportFileName(void) { return exportFileName; }

        inline void setExportFileName(const char *pexportFileName) { exportFileName = pexportFileName; }
    };
};

#endif
