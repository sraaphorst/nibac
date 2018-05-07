/**
 * statistics.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef STATISTICS_H
#define STATISTICS_H

#include <ostream>
#include <vector>
#include <map>
#include "common.h"
#include "node.h"
#include "timer.h"

namespace vorpal::nibac {
    /**
     * Stores statistical information for the branch-and-cut algorithm execution.
     */
    class Statistics final {
    private:
        // Number of calls to the canonicity testing algorithm
        unsigned long numberCanonicityCalls;

        // Number of successful non-canonical rejections
        unsigned long numberCanonicityRejections;

        // Maximum depth of a node rejected by the canonicity tester
        unsigned long nonCanonicalMaximumDepth;

        // Timer used to measure the amount of time spent in the canonicity
        // testing algorithm and 0-fixing.
        Timer margotTimer;

        // Timer used to measure the amount of time needed to calculate the
        // symmetry group.
        Timer groupTimer;

        // Timer used to measure the amount of time spent creating the Formulation
        // of the problem.
        Timer formulationTimer;

        // Number of nodes explored by the branch-and-bound
        unsigned long numberNodesExplored;

        // Number of times that we backtrack in the node stack
        unsigned long numberStackBacktracks;

        // Total time spent solving LP problems
        Timer lpSolveTime;

        // Number of LP problems solved
        unsigned long numberLPsSolved;

        // Total time in the separation algorithm
        Timer separationTimer;

        // Number of cuts
        std::vector<unsigned long> numberCuts;

        // Depth of tree
        unsigned long treeDepth;

#ifdef NODEGROUPS
        // Total time spent copying the group
        Timer groupCopyTimer;
#endif

        // Total time spent solving the problem
        Timer totalTimer;

        // Nodes per depth and fixings per depth
        std::map<int, int> nodesByDepth;
        std::map<int, int> fixingsByDepth;

#ifdef MARGOTIMERS
        public:
          // TODO: Are we even using these? I don't think so.
          // We've duplicated all of this is the Schreier Sims group, probably because it has
          // no access to the statistics.
          Timer margotEnterTimer;
          Timer margotDownTimer;
          Timer margotCanonicityTimer;
          Timer margotOrbitTimer;
          int margotNumberEnterCalls;
          int margotNumberDownCalls;
          int margotNumberReverseDownCalls;
          int margotNumberCanonicityCalls;
          int margotNumberOrbitCalls;
#endif

    public:
        Statistics();
        virtual ~Statistics() = default;

        // We make all the accessors for this class inline for simplicity and for
        // efficiency.

        // Number of calls to the canonicity testing algorithm
        inline unsigned long getNumberCanonicityCalls() const { return numberCanonicityCalls; }

        inline void reportCanonicityCall() { ++numberCanonicityCalls; }

        // Number of successful non-canonical rejections
        inline unsigned long getNumberCanonicityRejections() const { return numberCanonicityRejections; }

        inline void reportCanonicityRejection() { ++numberCanonicityRejections; }

        // Maximum depth of a node rejected by the canonicity test
        inline unsigned long getNonCanonicalMaximumDepth() const { return nonCanonicalMaximumDepth; }

        inline void reportNonCanonicalDepth(unsigned long p) {
          if (p > nonCanonicalMaximumDepth)nonCanonicalMaximumDepth = p;
        }

        // Canonicity algorithm timer
        inline Timer &getMargotTimer() { return margotTimer; }

        // Symmetry group algorithm timer
        inline Timer &getSymmetryGroupTimer() { return groupTimer; }

        // Formulation timer
        inline Timer &getFormulationTimer() { return formulationTimer; }

        // Number of nodes explored by the branch-and-bound.
        inline unsigned long getNumberNodesExplored() const { return numberNodesExplored; }

        inline void reportNode(Node &node) {
          ++numberNodesExplored;
          ++(nodesByDepth[node.getDepth()]);
        }

        // This reports multiple nodes solved. Note that this does not
        // record depth information, and is used for things like noting
        // information from an ILP solver like CPLEX that does not immediately
        // provide you with depth information.
        inline void reportNodesWithoutDepth(int count) { numberNodesExplored += count; }

        // Number of numberStackBacktracks
        inline unsigned long getNumberStackBacktracks() const { return numberStackBacktracks; }

        inline void reportBacktrack() { ++numberStackBacktracks; }

        // LP solver timer
        inline Timer &getLPSolverTimer() { return lpSolveTime; }

        // Number of LPs solved
        inline unsigned long getNumberLPsSolved() const { return numberLPsSolved; }

        inline void reportLPSolved(void) { ++numberLPsSolved; }

        // Separation algorithm timer
        inline Timer &getSeparationTimer() { return separationTimer; }

        // Number of cuts
        void setNumberCutProducers(unsigned int);

        inline std::vector<unsigned long> &getNumberCuts() { return numberCuts; }

        // Depth of the B&C tree
        inline unsigned long getTreeDepth() const { return treeDepth; }

        inline void reportBranchDepth(unsigned long p) { if (p > treeDepth) treeDepth = p; }

#ifdef NODEGROUPS
        // Group copy timer
        inline Timer &getGroupCopyTimer() { return groupCopyTimer; }
#endif

        // Total B&C timer
        inline Timer &getTotalTimer() { return totalTimer; }

        // Number of nodes at a certain depth.
        inline std::map<int, int> &getNodeCountByDepth() { return nodesByDepth; }

        // Number of fixings at each depth (by 0-fixing or other means). This does not
        // include branching fixings.
        inline std::map<int, int> &getVariableFixingCountByDepth() { return fixingsByDepth; }

        // Make the printer a friend of this class.
        friend std::ostream &operator<<(std::ostream &, const Statistics &);
    };


    // Print the statistics to the screen.
    std::ostream &operator<<(std::ostream &, Statistics &);
};
#endif
