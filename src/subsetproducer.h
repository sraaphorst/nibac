/**
 * subsetproducer.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef SUBSETPRODUCER_H
#define SUBSETPRODUCER_H

namespace vorpal::nibac::design {
    /**
     * Given a set size and a subset size, create all subsets of that size, and for each one, invoke a callback.
     * This class is not constructable and only offers static methods.
     */
    class SubsetProducer final {
    private:
        SubsetProducer() = default;
        virtual ~SubsetProducer() = default;

    public:
        // Given a base set size and a subset size, produce all subsets of the base
        // set. For each one, invoke the callback function, which takes the set size,
        // the subset size, the subset, and a piece of user data cast as a void*.
        static void createAllSubsets(int, int, void (*)(int, int, int *, void *), void * = 0);

    private:
        // Internal backtracking method used by createAllSubsets.
        static void backtrackAllSubsets(int, int, int *, int, void (*)(int, int, int *, void *), void *);
    };
};
#endif
