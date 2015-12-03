// constraint.h
//
// By Sebastian Raaphorst, 2004.
//
// A class representing a constraint for a formulation.
//
// $Author$
// $Date$

#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include <vector>
#include "common.h"


// Forward declaration to avoid recursive includes
class Formulation;

class Constraint
{
 private:
  // A unique constraint ID, which we use for columnar information.
  static unsigned long CURRID;

  // Our own implementation of constraints, coupled with the LP solver
  // specific implementation (stored in spec).
  unsigned long id;
  std::vector< int > positions;
  std::vector< int > coefficients;
  int lowerBound;
  int upperBound;
  void *implementation;

  Constraint(Formulation&, std::vector< int >&, std::vector< int >&, int, int);
  static void quicksort(std::vector< int >&, std::vector< int >&, int, int);

 public:
  virtual ~Constraint();
  // TODO: Make these const?
  inline std::vector< int > &getPositions(void) { return positions; }
  inline std::vector< int > &getCoefficients(void) { return coefficients; }
  inline unsigned long getID() const { return id; }
  inline int getLowerBound() const { return lowerBound; }
  inline int getUpperBound() const { return upperBound; }
  inline void *getImplementation() const { return implementation; }
  inline void setImplementation(void *pimplementation) { implementation = pimplementation; }

  // Evaluate the variable part of this constraint using the
  // provided variable values.
  double evaluateConstraint(double*);

  // Check if constraints are violated / unviolated using
  // the provided variable values.
  bool isViolated(double*);
  bool isUnviolated(double*);

  // Check if constraints are active / inactive using
  // the provided variable values.
  bool isActive(double*, double);
  bool isInactive(double*, double);

  // Static methods to create constraints.
  static Constraint *createConstraint(Formulation&, std::vector< int >&, std::vector< int >&, Sense, int);
  static Constraint *createConstraint(Formulation&, std::vector< int >&, std::vector< int >&, int, int);
  static Constraint *createConstraint(Formulation&, std::vector< int >&, Sense, int);
  static Constraint *createConstraint(Formulation&, int, int*, Sense, int);
  static Constraint *createConstraint(Formulation&, int, int*, int, int);
};


#endif
