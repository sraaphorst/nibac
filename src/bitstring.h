// bitstring.h
//
// By Sebastian Raaphorst, 2003.
//
// Efficient representation of a variable length string of bits.
// This is used by GeneratedGroup::findSymmetryGroup2, which uses
// partitioning to determine the symmetry group.
//
// $Author$
// $Date$

#ifndef BITSTRING_H
#define BITSTRING_H

#include "common.h"


class bitstring
{
 private:
  unsigned long *bits;
  int size;
  bool flipped;

 public:
  bitstring();
  virtual ~bitstring();

  bitstring &operator=(const bitstring&);
  bitstring &operator=(const unsigned long);
  int operator[](int) const;
  bool operator==(const bitstring&) const;
  bool operator==(const unsigned long) const;
  bool operator!=(const bitstring&) const;
  bool operator!=(const unsigned long) const;
  bool operator<(const bitstring&) const;

  void set(int, int=1);
  int get(int) const;
  int getSize() const;
  void flip();
  void band(const bitstring&);
  void bor(const bitstring&);
  void bxor(const bitstring&);

  int getBitCount() const;

 private:
  void resize(int);

#ifdef DEBUG
  // Friend functions
  friend std::ostream &operator<<(std::ostream&, const bitstring&);
#endif
};

#ifdef DEBUG
// Output function
std::ostream &operator<<(std::ostream&, const bitstring&);
#endif

#endif
