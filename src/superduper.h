/**
 * superduper.h
 *
 * Graciously donated by Dr. Rudi Mathon.
 *
 * This code is used to rank / unrank subsets of sets in a highly efficient way.
 */

#ifndef SUPERDUPER_H
#define SUPERDUPER_H

namespace vorpal::nibac::design {
    /**
     * A class to look up binomial coefficients and to rank and unrank subsets.
     */
    class SuperDuper final {
    private:
        static int _C[100][100];
        static int flag_first_call;

        SuperDuper();
        virtual ~SuperDuper();

        /**
         * Initialize the binamial matrix up to n choose n.
         * This method will be invoked automatically and does not require the user to invoke it.
         * @param n The size of the required matrix. Must be less than 100.
         */
        static inline void init_super_duper(int n);

    public:

        /**
         * Find the number of k-sets over an n-set, i.e. the function n choose k.
         * @param n The size of the base set.
         * @param k The size of the subsets.
         * @return The number of k-subsets of an n-set.
         */
        static inline int C(int n, int k);

        /**
         * Given the size of a base set (the v-set), and a k-subset, this function returns the rank
         * of the k-subset amongst all k-subsets of the v-set.
         * @param v The size of the base set.
         * @param k The size of the k-subset.
         * @param set The k-subset.
         * @return The rank of the k-subset amongst all k-subsets of the v-set.
         */
        static inline int super(int v, int k, int *set);

        /**
         * Given the size of a base set (the v-set), the size of a k-subset, and a lexicographical
         * rank, this function determines the k-subset represented by the rank.
         * @param v The size of the base set.
         * @param k The size of the subset.
         * @param num The lexicographical number of the subset.
         * @param set The array (of size at least k) into which we perform the unranking.
         */
        static inline void duper(int v, int k, int num, int *set);
    };
};
#endif
