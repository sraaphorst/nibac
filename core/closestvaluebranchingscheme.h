// closestvaluebranchingscheme.h
//
// By Sebastian Raaphorst, 2006.
//
// This is an extension of RankedBranchingScheme that, when
// given the option, picks the variable closest to a specified
// value.
//
// $Author$
// $Date$

#ifndef CLOSESTVALUEBRANCHINGSCHEME_H
#define CLOSESTVALUEBRANCHINGSCHEME_H

#include <map>
#include <string>
#include "common.h"
#include "nibacexception.h"
#include "rankedbranchingscheme.h"

// Forward declarations
class Node;


class ClosestValueBranchingScheme : public RankedBranchingScheme
{
private:
  double value;

public:
  static const double DEFAULT_VALUE;

  // By default, we use 0.5, since this is likely to perturb the problem
  // a lot, i.e. cause LP solutions to change dramatically from parent to
  // child.
  ClosestValueBranchingScheme(int, const double=DEFAULT_VALUE);
  virtual ~ClosestValueBranchingScheme();

  // There is no reason why the branching scheme should not be changeable
  // between iterations, so allow this.
  inline double getValue() const { return value; }
  inline void setValue(const double pvalue) {
    if (pvalue < 0 || pvalue > 1)
      throw IllegalParameterException("ClosestValueBranchingScheme::value", pvalue, "must be in [0,1]");
    value = pvalue;
  }

protected:
  // This method iterates over the free variables and picks the one closest
  // to value.
  int chooseBranchingVariableIndex(Node&);
};


// This is a way of creating branching schemes through CommandLineProcessing.
// If you do not wish to use CommandLineProcessing, this will be of no value to you.
class ClosestValueBranchingSchemeCreator : public BranchingSchemeCreator
{
private:
  int numberVariables;
  double value;

public:
  ClosestValueBranchingSchemeCreator();
  virtual ~ClosestValueBranchingSchemeCreator();

  inline void setNumberVariables(int pnumberVariables) { numberVariables = pnumberVariables; }
  inline int getNumberVariables(void) const { return numberVariables; }

  inline void setValue(double pvalue) { value = pvalue; }
  inline double getValue(void) { return value; }

protected:
  inline virtual std::string getBranchingSchemeName(void) {
    return std::string("Closest value branching scheme");
  }
  virtual std::map< std::string, std::pair< std::string, std::string > > getOptionsMap(void);

  virtual bool processOptionsString(const char*);

  // Make create protected so that users do not accidentally call
  // this, which would result in memory leakage.
  virtual BranchingScheme *create(void) const;
};

#endif
