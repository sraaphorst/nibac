// immediatesolutionmanager.h
//
// By Sebastian Raaphorst, 2004.
//
// An implementation of a solution manager that immediately outputs
// solutions as they're found. As we are outputting solutions as they
// are discovered, it is entirely possible that solutions that are
// not optimal may be output; solutions will certainly be output in
// order of increasing optimality for optimization problems, but if
// the user opts to use this technique, it is up to him / her to
// filter out the nonoptimal ones.
//
// $Author$
// $Date$

#ifndef IMMEDIATESOLUTIONMANAGER_H
#define IMMEDIATESOLUTIONMANAGER_H

#include <iostream>
#include <map>
#include <string>
#include "common.h"
#include "node.h"
#include "solutionmanager.h"


class ImmediateSolutionManager : public SolutionManager
{
 protected:
  std::ostream &ostr;
  double bestsoln;
  Formulation::ProblemType ptype;
  Formulation::SolutionType stype;
  bool generateall;

public:
  ImmediateSolutionManager(Formulation::ProblemType,
			   Formulation::SolutionType,
			   std::ostream& = std::cout);
  virtual ~ImmediateSolutionManager();

  virtual void newSolution(Node&);
};


// This is a way of creating solution managers through CommandLineProcessing.
// If you do not wish to use CommandLineProcessing, this will be of no value to you.
class ImmediateSolutionManagerCreator : public SolutionManagerCreator
{
private:
  Formulation::ProblemType problemType;
  Formulation::SolutionType solutionType;
  std::ostream *ostr;

public:
  ImmediateSolutionManagerCreator();
  virtual ~ImmediateSolutionManagerCreator();

  inline void setProblemType(Formulation::ProblemType pproblemType) {
    problemType = pproblemType;
  }
  inline Formulation::ProblemType getProblemType(void) { return problemType; }

  inline void setSolutionType(Formulation::SolutionType psolutionType) {
    solutionType = psolutionType;
  }
  inline Formulation::SolutionType getSolutionType(void) { return solutionType; }

  inline void setOutputStream(std::ostream &postr) { ostr = &postr; }
  inline std::ostream &getOutputStream(void) { return *ostr; }

protected:
  inline virtual std::string getSolutionManagerName(void) {
    return std::string("Immediate output solution manager");
  }
  virtual std::map< std::string, std::pair< std::string, std::string > > getOptionsMap(void);

  virtual bool processOptionsString(const char*);

  // Make create protected so that users do not accidentally call
  // this, which would result in memory leakage.
  virtual SolutionManager *create(void) const;
};

#endif
