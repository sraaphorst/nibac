// incidencematrixdesign.cpp
//
// By Sebastian Raaphorst, 2004

#include <iostream>
#include <vector>
#include <baclibrary.h>
#include "incidencematrixdesign.h"
using namespace std;


// NOTE: The last parameter to Problem is the number of variables; this is
// just a factorization of b * v + b * v * (v-1) / 2, where
// b = lambda * v * (v-1) / (k * (k-1)).
IncidenceMatrixDesign::IncidenceMatrixDesign(BACOptions &poptions, int pv, int pk, int plambda,
					     Formulation::SolutionType solutiontype)
  : Problem(poptions, Formulation::MAXIMIZATION, solutiontype,
 	    (plambda * pv * (pv-1) / (pk * (pk-1))) * (pv + pv * (pv-1)/2), pv * (plambda * pv * (pv-1) / (pk * (pk-1)))),
    v(pv), k(pk), lambda(plambda), b(lambda * v * (v-1) / (k * (k-1)))
{
  int val = 0;

  // For convenience, create a table that translates x variable indices into
  // an ILP variable index.
  xtable = new int*[v];
  for (int i=0; i < v; ++i)
    xtable[i] = new int[b];
  for (int l=0; l < b; ++l)
    for (int i=0; i < v; ++i, ++val)
      xtable[i][l] = val;

  // Also create a convenience table that translates y variable indices into
  // an ILP variable index.
  ytable = new int**[v];
  for (int i=0; i < v; ++i) {
    ytable[i] = new int*[v];
    for (int j=0; j < v; ++j)
      ytable[i][j] = new int[b];
  }

  for (int l=0; l < b; ++l)
    for (int i=0; i < v; ++i)
      for (int j=i+1; j < v; ++j, ++val)
	ytable[i][j][l] = ytable[j][i][l] = val;
}


IncidenceMatrixDesign::~IncidenceMatrixDesign()
{
  // Delete the conversion tables...
  for (int i=0; i < v; ++i)
    delete[] xtable[i];
  delete[] ytable;
  for (int i=0; i < v; ++i) {
    for (int j=0; j < v; ++j)
      delete[] ytable[i][j];
    delete[] ytable[i];
  }
  delete[] ytable;

  // ...and the group.
  delete group;
#ifndef SCHREIERSIMSTECHNIQUE
  MatrixGroup::destroy();
#else
  SchreierSimsGroup::destroy();
#endif
}



void IncidenceMatrixDesign::constructFormulation()
{
  // We only need an objective value to know the solution value; we simply make this the sum of
  // the x variables.
  vector< int > objective;
  int bound = v * b;
  for (int i=0; i < numberVariables; ++i)
    objective.push_back(i < bound ? 1 : 0);
  formulation.setObjectiveFunction(objective);
 
  // Add the constraints that say each block must contain k elements from Z_v
  for (int l=0; l < b; ++l) {
    vector< int > positions;
    vector< int > coefficients;

    for (int i=0; i < v; ++i) {
      positions.push_back(xtable[i][l]);
      coefficients.push_back(1);
    }

    Constraint *constraint = Constraint::createConstraint(formulation, positions, coefficients, EQUALS, k);
    formulation.addConstraint(constraint);
  }
 
  // Add the pair constraints, i.e. the constraints on the y variables.
  for (int i=0; i < v; ++i)
    for (int j=i+1; j < v; ++j) {
      vector< int > positions;
      vector< int > coefficients;

      for (int l=0; l < b; ++l) {
	positions.push_back(ytable[i][j][l]);
	coefficients.push_back(1);
      }

      Constraint *constraint = Constraint::createConstraint(formulation, positions, coefficients, EQUALS, lambda);
      formulation.addConstraint(constraint);
    }

  // Add the constraints that force y_i,j^l = x_i,l x_j,l.
  for (int l=0; l < b; ++l)
    for (int i=0; i < v; ++i)
      for (int j=i+1; j < v; ++j) {
	vector< int > positions1;
	vector< int > coefficients1;
	positions1.push_back(ytable[i][j][l]);
	coefficients1.push_back(1);
	positions1.push_back(xtable[i][l]);
	coefficients1.push_back(-1);
	Constraint *constraint1 = Constraint::createConstraint(formulation, positions1, coefficients1, LESSTHAN, 0);
	formulation.addConstraint(constraint1);

	vector< int > positions2;
	vector< int > coefficients2;
	positions2.push_back(ytable[i][j][l]);
	coefficients2.push_back(1);
	positions2.push_back(xtable[j][l]);
	coefficients2.push_back(-1);
	Constraint *constraint2 = Constraint::createConstraint(formulation, positions2, coefficients2, LESSTHAN, 0);
	formulation.addConstraint(constraint2);

	vector< int > positions3;
	vector< int > coefficients3;
	positions3.push_back(xtable[i][l]);
	coefficients3.push_back(1);
	positions3.push_back(xtable[j][l]);
	coefficients3.push_back(1);
	positions3.push_back(ytable[i][j][l]);
	coefficients3.push_back(-1);
	Constraint *constraint3 = Constraint::createConstraint(formulation, positions3, coefficients3, LESSTHAN, 1);
	formulation.addConstraint(constraint3);
      }
}


void IncidenceMatrixDesign::determineFixingFlags(int *fixings)
{
  // Number of blocks we fixed, for pair recording information.
  int numblocksfixed;

  // *** X VARIABLE FIXINGS ***
  // Perform the possible fixings on the xs.
  if (lambda == 1) {
    // We can fix the following (v-1)/(k-1) blocks:
    // {0, 1, ..., k-1}
    // {0, k, ..., 2k-2}
    // {0, 2k-1, ..., 3k-3}
    // ...
    numblocksfixed = (v-1)/(k-1);
    for (int l=0; l < numblocksfixed; ++l) {
      // Always fix x_0,l
      fixings[xtable[0][l]] = FIXEDTO1;

      // Now iterate over the rest of the block and fix appropriately.
      // We fix all x_i,l to 0 except for those in the range of
      // {1 + l(k-1), \ldots, (l+1)(k-1)}, which constitute the remainder
      // elements comprising this lexicographically smallest block, and they
      // get fixed to 1.
      int lower = 1 + l * (k-1);
      int upper = (l + 1) * (k - 1);
      for (int i=1; i < v; ++i)
	fixings[xtable[i][l]] = (i < lower || i > upper ? FIXEDTO0 : FIXEDTO1);
    }

    // There is one more block that we can fix as well, namely:
    // {1, k, 2k-1, 3k-2, ...}
    // provided that this block is defined.
    // The general term in position i is i * (k-1) + 1.
    // We begin by checking if this fixing is feasible, i.e. if
    // a vertex is defined for the last position.
    if ((k-1)*(k-1)+1 < v) {
      // Fix the appropriate variables to 1.
      for (int i=0; i < k; ++i)
	fixings[xtable[i*(k-1)+1][numblocksfixed]] = FIXEDTO1;
      // Set the other variables to 0.
      for (int i=0; i < v; ++i)
	if (fixings[xtable[i][numblocksfixed]] == FREE)
	  fixings[xtable[i][numblocksfixed]] = FIXEDTO0;
      ++numblocksfixed;
    }
  }
  else {
    // We can only fix the first block, as it may or may not be repeated.
    // This consists of setting the first k elements in the block to 1, and
    // the remaining to 0.
    numblocksfixed = 1;
    for (int i=0; i < v; ++i)
      fixings[xtable[i][0]] = (i < k ? FIXEDTO1 : FIXEDTO0);
  }
}


void IncidenceMatrixDesign::constructSymmetryGroup()
{
  // As we want our group to be exclusively over the branching variables, we specifically
  // create the initial base instead of calling establishInitialBase.
  int bound = v*b;
  int *base = new int[bound];

  int index = 0;
  for (int i=0; i < bound; ++i)
    if (fixingFlags[i] == FIXEDTO1) {
      base[index] = i;
      ++index;
    }
  for (int i=0; i < bound; ++i)
    if (fixingFlags[i] == FIXEDTO0) {
      base[index] = i;
      ++index;
    }
  for (int i=0; i < bound; ++i)
    if (fixingFlags[i] == FREE) {
      base[index] = i;
      ++index;
    }

#ifndef SCHREIERSIMSTECHNIQUE
  MatrixGroup::initialize(v * b);
  group = new MatrixGroup(v, b, xtable, true, true, base);
#else
  // Use the algorithm to calculate the group.
  SchreierSimsGroup::initialize(v * b);
  group = new SchreierSimsGroup(base);
  ((SchreierSimsGroup*)group)->createSymmetryGroup(f);
#endif

  delete[] base;
}


void IncidenceMatrixDesign::processSolutions()
{
  // We only do this if solutions were not output immediately.
  // Get the solutions and store them in block format.
  DefaultSolutionManager *dsm = dynamic_cast< DefaultSolutionManager* >(options.getSolutionManager());
  if (!dsm)
    return;

  vector< vector< int > * > &sols = dsm->getSolutions();
  vector< vector< int > * >::iterator sbeginIter = sols.begin();
  vector< vector< int > * >::iterator sendIter   = sols.end();
  int *block = new int[k];
  int index;

  solutions.clear();
  for (; sbeginIter != sendIter; ++sbeginIter) {
    // We have a solution; we need to iterate over its x variables and determine
    // the blocks.
    vector< int > &soln = **sbeginIter;

    vector< Block > solution;
    for (int l=0; l < b; ++l) {
      index = 0;
      for (int i=0; i < v; ++i) {
	if (soln[xtable[i][l]] == 1) {
	  assert(index < k);
	  block[index] = i;
	  ++index;
	}
      }

      assert(index == k);
      Block bk(k, block);
      solution.push_back(bk);
    }

    solutions.push_back(solution);
  }

  delete[] block;
}


vector< vector< Block > > &IncidenceMatrixDesign::getSolutions()
{
  return solutions;
}



// *** MAIN PROGRAM ***
int main(int argc, char *argv[])
{
  // Encapsulate everything in a try...catch block, since any errors are likely to be unrecoverable.
  try {
    // Command line parameters and derivatives, with sensible defaults.
    int v, k, lambda;
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
      cout << "Usage: " << argv[0] << " <options> v k lambda <sg>" << endl;
      cout << "<sg> \t\t search (default), or generate all" << endl;
      cout << "-s 0/1 \t\t output statistics (default: 1)" << endl;
      cout << endl;
      clp.outputOptions(cout);
      exit(EXIT_SUCCESS);
    }

    // We begin by using getopt to process the command line arguments.
    // We begin by using getopt to process the command line arguments.
    int opt;
    while ((opt = getopt(argc, argv, "s:")) != -1)
      switch (opt) {
      case 's':
	statisticsFlag = (atoi(optarg) == 1);;
	break;
      }

    // Make sure there are enough remaining parameters.
    if (argc - optind != 3 && argc - optind != 4) {
      cerr << "Need v, k, lambda [type of problem]" << endl;
      exit(EXIT_FAILURE);
    }

    v = atoi(argv[optind]);
    k = atoi(argv[optind+1]);
    lambda = atoi(argv[optind+2]);

    // Determine if a type was specified.
    if (argc - optind == 4) {
      if (strlen(argv[optind+3]) != 1) {
	cerr << "illegal problem type: " << argv[optind+3] << endl;
	exit(EXIT_FAILURE);
      }
      if (argv[optind+3][0] == 's')
	st = Formulation::SEARCH;
      else if (argv[optind+3][0] == 'g')
	st = Formulation::GENERATION;
      else {
	cerr << "illegal problem type: " << argv[optind+3] << endl;
	exit(EXIT_FAILURE);
      }
    }

    // Check basic feasibility here.
    if (k > v) {
      cerr << "cannot have k > v" << endl;
      exit(EXIT_FAILURE);
    }

    // Formulate the problem and solve.
    IncidenceMatrixDesign imd(options, v, k, lambda, st);

    // Finish the configuration on the registered objects.
    isomorphismCutProducerCreator.setNumberVariables(imd.getNumberVariables());
    defaultSolutionManagerCreator.setProblemType(Formulation::MAXIMIZATION);
    defaultSolutionManagerCreator.setSolutionType(st);
    immediateSolutionManagerCreator.setProblemType(Formulation::MAXIMIZATION);
    immediateSolutionManagerCreator.setSolutionType(st);
    randomVariableOrderCreator.setNumberVariables(imd.getNumberVariables());
    closestValueBranchingSchemeCreator.setNumberVariables(imd.getNumberVariables());

    clp.finishMargotBACOptionsConfiguration();

    imd.solve();

    // *** OUTPUT ***
    if (statisticsFlag)
      cerr << options.getStatistics() << endl;
    
    // Print the solutions
    vector< vector< Block > > &sols = imd.getSolutions();
    vector< vector< Block > >::iterator sbeginIter = sols.begin();
    vector< vector< Block > >::iterator sendIter   = sols.end();
    for (; sbeginIter != sendIter; ++sbeginIter) {
      // We process this solution.
      int counter = 0;
      int bound = (*sbeginIter).size() - 1;
      vector< Block >::iterator beginIter = (*sbeginIter).begin();
      vector< Block >::iterator endIter   = (*sbeginIter).end();
      for (; beginIter != endIter; ++beginIter, ++counter) {
	cout << (*beginIter);
	if (counter < bound)
	  cout << ", ";
      }
      cout << endl;
    }
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
