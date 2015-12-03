// lexicographicvariableorder.h
//
// By Sebastian Raaphorst, 2006.
//
// This subclass of VariableOrder provides the default ordering on
// variables, i.e. the order sorted on index.
//
// $Author$
// $Date$

#ifndef LEXICOGRAPHICVARIABLEORDER_H
#define LEXICOGRAPHICVARIABLEORDER_H

#include "common.h"
#include "variableorder.h"


class LexicographicVariableOrder : public VariableOrder
{
public:
  LexicographicVariableOrder();
  virtual ~LexicographicVariableOrder();

  void sort(int, const int*, int*);

  // This is just the identity function.
  inline int variableToIndex(int variable) { return variable; }
  inline int indexToVariable(int index) { return index; }
};


// This is a way of creating variable orders through CommandLineProcessing.
// If you do not wish to use CommandLineProcessing, this will be of no value to you.
class LexicographicVariableOrderCreator : public VariableOrderCreator
{
public:
  LexicographicVariableOrderCreator();
  virtual ~LexicographicVariableOrderCreator();

protected:
  inline virtual std::string getVariableOrderName(void) {
    return std::string("Lexicographic variable ordering");
  }
  virtual std::map< std::string, std::pair< std::string, std::string > > getOptionsMap(void);

  virtual bool processOptionsString(const char*);

  // Make create protected so that users do not accidentally call
  // this, which would result in memory leakage.
  virtual VariableOrder *create(void) const;
};

#endif

