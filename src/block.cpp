// block.cpp
//
// By Sebastian Raaphorst, 2004.
//
// $Author$
// $Date$

#include <ostream>
#include <set>
#include "common.h"
#include "block.h"
#include "superduper.h"



Block::Block(int v, int k, int lexnum)
{
  int *block = new int[k];
  duper(v, k, lexnum, block);
  for (int i=0; i < k; ++i)
    points.insert(block[i]);
  delete[] block;
}


Block::Block(int k, int *block)
{
  for (int i=0; i < k; ++i)
    points.insert(block[i]);
}


Block::~Block()
{
}


const std::set< int > &Block::getPoints() const
{
  return points;
}


std::ostream &operator<<(std::ostream &out, const Block &b)
{
  const std::set< int > &points = b.getPoints();
  std::set< int >::const_iterator beginIter = points.begin();
  std::set< int >::const_iterator endIter   = points.end();

  out << "{";

  int counter = 0;
  int bound = points.size()-1;
  for (; beginIter != endIter; ++beginIter, ++counter) {
    out << *beginIter;
    if (counter < bound)
      out << ", ";
  }

  out << "}";

  return out;
}
