// constraint.cpp
//
// By Sebastian Raaphorst, 2004.
//
// $Author$
// $Date$


#include <vector>
#include <limits.h>
#include "common.h"
#include "constraint.h"
#include "formulation.h"
#include "lpsolver.h"


// static initializers
unsigned long Constraint::CURRID = 0L;


Constraint::Constraint(Formulation &formulation,
		       std::vector< int > &ppositions,
		       std::vector< int > &pcoefficients,
		       int plowerBound,
		       int pupperBound)
  : id(CURRID++),
    positions(ppositions),
    coefficients(pcoefficients),
    lowerBound(plowerBound),
    upperBound(pupperBound)
{
  // Sort the coefficients
  quicksort(positions, coefficients, 0, positions.size()-1);

  // Create the LP implementationific implementation
  implementation = LPSolver::getInstance()->createConstraint(formulation, positions, coefficients, lowerBound, upperBound);
}


Constraint::~Constraint()
{
  LPSolver::getInstance()->deleteConstraint(implementation);
}


double Constraint::evaluateConstraint(double *variableValues)
{
  double evaluation = 0;

  // Iterate over the coefficients and positions and determine the
  // value for the variable expression of this constraint.
  std::vector< int >::iterator pbeginIter = positions.begin();
  std::vector< int >::iterator pendIter   = positions.end();
  std::vector< int >::iterator cbeginIter = coefficients.begin();
  std::vector< int >::iterator cendIter   = coefficients.end();

  for (; pbeginIter != pendIter; ++pbeginIter, ++cbeginIter)
    evaluation += *cbeginIter * variableValues[*pbeginIter];

  return evaluation;
}


bool Constraint::isViolated(double *variableValues)
{
  double evaluation = evaluateConstraint(variableValues);
  return isviolated(evaluation, lowerBound, upperBound);
}


bool Constraint::isUnviolated(double *variableValues)
{
  double evaluation = evaluateConstraint(variableValues);
  return isunviolated(evaluation, lowerBound, upperBound);
}


bool Constraint::isActive(double *variableValues, double activityTolerance)
{
  double evaluation = evaluateConstraint(variableValues);
  return lessthan(upperBound - evaluation, activityTolerance) && greaterthan(evaluation - lowerBound, activityTolerance);
}


bool Constraint::isInactive(double *variableValues, double activityTolerance)
{
  double evaluation = evaluateConstraint(variableValues);
  return greaterthan(upperBound - evaluation, activityTolerance) || lessthan(evaluation - lowerBound, activityTolerance);
}


Constraint *Constraint::createConstraint(Formulation &formulation,
					 std::vector< int > &ppositions,
					 std::vector< int > &pcoefficients,
					 Sense sense,
					 int pbound)
{
  return createConstraint(formulation,
			  ppositions,
			  pcoefficients,
			  ((sense == EQUALS || sense == GREATERTHAN) ? pbound : INT_MIN),
			  ((sense == EQUALS || sense == LESSTHAN) ? pbound : INT_MAX));
}


Constraint *Constraint::createConstraint(Formulation &formulation,
					 int length,
					 int *ppositions,
					 Sense sense,
					 int pbound)
{
  return createConstraint(formulation,
			  length, 
			  ppositions,
			  ((sense == EQUALS || sense == GREATERTHAN) ? pbound : INT_MIN),
			  ((sense == EQUALS || sense == LESSTHAN) ? pbound : INT_MAX));
}


Constraint *Constraint::createConstraint(Formulation &formulation,
					 std::vector< int > &ppositions,
					 std::vector< int > &pcoefficients,
					 int plowerBound,
					 int pupperBound)
{
  return new Constraint(formulation, ppositions, pcoefficients, plowerBound, pupperBound);
}


Constraint *Constraint::createConstraint(Formulation &formulation,
					 std::vector< int > &positions,
					 Sense sense,
					 int pbound)
{
  std::vector< int > coefficients;
  for (int i=0; i < positions.size(); ++i)
    coefficients.push_back(1);
  return new Constraint(formulation,
			positions,
			coefficients,
			((sense == EQUALS || sense == GREATERTHAN) ? pbound : INT_MIN),
			((sense == EQUALS || sense == LESSTHAN) ? pbound : INT_MAX));
}


Constraint *Constraint::createConstraint(Formulation &formulation,
					 int length,
					 int *ppositions,
					 int plowerBound,
					 int pupperBound)
{
  // Create the vectors
  std::vector< int > positions;
  std::vector< int > coefficients;
  for (int i=0; i < length; ++i) {
    positions.push_back(ppositions[i]);
    coefficients.push_back(1);
  }

  return new Constraint(formulation, positions, coefficients, plowerBound, pupperBound);
}


void Constraint::quicksort(std::vector< int > &sorter, std::vector< int > &corresp, int first, int last)
{
  if (first >= last)
    return;

  // Get a pivot element; we will pick the middle element to be the pivot element to avoid
  // an O(n^2) sorting time if the sequence is already sorted.
  int index = (first + last) / 2;

  // Move the pivot to the beginning of sorter and modify corresp accordingly.
  int tmp = sorter[first];
  sorter[first] = sorter[index];
  sorter[index] = tmp;
  tmp = corresp[first];
  corresp[first] = corresp[index];
  corresp[index] = tmp;

  // Shuffle the elements down as necessary
  int endindex = last;
  int pos;
  for (pos=first+1; pos <= endindex;) {
    if (sorter[pos] > sorter[first]) {
      // Swap pos with endindex, decrement endindex, and continue.
      // The sawp is pointless if pos is endindex.
      if (pos != endindex) {
	tmp = sorter[endindex];
	sorter[endindex] = sorter[pos];
	sorter[pos] = tmp;
	tmp = corresp[endindex];
	corresp[endindex] = corresp[pos];
	corresp[pos] = tmp;
      }
      --endindex;
      continue;
    }
    ++pos;
  }

  // The elements have been shuffled; we now swap the pivot into its proper position, which is
  // namely always pos-1.
  index = pos-1;
  tmp = sorter[index];
  sorter[index] = sorter[first];
  sorter[first] = tmp;
  tmp = corresp[index];
  corresp[index] = corresp[first];
  corresp[first] = tmp;

  // Sort the two halves recursively.
  quicksort(sorter, corresp, first, index-1);
  quicksort(sorter, corresp, index+1, last);
}
