// cutproducer.cpp
//
// By Sebastian Raaphorst, 2004.
//
// $Author$
// $Date$

#include <map>
#include <string>
#include <vector>
#include "common.h"
#include "cutproducer.h"
#include "nibacexception.h"
#include "node.h"


CutProducer::CutProducer()
{
}


CutProducer::~CutProducer()
{
}


void CutProducer::quicksort(std::vector< int > &sorter, double *values, int first, int last)
{
  if (first >= last)
    return;

  // Get a pivot element; we will pick the middle element.
  int index = (first + last) / 2;

  // Swap the pivot element with the beginning of sorter.
  int tmp = sorter[first];
  sorter[first] = sorter[index];
  sorter[index] = tmp;

  // Shuffle the elements down as necessary.
  int endindex = last;
  int pos;
  for (pos = first+1; pos <= endindex;) {
    if (values[sorter[pos]] > values[sorter[first]]) {
      // Swap pos with endindex, decrement endindex, and continue.
      // The swap is pointless if pos is endindex.
      if (pos != endindex) {
	tmp = sorter[endindex];
	sorter[endindex] = sorter[pos];
	sorter[pos] = tmp;
      }
      --endindex;
      continue;
    }
    ++pos;
  }

  // The elements have been shuffled correctly; we now swap the pivot into its
  // proper sorted position, which is always pos-1.
  index = pos-1;
  tmp = sorter[index];
  sorter[index] = sorter[first];
  sorter[first] = tmp;

  // Sort recursively.
  quicksort(sorter, values, first, index-1);
  quicksort(sorter, values, index+1, last);
}


void CutProducer::quicksort(int *sorter, double *values, int first, int last)
{
  if (first >= last)
    return;

  // Get a pivot element; we will pick the middle element.
  int index = (first + last) / 2;

  // Swap the pivot element with the beginning of sorter.
  int tmp = sorter[first];
  sorter[first] = sorter[index];
  sorter[index] = tmp;

  // Shuffle the elements down as necessary.
  int endindex = last;
  int pos;
  for (pos = first+1; pos <= endindex;) {
    if (values[sorter[pos]] > values[sorter[first]]) {
      // Swap pos with endindex, decrement endindex, and continue.
      // The swap is pointless if pos is endindex.
      if (pos != endindex) {
	tmp = sorter[endindex];
	sorter[endindex] = sorter[pos];
	sorter[pos] = tmp;
      }
      --endindex;
      continue;
    }
    ++pos;
  }

  // The elements have been shuffled correctly; we now swap the pivot into its
  // proper sorted position, which is always pos-1.
  index = pos-1;
  tmp = sorter[index];
  sorter[index] = sorter[first];
  sorter[first] = tmp;

  // Sort recursively.
  quicksort(sorter, values, first, index-1);
  quicksort(sorter, values, index+1, last);
}




// *** CUTPRODUCERCREATOR methods ***
CutProducerCreator::CutProducerCreator()
  : activeFlag(true)
{
}


CutProducerCreator::~CutProducerCreator()
{
}
