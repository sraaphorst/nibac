// formulation.h
//
// By Sebastian Raaphorst, 2003.
//
// A formulation of an ILP independent of LP solver.
// We use a structure similar to CPLEX, where we assume a
// sparsely populated matrix and we maintain indices into
// the matrix to represent inequalities. Note that we also
// make the assumption that variables are in Z_2 with coefficients
// in Z and bounds in Z, as these are the types of problems that we
// are interested in studying.
//
// $Author$
// $Date$

#ifndef FORMULATION_H
#define FORMULATION_H

#include <limits.h>
#include <vector>
#include <map>
#include "common.h"
#include "column.h"
#include "constraint.h"


class Formulation
{
 public:
  enum ProblemType {
    PROBLEMTYPE_UNDEFINED,
    MAXIMIZATION,
    MINIMIZATION
  };
  
  // This data type is used to determine whether we are generating
  // one object or enumerating all of them.
  enum SolutionType {
    SOLUTIONTYPE_UNDEFINED,
    SEARCH,
    GENERATION,
    MAXIMALGENERATION,
    ALLGENERATION
  };

 protected:
  ProblemType problemType;
  SolutionType solutionType;
  int numberVariables;
  std::vector< int > objectiveFunction;
  std::map< unsigned long, Constraint* > constraints;
  std::vector< Column > columns;

  // Cuts and fixings.
  std::map< unsigned long, Constraint* > cuts;
  std::map< int, int > fixings;

  // Some LP-solver specific data for the problem, if it should be needed.
  void *data;

  // A problem can have branching variables and non-branching variables;
  // non-branching variables are entirely dependent on branching variables
  // and while they may form an integral part of the problem formulation,
  // we do not consider them in solutions, in group algorithms, etc...
  // A good example of such variables is the use of additional variables
  // to make a non-linear ILP linear. To use these additional variables,
  // organize the variables in the following way:
  // BRANCHING NONBRANCHING
  // Do not attempt to fix the non-branching variables, as this is not
  // necessary since they are usually entirely dependent on the branching
  // variables. This parameter then specifies the first index of the
  // first non-branching variable. Default value is -1 to indicate that
  // there are no non-branching variables.
  int nonBranchingIndex;

 public:
  Formulation(ProblemType, SolutionType, int, int=-1);
  virtual ~Formulation();

  // Retrieve the problem type and the solution type.
  inline ProblemType getProblemType(void) { return problemType; }
  inline SolutionType getSolutionType(void) { return solutionType; }

  // Ways to manipulate the objective function
  void setObjectiveFunction(std::vector< int >&, int=INT_MIN, int=INT_MAX);
  inline const std::vector< int > &getObjectiveFunction(void) { return objectiveFunction; }

  // Adding / removing a constraint
  void addConstraint(Constraint*);
  void removeConstraint(Constraint*);
  inline const std::map< unsigned long, Constraint* > &getConstraints() { return constraints; }
  inline const std::vector< Column > &getColumns() { return columns; }

  // Adding / removing a cut
  void addCut(Constraint*);
  void removeCut(Constraint*);
  inline const std::map< unsigned long, Constraint* > &getCuts() { return cuts; }

  // Fixing variables. This is used to set constraints in the formulation with variables
  // fixed to certain values, and then to remove them.
  void fixVariable(int, int);
  void unfixVariable(int);

  // Overridden methods from ILP used in determining symmetry groups.
  inline virtual int getNumberVariables() { return numberVariables; }
  inline virtual int getNumberColumns() { return numberVariables; }
  inline virtual int getNumberRows() { return constraints.size(); }
  inline virtual int getObjectiveCoefficient(int index) { return objectiveFunction[index]; }
  virtual int getBCoefficient(int);
  virtual int getMatrixCoefficient(int, int);

  // Methods to alter the model and data
  inline void setData(void *pdata) { data = pdata; }
  inline void *getData() const { return data; }

  // Non-branching variable index.
  inline int getNonBranchingIndex(void) { return nonBranchingIndex; }
  inline void setNonBranchingIndex(int pnonBranchingIndex) { nonBranchingIndex = pnonBranchingIndex; }

  // Number of branching variables.
  inline int getNumberBranchingVariables(void) { return (nonBranchingIndex > -1 ? nonBranchingIndex : numberVariables); }

  // Export the formulation to a file.
  bool exportModel(const char*);

  // Evaluate the objective function for this solution vector.
  double evaluateObjectiveFunction(double*);

  // Check the feasibility of a partial solution.
  bool checkPartialFeasibility(short int*);

 private:
  // Method used to sort a vector of variable indices and maintain a corresponding
  // array of variable coefficients.
  void quicksort(std::vector< int >&, std::vector< int >&, int, int) const;
};


#endif
