// bac.h
//
// By Sebastian Raaphorst, 2003.
//
// This is a very simple class that provides branch-and-cut functionality.
// Given a model (cast as void*), assuming that an instance of LPSolver has
// been linked into this class (this is checked), we solve the ILP model
// using no cuts and simple branch-and-bound techniques. Subclasses of this
// class (e.g. MargotBAC) extend this functionality to employ cuts, exploit
// symmetries, etc...
//
// $Author$
// $Date$

#ifndef BAC_H
#define BAC_H

#include "common.h"
#include "formulation.h"
#include "group.h"
#include "node.h"
#include "nodestack.h"
#include "statistics.h"


// Class forward declaration.
class BACOptions;


class BAC
{
  // Friend classes
  // Node is a friend so it can call protected members like fixVariableTo0/1.
  friend class Node;

protected:
  // The problem itself.
  Formulation &formulation;

  // The options for the B&C.
  BACOptions &options;
  
  // Container to hold the nodes.
  NodeStack *nodeStack;

  // The value of the best solution found so far.
  int bestSolutionValue;

public:
  BAC(Formulation&, BACOptions&);
  virtual ~BAC();

protected:
  // Initialization routine.
  virtual void initialize(void);

  // These functions are called by the branch-and-cut routines, and while
  // they can be overridden in order to add functionality, they should
  // never be called externally to this object.
  virtual int preprocess(Node&);
  virtual bool isIntegerSolution(Node&, int&);
  virtual void determineViolationBounds(int, double&, double&);
  virtual int checkSolutionForGeneration(Node&);
  virtual int process(Node&);
  virtual int postprocess(Node&);
  virtual void removeInactiveCuts(Node&);
  virtual bool isSolved(Node&);
  virtual bool isMaximal(Node&);

  // This method sets a variable at a node to 1.
  virtual bool fixVariableTo1(Node&, int, bool=true);

  // This method sets a variable at a node to 0.
  virtual bool fixVariableTo0(Node&, int, bool=true);

public:
  // Begins solving the ILP
  // TODO: Should this be virtual? This seems like a bad idea.
  virtual void solve();

  // Get the formulation.
  inline Formulation &getFormulation() const { return formulation; }

  // Get the options.
  inline BACOptions &getOptions() { return options; }
};


#endif
