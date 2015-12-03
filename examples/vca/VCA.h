// VCA.h
//
// This class, given an ASC and v, tries to find VCAN(ASC, v).

#ifndef VCA_H
#define VCA_H

#include <vector>
#include <baclibrary.h>
#include "ASC.h"
using namespace std;
using namespace sr;

class VCA : public Problem
{
 private:
  // The necessary parameters.
  ASC asc;
  int v;

  // Table to convert between row / column and x variable index.
  int **xtable;

  // Table to convert between row / row / column and y variable index.
  int ***ytable;

  // The solution vector.
  vector< vector< Block > >  solutions;

 public:
  VCA(BACOptions&, const ASC &asc, int,
			Formulation::SolutionType=Formulation::GENERATION);
  virtual ~VCA();

  // Get the vector of solutions.
  vector< vector< Block > > &getSolutions();

  void constructFormulation(void);
  void constructSymmetryGroup(void);
};


#endif
