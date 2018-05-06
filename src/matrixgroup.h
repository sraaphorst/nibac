// matrixgroup.h
//
// By Sebastian Raaphorst, 2004.
//
// A general symmetry group based on the possible permutations
// over an incidence matrix, namely all row and column permutations.

#ifndef MATRIXGROUP_H
#define MATRIXGROUP_H

#include "common.h"
#include "schreiersimsgroup.h"


class MatrixGroup : public SchreierSimsGroup
{
 public:
  MatrixGroup(int, int, int**, bool, bool, int* = 0);
  virtual ~MatrixGroup();
};

#endif
