// lpsolver.h
//
// By Sebastian Raaphorst, 2003.
//
// This class encapsulates all of the LP-solver specific tasks so that
// the remainder of the code can remain as general as possible.

#ifndef LPSOLVER_H
#define LPSOLVER_H

#include <limits.h>
#include <vector>
#include "common.h"
#include "formulation.h"
#include "bac.h"
#include "node.h"


// We use this class in a factory-sort of way; when the constructor is
// called for a subclass of this class, the created object is set up
// as the default instance via the instance member. This can then be
// accessed statically using the getInstance() method. If the user
// wants multiple LPSolvers, this must be managed manually.

class LPSolver
{
 protected:
  static LPSolver *instance;

  LPSolver();
  virtual ~LPSolver();

 public:
  inline static LPSolver *getInstance() { return instance; }

  // Set up a formulation. We need to manipulate the data structures:
  // Formulation::data
  // Formulation::model
  virtual void setupFormulation(Formulation&) = 0;
  virtual void cleanupFormulation(Formulation&) = 0;

  // Set the objective function, if required.
  virtual void setObjectiveFunction(Formulation&, std::vector< int >&, int=INT_MIN, int=INT_MAX) = 0;

  // Add and remove constraints.
  virtual void addConstraint(Formulation&, Constraint*) = 0;
  virtual void removeConstraint(Formulation&, Constraint*) = 0;

  // Add and remove cuts.
  virtual void addCut(Formulation&, Constraint*) = 0;
  virtual void removeCut(Formulation&, Constraint*) = 0;

  // Given a node, solve the LP at this node and copy the solution
  // into the node. Should return number of subnodes processed, and
  // -1 for failure. Note that the return value will be 0 if we simply
  // performed LP here and did not process any additional nodes.
  // The third parameter indicates if we should try to solve the node fully here.
  // If this is not possible, it is okay to return a noninteger solution. If an
  // integer solution is returned, the node is assumed to have been fully solved.
  virtual int solveNode(BAC&, Node&, bool=false) = 0;

  // Create and delete implementation specific representations of constraints.
  // These calls are used by the Constraint class and should not be called elsewhere.
  virtual void *createConstraint(Formulation&, std::vector< int >&, std::vector< int >&, int, int) = 0;
  virtual void deleteConstraint(void*) = 0;

  // Fix and unfix variables.
  virtual void fixVariable(Formulation&, int, int) = 0;
  virtual void unfixVariable(Formulation&, int) = 0;

  // Export the LP to a file. If this is not supported, simply return false, as in
  // the default implementation.
  virtual inline bool exportModel(Formulation&, const char *filename) { return false; }

#ifdef DEBUG
  virtual void printVariables(Formulation&) = 0;
#endif
};


#endif
