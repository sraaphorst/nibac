// lexicographicvariableorder.cpp
//
// By Sebastian Raaphorst, 2006.

#include <string.h>
#include <map>
#include <sstream>
#include <string>
#include <baclibrary.h>
#include "colexicographicvariableorder.h"


// Static member declaration.
int ColexicographicVariableOrder::rank(int k, int *subset)
{
  int r = 0;
  for (int i=0; i < k; ++i)
    r += C[subset[k-i-1]][k-i];
  return r;
}


void ColexicographicVariableOrder::unrank(int v, int k, int r, int *subset)
{
  int x = v-1;
  for (int i=0; i < k; ++i) {
    while (C[x][k-i] > r)
      --x;
    subset[k-i-1] = x;
    r -= C[x][k-i];
  }
}


ColexicographicVariableOrder::ColexicographicVariableOrder(int pv, int pk, int plambda)
  : v(pv),
    k(pk),
    lambda(plambda)
{
}


ColexicographicVariableOrder::~ColexicographicVariableOrder()
{
}


void ColexicographicVariableOrder::sort(int len, const int *src, int *dst)
{
  // Find the index of each variable, sort them, and then populate the destination.
  // We do this using an STL multimap.
  std::multimap< int, int > sorter;
  for (int i=0; i < len; ++i)
    sorter.insert(std::pair< const int, int >(variableToIndex(src[i]), src[i]));

  int pos = 0;
  for (std::multimap< int, int >::iterator iter = sorter.begin();
       iter != sorter.end();
       ++iter, ++pos)
    dst[pos] = (*iter).second;
}


int ColexicographicVariableOrder::variableToIndex(int variable)
{
  int *block = new int[k];
  
  // Determine where the block appears in the set of indices representing
  // lambda copies of the same block.
  int blockSet = variable / lambda;
  int positionInBlockSet = variable % lambda;
  
  // Calculate the rank.
  duper(v, k, blockSet, block);
  int r = rank(k, block) * lambda + positionInBlockSet;
  
  // Free memory and return index.
  delete[] block;
  return r;
}


int ColexicographicVariableOrder::indexToVariable(int index)
{
  int *block = new int[k];

  // As lambda copies of each block appear consecutively in the index, we need to
  // convert from repeated indices to a system where each block appears once to
  // use rank and unrank.
  int indexSet = index / lambda;
  int positionInIndexSet = index % lambda;

  // Unrank the adjusted index in the colex order.
  unrank(v, k, indexSet, block);
  int r = super(v, k, block) * lambda + positionInIndexSet;

  // Free memory and return variable.
  delete[] block;
  return r;
}


// *** COLEXICOGRAPHICVARIABLEORDERCREATOR METHODS ***
ColexicographicVariableOrderCreator::ColexicographicVariableOrderCreator()
{
}


ColexicographicVariableOrderCreator::~ColexicographicVariableOrderCreator()
{
}


std::map< std::string, std::pair< std::string, std::string > > ColexicographicVariableOrderCreator::getOptionsMap(void)
{
  std::map< std::string, std::pair< std::string, std::string > > optionsMap;
  return optionsMap;
}


bool ColexicographicVariableOrderCreator::processOptionsString(const char *options)
{
  char ch, eqls;

  // We must explicitly check for empty string prior to processing, since an empty string does
  // not generate an EOF status.
  if (strlen(options) == 0)
    return true;

  std::istringstream stream(options);
  while (!stream.eof()) {
    stream >> ch;
    if (!stream)
      throw IllegalParameterException("ColexicographicVariableOrder::ConfigurationString",
				      options,
				      "could not process string");
    stream >> eqls;
    if (!stream || eqls != '=')
      throw IllegalParameterException("ColexicographicVariableOrder::ConfigurationString",
				      options,
				      "could not process string");

    switch (ch) {
    default:
      std::ostringstream outputstream;
      outputstream << ch;
      throw IllegalParameterException("ColexicographicVariableOrder::ConfigurationString",
				      outputstream.str().c_str(),
				      "not a supported option");
    }

    if (!stream.eof()) {
      stream >> ch;
      if (!stream || ch != ':')
	throw IllegalParameterException("ColexicographicVariableOrder::ConfigurationString",
					options,
					"could not process string");
    }
  }

  return true;
}


VariableOrder *ColexicographicVariableOrderCreator::create(void) const
{
  if (v <= 0)
    throw MissingDataException("ColexicographicVariableOrderCreator requires v to be populated.");
  if (k <= 0)
    throw MissingDataException("ColexicographicVariableOrderCreator requires k to be populated.");
  if (lambda <= 0)
    throw MissingDataException("ColexicographicVariableOrderCreator requires lambda to be populated.");
  return new ColexicographicVariableOrder(v, k, lambda);
}
