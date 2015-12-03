// blockgroup.cpp
//
// By Sebastian Raaphorst, 2003.

#include <iostream>
using namespace std;

#include <algorithm>
#include <set>
#include "common.h"
#include "permutationpool.h"
#include "superduper.h"
#include "schreiersimsgroup.h"
#include "blockgroup.h"


BlockGroup::BlockGroup(int v, int k, int lambda, int *base)
  : SchreierSimsGroup(base)
{
  std::set< int > dummy;
  initializeGroup(v, k, lambda, dummy);
}


BlockGroup::BlockGroup(int v, int k, int lambda, std::set< int > &hole, int *base)
  : SchreierSimsGroup(base)
{
  initializeGroup(v, k, lambda, hole);
}


#ifdef NOTWORKING
BlockGroup::BlockGroup(int t, int v, int k, int lambda, int *base) 
  : SchreierSimsGroup(base)
{
  initializeReducedGroup(t, v, k, lambda);
}
#endif


BlockGroup::~BlockGroup()
{
}


void BlockGroup::vertexPermutationToBlockPermutation(int v, int k, int lambda, int *vertexPermutation, int *blockPermutation)
{
  int numberDistinctBlocks = C[v][k];
  int *block = new int[k];
  int *permutedBlock = new int[k];
  int destination;

  for (int i=0; i < numberDistinctBlocks; ++i) {
    // Unrank the block.
    duper(v, k, i, block);

    // Permute the block.
    for (int j=0; j < k; ++j)
      permutedBlock[j] = vertexPermutation[block[j]];

    // Sort the block so that superduper can use it.
    std::sort(permutedBlock, permutedBlock+k);

    // Rank the block.
    destination = super(v, k, permutedBlock);

    // Modify the permutation accordingly.
    for (int j=0; j < lambda; ++j)
      blockPermutation[i*lambda+j] = destination * lambda + j;
  }

  delete[] permutedBlock;
  delete[] block;
}


#ifdef NOTWORKING
void BlockGroup::initializeReducedGroup(int t, int v, int k, int lambda)
{
  // This case requires that there be no hole in the design, which is
  // why we don't even allow one at all. If lambda = 1, we can greatly
  // simplify the group by using the canonical fixings. If lambda is
  // greater, we can simplify into two disjoint groups, namely the
  // 0...k-1 elements in one and k...v-1 in the other.
  int numblocks;
  int numhorizfixed = (v - t + 1)/(k - t + 1);
  int *vertexpermutation = new int[v];
  int *blockpermutation = pool->newPermutation();

  numblocks = C[v][k];

  // TODO_OBSOLETE: The first two parts of this if statement were never debugged.
  if (lambda > 1) {
    // We can only divide into two cases, namely S_k and S_{v-k}.

    // *** S_k ***
    // Long cycle.
    for (int i=0; i < v; ++i)
      vertexpermutation[i] = i;
    for (int i=0; i < k-1; ++i)
      vertexpermutation[i] = i+1;
    vertexpermutation[k-1] = 0;
    vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
    enter(blockpermutation);

    // Transposition.
    for (int i=0; i < v; ++i)
      vertexpermutation[i] = i;
    vertexpermutation[0] = 1;
    vertexpermutation[1] = 0;
    vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
    enter(blockpermutation);

    // *** S_{v-k} ***
    // Long cycle.
    for (int i=0; i < v; ++i)
      vertexpermutation[i] = i;
    for (int i=k; i < v-1; ++i)
      vertexpermutation[i] = i+1;
    vertexpermutation[v-1] = k;
    vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
    enter(blockpermutation);

    // Transposition.
    for (int i=0; i < v; ++i)
      vertexpermutation[i] = i;
    vertexpermutation[k] = k+1;
    vertexpermutation[k+1] = k;
    vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
    enter(blockpermutation);
  }

  else if (numhorizfixed < k-t+2) {
    // *** S_{t-1} over {0, ..., t-2} ***
    if (t > 2) {
      // Long cycle.
      for (int i=0; i < v; ++i)
	vertexpermutation[i] = i;
      for (int i=0; i < t-2; ++i)
	vertexpermutation[i] = i+1;
      vertexpermutation[t-2] = 0;
      vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
      enter(blockpermutation);
      
      // Transposition.
      for (int i=0; i < v; ++i)
	vertexpermutation[i] = i;
      vertexpermutation[0] = 1;
      vertexpermutation[1] = 0;
      vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
      enter(blockpermutation);
    }

    // *** (v-t+1)/(k-t+1) copies of S_{k-t+1} ***
    for (int b=0; b < numhorizfixed; ++b) {
      // Long cycle.
      for (int i=0; i < v; ++i)
	vertexpermutation[i] = i;

      int lbound = b*k - (b-1)*t - (b-1);
      int ubound = (b+1)*k - b*t - (b-1);
      for (int j=lbound; j < ubound; ++j)
	vertexpermutation[j] = j+1;
      vertexpermutation[ubound] = lbound;
      vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
      enter(blockpermutation);

      // Transposition.
      for (int i=0; i < v; ++i)
	vertexpermutation[i] = i;
      vertexpermutation[lbound] = lbound+1;
      vertexpermutation[lbound+1] = lbound;
      vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
      enter(blockpermutation);
    }

    // *** S_{(v-t+1)/(k-t+1)}
    // Long cycle.
    for (int i=0; i < v; ++i)
      vertexpermutation[i] = i;
    for (int b=0; b < numhorizfixed; ++b) {
      int lbound = b*k - (b-1)*t - (b-1);
      int ubound = (b+1)*k - b*t - (b-1);
      for (int j=lbound; j <= ubound; ++j)
	vertexpermutation[j] = (j+k-t+1 < v ? j+k-t+1 : ((j+k-t+1)%v)+t-1);
    }
    vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
    enter(blockpermutation);

    // Transposition.
    for (int i=0; i < v; ++i)
      vertexpermutation[i] = i;
    for (int j=t-1; j < k; ++j)
      vertexpermutation[j] = j+k-t+1;
    for (int j=k; j < 2*k-t+1; ++j)
      vertexpermutation[j] = j-k+t-1;
    vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
    enter(blockpermutation);
  }

  else {
    // *** S_{t-2} over {0, ..., t-3} ***
    if (t > 3) {
      // Long cycle.
      for (int i=0; i < v; ++i)
	vertexpermutation[i] = i;
      for (int i=0; i < t-3; ++i)
	vertexpermutation[i] = i+1;
      vertexpermutation[t-3] = 0;
      cerr << "S_{t-2} cycle:"; for (int i=0; i < v; ++i) cerr << " " << vertexpermutation[i]; cerr << endl;
      vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
      enter(blockpermutation);

      // Transposition.
      for (int i=0; i < v; ++i)
	vertexpermutation[i] = i;
      vertexpermutation[0] = 1;
      vertexpermutation[1] = 0;
#ifdef DEBUG
      cerr << "S_{t-2} trans:"; for (int i=0; i < v; ++i) cerr << " " << vertexpermutation[i]; cerr << endl;
#endif
      vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
      enter(blockpermutation);
    }

    // *** k-t+2 copies of S_{k-t} ***
    if (k-t > 1) {
      for (int b=0; b <= k-t+2; ++b) {
	// Long cycle in the bth fixed block, fixing first elem.
	for (int i=0; i < v; ++i)
	  vertexpermutation[i] = i;

	int lbound = b*k-(b-1)*t+(b-1)+1;
	int ubound = (b+1)*k-b*t+(b-1);
	for (int i=lbound; i < ubound; ++i)
	  vertexpermutation[i] = i+1;
	vertexpermutation[ubound] = lbound;
#ifdef DEBUG
	cerr << "S_{k-t} cycle:"; for (int i=0; i < v; ++i) cerr << " " << vertexpermutation[i]; cerr << endl;
#endif
	vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
	enter(blockpermutation);

	// Transposition.
	for (int i=0; i < v; ++i)
	  vertexpermutation[i] = i;
	vertexpermutation[lbound] = lbound+1;
	vertexpermutation[lbound+1] = lbound;
#ifdef DEBUG
	cerr << "S_{k-t} trans:"; for (int i=0; i < v; ++i) cerr << " " << vertexpermutation[i]; cerr << endl;
#endif
	vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
	enter(blockpermutation);
      }
    }

    // *** S_{k-t+2} ***
    // Long cycle.
    for (int i=0; i < v; ++i)
      vertexpermutation[i] = i;
    for (int b=0; b < k-t+2; ++b) {
      int lbound = b*k - (b-1)*t + (b-1);
      int ubound = (b+1)*k - b*t + (b-1);
      for (int j=lbound; j <= ubound; ++j)
	vertexpermutation[j] = (b < k-t+1 ? j+k-t+1 : t-1+j-lbound);
    }
#ifdef DEBUG
    cerr << "S_{k-t+2} cycle:"; for (int i=0; i < v; ++i) cerr << " " << vertexpermutation[i]; cerr << endl;
#endif
    vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
    enter(blockpermutation);

    // Transposition.
    for (int i=0; i < v; ++i)
      vertexpermutation[i] = i;
    for (int j=t-1; j < k; ++j) {
      vertexpermutation[j] = j+k-t+1;
      vertexpermutation[j+k-t+1] = j;
    }
#ifdef DEBUG
    cerr << "S_{k-t+2} trans:"; for (int i=0; i < v; ++i) cerr << " " << vertexpermutation[i]; cerr << endl;
#endif
    vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
    enter(blockpermutation);

    // *** (v-t+1)/(k-t+1) - (k-t+2) copies of S_{k-t+1} ***
    for (int b=k-t+2; b < numhorizfixed; ++b) {
      // Long cycle.
      for (int i=0; i < v; ++i)
	vertexpermutation[i] = i;

      int lbound = b*k-(b-1)*t+(b-1);
      int ubound = (b+1)*k-b*t+(b-1);
      for (int i=lbound; i < ubound; ++i)
	vertexpermutation[i] = i+1;
      vertexpermutation[ubound] = lbound;
#ifdef DEBUG
      cerr << "S_{k-t+1} cycle:"; for (int i=0; i < v; ++i) cerr << " " << vertexpermutation[i]; cerr << endl;
#endif
      vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
      enter(blockpermutation);

      // Transposition.
      for (int i=0; i < v; ++i)
	vertexpermutation[i] = i;
      vertexpermutation[lbound] = lbound+1;
      vertexpermutation[lbound+1] = lbound;
#ifdef DEBUG
      cerr << "S_{k-t+1} trans:"; for (int i=0; i < v; ++i) cerr << " " << vertexpermutation[i]; cerr << endl;
#endif
      vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
      enter(blockpermutation);
    }

    // *** S_{(v-t+1)/(k-t+1) - (k-t+2)} ***
    if (numhorizfixed - (k-t+2) > 1) {
      // Long cycle.
      for (int i=0; i < v; ++i)
	vertexpermutation[i] = i;
      for (int b=k-t+2; b < numhorizfixed; ++b) {
	int lbound = b*k - (b-1)*t + (b-1);
	int ubound = (b+1)*k - b*t + (b-1);
	for (int j=lbound; j <= ubound; ++j)
	  vertexpermutation[j] = (b < numhorizfixed-1 ? j+k-t+1 : (k-t+2)*k - (k-t+1)*t + (k-t+1) +(j-lbound));
      }
#ifdef DEBUG
      cerr << "S_{big} cycle:"; for (int i=0; i < v; ++i) cerr << " " << vertexpermutation[i]; cerr << endl;
#endif
      vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
      enter(blockpermutation);
      
      // Transposition.
      for (int i=0; i < v; ++i)
	vertexpermutation[i] = i;
      int b = k-t+2;
      int lbound = b*k - (b-1)*t + (b-1);
      int ubound = (b+1)*k - b*t + (b-1);
      for (int j=lbound; j <= ubound; ++j) {
	vertexpermutation[j] = j+k-t+1;
	vertexpermutation[j+k-t+1] = j;
      }
#ifdef DEBUG
      cerr << "S_{big} trans:"; for (int i=0; i < v; ++i) cerr << " " << vertexpermutation[i]; cerr << endl;
#endif
      vertexPermutationToBlockPermutation(v, k, lambda, vertexpermutation, blockpermutation);
      enter(blockpermutation);
    }
  }

  delete[] vertexpermutation;
  pool->freePermutation(blockpermutation);
}
#endif
 

void BlockGroup::initializeGroup(int v, int k, int lambda, std::set< int > &hole)
{
  // We need to deal with this in two ways:
  // 1. The vertices not in the hole.
  // 2. The vertices in the hole.
  // This will be done identically, so create an array of two sets to handle this.
  std::set< int > partitions[2];
  partitions[1] = hole;
  for (int i=0; i < v; ++i)
    if (hole.find(i) == hole.end())
      partitions[0].insert(partitions[0].end(), i);

  // Get two permutations ready. One will be a simple transposition and the
  // other a cycle of all points. These translate to permutations on the
  // blocks that generate the entire requisite symmetry groups.
  int *vertextransposition = new int[v];
  int *blocktransposition = pool->newPermutation();
  int *vertexcycle = new int[v];
  int *blockcycle = pool->newPermutation();
  
  // Now we iterate over each set and create the symmetric groups.
  // This could all be done more efficiently, of course, but as we are only O(vb*f(k)) and this
  // is done exactly once, who cares? We only call enter twice, which is the intensive operation.
  // In the old implementation, we constructed all v-1 transpositions of the form (0 i) for i in
  // 1..(v-1). This was about the same efficiency as our construction here, but the number of calls
  // to enter resulted in a much slower algorithm.
  for (int i=0; i < 2; ++i) {
    // In order to process this set, we need at least two elements.
    if (partitions[i].size() < 2)
      continue;

    // Set all permutations to the identity.
    for (int j=0; j < v; ++j)
      vertextransposition[j] = vertexcycle[j] = j;
    getIdentityPermutation(blocktransposition);
    getIdentityPermutation(blockcycle);

    // First we create the transposition. This is the permutation that is
    // simply the result of swapping the first two elements in our set.
    int first = *(partitions[i].begin());
    int second = *(++partitions[i].begin());
    vertextransposition[first] = second;
    vertextransposition[second] = first;

    // Now calculate the permutation on the blocks using the auxiliary function.
    vertexPermutationToBlockPermutation(v, k, lambda, vertextransposition, blocktransposition);
    enter(blocktransposition);

    // Now we create the cycle over the set.
    first = -1;
    for (std::set< int >::iterator iter = partitions[i].begin();
	 iter != partitions[i].end();
	 ++iter) {
      // If we do not have a first element, grab it.
      if (first < 0) {
	first = *iter;
	continue;
      }

      // Get the element to which the first should go.
      vertexcycle[first] = *iter;

      // Determine first for the next iteration.
      first = *iter;
    }

    // first should now be the last element in the set. As this is a cycle, it must map to the
    // earliest element in the set.
    vertexcycle[first] = *(partitions[i].begin());

    // Now calculate the permutation on the blocks using the auxiliary function.
    vertexPermutationToBlockPermutation(v, k, lambda, vertexcycle, blockcycle);
    enter(blockcycle);
  }

  // Free the permutations.
  pool->freePermutation(blockcycle);
  delete[] vertexcycle;
  pool->freePermutation(blocktransposition);
  delete[] vertextransposition;
}
