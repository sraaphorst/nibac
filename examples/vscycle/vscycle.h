// vscycle.h
//
// By Sebastian Raaphorst, 2004.
//
// Given an n, this program will generate or enumerate vector space basis
// cycles for all subspaces of dimension 2 over the vector space F_2^n.
//
// $Author$
// $Date$

#ifndef VSCYCLE_H
#define VSCYCLE_H

#include <vector>
#include <baclibrary.h>
using namespace std;


class VSCycle : public Problem
{
 private:
  // The necessary parameters.
  int n;

  // Derived parameters.
  int numv;
  int numpos;

  // Table to convert between row / column and x variable index.
  int **table;

  // Table to convert between two x variable indices and a y variable index.
  int **ytable;

  // A list of subspaces and all vectors in them.
  int **subspaces;

  // The solution vector.
  vector< vector< int > > solutions;

 public:
  VSCycle(BACOptions&, int, Formulation::SolutionType=Formulation::GENERATION);
  virtual ~VSCycle();

  // Get the vector of solutions.
  vector< vector< int > > &getSolutions();

 private:
  void constructDefaultFormulation();
  void determineFixings();
  void constructSymmetryGroup();
  void processSolutions();

  static int twoToN(int);
  static int qBinCoeff(int, int);
  static int calcNumVars(int, bool=false);
};


#endif

