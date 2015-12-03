// margotbac.cpp
//
// By Sebastian Raaphorst, 2003.
//
// $Author$
// $Date$

#include <set>
#include <iostream>
#include <sstream>
#include "common.h"
#include "margotbac.h"
#include "bac.h"
#include "bacoptions.h"
#include "cutproducer.h"
#include "formulation.h"
#include "group.h"
#include "lpsolver.h"
#include "margotbacoptions.h"
#include "nibacexception.h"
#include "node.h"
#include "solutionmanager.h"
#include "statistics.h"


MargotBAC::MargotBAC(Formulation &pformulation,
		     Group &prootGroup,
		     MargotBACOptions &poptions)
  : BAC(pformulation, (BACOptions&)poptions),
    rootGroup(prootGroup),
    part_zero(new int[pformulation.getNumberBranchingVariables()+1])
{
  // Complete initialization on the MargotBACOptions.
  // This gives us the highestCanonicityDepth, which we will need.
  // TODO: This *should* work with just the branching variables, but to
  // play it safe from the user perspective, who may specify non-branching,
  // for now we will leave it as all variables.
  poptions.initializeDepthFlags(pformulation.getNumberVariables());

  // This is not necessary, but we do it all same just in case.
  memset(part_zero, 0, (pformulation.getNumberBranchingVariables()+1) * sizeof(int));
}


MargotBAC::~MargotBAC()
{
  delete[] part_zero;
}


void MargotBAC::initialize(void)
{
  // Tell the statistics how many cutters we are using.
  options.getStatistics().setNumberCutProducers(options.getCutProducers().size());

  // Create the default node. Note that this node is handled by the nodestack and
  // we do not need to deallocate it.
  Node *node = new Node(*this,
			formulation,
			&rootGroup,
			formulation.getNumberVariables(),
			formulation.getNumberBranchingVariables(),
			&(options.getInitial0Fixings()),
			&(options.getInitial1Fixings()));

  // We must have a branching scheme defined in order to create a NodeStack.
  if (options.getBranchingScheme() == 0)
    throw NoBranchingSchemeException();

  // Create the nodestack and add the new node to it.
  nodeStack = new NodeStack(*(options.getBranchingScheme()), node, options.getStatistics());
}


bool MargotBAC::fixVariableTo1(Node &node, int variable, bool ignored)
{
#ifdef DEBUG
  std::cerr << "\t= Fixing " << variable << " to 1." << std::endl;
#endif

  Group *group = node.getSymmetryGroup();
  MargotBACOptions &margotOptions = (MargotBACOptions&) options;

  // We first need to move this variableent to the appropriate position in the base if it
  // is not already there. Theoretically, if variable is the branching variable (as it should
  // be), it *should* be there, because 0-fixing will ensure that it is there (it will be
  // moved there to test for canonicity), but we check, just to make sure.
  // We only do this if canonicity testing is important at this depth and we are still checking
  // canonicity, or if this is an enumeration.

  // We determine that we are - or will be - checking canonicity in the following cases:
  // 1. We are enumerating and we want to check final designs for canonicity.
  // 2. We have not turned off canonicity testing at this node.
  // 3. We have turned off canonicity testing at this node, and we will turn it on in the future.
  bool checkingCanonicity = (formulation.getSolutionType() != Formulation::GENERATION && margotOptions.getTestFinalSolutions()) ||
    (node.shouldTestCanonicity() &&
     ((!margotOptions.getOrbitDepthFlags() || margotOptions.getOrbitDepthFlags()[node.getDepth()])
      || (margotOptions.getCanonicityDepthFlags() && margotOptions.getCanonicityDepthFlags()[node.getDepth()])));
  int num1fixed = node.getNumberFixedVariables() - node.getNumber0FixedVariables();
  if (checkingCanonicity) {
    if (group->getPosition(variable) != num1fixed)
      group->down(group->getPosition(variable), num1fixed);
  }

  // Tell the node that we have fixed a variable, and update part_zero.
  node.fixVariable(variable, 1);
  part_zero[num1fixed] = node.getNumberBranchingVariables() - node.getNumber0FixedVariables();

  return true;
}
  

bool MargotBAC::fixVariableTo0(Node &node, int variable, bool forceflag)
{
  Group *group = node.getSymmetryGroup();

  // If we are not 0-fixing at this point, we don't want to calculate the orbit.
  // We just fix this variableent to 0 without calculating the orbit.
  MargotBACOptions &margotOptions = (MargotBACOptions&) options;
  if (!(node.shouldTestCanonicity())
      || (margotOptions.getOrbitDepthFlags() && margotOptions.getOrbitDepthFlags()[node.getDepth()] == FALSE)) {
#ifdef DEBUG
    std::cerr << "\t= Fixing " << variable << " to 0." << std::endl;
#endif

    // We determine that we are - or will be - checking canonicity in the following cases:
    // 1. We are enumerating and we want to check final designs for canonicity.
    // 2. We have not turned off canonicity testing at this node.
    // 3. We have turned off canonicity testing at this node, and we will turn it on in the future.
    bool checkingCanonicity = (formulation.getSolutionType() != Formulation::GENERATION && margotOptions.getTestFinalSolutions()) ||
      (node.shouldTestCanonicity() &&
      ((!margotOptions.getOrbitDepthFlags() || margotOptions.getOrbitDepthFlags()[node.getDepth()])
       || (margotOptions.getCanonicityDepthFlags() && margotOptions.getCanonicityDepthFlags()[node.getDepth()])));

    // Down the variable if it is going to be useful in the future to do so.
    // TODO: FOR SOME REASON, IF WE DON'T DOWN HERE, WE FAIL AND ENTER INTO ENDLESS DEPTH!
    if (checkingCanonicity) {
      // Make sure it isn't already set to 0.
      int pos = node.getNumberBranchingVariables() - node.getNumber0FixedVariables() - 1;
      int oldpos = group->getPosition(variable);
      if (oldpos > pos)
	return false;

      group->down(oldpos, pos);
    }

    // Tell the node to fix the variable to 0.
    node.fixVariable(variable, 0);

    // We have completed the task successfully and can return.
    return true;
  }

  // If we reach this point, we want to set the variable and its orbit under
  // the stabilizer to 0.
#ifdef DEBUG
  std::cerr << "\t= Fixing " << variable << " and its orbit to 0." << std::endl;
#endif
  Statistics &statistics = options.getStatistics();

  // Determine some necessary or useful values.
  int num = node.getNumberBranchingVariables();
  int numfixed = node.getNumberFixedVariables();
  int num0fixed = node.getNumber0FixedVariables();
  int num1fixed = numfixed - num0fixed;
  int f0index = num - num0fixed;
  int pos = f0index-1;

  // We cannot 0-fix anything set to 1, obviously.
  assert(group->getPosition(variable) >= num1fixed);

  // If the variable has already been fixed to 0, we have nothing to do.
  if (group->getPosition(variable) >= num - num0fixed)
    return false;

  // We now calculate the orbit in the stabilizer. As our variableent is in the base
  // position num1fixed, we can use this as our index to pass to the algorithm.
  std::set< int > orbit;
#ifdef DEBUG
  std::cerr << "\t+ Testing canonicity of " << variable << " in stab of ";
  for (int i=0; i < num1fixed; ++i)
    std::cerr << group->getBaseElement(i) << " ";
  std::cerr << std::endl;
#endif

  // If we are forcing this variable to 0 (e.g. as in pre-fixing), we're not interested
  // in testing canonicity, which is why we pass !forceflag to the canonicity tester /
  // orbit calculator, indicating that we don't care about canonicity.
  statistics.reportCanonicityCall();
  bool canonical = group->isCanonicalAndOrbInStab(variable, num1fixed, orbit, part_zero,
					      !forceflag, true, node.ancestorsCanonical());

  // If there are not enough variables in the orbit to satisfy the threshold specified in
  // MargotBACOptions, we turn off canonicity testing for this branch.
  if (orbit.size() < margotOptions.getOrbitThreshold())
    node.setTestCanonicity(false);

  // If we are not canonical, we need to inform the statistics about this.
  if (!canonical) {
    statistics.reportCanonicityRejection();
    statistics.reportNonCanonicalDepth(node.getDepth());
  }

#ifdef DEBUG
  std::cerr << "\t- Canonical: " << (canonical ? "yes" : "no")
       << " with orbit ";
  std::set< int >::iterator dbeginIter = orbit.begin();
  std::set< int >::iterator dendIter   = orbit.end();
  for (; dbeginIter != dendIter; ++dbeginIter)
    std::cerr << *dbeginIter << " ";
  std::cerr << std::endl;
#endif

  // If setting this variable to 1 will give us a canonical solution, we don't
  // want to 0-fix it unless we are explicitly forced (via forceflag) or unless
  // setting it to 1 will give us an infeasible solution (in which case, the only
  //  option is to set it to 0).
  if (canonical && !forceflag) {
    // Check feasibility of the partial solution.
    // Instead of copying the partial solution from the node, altering it, and
    // then deleting it, we change it temporarily and restore.
    short int *soln = node.getPartialSolutionArray();
    soln[variable] = 1;
    bool isfeasible = formulation.checkPartialFeasibility(soln);
    soln[variable] = -1;

    // If the solution is feasible, we cannot 0-fix.
    if (isfeasible)
      return false;
  }

  // In order to obtain the optimal base structure for efficiency, we would prefer to
  // set the variables to 0 in decreasing order. Thus, the smallest index variable (which
  // should be variable) will be fixed lastly to 0 and will appear earliest in the base.
  // Hence, we will require less downing in the future in order to move the smallest
  // base variable to the first free position when we backtrack.
  std::set< int >::reverse_iterator rbeginIter = orbit.rbegin();
  std::set< int >::reverse_iterator rendIter   = orbit.rend();

  // Keep track of the number of variables we're fixing to 0.
  int num0fixes = 0;

  // TODO: Possible optimization
  // If we reach variable in the orbit and variable wasn't already fixed, then
  // we probably don't need to consider anything less than variable, so
  // we might be able to break and just stop.

  for (; rbeginIter != rendIter; ++rbeginIter) {
    // The variable *should* be free. There are a few circumstances in which
    // it might not be (i.e. this call comes from an initial fixing specified
    // by the user, in which case, some of the other variables in the orbit
    // might have been set to 0, or if the variable was already set to 0).
    // Because of this, we simply check if the variable is fixed, and if
    // it is, we loop.
    if (group->getPosition(*rbeginIter) < num1fixed || group->getPosition(*rbeginIter) > pos)
      // This variable is already 1 (< num1fixed) or 0 (> pos), so
      // we ignore it and loop.
      continue;

    // Tell the node to fix this variable to 0.
    node.fixVariable(*rbeginIter, 0);

    // Down the variable as required. As f0index is the position that marks
    // the start of F_0, we want it to be moved into pos.
    if (group->getPosition(*rbeginIter) < pos)
	group->down(group->getPosition(*rbeginIter), pos);
    --pos;
    ++num0fixes;
  }

  // Tell the statistics what we've fixed to 0.
  // TODO: If forceflag is true, should we reduce this count by 1 to indicate that
  // one of the variables was set by branching?
  (statistics.getVariableFixingCountByDepth())[node.getDepth()] += num0fixes;

  return true;
}


void MargotBAC::fix0(Node &node)
{
  // The assumption here is that:
  // 1. The node is canonical (guaranteed by 0-fixings in parent nodes)
  // 2. If this node was reached by branching on 0 on f, then
  //    fixTo0(n, f) has been called.

  // We loop indefinitely, ending when the canonicity test fails.
  // We continuously check the smallest free variable to see if
  // setting it to 1 produces a canonical node.
  int smallestfree;

  // Useful, frequently used values that we precalculate.
  int num1fixed = node.getNumberFixedVariables() - node.getNumber0FixedVariables();

  for (;;) {
    // Determine what is the smallest free variable at the node (this is simply
    // the first variable in the sorted set of free variables at the node).
    smallestfree = node.getLowestFreeVariableIndex();

    // If all the variables are fixed, then we cannot continue and we have
    // reached an integer solution.
    if (smallestfree == -1)
      break;

    // Set up the part_zero array and try to set the variable to 0.
    part_zero[num1fixed] = node.getNumberBranchingVariables() - node.getNumber0FixedVariables();

    // Try to fix to 0 smallestfree and its orbit under the stabilizer,
    // provided that the canonicity test holds.
    if (!fixVariableTo0(node, smallestfree, false))
      break;
  }
}


int MargotBAC::preprocess(Node &node)
{
  // Only preprocess if we want to do so at this node.
  MargotBACOptions &margotOptions = (MargotBACOptions&) options;

  // If we are performing 0-fixing at this node, we must perform canonicity
  // testing as well; they are inherently linked and cannot be separated.
  // The default is to always 0-fix.
  if (node.shouldTestCanonicity() && (!margotOptions.getOrbitDepthFlags() || margotOptions.getOrbitDepthFlags()[node.getDepth()])) {
    options.getStatistics().getMargotTimer().start();
    fix0(node);

    // We are always canonical if we have 0-fixed.
    node.setCanonical(true);

    options.getStatistics().getMargotTimer().stop();
    return TRUE;
  }

  // If we reach this point, we haven't peformed complete canonicity testing
  // on the node with regards to Margot's algorithms, so we instruct the node
  // that an ancestor may not be canonical.
  // TODO: Can we move this below the next test?
  node.setAncestorsCanonical(false);

  // Otherwise, we are not interested in 0-fixing, but we may be interested
  // in canonicity testing. This is perfectly fine and we allow it. The default
  // option is to never test canonicity exclusively unless otherwise explicitly
  // specified, unlike 0-fixing where the default is to always 0-fix.
  if (margotOptions.getCanonicityDepthFlags() && margotOptions.getCanonicityDepthFlags()[node.getDepth()])
    if (node.isCanonical())
      return TRUE;
    else
      return (testCanonicity(node) ? TRUE : FALSE);

  // If we aren't interested in either, we simply return true. Our work
  // here is done.
  return TRUE;
}


int MargotBAC::checkSolutionForGeneration(Node &node)
{
  MargotBACOptions &margotOptions = (MargotBACOptions&) options;

  // If we don't care about solutions being canonical, we always return true.
  if (!margotOptions.getTestFinalSolutions())
    return TRUE;

  // If this node has been marked canonical, then an ancestor was tested for canonicity
  // and all nodes on the branch leading to this leaf represent variables fixed to 0,
  // so we are automatically canonical.
  if (node.isCanonical())
    return TRUE;

  // If the node is solved and we didn't 0-fix, we need to test canonicity here.
  // If we have 0-fixed, then we are already guaranteed canonicity, as this
  // method will only ever be called if the node is solved (i.e. no free variables).
  // This should be covered by node.isCanonical, but we do this just in case.
  if (!margotOptions.getOrbitDepthFlags() || margotOptions.getOrbitDepthFlags()[node.getDepth()])
    return TRUE;

  // Test canonicity.
  return (testCanonicity(node) ? TRUE : FALSE);
}


bool MargotBAC::testCanonicity(Node &node)
{
  // Dummy set for orbits since we're not interested in them.
  static std::set< int > dummy;

  Group *group = node.getSymmetryGroup();
  Statistics &statistics = options.getStatistics();

  // By Margot's paper, part_zero[fixed1+1] must be set so that
  // F_0 = {B[part_zero[fixed1p1]], ..., B[numvars-1]}.
  // We take care of this here to avoid any indiscrepancies.
  int fixed1p1 = node.getNumberFixedVariables() - node.getNumber0FixedVariables();
  part_zero[fixed1p1] = node.getNumberBranchingVariables() - node.getNumber0FixedVariables();

  options.getStatistics().getMargotTimer().start();
  int last1fixed = node.getNumberFixedVariables() - node.getNumber0FixedVariables() - 1;
  statistics.reportCanonicityCall();
  bool canonical = group->isCanonicalAndOrbInStab(group->getBaseElement(last1fixed), last1fixed,
					      dummy, part_zero, true, false,
					      node.ancestorsCanonical());
  node.setCanonical(canonical);

  // If we are not canonical, we need to inform the statistics about this.
  if (!canonical) {
    statistics.reportCanonicityRejection();
    statistics.reportNonCanonicalDepth(node.getDepth());
  }

  options.getStatistics().getMargotTimer().stop();
  return canonical;
}
