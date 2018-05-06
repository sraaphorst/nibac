// subsetproducer.cpp
//
// By Sebastian Raaphorst, 2006.

#include "subsetproducer.h"
#include "common.h"


SubsetProducer::SubsetProducer()
{
}


SubsetProducer::~SubsetProducer()
{
}


void SubsetProducer::createAllSubsets(int setsize, int subsetsize,
				      void (*callback)(int, int, int*, void*),
				      void *userdata)
{
  int *subset = new int[subsetsize];
  backtrackAllSubsets(setsize, subsetsize, subset, 0, callback, userdata);
  delete[] subset;
}


void SubsetProducer::backtrackAllSubsets(int setsize, int subsetsize,
					 int *subset, int depth,
					 void (*callback)(int, int, int*, void*),
					 void *userdata)
{
  if (depth == subsetsize) {
    // Call the callback.
    callback(setsize, subsetsize, subset, userdata);
    return;
  }

  // Try all extensions to the current position, using the elements left over.
  int i = (depth == 0 ? 0 : subset[depth-1]+1);
  for (; i < setsize; ++i) {
    subset[depth] = i;
    backtrackAllSubsets(setsize, subsetsize, subset, depth+1, callback, userdata);
  }

  return;
}
