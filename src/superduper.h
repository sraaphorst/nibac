// superduper.h
//
// This code graciously provided by Dr. Rudi Mathon for use with NIBAC.
// superduper is used to rank / unrank subsets in a highly efficient way.

#ifndef superduper_h
#define superduper_h

/// Evaluations for nCr.
/**
 * If init_super_duper(v) is called, this array may be used to evaluate
 * nCr for 0 <= n <= v, 0 <= r <= n.
 */
extern int C[100][100];

/// Initialization for the superduper library.
/**
 * This function initializes the superduper library.
 * @param v The size of the base set, or the maximum size amongst all base sets.
 */
void init_super_duper(int v);

/// Ranking function
/**
 * Given the size of a base set (the v-set), and a k-subset, this function returns the rank
 * of the k-subset amongst all k-subsets of the v-set.
 * @param v The size of the base set.
 * @param k The size of the k-subset.
 * @param set The k-subset.
 * @return The rank of the k-subset amongst all k-subsets of the v-set.
 */
int super (int v, int k, int* set);

/// Unranking function.
/**
 * Given the size of a base set (the v-set), the size of a k-subset, and a lexicographical
 * number, this function determines the k-subset represented by the lexicographical number.
 * @param v The size of the base set.
 * @param k The size of the subset.
 * @param num The lexicographical number of the subset.
 * @param set The array into which we perform the unranking.
 */
void duper (int v, int k, int num, int* set);

#endif
