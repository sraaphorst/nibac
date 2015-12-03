// specialcliquecutproducer.h
//
// By Sebastian Raaphorst, 2004.
//
// An optimal clique cut producer for t-(v,k,l) designs
// when k = t+1.
//
// $Author$
// $Date$

#ifndef SPECIALCLIQUECUTPRODUCER_H
#define SPECIALCLIQUECUTPRODUCER_H

#include <baclibrary.h>


class SpecialCliqueCutProducer : public CutProducer
{
 private:
  int v, k;

  // The fractional threshhold; we only include variables into
  // our clique graph if they fall between fracthreshhold and
  // 1 - fracthreshhold.
  double fracthreshhold;

  // Some work arrays.
  int *kset;
  int *kp1set;
  int *kp1flags;
  int *flags;

 public:
  // There are two constructors for this class; one of them accepts
  // a group, because a base ordering in the manner prescribed by
  // Margot helps us to create cliques more effectively.
  SpecialCliqueCutProducer(int, int, double=CliqueCutProducer::CC_DEFAULT_FRACTIONAL_THRESHOLD);
  virtual ~SpecialCliqueCutProducer();

  virtual void generateCuts(BAC&, Node&, double, int&, double&);
};


// Very basic structure for creating a clique cut producer. Used by CommandLineProcessing.
class SpecialCliqueCutProducerCreator : public CutProducerCreator
{
protected:
  // Parameters required to formulate special cliques.
  int v;
  int k;

  double CC_FRACTIONAL_THRESHOLD;

  virtual inline std::string getCutProducerName(void) {
    return std::string("Special Clique Cuts");
  }
  virtual std::map< std::string, std::pair< std::string, std::string > > getOptionsMap(void);

  virtual CutProducer *create(void) const;
  virtual bool processOptionsString(const char*);

public:
  SpecialCliqueCutProducerCreator();
  virtual ~SpecialCliqueCutProducerCreator();

  inline void setV(int pv) { v = pv; }
  inline int getV(void) const { return v; }
  inline void setK(int pk) { k = pk; }
  inline int getK(void) const { return k; }

  inline double getFractionalThreshold(void) const { return CC_FRACTIONAL_THRESHOLD; }
  inline void setFractionalThreshold(double pCC_FRACTIONAL_THRESHOLD) {
    CC_FRACTIONAL_THRESHOLD = pCC_FRACTIONAL_THRESHOLD;
  }
};

#endif
