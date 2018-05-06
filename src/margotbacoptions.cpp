// margotbacoptions.cpp
//
// By Sebastian Raaphorst, 2004.
//
// $Author$
// $Date$

#include <limits.h>
#include "common.h"
#include "margotbacoptions.h"
#include "bacoptions.h"
#include "util.h"


MargotBACOptions::MargotBACOptions()
  : canonicityDepthFlags(0),
    orbitDepthFlags(0),
    canonicityDepthFlagsString(0),
    orbitDepthFlagsString(0),
    orbitThreshold(0),
    highestCanonicityDepth(INT_MAX),
    testFinalSolutions(true)
{
}


MargotBACOptions::~MargotBACOptions()
{
  if (canonicityDepthFlags)
    delete[] canonicityDepthFlags;
  canonicityDepthFlags = 0;
  if (orbitDepthFlags)
    delete[] orbitDepthFlags;
  orbitDepthFlags = 0;

  if (canonicityDepthFlagsString)
    delete[] canonicityDepthFlagsString;
  canonicityDepthFlagsString = 0;
  if (orbitDepthFlagsString)
    delete[] orbitDepthFlagsString;
  orbitDepthFlagsString = 0;
}


void MargotBACOptions::initializeDepthFlags(int numberVariables)
{
  // Only create the depth flags arrays if necessary, i.e. depths
  // were specified in the forms of strings. Otherwise, we leave
  // them as null pointers, indicating to always test.
  if (canonicityDepthFlagsString) {
    canonicityDepthFlags = new int[numberVariables];
    if (!parseFlagsFromString(numberVariables, canonicityDepthFlags, canonicityDepthFlagsString))
      throw IllegalParameterException("MargotBACOptions::CanonicityDepthTestingString",
				      canonicityDepthFlagsString,
				      "not a valid depth string");
 
    // Calculate the highest depth at which we want to test canonicity.
    // If no canonicity string was specified, then we always test, which
    // was taken care of by the constructor (highest = INT_MAX).
    highestCanonicityDepth = -1;
    for (int i=numberVariables; i >= 0; --i)
      if (canonicityDepthFlags[i]) {
	highestCanonicityDepth = i;
	break;
      }
  }

  if (orbitDepthFlagsString) {
    orbitDepthFlags = new int[numberVariables];
    if (!parseFlagsFromString(numberVariables, canonicityDepthFlags, canonicityDepthFlagsString))
      throw IllegalParameterException("MargotBACOptions::OrbitDepthTestingString",
				      orbitDepthFlagsString,
				      "not a valid depth string");
  }
}
