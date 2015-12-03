// intersectingsetsystem.h
//
// By Sebastian Raaphorst, 2004
//
// Given parameters v, k, and t, this class encapsulates the
// code to generate a maximum or enumerate all maximum or maximal sets
// B of k-sets over Z_v such that for each b_1, b_2 \in B,
// |b_1 \cap b_2| \ge t.
//
// Note: For this program to work correctly, we require that superduper
// be initialized with minimum value v.

#ifndef INTERSECTINGSETSYSTEM_H
#define INTERSECTINGSETSYSTEM_H

#include <vector>
#include <baclibrary.h>
using namespace std;


class IntersectingSetSystem : public Problem
{
 private:
  // The necessary parameters.
  int v;
  int k;
  int t;

  // The solutions.
  vector< vector< Block > > solutions;

 public:
  // Constructs and solves the formulation for a (v, k, t)
  // intersecting set system. Both lower and upper bounds may
  // be specified on the number of sets that must appear in
  // a final solution, and the user may opt to generate a single
  // ISS, enumerate all maximum ISS, or enumerate all maximal ISS.
  IntersectingSetSystem(BACOptions&, int, int, int,
			Formulation::SolutionType=Formulation::GENERATION);
  virtual ~IntersectingSetSystem();

  // Accessor to get the solutions.
  vector< vector< Block > > &getSolutions(void);

  bool extendSet(int, int, int*, int, bool*, int, int&);
  virtual void constructFormulation(void);
  virtual void determineFixingFlags(int*);
  virtual void constructSymmetryGroup(void);
  virtual void processSolutions(void);
};

#endif
