// vscycle.cpp
//
// By Sebastian Raaphorst, 2004.
//
// $Author$
// $Date$

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <baclibrary.h>
#include "vscycle.h"
using namespace std;


VSCycle::VSCycle(BACOptions &poptions, int pn, Formulation::SolutionType solutiontype)
  : Problem(poptions, Formulation::MAXIMIZATION, solutiontype,
	    calcNumVars(pn, false), calcNumVars(pn, true)),
    n(pn), numv(twoToN(n)-1), numpos(qBinCoeff(n, 2))
{
  // We now create a table to translate a vector and cycle position
  // to a variable.
  int varindex = 0;
  int bound = numv * numpos;
  matrix_2d(&table, numv, numpos);
  for (; varindex < bound; ++varindex)
    table[varindex % numv][varindex / numv] = varindex;

  // Now we want a table that translates from x_i,x_j to a y variable
  // if one exists to represent this pair.
  bound = numv*numpos;
  matrix_2d(&ytable, bound, bound);

  // Initialize the entire table to 0.
  for (int i=0; i < bound; ++i)
    memset(ytable[i], 0, sizeof(int)*bound);

  // For the valid variables, we enter variable indices.
  // Process column by column.
  for (int i=0; i < numpos; ++i) {
    // For each column, we pick a variable.
    for (int j=0; j < numv; ++j) {
      // We now iterate over the variables in the following column,
      // skipping over the one representing the same vector as i.
      for (int m=0; m < numv; ++m) {
	if (j == m)
	  continue;

	// We now have a pair of vectors {j+1, m+1} corresponding to two
	// variables. We want to create a variable to represent incidence
	// of them in position i and i+1. The j+1 in position i corresponds
	// to variable table[j][i], and the m+1 in position i+1 corresponds
	// to variable table[m][(i+1)%numpos].
	ytable[table[j][i]][table[m][(i+1)%numpos]] = varindex;
	++varindex;
      }
    }
  }

  // For each subspace, we need a list of the nonzero vectors that it
  // contains.
  matrix_2d(&subspaces, numpos, 3);

  // Now what we want to do is pick every unmarked pair of nonzero vectors in
  // an ordered fashion, find the third vector that they determine, mark
  // all pairs of vectors in the triple, and then record the triple as
  // a subspace.
  int **vectors;
  matrix_2d(&vectors, numv+1, numv+1);
  for (int i=0; i < numv; ++i)
    memset(vectors[i], 0, sizeof(int)*(numv+1));

  // Now try every pair.
  int vscnt = 0;
  int m;
  for (int i=1; i < numv; ++i)
    for (int j=i+1; j < numv; ++j) {
      // If this pair is marked, skip it.
      if (vectors[i][j])
	continue;

      // We have found a subspace. Determine the third vector.
      m = i ^ j;

      // Mark the vector pairs.
      vectors[i][j] = vectors[j][i] = 1;
      vectors[i][m] = vectors[m][i] = 1;
      vectors[j][m] = vectors[m][j] = 1;

      // Record the triple.
      subspaces[vscnt][0] = i-1;
      subspaces[vscnt][1] = j-1;
      subspaces[vscnt][2] = m-1;

      ++vscnt;
    }

  matrix_free_2d(&vectors, numv+1, numv+1);
}


VSCycle::~VSCycle()
{
  // Free the group.
  delete g;
  SchreierSimsGroup::destroy();

  int bound = numv*numpos;
  matrix_free_2d(&subspaces, numpos, 3);
  matrix_free_2d(&ytable, bound, bound);
  matrix_free_2d(&table, numv, numpos);
}


void VSCycle::constructDefaultFormulation()
{
  int v1, v2, y;

  // We set the objective function to be the sum of all the x variables.
  vector< int > objective;
  int bound = numv * numpos;
  for (int i=0; i < numvariables; ++i)
    objective.push_back(i < bound ? 1 : 0);
  f.setObjectiveFunction(objective);

  // Add the constraints for the columns, namely that the sum of all
  // the variables in the column must equal one (one vector per
  // position of the cycle).
  for (int i=0; i < numpos; ++i) {
    vector< int > positions;
    vector< int > coefficients;

    for (int j=0; j < numv; ++j) {
      positions.push_back(table[j][i]);
      coefficients.push_back(1);
    }

    Constraint constraint = ConstraintC::createConstraint(f, positions, coefficients, EQUALS, 1);
    f.addConstraint(constraint);
  }

  // Now we have the complicated constraints, one per subspace.
  // We basically want to have a sum equal to 1 stating that a
  // subspace is covered by our solution.
  // We have numpos subspaces.
  for (int i=0; i < numpos; ++i) {
    vector< int > positions;
    vector< int > coefficients;

    // We have now determined a subspace.
    // Iterate over the columns.
    for (int j=0; j < numpos; ++j) {
      // We want to see if, for column j, there is a vector covered
      // along with a vector in column (j+1)%numpos that serves as
      // a basis for this vector space.
      for (int l=0; l < 3; ++l) {
	// We consider vector l in subspaces[i].
	// We want it to appear with the other vectors in subspaces[i].
	for (int m=0; m < 3; ++m) {
	  // A vector can't appear with itself.
	  if (m == l)
	    continue;

	  // Push back the y variable corresponding to these variables.
	  v1 = table[subspaces[i][l]][j];
	  v2 = table[subspaces[i][m]][(j+1)%numpos];
	  positions.push_back(ytable[v1][v2]);
	  coefficients.push_back(1);
	}
      }
    }

    Constraint constraint = ConstraintC::createConstraint(f, positions, coefficients, EQUALS, 1);
    f.addConstraint(constraint);
  }

  // Now we need to make the constraints that force the problem to
  // be linear, i.e. make the values of the y variables dependent
  // on the linearization of the x variables.
  // We need to iterate over all pairs of vectors in consequent positions
  // Iterate over the columns.
  for (int i=0; i < numpos; ++i) {
    // Iterate over the vectors.
    for (int j=0; j < numv; ++j) {
      // Iterate over the vectors in the preceeding column.
      for (int m=0; m < numv; ++m) {
	if (j == m)
	  continue;

	// We now have a pair of variables that will appear in the subspace
	// constraints in a nonlinear fashion.
	v1 = table[j][i];
	v2 = table[m][(i+1)%numpos];
	y = ytable[v1][v2];

	// Now we create the three linearizing constraints.
	vector< int > positions1;
	vector< int > coefficients1;
	positions1.push_back(y);
	coefficients1.push_back(1);
	positions1.push_back(v1);
	coefficients1.push_back(-1);
	Constraint constraint1 = ConstraintC::createConstraint(f, positions1, coefficients1, LESSTHAN, 0);
	f.addConstraint(constraint1);

	vector< int > positions2;
	vector< int > coefficients2;
	positions2.push_back(y);
	coefficients2.push_back(1);
	positions2.push_back(v2);
	coefficients2.push_back(-1);
	Constraint constraint2 = ConstraintC::createConstraint(f, positions2, coefficients2, LESSTHAN, 0);
	f.addConstraint(constraint2);

	vector< int > positions3;
	vector< int > coefficients3;
	positions3.push_back(v1);
	coefficients3.push_back(1);
	positions3.push_back(v2);
	coefficients3.push_back(1);
	positions3.push_back(y);
	coefficients3.push_back(-1);
	Constraint constraint3 = ConstraintC::createConstraint(f, positions3, coefficients3, LESSTHAN, 1);
	f.addConstraint(constraint3);
      }
    }
  }
}


void VSCycle::determineFixings()
{
  // Call the superclass method.
  Problem::determineFixings();

  // If the fixing was done manually, we don't bother doing anything here.
  if (options.fixingManually())
    return;

  // We can only fix the first vector in position one, and the second
  // in position two.
  for (int i=0; i < numv; ++i) {
    fixings[table[i][0]] = (i == 0 ? FIXEDTO1 : FIXEDTO0);
    fixings[table[i][1]] = (i == 1 ? FIXEDTO1 : FIXEDTO0);
  }
}


void VSCycle::constructSymmetryGroup()
{
  // We cannot construct a symmetry group using nauty, since by our
  // linearization, we have variables with -1 coefficients. We could
  // further proceed to add more variables to make these positive, but
  // this would be horrid. Instead, we simply specify the symmetry
  // group ourselves.

  // We first create an initial base over the branching variables.
  int bound = numv * numpos;
  int *base = new int[bound];

  int index = 0;
  for (int i=0; i < bound; ++i)
    if (fixings[i] == FIXEDTO1) {
      base[index] = i;
      ++index;
    }
  for (int i=0; i < bound; ++i)
    if (fixings[i] == FIXEDTO0) {
      base[index] = i;
      ++index;
    }
  for (int i=0; i < bound; ++i)
    if (fixings[i] == FREE) {
      base[index] = i;
      ++index;
    }

  // Initialize the Schreier-Sims group class and create the perm.
  SchreierSimsGroup::initialize(bound);
  SchreierSimsGroup *ssg = new SchreierSimsGroup(base);

#ifndef SCHREIERSIMSTECHNIQUE
  int col, oldpos, newpos;
  int *perm = new int[bound];
  ssg->getIdentityPermutation(perm);

  // Now we add generators for all the basis permutations.
  // The basis permutations are isomorphic to S_n, so we only
  // need to add n-1 generators, namely swapping basis vectors
  // 1 and 2, 1 and 4, etc...
  for (int i=1; i < n; ++i) {
    // We swap vectors 1 (in pos 0) and 2^i (in pos 2^i-1).
    // For every row, we determine if this swap permutes the row.
    for (int j=0; j < numv; ++j) {
      newpos = oldpos = j+1;
      if (oldpos & 1) {
	// Clear the 1 in the first position.
	newpos &= (~1);

	// Set the permuted 1.
	newpos |= (1 << i);
      }
      if (oldpos & (1 << i)) {
	if (!(oldpos & 1))
	  newpos &= ~(1 << i);

	// Set the permuted 1.
	newpos |= 1;
      }

      // Permute across all columns.
      for (int k=0; k < numpos; ++k)
	perm[table[j][k]] = table[newpos-1][k];
    }
    ssg->enter(perm);
  }

  // Now we must add the two column permutations that generate
  // this subgroup. These are the cyclic permutation over the
  // columns, and the mirror permutation.
  // Begin with the cyclic permutation.
  for (int i=0; i < numpos; ++i) {
    // Permute column i to column (i+1)%numpos.
    for (int j=0; j < numv; ++j)
      perm[table[j][i]] = table[j][(i+1)%numpos];
  }
  ssg->enter(perm);

  // We restore the identity permutation, and create the mirror
  // image permutation.
  ssg->getIdentityPermutation(perm);
  bound = numpos/2;
  for (int i=0; i < bound; ++i) {
    // Permute column i and column numpos-i-1.
    col = numpos - i - 1;
    for (int j=0; j < numv; ++j) {
      perm[table[j][i]] = table[j][col];
      perm[table[j][col]] = table[j][i];
    }
  }
  ssg->enter(perm);
  delete[] perm;
#else
  // Use the algorithm to calculate the group.
  ((SchreierSimsGroup*)ssg)->createSymmetryGroup(f);
#endif

  g = ssg;

  // We are done. The group for our problem is simply the direct
  // composition of these two subgroups, so we free the permutation
  // and terminate.
  delete[] base;

}

void VSCycle::processSolutions()
{
  // Get the solutions and store them in vector format.
  DefaultSolutionManager *dsm = (DefaultSolutionManager*) options.getSolutionManager();
  vector< vector< int > * > &sols = dsm->getSolutions();
  vector< vector< int > * >::iterator sbeginIter = sols.begin();
  vector< vector< int > * >::iterator sendIter   = sols.end();
  for (; sbeginIter != sendIter; ++sbeginIter) {
    // Get the solution and iterate over its x variables, determining the
    // vectors.
    vector< int > &soln = **sbeginIter;
    vector< int >::iterator beginIter = soln.begin();
    vector< int >::iterator endIter   = soln.end();

    vector< int > solution;
    int pos = 0;
    for (; beginIter != endIter; ++beginIter, ++pos) {
      if (*beginIter == 1)
	solution.push_back((pos % numv)+1);
    }
    cerr << endl;

    solutions.push_back(solution);
  }
}


vector< vector< int > > &VSCycle::getSolutions()
{
  return solutions;
}


int VSCycle::twoToN(int n)
{
  return 1 << n;
}


int VSCycle::qBinCoeff(int n, int k)
{
  int twok = 1 << k;
  int twon = 1 << n;
  
  // Calculate [n,k]_2.
  int bincoeff = 1;
  for (int i=0; i < k; ++i)
    bincoeff *= (twon - (1 << i));
  for (int i=0; i < k; ++i)
    bincoeff /= (twok - (1 << i));
  
  return bincoeff;
}


int VSCycle::calcNumVars(int n, bool branching)
{
  // We now have all the information we need.
  // If we want the number of branching variables, this is just the number
  // of entries in the matrix itself, i.e. [n,k]_2(2^n-1). If we want the
  // total number of variables, this is [n,k]_2(2^n-1) + [n,k]_2(2^n-1)(2^n-2)
  // = [n,k]_2(2^n-1)^2.
  return qBinCoeff(n, 2) * (twoToN(n)-1) * (branching ? 1 : (twoToN(n)-1));
}


// *** MAIN PROGRAM ***


// Program name for error handling
static char *progname;


static void print_usage_and_abort(BACOptions &options)
{
  cerr << "Usage: " << progname << " <options> n <sg>" << endl;
  options.printParameters(cerr);
  cerr << "\t-s\tDisplay statistics." << endl;
  cerr << "\t-i\tUse isomorphism cuts (default: 1)." << endl;
  cerr << "\t<sg>\tSearch or generation (search default)." << endl;
					    exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{
  // Command line parameters and derivatives, with sensible defaults.
  int n, k;
  int depth = -1;
  Formulation::SolutionType st = Formulation::SEARCH;
  bool statsflg = false;

  // Options.
  MargotBACOptions options;
  bool useIsomCuts = true;

  // Set the program name.
  progname = argv[0];

  // Process the BAC options
  if (!options.initialize(argc, argv))
    print_usage_and_abort(options);

  // We begin by using getopt to process the command line arguments.
  // NOTE: This may not work on Windows systems. Do they have getopt?
  // It's POSIX, defined in unistd.h, so unsure. We could work around,
  // but that would be tedious.
  int opt;
  while ((opt = getopt(argc, argv, "i:s")) != -1)
    switch (opt) {
    case 'i':
      useIsomCuts = (atoi(optarg) == 1);
      break;
    case 's':
      statsflg = true;
      break;
    }

  // Make sure there are enough remaining parameters.
  if (argc - optind != 1 && argc - optind != 2)
    print_usage_and_abort(options);

  n = atoi(argv[optind]);

  // Determine if a type was specified.
  if (argc - optind == 2) {
    if (strlen(argv[optind+1]) != 1)
      print_usage_and_abort(options);
    if (argv[optind+1][0] == 's')
      st = Formulation::SEARCH;
    else if (argv[optind+1][0] == 'g')
      st = Formulation::GENERATION;
    else
      print_usage_and_abort(options);
  }

  try {
    // Formulate the problem and solve.
    VSCycle vscycle(options, n, st);
    vscycle.useIsomorphismCuts(useIsomCuts);
    vscycle.useCliqueCuts(false);
    vscycle.solve();

    // *** OUTPUT ***
    if (statsflg)
      cerr << options.getStatistics() << endl;

    // Print the solutions
    vector< vector< int > > &sols = vscycle.getSolutions();
    vector< vector< int > >::iterator sbeginIter = sols.begin();
    vector< vector< int > >::iterator sendIter   = sols.end();
    for (; sbeginIter != sendIter; ++sbeginIter) {
      // We process this solution.
      vector< int >::iterator beginIter = (*sbeginIter).begin();
      vector< int >::iterator endIter   = (*sbeginIter).end();
      for (; beginIter != endIter; ++beginIter) {
	// We need to print out an integer number with n bits in binary.
	for (int i=n-1; i >= 0; --i)
	  cout << ((((*beginIter) >> i) & 1) ? '1' : '0');
	cout << " ";
      }
      cout << endl;
    }
  } catch (std::bad_alloc &ex) {
    cerr << "Memory problem: " << ex.what() << endl;
  } catch (std::exception &ex) {
    cerr << "Other problem: " << ex.what() << endl;
  }
  
  return 0;
}
