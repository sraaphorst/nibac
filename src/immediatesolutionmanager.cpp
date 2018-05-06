// immediatesolutionmanager.cpp
//
// By Sebastian Raaphorst, 2004.
//
// $Author$
// $Date$

#include <iostream>
#include <float.h>
#include <limits.h>
#include <map>
#include <sstream>
#include <string>
#include "common.h"
#include "nibacexception.h"
#include "node.h"
#include "solutionmanager.h"
#include "immediatesolutionmanager.h"


ImmediateSolutionManager::ImmediateSolutionManager(Formulation::ProblemType p,
						   Formulation::SolutionType s,
						   std::ostream &postr)
  : ostr(postr), ptype(p), stype(s)
{
  bestsoln = (p == Formulation::MAXIMIZATION ? DBL_MIN : DBL_MAX);
  generateall = (s == Formulation::MAXIMALGENERATION || s == Formulation::ALLGENERATION);
}


ImmediateSolutionManager::~ImmediateSolutionManager()
{
}


void ImmediateSolutionManager::newSolution(Node &n)
{
  // If we are working with searches or generation of optimal, we need
  // to check if the current solution is better than the previously
  // recorded solutions.
  if (stype == Formulation::SEARCH || stype == Formulation::GENERATION) {
    // Determine if the solution at n is better than what we have
    // encountered so far.
    bool bestsolflag = ((ptype == Formulation::MAXIMIZATION && greaterthan(n.getSolutionValue(), bestsoln))
			|| (ptype == Formulation::MINIMIZATION && lessthan(n.getSolutionValue(), bestsoln)));

    // If this solution is better, we set the best solution for the manager.
    if (bestsolflag) {
      bestsoln = n.getSolutionValue();

      // NOTE THAT IT IS LIKELY THAT NONOPTIMAL SOLUTIONS MAY HAVE BEEN
      // OUTPUT AT THIS POINT. WE INDICATE THAT WE HAVE FOUND A BETTER
      // SOLUTION.
      if (!generateall)
	ostr << "*** CLEAR ***" << std::endl;
    }
  }

  double *d = n.getSolutionVariableArray();
  int bound = n.getNumberBranchingVariables();
  bool flag = false;
  for (int i=0; i < bound; ++i)
    if (varround(d[i])) {
      if (flag)
	ostr << " ";
      flag = true;
      ostr << i;
  }
  ostr << std::endl;
}



// *** IMMEDIATESOLUTIONMANAGERCREATOR METHODS ***
ImmediateSolutionManagerCreator::ImmediateSolutionManagerCreator()
  : problemType(Formulation::PROBLEMTYPE_UNDEFINED),
    solutionType(Formulation::SOLUTIONTYPE_UNDEFINED),
    ostr(&(std::cout))
{
}


ImmediateSolutionManagerCreator::~ImmediateSolutionManagerCreator()
{
}


std::map< std::string, std::pair< std::string, std::string > > ImmediateSolutionManagerCreator::getOptionsMap(void)
{
  std::map< std::string, std::pair< std::string, std::string > > optionsMap;
  return optionsMap;
}


bool ImmediateSolutionManagerCreator::processOptionsString(const char *options)
{
  char ch, eqls;

  // We must explicitly check for empty string prior to processing, since an empty string does
  // not generate an EOF status.
  if (strlen(options) == 0)
    return true;

  std::istringstream stream(options);
  while (!stream.eof()) {
    stream >> ch;
    if (!stream)
      throw IllegalParameterException("ImmediateSolutionManager::ConfigurationString",
				      options,
				      "could not process string");
    stream >> eqls;
    if (!stream || eqls != '=')
      throw IllegalParameterException("ImmediateSolutionManager::ConfigurationString",
				      options,
				      "could not process string");

    switch (ch) {
    default:
      std::ostringstream outputstream;
      outputstream << ch;
      throw IllegalParameterException("ImmediateSolutionManager::ConfigurationString",
				      outputstream.str().c_str(),
				      "not a supported option");
    }

    if (!stream.eof()) {
      stream >> ch;
      if (!stream || ch != ':')
	throw IllegalParameterException("ImmediateSolutionManager::ConfigurationString",
					options,
					"could not process string");
    }
  }

  return true;
}


SolutionManager *ImmediateSolutionManagerCreator::create(void) const
{
  return new ImmediateSolutionManager(problemType, solutionType, *ostr);
}
