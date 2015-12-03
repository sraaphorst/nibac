// design.cpp
//
// By Sebastian Raaphorst, 2004.

#include <sstream>
#include <vector>
#include <baclibrary.h>
#include "colexicographicvariableorder.h"
#include "specialcliquecutproducer.h"
#include "design.h"
using namespace std;


Design::Design(BACOptions &poptions,
	       int pt,
	       int pv,
	       int pk,
	       int plambda,
	       type ptype,
	       Formulation::SolutionType solutiontype,
	       bool psimpleFlag)
  : Problem(poptions,
	    ptype == COVERING ? Formulation::MINIMIZATION : Formulation::MAXIMIZATION,
	    solutiontype,
	    (psimpleFlag ? 1 : plambda) * C[pv][pk]),
    t(pt),
    v(pv),
    k(pk),
    lambda(plambda),
    designType(ptype),
    numberBlocks(C[v][k]),
    simpleFlag(psimpleFlag)
{
}


Design::~Design()
{
  // As we created the group here, destroy it here.
  delete group;

  // We initialized one of these two, so destroy it here.
#ifndef SCHREIERSIMSTECHNIQUE
  BlockGroup::destroy();
#else
  SchreierSimsGroup::destroy();
#endif
}


void Design::constructFormulation()
{
  int alambda = (simpleFlag ? 1 : lambda);

  // Create the objective function
  vector< int > objective;
  for (int i=0; i < numberVariables; ++i)
    objective.push_back(1);
  formulation.setObjectiveFunction(objective, options.getLowerBound(), options.getUpperBound());

  // Add the constraints based on the t-sets
  int *tset = new int[t];
  int *kset = new int[k];
  int *ksetsorted = new int[k];
  int *flags = new int[v];

  int bound = C[v][t];
  for (int i=0; i < bound; ++i) {
    // Unrank the t-set
    duper(v, t, i, tset);

    // Complete the t-set into a k-set using backtracking
    // We begin by initializing
    memset(flags, 0, sizeof(int) * v);
    memcpy(kset, tset, sizeof(int) * t);

    // We use flags to indicate whether a point appears in the k block
    // or not
    for (int j=0; j < v; ++j)
      flags[j] = 1;
    for (int j=0; j < t; ++j)
      flags[tset[j]] = 0;

    // Create the constraint corresponding to the t-block
    vector< int > positions;
    vector< int > coefficients;

    // Now we perform the backtracking
    // We need to complete positions t...k-1 of our block
    // Begin by inserting in the smallest possible values
    int pos = t;
    int next = 0;
    while (pos >= t) {
      // If we have a complete k-set, we can add it to the
      // constraint and backtrack
      if (pos == k) {
	// We need to sort the k-set so that superduper will work properly;
	// we know that 0..t-1 is sorted, and t..k-1 is sorted, so we can
	// simply insert into another vector
	int pos1 = 0;
	int pos2 = t;
	for (int j=0; j < k; ++j)
	  if (pos1 < t)
	    if (pos2 < k)
	      if (kset[pos1] < kset[pos2]) {
		ksetsorted[j] = kset[pos1];
		++pos1;
	      }
	      else {
		ksetsorted[j] = kset[pos2];
		++pos2;
	      }
	    else {
	      ksetsorted[j] = kset[pos1];
	      ++pos1;
	    }
	  else {
	    ksetsorted[j] = kset[pos2];
	    ++pos2;
	  }

	int index = super(v, k, ksetsorted);
	for (int z=0; z < alambda; ++z) {
	  positions.push_back(alambda * index + z);
	  coefficients.push_back(1);
	}

	// Backtrack and return the last element of the k-set to the pool
	// of free elements - begin looking for new completions at position
	// k-1 using the next largest element
	--pos;
	flags[kset[pos]] = 1;
	next = kset[pos] + 1;
	continue;
      }

      // Fill in the positions of kset starting at pos and looking for
      // elements at next
      bool flag = true;
      for (int j=next; j < v; ++j)
	if (flags[j]) {
	  flags[j] = 0;
	  kset[pos] = j;
	  flag = false;
	  break;
	}

      // If we couldn't find an element, we backtrack
      if (flag) {
	--pos;
	flags[kset[pos]] = 1;
	next = kset[pos] + 1;
	continue;
      }

      // We've found an element and added it, so we extend
      next = kset[pos] + 1;
      ++pos;
    }

    // We have completed the constraint. We specify the range and
    // add it to our formulation.
    // Determine if the t-set is in our hole.
    bool inhole = true;
    for (int j=0; j < t; ++j)
      if (hole.find(tset[j]) == hole.end()) {
	inhole = false;
	break;
      }

    // We can now create the constraint.
    Constraint *constraint;
    if (inhole)
      constraint = Constraint::createConstraint(formulation, positions, coefficients, EQUALS, 0);
    else
      constraint = Constraint::createConstraint(formulation, positions, coefficients,
						(designType == DESIGN ? EQUALS :
						 (designType == COVERING ? GREATERTHAN :
						  LESSTHAN)), lambda);
    formulation.addConstraint(constraint);
  }

  // If lambda > 1 and the structure is not simpleFlag, we have lambda variables per block.
  // We force earlier variables to be set to 1 before later ones. This will allow a higher
  // degree of simplicity in the symmetry group and lets us avoid a large number of small
  // subgroups (permuting amongst the variables, which, incidentally, would guarantee the
  // above condition via canonicity).
  if (!simpleFlag && lambda > 1) {
    int bound2 = lambda - 1;
    for (int i=0; i < numberBlocks; ++i) {
      for (int j=0; j < bound2; ++j) {
	vector< int > poss;
	vector< int > coeffs;
	poss.push_back(lambda * i + j);
	coeffs.push_back(1);
	poss.push_back(lambda * i + j + 1);
	coeffs.push_back(-1);
	Constraint *constraint = Constraint::createConstraint(formulation, poss, coeffs, GREATERTHAN, 0);
	formulation.addConstraint(constraint);
      }
    }
  }

  // Free the allocated memory
  delete[] flags;
  delete[] ksetsorted;
  delete[] kset;
  delete[] tset;
}


void Design::determineFixingFlags(int *fixings)
{
  // Convenience variables.
  int alambda = (simpleFlag ? 1 : lambda);

  // Arrays for analyzing and constructing blocks.
  int *block = new int[k];
  int *constructedblock = new int[k];

  // If there is a hole in our design, we fix nothing to 1 and only fix obvious choices to 0.
  // We now process the hole. Any tset in the hole must have all variables in its
  // corrresponding constraint fixed to 0, obviously. Of course, this is only done if
  // there are at least t elements in the hole.
  if (hole.size() >= t) {
    int *tset = new int[t];
    int *notintset = new int[v];
    int notintsetcount;

    // Set up an array for backtracking to create t-sets in the hole.
    int subsetsize = k-t;
    int *extensionset = new int[subsetsize];
    set< int >::iterator *iters = new set< int >::iterator[t];
    for (int i=0; i < t; ++i)
      iters[i] = hole.end();

    // Begin backtracking.
    for (int pos=0; pos >= 0;) {
      // Do we have a complete t-set?
      if (pos == t) {
	// Find all k-set extensions of this t-set. This amounts of finding all k-t subsets of vertices
	// not in our hole.
	// Determine the elements not in the t-set.
	notintsetcount = 0;
	bool flag;
	for (int i=0; i < v; ++i) {
	  flag = false;
	  for (int j=0; j < t; ++j)
	    if (tset[j] == i) {
	      flag = true;
	      break;
	    }
	  
	  if (flag)
	    continue;
	  
	  notintset[notintsetcount] = i;
	  ++notintsetcount;
	}
	
	int bound = C[notintsetcount][subsetsize];
	for (int i=0; i < bound; ++i) {
	  // Unrank to get the extension set. These are actually indices into notinhole.
	  duper(notintsetcount, subsetsize, i, extensionset);

	  // Merge the t-set and the extension to form a k-set.
	  // These are both sorted, so we can do linear insertion.
	  int tpos = 0;
	  int epos = 0;
	  for (int j=0; j < k; ++j) {
	    // Determine what to insert into pos j of our block.
	    if (tpos == t) {
	      block[j] = notintset[extensionset[epos]];
	      ++epos;
	    }
	    else if (epos == subsetsize) {
	      block[j] = tset[tpos];
	      ++tpos;
	    }
	    else {
	      // Determine what element is lower and insert it.
	      if (notintset[extensionset[epos]] < tset[tpos]) {
		block[j] = notintset[extensionset[epos]];
		++epos;
	      }
	      else {
		block[j] = tset[tpos];
		++tpos;
	      }
	    }
	  }

	  int rank = super(v, k, block);
	  for (int j=0; j < alambda; ++j)
	    fixings[alambda * rank + j] = FIXEDTO0;
	}

	// We have processed all k-set extensions of this t-set, so backtrack.
	--pos;
	continue;
      }

      // We want to extend, if possible, at position pos. Determine the starting iterator to consider.
      // If we have extended to this point, we try the next possibility here.
      if (iters[pos] == hole.end())
	if (pos == 0)
	  iters[pos] = hole.begin();
	else {
	  iters[pos] = iters[pos-1];
	  ++iters[pos];
	}
      else
	++iters[pos];

      // If we are at the end, we must backtrack as it is not possible to continue.
      if (iters[pos] == hole.end()) {
	--pos;
	continue;
      }

      // Otherwise, we extend.
      tset[pos] = *(iters[pos]);
      ++pos;
    }

    delete[] iters;
    delete[] extensionset;
    delete[] tset;
    delete[] constructedblock;
    delete[] block;

    // *** NOTE: We only perform 1-fixing if there is no hole. ***
    // Can we do any 1-fixing if there is a hole? If we consider the 2-(7,3,1) design with hole
    // {4, 5, 6}, then if we fix 012 and 034 to 1, we have the following constraint that cannot
    // be satisfied:
    // Pair 05: 015 + 025 + 035 + 045 + 056 = 1
    return;
  }

  // Perform the 1-fixing as appropriate.
  int num1fixed = 0;
  if (designType == DESIGN) {
    if (simpleFlag || lambda == 1) {
      // Fix the 0...(t-2) blocks.
      int bound = t-1;
      for (int i=0; i < bound; ++i)
	block[i] = i;
      for (int i=t-1, pos=t-1; i < v; ++i, ++pos) {
	// Extend the block
	block[pos] = i;
	
	// Process complete blocks
	if (pos == k-1) {
	  fixings[alambda * super(v, k, block)] = FIXEDTO1;
	  ++num1fixed;
	  pos = t-2;
	}
      }

      // Fix the vertical block if possible.
      if (num1fixed >= k-t+2) {
	for (int i=0; i < t-2; ++i)
	  block[i] = i;
	for (int i=t-2; i < k; ++i)
	  block[i] = (t-1) + (i-t+2) * (k-t+1);
	
	fixings[alambda * super(v, k, block)] = FIXEDTO1;
	++num1fixed;
      }
    }
    else {
      // The fact that lambda > 1 makes these potentially non-simpleFlag designs
      // unpredictable, so we may only fix the 0...k-1 block,
      // which is, lexicographically speaking, clearly block 0.
      fixings[0] = FIXEDTO1;
      ++num1fixed;
    }
  }
  else {
    // We can only fix the 0...k-1 block, which is, lexicographically
    // speaking, clearly block 0.
    fixings[0] = FIXEDTO1;
    ++num1fixed;
  }
  
  // If we are doing a covering, we cannot set anything else. Every other
  // variable might be crucial to the solution.
  if (designType == COVERING) {
    delete[] constructedblock;
    delete[] block;
    return;
  }

  // Now that we have all the 1 fixings, we will use this information
  // to determine which t-sets have been covered so far. This information
  // will be used to determine the 0 fixings and which constraints need
  // to be formulated in what manner.
  int numtsets = C[v][t];
  int *tcoverings = new int[numtsets];
  int *tcounters = new int[t];
  int *tset = new int[t];
  memset(tcoverings, 0, numtsets * sizeof(int));

  for (int i=0; i < numberVariables; ++i)
    if (fixings[i] == FIXEDTO1) {
      // Unrank and determine all t-sets, indicating that they
      // are covered.
      duper(v, k, i/alambda, block);

      for (int j=0; j < t; ++j)
	tcounters[j] = -1;

      for (int j=0; j >= 0; ) {
	// If we have a complete t-set, record.
	if (j == t) {
	  ++tcoverings[super(v, t, tset)];

	  --j;
	  continue;
	}

	// Extend the t-set.
	tcounters[j] = (tcounters[j] == -1 ?
			(j > 0 ? tcounters[j-1]+1 : 0) :
			tcounters[j]+1);
	if (tcounters[j] >= k) {
	  tcounters[j] = -1;
	  --j;
	  continue;
	}

	tset[j] = block[tcounters[j]];
	++j;
      }
    }

  // We now use the information regarding the t-set coverings to ascertain
  // the possible number of variables we will maintain for each block. Initially,
  // each unfixed block can appear lambda times, but due to t-set covering, this may not
  // be possible.
  int *numvarsperblock = new int[numberBlocks];
  int numfreevars = 0;
  for (int i=0; i < numberBlocks; ++i) {
    numvarsperblock[i] = (fixings[i] == FIXEDTO1 ? alambda - 1 : alambda);

    // We determine all t-sets of this block and see if they are covered.
    duper(v, k, i, block);
    
    for (int j=0; j < t; ++j)
      tcounters[j] = -1;
    
    for (int j=0; j >= 0; ) {
      // If we have a complete t-set, check if it is covered.
      if (j == t) {
	// This can't be alambda or we accidentally fix blocks that we shouldn't to 0.
 	int numvarsforthistset = lambda - tcoverings[super(v, t, tset)];
 	if (numvarsforthistset < numvarsperblock[i])
 	  numvarsperblock[i] = numvarsforthistset;

 	--j;
 	continue;
      }
      
      // Extend the t-set.
      tcounters[j] = (tcounters[j] == -1 ?
 		      (j > 0 ? tcounters[j-1]+1 : 0) :
 		      tcounters[j]+1);
      
      if (tcounters[j] >= k) {
 	tcounters[j] = -1;
 	--j;
 	continue;
      }
      
      tset[j] = block[tcounters[j]];
      ++j;
    }
    
    numfreevars += numvarsperblock[i];
  }

  // We may now determine what variables can be fixed to 0. These are simply
  // the ones that correspond to blocks with 0 vars per block.
  for (int i=0; i < numberBlocks; ++i)
    if (numvarsperblock[i] == 0)
      for (int j=0; j < alambda; ++j)
	if (fixings[alambda*i + j] == FREE)
	  fixings[alambda*i + j] = FIXEDTO0;

  // Deallocate all allocated memory.
  delete[] numvarsperblock;
  delete[] tset;
  delete[] tcounters;
  delete[] tcoverings;
  delete[] constructedblock;
  delete[] block;
}


void Design::constructSymmetryGroup()
{
  int alambda = (simpleFlag ? 1 : lambda);

  // We construct an initial base.
  int *base = new int[numberVariables];
  establishInitialBase(base);

#ifndef SCHREIERSIMSTECHNIQUE
  BlockGroup::initialize(C[v][k] * alambda);
  // The reduced group does not work; the idea was flawed.
  // group = new BlockGroup(t, v, k, alambda, base);
  group = new BlockGroup(v, k, alambda, hole, base);
#else
  SchreierSimsGroup::initialize(C[v][k] * alambda);
  group = new SchreierSimsGroup(base);
  ((SchreierSimsGroup*)group)->createSymmetryGroup(formulation);
#endif

  delete[] base;
}


// *** CALLBACK FOR ADDING A VARIABLE TO A CONSTRAINT ***
// Given a k-set, convert it to a variable by looking it up
// in a specified set. For example, given {0, 1, 3} to look
// up in {0, 4, 5, 7}, we get 0-0, 1-4, 3-7, giving block
// {0, 4, 7}.
// To use this, the userdata needs to be a void*[4]:
// 0: v
// 1: vector< int > *positions
// 2: vector< int > *coefficients
// 3: int * index.
static void _addVariableToConstraint(int indexsize, int k, int *kset, void *userdata)
{
  // Retrieve the user info.
  void **array = (void**)userdata;
  long v = (long)array[0];
  vector< int > &positions = *((vector< int >*)array[1]);
  vector< int > &coefficients = *((vector< int >*)array[2]);
  int *index = (int*)array[3];
  long lambda = (long)array[4];

  // Create the variable data.
  int *ikset = new int[k];
  for (int i=0; i < k; ++i)
    ikset[i] = index[kset[i]];
  sort(ikset, ikset+k);
  int var = super(v, k, ikset) * lambda;
  delete[] ikset;

  positions.push_back(var);
  coefficients.push_back(1);
}


// Callback for addSpecialCliqueConstraints.
static void _addSpecialCliqueConstraintsCallback(int, int, int *subset, void *userdata)
{
  ((Design*)userdata)->addSpecialCliqueConstraint(subset);
}


void Design::addSpecialCliqueConstraints()
{
  // This could probably be expanded to deal with more complicated cases, i.e.
  // cases where k != t+1 and lambda > 1.
  if (designType == COVERING || k != t+1 || lambda != 1)
    return;

  // For every k+1 set, we want to pick all its k-sets. Thus, generate all
  // k+1 sets from v to start.
  SubsetProducer::createAllSubsets(v, k+1, _addSpecialCliqueConstraintsCallback, (void*)this);
}


void Design::addSpecialCliqueConstraint(int *kp1set)
{
  // We need two vectors, one for position, and one for coefficient.
  vector< int > positions;
  vector< int > coefficients;

  // Now from the kp1set, we need every k-set added to these vectors, indexed by
  // the kp1 set. Create an array of void* to pass all this to the callback.
  void *userinfo[5] = { (void*)v, (void*)&positions, (void*)&coefficients, (void*)kp1set, (void*)1 };
  SubsetProducer::createAllSubsets(k+1, k, _addVariableToConstraint, (void*)userinfo);

  // We have everything we need to make the constraint; add it.
  Constraint *c = Constraint::createConstraint(formulation, positions, coefficients, LESSTHAN, 1);
  formulation.addConstraint(c);
}


void _addPaschConstraintsCallback(int, int, int *subset6, void *userdata)
{
  ((Design*)userdata)->addPaschConstraints(subset6);
}


void Design::addPaschConstraints()
{
  // We need to be dealing with a 2-(v,3,1) packing or design.
  if (designType != PACKING && designType != DESIGN)
    throw string("Pasches cannot be avoided in coverings.");
  if (t != 2)
    throw string("For pasch constraints, need t=2.");
  if (k != 3)
    throw string("For pasch constraints, need k=3.");
  if (lambda != 1)
    throw string("For pasch constraints, need lambda=1.");

  // We need every unique 6-set from v. The idea is then to find the orbit of the
  // set in S_6, which gives us the configurations that we need to avoid. This is
  // done by noting that the size of the automorphism group of a pasch is 24 with
  // generators (1 2)(3 4), (1 3)(2 4), and (0 1)(4 5) (found with nauty). Thus,
  // the size of the orbit is 720/24 = 30, and we can obtain the orbit easily
  // by taking a left transversal of the automorphism group in S_6 to get a coset
  // representative, which will permute the pasch to the pasch represented by the
  // coset, as desired. For each of these, we create a constraint disallowing the
  // configuration.
  SubsetProducer::createAllSubsets(v, 6, _addPaschConstraintsCallback, (void*)this);
}


void Design::addPaschConstraints(int *subset6)
{
  // We now have a 6-set. We want to find all possible pasches over it, of which
  // there are exactly 30 unique ones. We define the 30 pasches here statically and
  // use them as a lookup into the 6-set to get the blocks of the actual pasches.
  // A permutation from the coset generating this pasch is given in the comment on the left.
  const static int pasches[30][4][3] = {
    /* 0 1 2 3 4 5 */ { {0, 1, 2}, {0, 3, 4}, {1, 3, 5}, {2, 4, 5} },
    /* 0 1 2 3 5 4 */ { {0, 1, 2}, {0, 3, 5}, {1, 3, 4}, {2, 4, 5} },
    /* 0 1 2 4 3 5 */ { {0, 1, 2}, {0, 3, 4}, {1, 4, 5}, {2, 3, 5} },
    /* 0 1 2 4 5 3 */ { {0, 1, 2}, {0, 4, 5}, {1, 3, 4}, {2, 3, 5} },
    /* 0 1 2 5 3 4 */ { {0, 1, 2}, {0, 3, 5}, {1, 4, 5}, {2, 3, 4} },
    /* 0 1 2 5 4 3 */ { {0, 1, 2}, {0, 4, 5}, {1, 3, 5}, {2, 3, 4} },
    /* 0 1 3 2 4 5 */ { {0, 1, 3}, {0, 2, 4}, {1, 2, 5}, {3, 4, 5} },
    /* 0 1 3 2 5 4 */ { {0, 1, 3}, {0, 2, 5}, {1, 2, 4}, {3, 4, 5} },
    /* 0 1 3 4 2 5 */ { {0, 1, 3}, {0, 2, 4}, {1, 4, 5}, {2, 3, 5} },
    /* 0 1 3 4 5 2 */ { {0, 1, 3}, {0, 4, 5}, {1, 2, 4}, {2, 3, 5} },
    /* 0 1 3 5 2 4 */ { {0, 1, 3}, {0, 2, 5}, {1, 4, 5}, {2, 3, 4} },
    /* 0 1 3 5 4 2 */ { {0, 1, 3}, {0, 4, 5}, {1, 2, 5}, {2, 3, 4} },
    /* 0 1 4 2 3 5 */ { {0, 1, 4}, {0, 2, 3}, {1, 2, 5}, {3, 4, 5} },
    /* 0 1 4 2 5 3 */ { {0, 1, 4}, {0, 2, 5}, {1, 2, 3}, {3, 4, 5} },
    /* 0 1 4 3 2 5 */ { {0, 1, 4}, {0, 2, 3}, {1, 3, 5}, {2, 4, 5} },
    /* 0 1 4 3 5 2 */ { {0, 1, 4}, {0, 3, 5}, {1, 2, 3}, {2, 4, 5} },
    /* 0 1 4 5 2 3 */ { {0, 1, 4}, {0, 2, 5}, {1, 3, 5}, {2, 3, 4} },
    /* 0 1 4 5 3 2 */ { {0, 1, 4}, {0, 3, 5}, {1, 2, 5}, {2, 3, 4} },
    /* 0 1 5 2 3 4 */ { {0, 1, 5}, {0, 2, 3}, {1, 2, 4}, {3, 4, 5} },
    /* 0 1 5 2 4 3 */ { {0, 1, 5}, {0, 2, 4}, {1, 2, 3}, {3, 4, 5} },
    /* 0 1 5 3 2 4 */ { {0, 1, 5}, {0, 2, 3}, {1, 3, 4}, {2, 4, 5} },
    /* 0 1 5 3 4 2 */ { {0, 1, 5}, {0, 3, 4}, {1, 2, 3}, {2, 4, 5} },
    /* 0 1 5 4 2 3 */ { {0, 1, 5}, {0, 2, 4}, {1, 3, 4}, {2, 3, 5} },
    /* 0 1 5 4 3 2 */ { {0, 1, 5}, {0, 3, 4}, {1, 2, 4}, {2, 3, 5} },
    /* 0 2 3 4 5 1 */ { {0, 2, 3}, {0, 4, 5}, {1, 2, 4}, {1, 3, 5} },
    /* 0 2 3 5 4 1 */ { {0, 2, 3}, {0, 4, 5}, {1, 2, 5}, {1, 3, 4} },
    /* 0 2 4 3 5 1 */ { {0, 2, 4}, {0, 3, 5}, {1, 2, 3}, {1, 4, 5} },
    /* 0 2 4 5 3 1 */ { {0, 2, 4}, {0, 3, 5}, {1, 2, 5}, {1, 3, 4} },
    /* 0 2 5 3 4 1 */ { {0, 2, 5}, {0, 3, 4}, {1, 2, 3}, {1, 4, 5} },
    /* 0 2 5 4 3 1 */ { {0, 2, 5}, {0, 3, 4}, {1, 2, 4}, {1, 3, 5} }
  };

  // Now, for each possible pasch over this 6-set, add a constraint.
  int block[3];
  for (int i=0; i < 30; ++i) {
    vector< int > positions;
    vector< int > coefficients;

    for (int j=0; j < 4; ++j) {
      // Find the permuted block.
      for (int l=0; l < 3; ++l)
	block[l] = subset6[pasches[i][j][l]];

      // We should never need to sort it, since the blocks
      // appear in increasing order, as do the elems in the
      // 6-subset. Rank it and add it to the constraint.
      positions.push_back(super(v, k, block));
      coefficients.push_back(1);
    }

    Constraint *c = Constraint::createConstraint(formulation, positions, coefficients, LESSTHAN, 3);
    formulation.addConstraint(c);
  }
}


void Design::processSolutions()
{
  // We only do this if solutions were not output immediately.
  // Get the solutions and store them in block format.
  DefaultSolutionManager *dsm = dynamic_cast< DefaultSolutionManager* >(options.getSolutionManager());
  if (!dsm)
    return;

  int alambda = (simpleFlag ? 1 : lambda);
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
	Block b(v, k, i/alambda);
	solution.push_back(b);
      }
    solutions.push_back(solution);
  }
}


vector< vector< Block > > &Design::getSolutions()
{
  return solutions;
}



// *** MAIN PROGRAM ***
int main(int argc, char *argv[])
{
  // Encapsulate everything in a try...catch block, since any errors are likely to be unrecoverable.
  try {
    // Command line parameters and derivatives, with sensible defaults.
    int t, v, k, lambda;
    Design::type designType;
    Formulation::SolutionType st = Formulation::SEARCH;
    bool statisticsFlag = true;
    bool simpleFlag = false;

    // Configure a CommandLineProcessing object.
    MargotBACOptions options;
    CommandLineProcessing clp(options);

    // Register the cut producers.
    CliqueCutProducerCreator cliqueCutProducerCreator;
    IsomorphismCutProducerCreator isomorphismCutProducerCreator;
    SpecialCliqueCutProducerCreator specialCliqueCutProducerCreator;
    clp.registerCreator(cliqueCutProducerCreator, 0, true);
    clp.registerCreator(isomorphismCutProducerCreator, 1, true);
    clp.registerCreator(specialCliqueCutProducerCreator, 2, true);

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
      cout << "Usage: " << argv[0] << " <options> [dpc] t v k lamba <sgma>" << endl;
      cout << "[dpc] \t\t design, packing, or covering" << endl;
      cout << "<sgma> \t\t search (default), generation, maximal generation, or all" << endl;
      cout << "-E 0/1 \t\t simple design flag (default: 0)" << endl;
      cout << "-H list \t\t specify a design hole as a comma separated list of vertices" << endl;
      cout << "-s 0/1 \t\t output statistics (default: 1)" << endl;
      cout << endl;
      clp.outputOptions(cout);
      exit(EXIT_SUCCESS);
    }

    // Now we need to ascertain the rest of the command line arguments that are
    // specific to this program and could not be digested by the command line processing.
    bool addSpecialCliqueConstraints = false;
    bool addPaschConstraints = false;
    set< int > hole;
    char *holestr = 0;
    
    // We begin by using getopt to process the command line arguments.
    int opt;
    while ((opt = getopt(argc, argv, "E:H:s:AP")) != -1)
      switch (opt) {
      case 'E':
	simpleFlag = (atoi(optarg) == 1);
	break;
      case 'H':
	holestr = optarg;
	break;
      case 's':
	statisticsFlag = (atoi(optarg) == 1);;
	break;
      case 'A':
	addSpecialCliqueConstraints = true;
	break;
      case 'P':
	addPaschConstraints = true;
	break;
      }
    
    // Make sure there are enough remaining command line options to populate the
    // remaining design parameters.
    if (argc - optind != 5 && argc - optind != 6) {
      cerr << "Need design type, t, v, k, lambda [type of problem]" << endl;
      exit(EXIT_FAILURE);
    }
    
    // The first parameter must be the problem type.
    if (strlen(argv[optind]) != 1) {
      cerr << "Illegal design type: " << argv[optind] << endl;
      exit(EXIT_FAILURE);
    }
    if (argv[optind][0] == 'd')
      designType = Design::DESIGN;
    else if (argv[optind][0] == 'p')
      designType = Design::PACKING;
    else if (argv[optind][0] == 'c')
      designType = Design::COVERING;
    else {
      cerr << "Illegal design type: " << argv[optind] << endl;
      exit(EXIT_FAILURE);
    }
      
    
    // The second parameter must be the t value.
    t = atoi(argv[optind+1]);
    v = atoi(argv[optind+2]);
    k = atoi(argv[optind+3]);
    lambda = atoi(argv[optind+4]);

    // Determine if a type was specified.
    if (argc - optind == 6) {
      if (strlen(argv[optind+5]) != 1) {
	cerr << "illegal problem type: " << argv[optind+5] << endl;
	exit(EXIT_FAILURE);
      }
      switch (argv[optind+5][0]) {
      case 's':
	st = Formulation::SEARCH;
	break;
      case 'g':
	st = Formulation::GENERATION;
	break;
      case 'm':
	st = Formulation::MAXIMALGENERATION;
	break;
      case 'a':
	st = Formulation::ALLGENERATION;
	break;
      default:
	cerr << "illegal problem type: " << argv[optind+5] << endl;
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

    // If there was a hole string, we must now parse it.
    if (holestr && strlen(holestr) > 0) {
      istringstream stream(holestr);
      int vertex;
      while (!stream.eof()) {
	stream >> vertex;
	if (vertex < 0 || vertex >= v) {
	  cerr << "illegal vertex in hole string: " << vertex << endl;
	  exit(EXIT_FAILURE);
	}
	hole.insert(vertex);
      }
    }

    // Formulate the problem and solve.
    Design bd(options, t, v, k, lambda, designType, st, simpleFlag);
    if (hole.size() > 0)
      bd.setHole(hole);
    if (addSpecialCliqueConstraints)
      bd.addSpecialCliqueConstraints();
    if (addPaschConstraints)
      bd.addPaschConstraints();

    // Finish the configuration on the registered objects.
    isomorphismCutProducerCreator.setNumberVariables(bd.getNumberVariables());
    specialCliqueCutProducerCreator.setV(v);
    specialCliqueCutProducerCreator.setK(k);
    defaultSolutionManagerCreator.setProblemType(designType == Design::COVERING ? Formulation::MINIMIZATION : Formulation::MAXIMIZATION);
    defaultSolutionManagerCreator.setSolutionType(st);
    immediateSolutionManagerCreator.setProblemType(designType == Design::COVERING ? Formulation::MINIMIZATION : Formulation::MAXIMIZATION);
    immediateSolutionManagerCreator.setSolutionType(st);
    colexicographicVariableOrderCreator.setV(v);
    colexicographicVariableOrderCreator.setK(k);
    colexicographicVariableOrderCreator.setLambda(lambda);
    randomVariableOrderCreator.setNumberVariables(bd.getNumberVariables());
    closestValueBranchingSchemeCreator.setNumberVariables(bd.getNumberVariables());

    clp.finishMargotBACOptionsConfiguration();

    bd.solve();

    // *** OUTPUT ***
    if (statisticsFlag)
      cout << options.getStatistics() << endl;
    
    // Print the solutions
    vector< vector< Block > > &sols = bd.getSolutions();
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
