// design.h
//
// By Sebastian Raaphorst, 2004.
//
// This class, given parameters t, v, k, lambda, and design
// type (i.e. BIBD, packing, or covering), provides the framework
// for generating or enumerating all relevant t-(v, k, lambda)
// structures of the specified type.
//
// $Author$
// $Date$

#ifndef DESIGN_H
#define DESIGN_H

#include <vector>
#include <set>
#include <baclibrary.h>
using namespace std;


class Design : public Problem
{
public:
  enum type {
    DESIGN,
    PACKING,
    COVERING
  };

private:
  // The necessary parameters.
  int t;
  int v;
  int k;
  int lambda;
  type designType;
  bool simpleFlag;

  // A hole, if one is desired.
  set< int > hole;

  // The number of blocks, which is numberVariables / lambda.
  int numberBlocks;

  // The solutions, in nice block format.
  vector< vector< Block > > solutions;

public:
  Design(BACOptions&, int, int, int, int, type=DESIGN, Formulation::SolutionType=Formulation::SEARCH, bool=false);
  virtual ~Design();

  // Introduce a hole into the design.
  inline void setHole(set< int > &phole) { hole = phole; }

  // Overridden methods.
  virtual void constructFormulation(void);
  virtual void determineFixingFlags(int*);
  virtual void constructSymmetryGroup(void);
  virtual void processSolutions(void);

  // Add all of the special clique constraints immediately, if possible. The second method is used internally
  // and takes a k+1 set, using it to create one specific constraint.
  void addSpecialCliqueConstraints();
  void addSpecialCliqueConstraint(int*);

  // Add all of the anti-Pasch constraints.
  // The second method requires a 6-set, over which constraints for all possible pasches are added.
  void addPaschConstraints();
  void addPaschConstraints(int*);

  // Get the vector of solutions.
  vector< vector< Block > > &getSolutions();
};

#endif
