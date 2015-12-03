// schreiersimsgroup.h
//
// By Sebastian Raaphorst, 2003.
//
// Schreier-Sims implementation of a group, using certain
// routines that are specific to Margot (i.e. first in orbit,
// stabilizer in group).

#ifndef SCHREIERSIMSGROUP_H
#define SCHREIERSIMSGROUP_H

#include <map>
#include <set>
#include <vector>
#include "common.h"
#ifdef MARGOTTIMERS
#include "timer.h"
#endif
#include "generatedgroup.h"


class SchreierSimsGroup : public GeneratedGroup
{
 protected:
#ifdef USEUSETS
  // The array of ordered U sets
  int ***usets;
#endif

  // Stored lists of permutations for each element in the group
  std::map< int, int* > *lists;

  // The base for the group; set to the identity if not being used
  // The base is a bijection from {0, ..., x-1} to {0, ..., x-1}, so
  // we can actually consider it a member of Sx and hence, it has an
  // inverse. We store the inverse to avoid costly calculations later on.
  int *base;
  int *baseinv;

  // Temporary workspace permutations.
  static int *tmpperm1, *tmpperm2;

  // We simulate recursion by maintaining a stack of permutations.
  // There is no advantage to using an STL stack to do this over a
  // vector, so we'll simply go with a vector.
  static std::vector< int* > rstack;

  // These variables are used by the backtracking for testing canonicity
  // and determining orbits in stabilizers. We declare them here and
  // initialize them in the constructor to avoid repeating this work
  // for every call. They are static since we do not need an individual
  // set for each group.
  static bool *used;
  static int *remain;
  static int *pos;
  static int **hperms;
  static int **locperms;
  static std::vector< int > Jk;
  static std::map< int, int* >::iterator *mapiters;

 public:
  static void initialize(int);
  static void destroy();

  SchreierSimsGroup(int* = 0, int* = 0);
  virtual ~SchreierSimsGroup();

  // Get an arbitrary permutation from the table.
  virtual int *getPermutation(int, int);

  // Implementation of enter from GeneratedGroup
  virtual void enter(int*);

  // Concrete implementation of down from Group
  virtual void down(int, int);

 private:
  // Schreier-Sims specific functions
  int test(int*, int);
  void enter(int*, int, bool=true);

 public:
  // The first flag determines if we want to test canonicity. The second determines if
  // we want to calculate the orbit in the stabilizer. The third tells us if we can
  // use the quick canonicity test (i.e. all parent nodes have been tested for canonicity).
  virtual bool isCanonicalAndOrbInStab(int, int, std::set< int >&, int*,
				       bool=true, bool=true, bool=true);

  // A method to determine if B[0], ..., B[k] is canonical. If we are generating a completely
  // isomorph-free tree, then using isCanonicalAndOrbInStab is much more efficient; however,
  // that algorithm depends on all the parent nodes being canonical, whereas this one
  // does not. This method is called if isCanonicalAndOrbInStab is called with the quick
  // canonicity test turned off.
  bool isCanonical(int);

  // Returns the index of an element in the base as per the description
  // in group.h.
  inline int getPosition(int ppos) { return baseinv[ppos]; }

  // Returns the element in a specified base position, as per the description
  // in group.h.
  inline int getBaseElement(int elem) { return base[elem]; }

  // Returns the number of generators in the group, for statistical purposes.
  int getNumGenerators(void);

  // Returns the size of the group.
  unsigned long getSize(void);

#ifdef NODEGROUPS
  virtual Group *makeCopy();
#endif

 private:
#ifdef DEBUG
  void printTableStructure(void);
#endif

 protected:
  void initializeSets();
  void deleteSets();

  // A static method that we use to insert an element into a sorted set of elements. We do not
  // want to use an STL set for this because the position of the inserted element is crucial.
  static int insertSorted(int, std::vector< int > &);

  // A static method that we use to delete an element from a sorted set of elements.
  static void removeSorted(int, std::vector< int >&);
};


#endif
