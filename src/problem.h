// problem.h
//
// By Sebastian Raaphorst, 2004.
//
// An abstract framework for constructing problems for the BAC library.
// While it is not necesssary to use this class, it does provide a
// standardized approach to constructing a problem. and streamlines the
// approach, ensuring that everything is done in the correct order.
// For examples of concrete implementations of this class, see the design
// and intersectingsetsystem programs.
//
// $Author$
// $Date$

#ifndef PROBLEM_H
#define PROBLEM_H

#include "common.h"
#include "bacoptions.h"
#include "branchingscheme.h"
#include "formulation.h"
#include "group.h"
#include "margotbacoptions.h"
#include "solutionmanager.h"


class Problem
{
public:
  const static int FREE;
  const static int FIXEDTO0;
  const static int FIXEDTO1;

protected:
  // The B&C options.
  BACOptions &options;

  // The type of problem.
  Formulation::ProblemType  problemType;
  Formulation::SolutionType solutionType;

  // The number of variables.
  int numberVariables;

  // The number of branching variables.
  int numberBranchingVariables;

  // A list of variables for the problem that allows fixings to be specified.
  // fixings[i] = FREE (default) indicates that a variable is free.
  // fixings[i] = FIXEDTO0 indicates that a variable is fixed to 0.
  // fixings[i] = FIXEDTO1 indicates that a variable is fixed to 1.
  // Imposing a fixing on variables will alter the variable order, which is
  // fine. The order will still be respected by the VariableOrder, but will
  // be modified so that the variables fixed to 1 appear before all other
  // variables, followed by those fixed to 0, followed by the free variables.
  int *fixingFlags;

  // The problem formulation.
  Formulation formulation;

  // The symmetry group. Default value is null, in which case, BAC will be used instead
  // of MargotBAC.
  Group *group;

  // Perform any necessary initialization on the problem.
  // This method is currently empty, as most tasks that must be done here will be
  // handled either in the constructor or will already have been established in the
  // BACOptions.
  virtual void initialize(void);

  // Construct the formulation.
  // NOTE: No fixings should be performed here. This will be taken care of by NIBAC
  // by setting the fixingFlags member.
  virtual void constructFormulation(void) = 0;

  // NOTE ABOUT FIXINGS.
  // The programmer should not manually configure the BACOptions initial fixings sets. This
  // will be handled by Problem, and instead the programmer using Problem should focus on
  // the fixingFlags. Ultimately, whether or not manual fixings are specified through
  // CommandLineProcessing, the program is used without CommandLineProcessing and without
  // Problem and the sets in BACOptions are configured by the programmer, or Problem is
  // used and the programmer configures fixingFlags, the same effect will be achieved.
  
  // Set up the fixing flags for the problem. The default implementation of this method assumes
  // no fixings unless specifically dictated by the user (i.e. BACOptions::getManualFixings()).
  // If no manual fixings have been set up, it invokes the determineFixingFlags() method, which
  // allows the programmer to manually configure this array, which has size numberVariables and
  // is indexed by variables.
  //
  // While programmers may override this method, they are STRONGLY cautioned against doing so.
  // Not doing so will ensure consistent behaviour across all subclasses of Problem.
  virtual void setupFixingFlags(void);

  // Set up the fixingFlags member array, which is passed in.
  // Initially, this array is initialized entirely to FREE, so only the variables to be set to
  // 0 or 1 need to be explicitly set to FIXEDTO0 or FIXEDTO1 respectively. The default
  // implementation of this does nothing, i.e. all variables are left free.
  virtual void determineFixingFlags(int*);

  // Establish an initial base for the symmetry group given the variable fixings.
  // The implementation of this method in this class handles this task automatically based
  // on the fixingFlags and the variable order in the BACOptions, choosing the best base
  // for the SchreierSimsScheme with regards to these considerations (F_1 F_0 F, with
  // each block sorted according to variable order).
  //
  // While programmers may override this method, they are STRONGLY cautioned against doing so.
  // If necessary, ensure that all functionality performed by this method is properly duplicated.
  virtual void establishInitialBase(int*);

  // Construct the symmetry group for the problem.
  // This default method simply sets the group to the null pointer, i.e.
  // implying that we want to run pure B&C. The group can either be constructed
  // explicitly, or discovered dynamically using the methods of GeneratedGroup,
  // or conveniently by using one of the subclasses of SchreierSimsGroup.
  // Note that any subgroup of the symmetry group can be specified, although specifying
  // a proper subgroup will likely result in the output of isomorphic solutions.
  //
  // Call establishInitialBase to get the base for the group.
  virtual void constructSymmetryGroup(void);

  // This method is called after the problem is solved, and provides a chance for the programmer
  // to convert solutions in characteristic vector format into one that makes more sense to the
  // program user (e.g. unrank block indices, etc). In the default implementation, it does nothing.
  virtual void processSolutions(void);

 public:
  // NOTE: Before these constructors are called, they rely on the BACOptions / MargotBACOptions
  // to be fully configured and populated. This can easily be done by using the convenience
  // class, CommandLineProcessing, which provides a standard way to accept command line
  // parameters and offer a variety of different cut producers, solution managers, variable
  // orders, branching schemes, etc. If one does not wish to offer command line invocation
  // or does not wish to allow the user such specific configuration, this is not necessary,
  // of course, and one need only configure the options by hand. This should not be too
  // difficult, as sensible defaults are usually provided, although the aforementioned
  // cut producers, solution manager, variable order, and branching scheme will need to
  // be set up.
  Problem(BACOptions&, Formulation::ProblemType, Formulation::SolutionType, int, int=-1);
  virtual ~Problem();

  // This method invokes all of the previously defined ones in the appropriate order, creates the
  // algorithm, solves the problem, and then processes the solutions. The solutions themselves
  // are not output, and this must be handled by the calling code.
  virtual void solve(void);

  // Retrieve the number of variables.
  inline int getNumberVariables(void) const { return numberVariables; }
};

#endif
