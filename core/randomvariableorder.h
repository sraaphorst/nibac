// randomvariableorder.h
//
// By Sebastian Raaphorst, 2006.
//
// This subclass of VariableOrder provides a completely random ordering
// on the variables it is given. It requires that the RNG consisting of
// random() in stdlib.h be seeded using srandom() for the desired
// unpredictability.
//
// $Author$
// $Date$

#ifndef RANDOMVARIABLEORDER_H
#define RANDOMVARIABLEORDER_H

#include <map>
#include <string>
#include "common.h"
#include "variableorder.h"


class RandomVariableOrder : public VariableOrder
{
private:
  // We use arrays to establish an order on the variables.
  int numberVariables;
  int *indexList;
  int *variableList;

public:
  // Static method to initialize the RNG, if users do not wish to do so manually.
  static void initializeRNG(void);

  RandomVariableOrder(int);
  virtual ~RandomVariableOrder();

  void sort(int, const int*, int*);

  inline int variableToIndex(int variable) { return indexList[variable]; }
  inline int indexToVariable(int index) { return variableList[index]; }
};



// This is a way of creating variable orders through CommandLineProcessing.
// If you do not wish to use CommandLineProcessing, this will be of no value to you.
class RandomVariableOrderCreator : public VariableOrderCreator
{
protected:
  int numberVariables;

public:
  RandomVariableOrderCreator();
  virtual ~RandomVariableOrderCreator();

  inline void setNumberVariables(int pnumberVariables) { numberVariables = pnumberVariables; }
  inline int getNumberVariables(void) const { return numberVariables; }

protected:
  inline virtual std::string getVariableOrderName(void) {
    return std::string("Random variable ordering");
  }
  virtual std::map< std::string, std::pair< std::string, std::string > > getOptionsMap(void);

  virtual bool processOptionsString(const char*);

  // Make create protected so that users do not accidentally call
  // this, which would result in memory leakage.
  virtual VariableOrder *create(void) const;
};

#endif

