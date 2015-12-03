// isomorphismcutproducer.h
//
// By Sebastian Raaphorst, 2004.
//
// This cut producer generates the isomorphism cuts outlined
// by Margot in his papers.
//
// $Author$
// $Date$

#ifndef ISOMORPHISMCUTPRODUCER_H
#define ISOMORPHISMCUTPRODUCER_H

#include <string>
#include <map>
#include "common.h"
#include "bac.h"
#include "cutproducer.h"
#include "group.h"
#include "isomorphismcut.h"
#include "node.h"
#include "permutationpool.h"


class IsomorphismCutProducer : public CutProducer
{
public:
  static const double IC_DEFAULT_MIN_FRACTIONAL_VALUE;

private:
  // The number of variables.
  int numberVariables;

  // The minimum fractional value of the variables that we want
  // to consider.
  double IC_MIN_FRACTIONAL_VALUE;

  // Some things used for the backtracking.
  int *tmpperm;
  bool *used;
  int *remain;
  int *pos;
  int *selected;
  int **hperms;
  double *sum_x;

public:
  IsomorphismCutProducer(int, double=IC_DEFAULT_MIN_FRACTIONAL_VALUE);
  virtual ~IsomorphismCutProducer();

  // Cut generator
  virtual void generateCuts(BAC&, Node&, double, int&, double&);
};




// Very basic structure for configuring an isomorphism cut producer.
class IsomorphismCutProducerCreator : public CutProducerCreator
{
protected:
  // numberVariables *MUST* be configured prior to CommandLineProcessing::finalize
  // being called or a MissingDataException will be thrown.
  int numberVariables;

  // This has default according to the IsomorphismCutProducer.
  double IC_MIN_FRACTIONAL_VALUE;

  virtual inline std::string getCutProducerName(void) {
    return std::string("Isomorphism Cuts");
  }
  virtual std::map< std::string, std::pair< std::string, std::string > > getOptionsMap(void);

  virtual CutProducer *create(void) const;
  virtual bool processOptionsString(const char*);

public:
  IsomorphismCutProducerCreator();
  virtual ~IsomorphismCutProducerCreator();

  // This needs to be explicitly set by the end user.
  inline int getNumberVariables(void) const { return numberVariables; }
  inline void setNumberVariables(int pnumberVariables) {
    numberVariables = pnumberVariables;
  }

  // This takes on the default value as per the class.
  inline double getMinimumFractionalValue(void) const { return IC_MIN_FRACTIONAL_VALUE; }
  inline void setMinimumFractionalValue(double pIC_MIN_FRACTIONAL_VALUE) {
    IC_MIN_FRACTIONAL_VALUE = pIC_MIN_FRACTIONAL_VALUE;
  }
};

#endif
