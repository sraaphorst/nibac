/**
 * margotbacoptions.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef MARGOTBACOPTIONS_H
#define MARGOTBACOPTIONS_H

#include <string.h>
#include "common.h"
#include "bacoptions.h"

namespace vorpal::nibac {
// Forward class declarations.
    class MargotBAC;

    class MargotBACOptions final : public BACOptions {
    protected:
        // A subclass of BACOptions which includes some options specific for Margot's algorithms.
        // The main component of this class is a series of flags that determine at what
        // points in the tree we want to perform 0-fixing and canonicity testing.
        // Obviously, it is essential that we test final results for canonicity, but
        // apart from that, canonicity testing and 0-fixing, both costly operations, may
        // be best left undone at certain depths if we don't think that the payoff of these
        // operations will be worth the time consumed in calculating them. It seems, from
        // experimentation, that orbits in stabilizers tend to be very large at the beginning
        // of the tree and nonexistent at other levels in the tree.
        // In terms of data structures, for compactness, we could use an STL set. However,
        // we would then need some annoying iterator manipulation or logarithmic time lookup
        // in order to determine whether or not a node at a given depth should be examined
        // in these ways. For this reason, we simply use an array, specified by the user.
        // These arrays are initialized to 0 (indicating to always test) until initializeFlags is
        // called to specify the number of variables (maximum depth), in which case, both
        // arrays are initialized to always contain 1, hence still indicating to always test.
        int *canonicityDepthFlags;
        int *orbitDepthFlags;

        // We need some way of populating these flags. This is a bit of a conundrum, because
        // we need the number of variables to do so. When the MargotBACOptions is created,
        // it is quite possible that this information will not yet be known. In light of this,
        // what we allow is the desired depths to be specified as strings consisting of comma
        // separated ranges of the form a (a), -a (0, ..., a), a-b (a, ..., b), and b- (b, ...).
        // The initialization routines in MargotBAC, which will know the number of variables,
        // then invokes the initializeDepthFlags method, which takes this information and processes
        // it to populate the above flags.
        char *canonicityDepthFlagsString;
        char *orbitDepthFlagsString;

        // This value specifies the highest depth of canonicity testing. This allows us to stop
        // downing after we reach the specified depth and focus strictly on B&C without having
        // errors later on.
        int highestCanonicityDepth;

        // A threshold for 0-fixing and canonicity testing; on a given branch, if we reach a node
        // where an orbit-in-stabilizer calculation has less than this threshold of variables in
        // the resultant orbit, we stop canonicity testing on the branch (until necessary).
        // DEFAULT: 0, which indicates to always continue canonicity testing
        int orbitThreshold;

        // A flag indicating whether or not we want to test final solutions for canonicity.
        // DEFAULT: true
        bool testFinalSolutions;

    public:
        MargotBACOptions();
        virtual ~MargotBACOptions();

        // These will not work until the MargotBACOptions have been passed
        // to a MargotBAC constructor, which will parse the strings representing
        // the ranges. Until then, they return null.
        inline int *getCanonicityDepthFlags(void) { return canonicityDepthFlags; }

        inline int *getOrbitDepthFlags(void) { return orbitDepthFlags; }

        // Accessors for the strings.
        inline const char *getCanonicityDepthFlagsString(void) { return canonicityDepthFlagsString; }

        inline void setCanonicityDepthFlagsString(const char *pcanonicityDepthFlagsString) {
            if (canonicityDepthFlagsString)
                delete[] canonicityDepthFlagsString;
            canonicityDepthFlagsString = new char[strlen(pcanonicityDepthFlagsString) + 1];
            strcpy(canonicityDepthFlagsString, pcanonicityDepthFlagsString);
        }

        inline const char *getOrbitDepthFlagsString(void) { return orbitDepthFlagsString; }

        inline void setOrbitDepthFlagsString(const char *porbitDepthFlagsString) {
            if (orbitDepthFlagsString)
                delete[] orbitDepthFlagsString;
            orbitDepthFlagsString = new char[strlen(porbitDepthFlagsString) + 1];
            strcpy(orbitDepthFlagsString, porbitDepthFlagsString);
        }

        inline int getHighestCanonicityDepth(void) { return highestCanonicityDepth; }

        // Get / set method for orbit threshhold.
        inline int getOrbitThreshold(void) { return orbitThreshold; }

        inline void setOrbitThreshold(int porbitThreshold) {
            if (porbitThreshold < 0)
                throw IllegalParameterException("MargotBACOptions::OrbitThreshold", porbitThreshold,
                                                "OrbitThreshold must be nonnegative");
            orbitThreshold = porbitThreshold;
        }

        inline static int getOrbitThresholdDefault(void) { return 0; }

        // Get / set method for testing final solutions.
        inline bool getTestFinalSolutions(void) { return testFinalSolutions; }

        inline void setTestFinalSolutions(bool ptestFinalSolutions) { testFinalSolutions = ptestFinalSolutions; }

        inline static bool getTestFinalSolutionsDefault(void) { return true; }

    private:
        // Initialize the arrays: allocate space for them, and process the strings if they have been set.
        void initializeDepthFlags(int);

        // Declare MargotBAC a friend so we can call this initializeDepthFlags.
        friend class MargotBAC;
    };
};
#endif
