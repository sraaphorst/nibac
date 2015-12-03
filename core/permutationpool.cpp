// permutationpool.cpp
//
// By Sebastian Raaphorst, 2004.

#include <assert.h>
#include <string.h>
#include "common.h"
#include "permutationpool.h"


// Initialize the static member.
PermutationPool *PermutationPool::staticpool = 0;


PermutationPool::PermutationPool(int ppermsize
#ifndef NOPERMPOOL
				 , int initialcapacity
				 , double pincreasefactor
#endif
				 )
  : permsize(ppermsize)
#ifndef NOPERMPOOL
  , increasefactor(pincreasefactor)
  , poolsize(initialcapacity)
#endif
{
#ifndef NOPERMPOOL
  // Allocate the permutations.
  permpool = new int*[poolsize];
  for (int i=0; i < poolsize; ++i)
    permpool[i] = new int[permsize];

  // Indicate that all permutations are unused.
  freetop = poolsize-1;
#endif
}


PermutationPool::~PermutationPool()
{
#ifndef NOPERMPOOL
  // Delete all the free permutations.
  for (int i=0; i <= freetop; ++i)
    delete[] permpool[i];
  delete[] permpool;
#endif
}


#ifndef NOPERMPOOL
int *PermutationPool::newPermutation()
{
  // Check if there are no free permutations, and if that is the case, then allocate more.
  if (freetop < 0) {
    // Create the new pool.
    int newcapacity = poolsize * (1 + increasefactor);
    int **newpermpool = new int*[newcapacity];
    int bound = newcapacity - poolsize;
    for (int i=0; i < bound; ++i)
      newpermpool[i] = new int[permsize];

    // We are done; destroy the old data structures and set the new ones.
    delete[] permpool;
    permpool = newpermpool;
    freetop = newcapacity - poolsize - 1;
    poolsize = newcapacity;
  }

  // We now have a permutation to return.
  int *perm = permpool[freetop];
  --freetop;
  return perm;
}
#endif


#ifndef NOPERMPOOL
void PermutationPool::freePermutation(int *perm)
{
  ++freetop;
  assert(freetop < poolsize);
  permpool[freetop] = perm;
}
#endif


void PermutationPool::createPool(int ppermsize
#ifndef NOPERMPOOL
				 , int initialcapacity
				 , double pincreasefactor
#endif
				 )
{
  if (staticpool)
    delete staticpool;
  staticpool = new PermutationPool(ppermsize
#ifndef NOPERMPOOL
				   , initialcapacity
				   , pincreasefactor
#endif
				   );
}


void PermutationPool::deletePool()
{
  delete staticpool;
  staticpool = 0;
}


PermutationPool *PermutationPool::getPool()
{
  return staticpool;
}
