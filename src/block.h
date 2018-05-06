// block.h
//
// By Sebastian Raaphorst, 2004.
//
// A convenience class that uses a set of integers to represent
// a collection of points. This class requires superduper to
// have been initialized.
//
// $Author$
// $Date$

#ifndef BLOCK_H
#define BLOCK_H

#include <set>
#include <ostream>
#include "common.h"


class Block
{
 private:
  std::set< int > points;

 public:
  // Given v, k, and a lex number, create the block.
  Block(int, int, int);

  // Given k and an array of size k, create the block.
  Block(int, int*);

  // Destructor
  ~Block();

  // Accessor to the points.
  const std::set< int > &getPoints(void) const;
};


// Printing routine.
std::ostream &operator<<(std::ostream&, const Block&);


#endif
