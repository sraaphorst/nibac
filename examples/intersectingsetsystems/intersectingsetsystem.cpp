// intersectingsetsystem.cpp
//
// By Sebastian Raaphorst, 2004

#include <sstream>
#include <algorithm>
#include <vector>
#include <baclibrary.h>
#include "colexicographicvariableorder.h"
#include "intersectingsetsystem.h"
using namespace std;


IntersectingSetSystem::IntersectingSetSystem(BACOptions &poptions, int pv, int pk, int pt, Formulation::SolutionType stype)
  : Problem(poptions, Formulation::MAXIMIZATION, stype, C[pv][pk]),
    v(pv), k(pk), t(pt)
{
}


IntersectingSetSystem::~IntersectingSetSystem()
{
  delete group;
#ifndef SCHREIERSIMSTECHNIQUE
  BlockGroup::destroy();
#else
  SchreierSimsGroup::destroy();
#endif
}


bool IntersectingSetSystem::extendSet(int fixed, int extensionsize, int *set, int numavailable, bool *available, int bound, int &pos)
{
  int nextelem;

  // Given a list of length numavailable of available elements, we extend set from fixed to extensionsize-1
  // using backtracking techniques, starting at pos.
  // bound provides a lower bound (not inclusive) on where in available we start looking for elements.
  while (pos >= fixed) {
    // If we have a complete and valid set, then we are done.
    if (pos == extensionsize)
      return true;

    // Return any element already in position pos to the list of available elements.
    if (set[pos] > bound)
      available[set[pos]] = true;

    // Find the next available element, if one exists.
    nextelem = (set[pos] > bound ? set[pos]+1 : (pos == fixed ? bound+1 : set[pos-1]+1));
    for (; nextelem < numavailable; ++nextelem)
      if (available[nextelem])
	break;

    // If nextelem is now v, we could not find such an element, so we backtrack.
    if (nextelem >= numavailable) {
      set[pos] = bound;
      --pos;
      continue;
    }

    // Add the new element to the set and attempt to extend.
    set[pos] = nextelem;
    available[nextelem] = false;
    ++pos;
  }

  // If we reach this point, we could not generate a valid set.
  return false;
}


void IntersectingSetSystem::constructFormulation()
{
  // Create the objective function
  vector< int > objective;
  for (int i=0; i < numberVariables; ++i)
    objective.push_back(1);
  formulation.setObjectiveFunction(objective);

  // We now create the constraints. For any two k-sets, b_i and b_j, if
  // |b_i \cap b_j| < t, we add the constraint x_i + x_j <= 1.
  // For each k-set, we enumerate its i-sets for i \in Z_t, and then create
  // lexicographically larger k-sets containing the i-set. Then a constraint
  // should exist between these two sets.
  int *kset   = new int[k];
  int *iset   = new int[t];
  int *kmiset = new int[k];
  int *sortedkset = new int[k];
  int pos1, pos2, pos3;
  int bound1, bound2, bound3;

  // Initialize the available array to indicate that everything is
  // initially available.
  bool *available = new bool[v];
  for (int i=0; i < v; ++i)
    available[i] = true;

  // Create a second available array for i-sets.
  bool *availablei = new bool[v];
  for (int i=0; i < v; ++i)
    availablei[i] = false;

  // Continuously generate k-sets.
  bound1 = -1;
  for (int i=0; i < k; ++i)
    kset[i] = bound1;

  pos1 = 0;
  while (extendSet(0, k, kset, v, available, bound1, pos1)) {
    // We now have a k-set. Rank it.
    int index = super(v, k, kset);

    // Generate all its i-sets for i in Z_t.
    // Initialize the availablei array.
    for (int i=0; i < k; ++i)
      availablei[kset[i]] = true;

    for (int i=0; i < t; ++i) {
      // Convenience variable to reduce looping time.
      int bound = k-i;

      // We create all i-sets.
      bound2 = -1;
      for (int j=0; j < i; ++j)
	iset[j] = bound2;

      pos2 = 0;
      while (extendSet(0, i, iset, v, availablei, bound2, pos2)) {
	// We have generated an i-set. We now need a (k-i)-set that,
	// when adjoined with the i-set, will be lexicographically
	// larger than the original k-set.
	bound3 = kset[0];
	for (int j=0; j < bound; ++j)
	  kmiset[j] = bound3;

	pos3 = 0;
	while (extendSet(0, bound, kmiset, v, available, bound3, pos3)) {
	  // Now we adjoin the i-set to the (k-i)-set, sort (as superduper
	  // can only deal with sorted sets), rank, and create a constraint.
	  // As iset and kset2 are both sorted, we could just insert, but
	  // laziness compels us to use STL sort.
	  for (int j=0; j < i; ++j)
	    sortedkset[j] = iset[j];
	  for (int j=i; j < k; ++j)
	    sortedkset[j] = kmiset[j-i];
	  sort(sortedkset, sortedkset+k);

	  // We now have two ksets that intersect in exactly i places, so
	  // we may create a constraint based on this information. Note that
	  // despite our attempts to ensure that sortedkset is lexicographically
	  // larger than kset, we are still not guaranteed this, so we only
	  // create a constraint if this is the case.
	  int index2 = super(v, k, sortedkset);
	  if (index < index2) {
	    vector< int > pos;
	    vector< int > coeff;
	    pos.push_back(index);
	    coeff.push_back(1);
	    pos.push_back(index2);
	    coeff.push_back(1);
	    Constraint *constraint = Constraint::createConstraint(formulation, pos, coeff, LESSTHAN, 1);
	    formulation.addConstraint(constraint);
	  }

	  // We now backtrack on kmiset.
	  --pos3;
	}

	// Backtrack on the iset.
	--pos2;
      }
    }

    // We now backtrack on kset.
    // First, we fix the availablei array.
    for (int i=0; i < k; ++i)
      availablei[kset[i]] = false;

    --pos1;
  }

  // Free the memory.
  delete[] availablei;
  delete[] available;
  delete[] sortedkset;
  delete[] kmiset;
  delete[] iset;
  delete[] kset;
}


void IntersectingSetSystem::determineFixingFlags(int *fixings)
{
  // The only k-set that we know we can fix is {0, ..., k-1}.
  // We do know that there are two k-sets that intersect in t places,
  // but we must fix k-sets in lexicographical order for Margot's
  // algorithms to work properly, and {0, ..., t-1, k, ..., 2k-t-1}, the
  // next obvious k-set to fix, might not be the next lexicographically
  // largest in a solution.
  fixings[0] = FIXEDTO1;

  // We may now fix to 0 all k-sets that have intersection size smaller than
  // t with {0, ..., k-1}. We construct these k-sets through backtracking
  // techniques, constructing i-sets (for i \in Z_t) from Z_k, and extending
  // these i-sets to k-set using elements in {k, ..., v-1}.
  int *kset = new int[k];

  // Maintain an array of available elements. At first, no elements are selected,
  // so all elements are available.
  bool *available = new bool[v];
  for (int i=0; i < v; ++i)
    available[i] = true;

  int pos1, pos2;
  int bound1, bound2;

  // We want all i-sets for i \in Z_t.
  for (int i=0; i < t; ++i) {
    // Set up the backtracking for this i-set.
    bound1 = -1;
    for (int j=0; j < i; ++j)
      kset[j] = bound1;

    // Use backtracking to create the valid i-set from elements in Z_k.
    pos1 = 0;
    while (extendSet(0, i, kset, k, available, bound1, pos1)) {
      // We now have a valid i-set, so we want all k-set extensions of it
      // from elements in {k, ..., v-1}.

      // The bound on the set extension is k-1 (all elements in the extension
      // are greater than this value).
      bound2 = k-1;

      // Now we initialize the k-set extension.
      for (int j=i; j < k; ++j)
	kset[j] = bound2;

      // Backtrack to create all the k-set extensions.
      pos2 = i;
      while (extendSet(i, k, kset, v, available, bound2, pos2)) {
	// Fix the k-set to 0.
	fixings[super(v, k, kset)] = FIXEDTO0;

	// Backtrack.
	--pos2;
      }

      // Backtrack.
      --pos1;
    }
  }

  delete[] available;
  delete[] kset;
}


void IntersectingSetSystem::constructSymmetryGroup()
{
  // We construct an initial base.
  int *base = new int[numberVariables];
  establishInitialBase(base);

#ifndef SCHREIERSIMSTECHNIQUE
  // A (v, k, t) intersecting set system's symmetry group is
  // simply the standard permutation group over k-sets from
  // a v-set with each k-set appearing once.
  BlockGroup::initialize(C[v][k]);
  group = new BlockGroup(v, k, 1, base);
#else
  SchreierSimsGroup::initialize(C[v][k]);
  group = new SchreierSimsGroup(base);
  ((SchreierSimsGroup*)group)->createSymmetryGroup(f);
#endif

  delete[] base;
}


void IntersectingSetSystem::processSolutions()
{
  // We only do this if solutions were not output immediately.
  // Get the solutions and store them in block format.
  DefaultSolutionManager *dsm = dynamic_cast< DefaultSolutionManager* >(options.getSolutionManager());
  if (!dsm)
    return;

  vector< vector< int > * > &sols = dsm->getSolutions();
  vector< vector< int > * >::iterator sbeginIter = sols.begin();
  vector< vector< int > * >::iterator sendIter   = sols.end();
  vector< int >::iterator beginIter, endIter;

  solutions.clear();
  for (; sbeginIter != sendIter; ++sbeginIter) {
    vector< Block > solution;
    beginIter = (*sbeginIter)->begin();
    endIter   = (*sbeginIter)->end();
    for (int i=0; beginIter != endIter; ++beginIter, ++i)
      if (*beginIter) {
	Block b(v, k, i);
	solution.push_back(b);
      }
    solutions.push_back(solution);
  }
}


vector< vector< Block > > &IntersectingSetSystem::getSolutions()
{
  return solutions;
}



// *** MAIN PROGRAM ***
int main(int argc, char *argv[])
{
  // Encapsulate everything in a try...catch block, since any errors are likely to be unrecoverable.
  try {
    // Command line parameters and derivatives, with sensible defaults.
    int t, v, k;
    Formulation::SolutionType st = Formulation::MAXIMALGENERATION;
    bool statisticsFlag = true;

    // Options.
    MargotBACOptions options;
    CommandLineProcessing clp(options);

    // Register the cut producers.
    CliqueCutProducerCreator cliqueCutProducerCreator;
    IsomorphismCutProducerCreator isomorphismCutProducerCreator;
    clp.registerCreator(cliqueCutProducerCreator, 0, true);
    clp.registerCreator(isomorphismCutProducerCreator, 1, true);

    // Register the solution managers.
    DefaultSolutionManagerCreator defaultSolutionManagerCreator;
    ImmediateSolutionManagerCreator immediateSolutionManagerCreator;
    clp.registerCreator(defaultSolutionManagerCreator, 0, true);
    clp.registerCreator(immediateSolutionManagerCreator, 1, false);

    // Register the variable orders.
    RandomVariableOrder::initializeRNG();
    LexicographicVariableOrderCreator lexicographicVariableOrderCreator;
    ColexicographicVariableOrderCreator colexicographicVariableOrderCreator;
    RandomVariableOrderCreator randomVariableOrderCreator;
    clp.registerCreator(lexicographicVariableOrderCreator, 0, true);
    clp.registerCreator(colexicographicVariableOrderCreator, 1, false);
    clp.registerCreator(randomVariableOrderCreator, 2, false);

    // Register the branching schemes.
    LowestIndexBranchingSchemeCreator lowestIndexBranchingSchemeCreator;
    ClosestValueBranchingSchemeCreator closestValueBranchingSchemeCreator;
    clp.registerCreator(lowestIndexBranchingSchemeCreator, 0, true);
    clp.registerCreator(closestValueBranchingSchemeCreator, 1, false);

    // Populate the options using the CommandLineProcessing object.
    int status = clp.populateMargotBACOptions(argc, argv);
    if (status == CommandLineProcessing::HELP) {
      cout << "Usage: " << argv[0] << " <options> t v k <sma>" << endl;
      cout << "<sma> \t\t search (default), maximal generation, or all" << endl;
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
	statisticsFlag = (atoi(optarg) == 1);;
	break;
      }

    // Make sure there are enough remaining parameters.
    if (argc - optind != 3 && argc - optind != 4) {
      cerr << "Need t, v, k [type of problem]" << endl;
      exit(EXIT_FAILURE);
    }

    // Extract the numerical parameters.
    v = atoi(argv[optind]);
    k = atoi(argv[optind+1]);
    t = atoi(argv[optind+2]);

    // Determine if a type was specified.
    if (argc - optind == 4) {
      if (strlen(argv[optind+3]) != 1) {
	cerr << "illegal problem type: " << argv[optind+3] << endl;
	exit(EXIT_FAILURE);
      }
      if (argv[optind+3][0] == 's')
	st = Formulation::SEARCH;
      else if (argv[optind+3][0] == 'm')
	st = Formulation::MAXIMALGENERATION;
      else if (argv[optind+3][0] == 'a')
	st = Formulation::ALLGENERATION;
      else {
	cerr << "illegal problem type: " << argv[optind+3] << endl;
	exit(EXIT_FAILURE);
      }
    }

    // Check basic feasibility here.
    if (t > k) {
      cerr << "cannot have t > k" << endl;
      exit(EXIT_FAILURE);
    }
    if (k > v) {
      cerr << "cannot have k > v" << endl;
      exit(EXIT_FAILURE);
    }
    if (v > 100) {
      cerr << "cannot have v > 100" << endl;
      exit(EXIT_FAILURE);
    }
    
    // Initialize superduper.
    init_super_duper(v);
    
    // Formulate the problem.
    IntersectingSetSystem iss(options, v, k, t, st);

    // Finish the configuration on the registered objects.
    isomorphismCutProducerCreator.setNumberVariables(iss.getNumberVariables());
    defaultSolutionManagerCreator.setProblemType(Formulation::MAXIMIZATION);
    defaultSolutionManagerCreator.setSolutionType(st);
    immediateSolutionManagerCreator.setProblemType(Formulation::MAXIMIZATION);
    immediateSolutionManagerCreator.setSolutionType(st);
    colexicographicVariableOrderCreator.setV(v);
    colexicographicVariableOrderCreator.setK(k);
    colexicographicVariableOrderCreator.setLambda(1);
    randomVariableOrderCreator.setNumberVariables(iss.getNumberVariables());
    closestValueBranchingSchemeCreator.setNumberVariables(iss.getNumberVariables());

    clp.finishMargotBACOptionsConfiguration();

    iss.solve();

    // *** OUTPUT ***
    if (statisticsFlag)
      cout << options.getStatistics() << endl;

    // Print the solutions
    vector< vector< Block > > &sols = iss.getSolutions();
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
