// cliquecutproducer.h
//
// By Sebastian Raaphorst, 2004.
//
// This subclass of CutProducer generates generic clique cuts for
// all the inequalities of the form ax <= b, where b=1. This could be
// generalized for other b using hypergraphs, but due to time
// constraints, we shall not implement this for the present moment.
//
// $Author$
// $Date$

#ifndef CLIQUECUTPRODUCER_H
#define CLIQUECUTPRODUCER_H

#include <map>
#include <string>
#include <vector>
#include "common.h"
#include "bac.h"
#include "cutproducer.h"
#include "graph.h"
#include "node.h"


class CliqueCutProducer : public CutProducer
{
 public:
  static const double CC_DEFAULT_FRACTIONAL_THRESHOLD;
  static const double CC_DEFAULT_FRACTIONAL_PREFERENCE;
  static const int    CC_DEFAULT_ENUMERATION_VALUE;

 private:
  double CC_FRACTIONAL_THRESHOLD;
  double CC_FRACTIONAL_PREFERENCE;
  int CC_ENUMERATION_VALUE;

 public:
  // There are two constructors for this class. One of them involves
  // a group, which is necessary because we need to know the base in
  // order to properly create cliques with the right variables efficiently.
  CliqueCutProducer(double=CC_DEFAULT_FRACTIONAL_THRESHOLD,
		    double=CC_DEFAULT_FRACTIONAL_PREFERENCE,
		    int=CC_DEFAULT_ENUMERATION_VALUE);
  virtual ~CliqueCutProducer();

  virtual void generateCuts(BAC&, Node&, double, int&, double&);

 private:
  // Given a graph and a subset of the vertices in the graph (i.e.
  // a component), determine all maximal cliques over the vertices
  // and return this list as the third parameter to the function.
  void determineMaximalCliques(Graph&, std::vector< int >&,
			       std::vector< std::vector< int > >&);

  void cliqueHeuristic(Graph&, std::vector< int >&, double*, int, int, int*, float,
		       std::vector< std::vector< int > >&);
  void cliqueHeuristicAux(Graph&, int, double*, std::vector< int >&);
};



// Very basic structure for creating a clique cut producer. Used by CommandLineProcessing.
class CliqueCutProducerCreator : public CutProducerCreator
{
protected:
  double CC_FRACTIONAL_THRESHOLD;
  double CC_FRACTIONAL_PREFERENCE;
  int CC_ENUMERATION_VALUE;

  virtual inline std::string getCutProducerName(void) {
    return std::string("Generalized Clique Cuts");
  }
  virtual std::map< std::string, std::pair< std::string, std::string > > getOptionsMap(void);

  virtual CutProducer *create(void) const;
  virtual bool processOptionsString(const char*);

public:
  CliqueCutProducerCreator();
  virtual ~CliqueCutProducerCreator();

  inline double getFractionalThreshold(void) const { return CC_FRACTIONAL_THRESHOLD; }
  inline void setFractionalThreshold(double pCC_FRACTIONAL_THRESHOLD) {
    CC_FRACTIONAL_THRESHOLD = pCC_FRACTIONAL_THRESHOLD;
  }

  inline double getFractionalPreference(void) const { return CC_FRACTIONAL_PREFERENCE; }
  inline void setFractionalPreference(double pCC_FRACTIONAL_PREFERENCE) {
    CC_FRACTIONAL_PREFERENCE = pCC_FRACTIONAL_PREFERENCE;
  }

  inline int getEnumerationValue(void) const { return CC_ENUMERATION_VALUE; }
  inline void setEnumerationValue(int pCC_ENUMERATION_VALUE) {
    CC_ENUMERATION_VALUE = pCC_ENUMERATION_VALUE;
  }
};

#endif
