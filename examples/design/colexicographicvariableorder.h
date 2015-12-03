// colexicographicvariableorder.h
//
// By Sebastian Raaphorst, 2006.
//
// This subclass of VariableOrder provides the colexicographic
// order on blocks, translating that to variables. This is induced
// by the lexicographic ordering on reverse sequences and is described
// more fully in Combinatorial Algorithms: Generation, Enumeration
// and Search, by Kreher and Stinson, 1999.
//
// Note that this assumes that the problem has been formulated with
// the variables indexed by their lexicographic number. The
// colexicographic element comes in simply in ordering the variables.
//
// We require superduper to be initialized up to at least v to use
// this class.
//
// $Author$
// $Date$

#ifndef COLEXICOGRAPHICVARIABLEORDER_H
#define COLEXICOGRAPHICVARIABLEORDER_H

#include <baclibrary.h>


class ColexicographicVariableOrder : public VariableOrder
{
private:
  int v;
  int k;
  int lambda;

public:
  ColexicographicVariableOrder(int, int, int);
  virtual ~ColexicographicVariableOrder();

  void sort(int, const int*, int*);

  // To convert a variable to an index, we simply unrank with respect
  // to lexicographical order to get the block and then rerank it with
  // respect to colexicographic order.
  int variableToIndex(int);
  int indexToVariable(int);

  // Static members to perform standard co-lex ranking and unranking.
  // Note that ranking does not require the size of the base set for co-lex.
  static int rank(int, int*);
  static void unrank(int, int, int, int*);
};


// This is a way of creating variable orders through CommandLineProcessing.
// If you do not wish to use CommandLineProcessing, this will be of no value to you.
class ColexicographicVariableOrderCreator : public VariableOrderCreator
{
private:
  int v;
  int k;
  int lambda;

public:
  ColexicographicVariableOrderCreator();
  virtual ~ColexicographicVariableOrderCreator();

  inline void setV(int pv) { v = pv; }
  inline int getV(void) { return v; }

  inline void setK(int pk) { k = pk; }
  inline int getK(void) { return k; }

  inline void setLambda(int plambda) { lambda = plambda; }
  inline int getLambda(void) { return lambda; }

protected:
  inline virtual std::string getVariableOrderName(void) {
    return std::string("Colexicographic variable ordering");
  }
  virtual std::map< std::string, std::pair< std::string, std::string > > getOptionsMap(void);

  virtual bool processOptionsString(const char*);

  // Make create protected so that users do not accidentally call
  // this, which would result in memory leakage.
  virtual VariableOrder *create(void) const;
};

#endif

