// defaultsolutionmanager.cpp
//
// By Sebastian Raaphorst, 2003.

#include <limits.h>
#include <float.h>
#include <map>
#include <string>
#include <vector>
#include "common.h"
#include "nibacexception.h"
#include "node.h"
#include "solutionmanager.h"
#include "defaultsolutionmanager.h"


DefaultSolutionManager::DefaultSolutionManager(Formulation::ProblemType p, Formulation::SolutionType s)
  : ptype(p), stype(s)
{
  if (p == Formulation::PROBLEMTYPE_UNDEFINED)
    throw IllegalParameterException("DefaultSolutionManager::ProblemType", "undefined", "Must specify if maximization / minimization problem.");
  if (s == Formulation::SOLUTIONTYPE_UNDEFINED)
    throw IllegalParameterException("DefaultSolutionManager::SolutionType", "undefined", "Must specify type of problem.");

  bestsoln = (p == Formulation::MAXIMIZATION ? DBL_MIN : DBL_MAX);
  generateall = (s == Formulation::MAXIMALGENERATION || s == Formulation::ALLGENERATION);
}


DefaultSolutionManager::~DefaultSolutionManager()
{
  clearVector();
}


void DefaultSolutionManager::newSolution(Node &n)
{
  // If we are working with searches or generation of optimal, we need to check if the current solution
  // is better than the previously recorded solutions.
  if (stype == Formulation::SEARCH || stype == Formulation::GENERATION) {
    // Determine if the solution at n is better than what we have
    // encountered so far.
    bool bestsolflag = ((ptype == Formulation::MAXIMIZATION && greaterthan(n.getSolutionValue(), bestsoln))
			|| (ptype == Formulation::MINIMIZATION && lessthan(n.getSolutionValue(), bestsoln)));

    // If this solution is better, we set the best solution for the manager.
    if (bestsolflag) {
      bestsoln = n.getSolutionValue();

      // If we are not generating all solutions of a certain type and are only interested in
      // optimal solutions, we may clear the solution vector. Any previously stored solutions
      // are certainly not optimal.
      if (!generateall)
	clearVector();
    }
  }

  std::vector< int > *sol = new std::vector< int >;
  double *d = n.getSolutionVariableArray();
  int bound = n.getNumberBranchingVariables();
  for (int i=0; i < bound; ++i)
    sol->push_back(varround(d[i]));
  solutions.push_back(sol);
}


std::vector< std::vector< int > * > &DefaultSolutionManager::getSolutions()
{
  return solutions;
}


void DefaultSolutionManager::clearVector()
{
  while (solutions.size() > 0) {
    std::vector< int > *sol = solutions.back();
    solutions.pop_back();
    delete sol;
  }
}



// *** DEFAULTSOLUTIONMANAGERCREATOR METHODS ***
DefaultSolutionManagerCreator::DefaultSolutionManagerCreator()
  : problemType(Formulation::PROBLEMTYPE_UNDEFINED),
    solutionType(Formulation::SOLUTIONTYPE_UNDEFINED)
{
}


DefaultSolutionManagerCreator::~DefaultSolutionManagerCreator()
{
}


std::map< std::string, std::pair< std::string, std::string > > DefaultSolutionManagerCreator::getOptionsMap(void)
{
  std::map< std::string, std::pair< std::string, std::string > > optionsMap;
  return optionsMap;
}


bool DefaultSolutionManagerCreator::processOptionsString(const char *options)
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
      throw IllegalParameterException("DefaultSolutionManager::ConfigurationString",
				      options,
				      "could not process string");
    stream >> eqls;
    if (!stream || eqls != '=')
      throw IllegalParameterException("DefaultSolutionManager::ConfigurationString",
				      options,
				      "could not process string");

    switch (ch) {
    default:
      std::ostringstream outputstream;
      outputstream << ch;
      throw IllegalParameterException("DefaultSolutionManager::ConfigurationString",
				      outputstream.str().c_str(),
				      "not a supported option");
    }

    if (!stream.eof()) {
      stream >> ch;
      if (!stream || ch != ':')
	throw IllegalParameterException("DefaultSolutionManager::ConfigurationString",
					options,
					"could not process string");
    }
  }

  return true;
}


SolutionManager *DefaultSolutionManagerCreator::create(void) const
{
  return new DefaultSolutionManager(problemType, solutionType);
}

