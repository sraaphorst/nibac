// cplexsolver.h
//
// By Sebastian Raaphorst, 2003.
//
// An interface between the generic BAC framework and CPLEX. This is a
// concrete implementation of LPSolver.

#ifndef CPLEXSOLVER_H
#define CPLEXSOLVER_H

#include <ilcplex/ilocplex.h>
#include <limits.h>
#include <vector>
#include <map>
#include "common.h"
#include "formulation.h"
#include "bac.h"
#include "node.h"
#include "lpsolver.h"


class CPLEXSolver : public LPSolver
{
 private:
  static CPLEXSolver _cplexsolver;

  // Data structure to hold the relevant information for a formulation.
  // This organization may seem profoundly ridiculous, but since IloCplex::solveRelaxation
  // crashes on repeated calls for MIPs, and IloNumVarArray::setType does not exist despite
  // documentation to the contrary, I can find no way to alternate between solving LP relaxations
  // and solving ILPs using CPLEX without maintaining two sets of variables (one Int, one Float)
  // and otherwise identical models and algorithms.
  struct CPLEXInfo {
    std::map< int, IloRange* > fixingsL;
    std::map< int, IloRange* > fixingsI;
    IloNumVarArray *varsL;
    IloNumVarArray *varsI;
    IloModel *modelL;
    IloModel *modelI;
    IloCplex *cplexL;
    IloCplex *cplexI;
  };

  struct ConstraintInfo {
    IloRange *rangeL;
    IloRange *rangeI;
  };

  // The CPLEX environment object.
  IloEnv env;

 protected:
  CPLEXSolver();
  virtual ~CPLEXSolver();

 public:
  virtual void setupFormulation(Formulation&);
  virtual void cleanupFormulation(Formulation&);
  virtual void setObjectiveFunction(Formulation&, std::vector< int >&, int=INT_MIN, int=INT_MAX);
  virtual void addConstraint(Formulation&, Constraint*);
  virtual void removeConstraint(Formulation&, Constraint*);
  virtual void addCut(Formulation&, Constraint*);
  virtual void removeCut(Formulation&, Constraint*);
  virtual void fixVariable(Formulation&, int, int);
  virtual void unfixVariable(Formulation&, int);
  virtual int solveNode(BAC&, Node&, bool=false);

  void *createConstraint(Formulation&, std::vector< int >&, std::vector< int >&, int, int);
  void deleteConstraint(void*);

  bool exportModel(Formulation&, const char*);

#ifdef DEBUG
  void printVariables(Formulation&);
#endif
};


#endif
