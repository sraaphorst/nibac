// incidencematrixdesign.h
//
// This class, given parameters v, k, and lambda,
// provides a framework to generate or enumerate
// 2-(v, k, lambda) designs using the incidence matrix
// representation.

#ifndef INCIDENCEMATRIXDESIGN_H
#define INCIDENCEMATRIXDESIGN_H

#include <vector>
#include <baclibrary.h>
using namespace std;


class IncidenceMatrixDesign : public Problem
{
 private:
  // The necessary parameters.
  int v;
  int k;
  int lambda;
  int b;

  // Table to convert between row / column and x variable index.
  int **xtable;

  // Table to convert between row / row / column and y variable index.
  int ***ytable;

  // The solution vector.
  vector< vector< Block > >  solutions;

 public:
  IncidenceMatrixDesign(BACOptions&, int, int, int,
			Formulation::SolutionType=Formulation::GENERATION);
  virtual ~IncidenceMatrixDesign();

  // Get the vector of solutions.
  vector< vector< Block > > &getSolutions();

  void constructFormulation(void);
  void determineFixingFlags(int*);
  void constructSymmetryGroup(void);
  void processSolutions(void);
};


#endif
