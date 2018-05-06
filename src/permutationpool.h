// permutationpool.h
//
// By Sebastian Raaphorst, 2004.
//
// This class basically stores a pool of pre-allocated
// permutations. The reason for this is to avoid continuously
// allocating memory for integer arrays using new and delete,
// which is highly inefficient. In this way, we can simply
// reuse old permutations. The pool grows as needed.
//
// $Author$
// $Date$

#ifndef PERMUTATIONPOOL_H
#define PERMUTATIONPOOL_H

#include "common.h"


class PermutationPool
{
 private:
  // Static factory.
  static PermutationPool *staticpool;

#ifndef NOPERMPOOL
  int **permpool;
  int freetop;
  int poolsize;
  double increasefactor;
#endif

  // Size of permutations.
  int permsize;

#ifdef NOPERMPOOL
  PermutationPool(int);
#else
  PermutationPool(int, int, double);
#endif
  virtual ~PermutationPool();

 public:
  static void createPool(int
#ifndef NOPERMPOOL
			 , int=100000
			 , double=0.5
#endif
			 );
  static void deletePool();
  static PermutationPool *getPool();

#ifdef NOPERMPOOL
  inline int *newPermutation() { return new int[permsize]; }
#else
  int *newPermutation();
#endif

#ifdef NOPERMPOOL
  inline void freePermutation(int *p) { delete[] p; }
#else
  void freePermutation(int*);
#endif
};

#endif
