// VCA.cpp
//
// By Sebastian Raaphorst, 2011.

#include <iostream>
#include <fstream>
#include <vector>
#include <baclibrary.h>
#include "ASC.h"
#include "VCA.h"
using namespace std;
using namespace sr;


// Given an ASC and an alphabet size, determine the number of rows in the problem.
int numRows(const ASC &asc, int v)
{
  int size = -1;

  // Find the largest facet size in the ASC.
  for (EdgeList::const_iterator iter = asc.begin(); iter != asc.end(); ++iter) {
    const Edge e = *iter;
    if (e.size() > size)
      size = e.size();
  }
  int val = 1;
  for (int i=1; i <= size; ++i)
    val *= v;
  return val;
}

// For a row representing a vector in 0...v^k-1, determine the value in position pos.
// Take the int to represent a number in base v format and pick out the vth position.
int val(int rowidx, int v, int pos)
{
  // i_0 + i_1v^1 + i_2v^2 + i_3v^3 etc
  // To get, say, i_j, we can take rowidx mod v^{j+1} - mod v^j and divide by v^j.
  int vpos = 1;
  for (int i=1; i <= pos; ++i)
    vpos *= v;
  int vpos_1 = vpos * v;
  return ((rowidx % vpos_1) - (rowidx % vpos)) / vpos;
}

VCA::VCA(BACOptions &poptions, const ASC &pasc, int pv, Formulation::SolutionType solutiontype)
  : Problem(poptions, Formulation::MINIMIZATION, solutiontype, numRows(pasc, pv)),
    asc(pasc), v(pv)
{
}


VCA::~VCA()
{
  // Delete the group.
  delete group;
#ifndef SCHREIERSIMSTECHNIQUE
  MatrixGroup::destroy();
#else
  SchreierSimsGroup::destroy();
#endif
}



void VCA::constructFormulation()
{
  cerr << "Constructing formulation...\n";
  // We only need an objective value to know the solution value; we simply make this the sum of
  // the x variables.
  vector< int > objective;
  for (int i=0; i < numberVariables; ++i)
    objective.push_back(1);
  formulation.setObjectiveFunction(objective);
 
  // Create the constraints.
  // Iterate over all the facets of the ASC.
  for (EdgeList::const_iterator iter = asc.begin(); iter != asc.end(); ++iter) {
    const Edge &facet = *iter;

    // Get the strength of this facet.
    int t = facet.size();

    // For this facet, we need v^t constraints.
    int vt = 1;
    for (int i=1; i <= t; ++i)
      vt *= v;

    // Now iterate over the v^t t-sets.
    for (int rank=0; rank < vt; ++rank) {
      // Create the constraint that corresponds to this facet and t-set.
      vector< int > positions;
      vector< int > coefficients;

      // Iterate over all the variables and determine if they contain the tset.
      for (int x=0; x < numberVariables; ++x) {
	bool flag = true;
	Edge::const_iterator eiter = facet.begin();
	for (int i=0; i < t; ++i) {
	  if (val(x, v, *eiter) != val(rank, v, i)) {
	    flag = false;
	    break;
	  }
	  ++eiter;
	}

	if (flag) {
	  positions.push_back(x);
	  coefficients.push_back(1);
	}
      }

      // Add the constraint.
      Constraint *constraint = Constraint::createConstraint(formulation, positions, coefficients, GREATERTHAN, 1);
      formulation.addConstraint(constraint);
    }
  }
  cerr << "Done.\n";
}


void VCA::constructSymmetryGroup()
{
  cerr << "Finding symmetry group...\n";
  // Construct the initial base.
  int *base= new int[numberVariables];
  establishInitialBase(base);

  // Use the algorithm to calculate the group.
  SchreierSimsGroup::initialize(numberVariables);
  group = new SchreierSimsGroup(base);
  ((SchreierSimsGroup*)group)->createSymmetryGroup(formulation);
  delete[] base;
  cout << "Done.\n";
  cout << "Size of group is " << ((SchreierSimsGroup*)group)->getSize() << endl;
}


// *** MAIN PROGRAM ***
int main(int argc, char *argv[])
{
  // Encapsulate everything in a try...catch block, since any errors are likely to be unrecoverable.
  try {
    // Command line parameters and derivatives, with sensible defaults.
    Formulation::SolutionType st = Formulation::SEARCH;
    bool statisticsFlag = true;

    // Options.
    MargotBACOptions options;
    CommandLineProcessing clp(options);

    // Register the cut producers.
    IsomorphismCutProducerCreator isomorphismCutProducerCreator;
    clp.registerCreator(isomorphismCutProducerCreator, 0, true);

    // Register the solution managers.
    DefaultSolutionManagerCreator defaultSolutionManagerCreator;
    ImmediateSolutionManagerCreator immediateSolutionManagerCreator;
    clp.registerCreator(defaultSolutionManagerCreator, 0, true);
    clp.registerCreator(immediateSolutionManagerCreator, 1, false);

    // Register the variable orders.
    RandomVariableOrder::initializeRNG();
    LexicographicVariableOrderCreator lexicographicVariableOrderCreator;
    RandomVariableOrderCreator randomVariableOrderCreator;
    clp.registerCreator(lexicographicVariableOrderCreator, 0, true);
    clp.registerCreator(randomVariableOrderCreator, 1, false);

    // Register the branching schemes.
    LowestIndexBranchingSchemeCreator lowestIndexBranchingSchemeCreator;
    ClosestValueBranchingSchemeCreator closestValueBranchingSchemeCreator;
    clp.registerCreator(lowestIndexBranchingSchemeCreator, 0, true);
    clp.registerCreator(closestValueBranchingSchemeCreator, 1, false);

    // Populate the options using the CommandLineProcessing object.
    int status = clp.populateMargotBACOptions(argc, argv);
    if (status == CommandLineProcessing::HELP) {
      cout << "Usage: " << argv[0] << " <options> ASCfile v" << endl;
      cout << "-s 0/1 \t\t output statistics (default: 1)" << endl;
      cout << endl;
      clp.outputOptions(cout);
      exit(EXIT_SUCCESS);
    }

    // We begin by using getopt to process the command line arguments.
    int opt;
    while ((opt = getopt(argc, argv, "s:")) != -1)
      switch (opt) {
      case 's':
	statisticsFlag = (atoi(optarg) == 1);
	break;
      }

    // Make sure there are enough remaining parameters.
    if (argc - optind != 2) {
      cerr << "Need ASC, v" << endl;
      exit(EXIT_FAILURE);
    }

    ifstream asc_file(argv[optind]);
    ASC asc(asc_file);
    int v = strtol(argv[optind+1], 0, 10);

    // Formulate the problem and solve.
    VCA imd(options, asc, v);

    // Finish the configuration on the registered objects.
    isomorphismCutProducerCreator.setNumberVariables(imd.getNumberVariables());
    defaultSolutionManagerCreator.setProblemType(Formulation::MINIMIZATION);
    defaultSolutionManagerCreator.setSolutionType(st);
    immediateSolutionManagerCreator.setProblemType(Formulation::MINIMIZATION);
    immediateSolutionManagerCreator.setSolutionType(st);
    randomVariableOrderCreator.setNumberVariables(imd.getNumberVariables());
    closestValueBranchingSchemeCreator.setNumberVariables(imd.getNumberVariables());

    clp.finishMargotBACOptionsConfiguration();

    imd.solve();

    // *** OUTPUT ***
    if (statisticsFlag)
      cerr << options.getStatistics() << endl;
  } catch (NIBACException &ex) {
    cerr << ex << endl;
    exit(EXIT_FAILURE);
  } catch (std::bad_alloc &ex) {
    cerr << "Memory problem: " << ex.what() << endl;
    exit(EXIT_FAILURE);
  } catch (std::exception &ex) {
    cerr << "Other problem: " << ex.what() << endl;
    exit(EXIT_FAILURE);
  }
  
  exit(EXIT_SUCCESS);
}
