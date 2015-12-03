// margotbac.h
//
// By Sebastian Raaphorst, 2003.
//
// This is a subclass of BAC that implements some of the algorithms
// proposed by Dr. Francois Margot in some of his papers. These
// algorithms perform things like isomorphism testing, 0-fixing, etc...
// They rely on the BAC tree being processed in a depth-first fashion
// with lowest index branching.
//
// $Author$
// $Date$

#ifndef MARGOTBAC_H
#define MARGOTBAC_H

#include <limits.h>
#include "common.h"
#include "bac.h"
#include "cutproducer.h"
#include "formulation.h"
#include "group.h"
#include "margotbacoptions.h"
#include "solutionmanager.h"
#include "statistics.h"


// Forward class declaration.
class BACOptions;


class MargotBAC : public BAC
{
 protected:
  // This group represents the symmetry group of the root node.
  Group &rootGroup;

  // This array is used for determining isomorphism of a node, and
  // we maintain it here. How it works is that for x_i set to 1,
  // part_zero[i] = numvars - numvarsfixedto0.
  int *part_zero;

 public:
  MargotBAC(Formulation&, Group&, MargotBACOptions&);
  virtual ~MargotBAC();

 protected:
  virtual void initialize();

  // We perform 0-fixing in the preprocessing stage.
  virtual int preprocess(Node&);

  // In generation, we only want nonisomorphic solutions. This is
  // called by BAC::solve whenever we're doing some kind of generation.
  virtual int checkSolutionForGeneration(Node&);

  // A method that may be called directly on a node to test canonicity.
  bool testCanonicity(Node&);

  // This method is called by a node whenever a variable is fixed to 1.
  // It handles all crucial tasks, like updating the part_zero array,
  // indicating that a variable was fixed, adjusting the base, etc...
  virtual bool fixVariableTo1(Node&, int, bool=true);

  // This method is called by a node whenever a variable is fixed to 0.
  // It handles all the crucial tasks, like adjusting the base, setting
  // the orbit of the variable under the stabilizer of F_1 to 0, etc...
  virtual bool fixVariableTo0(Node&, int, bool=true);

 private:
  // The 0-fixing routine.
  void fix0(Node&);

 public:
  // This method is used by cut generators to access information required
  // to produce certain cuts.
  inline int *getPartZero(void) { return part_zero; }
};

#endif
