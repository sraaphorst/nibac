// specialcliquecutproducer.cpp
//
// By Sebastian Raaphorst, 2004.
//
// $Author$
// $Date$

#include <vector>
#include <baclibrary.h>
#include "specialcliquecutproducer.h"
using namespace std;


SpecialCliqueCutProducer::SpecialCliqueCutProducer(int pv,
						   int pk,
						   double pfracthreshhold)
  : v(pv),
    k(pk),
    fracthreshhold(pfracthreshhold),
    kset(new int[k]),
    kp1set(new int[k+1]),
    kp1flags(new int[C[v][k+1]]),
    flags(new int[v])
{
  if (v < 0)
    throw IllegalParameterException("SpecialCliqueCutProducer::v", pv, "v must be nonnegative");
  if (k < 0)
    throw IllegalParameterException("SpecialCliqueCutProducer::k", pk, "k must be nonnegative");
  if (k > v)
    throw IllegalParameterException("SpecialCliqueCutProducer::k", pv, "k cannot be greator than v");
}


SpecialCliqueCutProducer::~SpecialCliqueCutProducer()
{
  delete[] kset;
  delete[] kp1set;
  delete[] kp1flags;
  delete[] flags;
}


void SpecialCliqueCutProducer::generateCuts(BAC &bac,
					    Node &n,
					    double violationTolerance,
					    int &numberOfCuts,
					    double &maximumViolation)
{
  numberOfCuts = 0;
  maximumViolation = 0;

  Group *g = n.getSymmetryGroup();

  // Reset the kp1flags array.
  int kp1 = k+1;
  memset(kp1flags, 0, sizeof(int) * C[v][kp1]);

  // For each fractional variable that satisfies our requirements, we simply try all of
  // its k+1-sets and see if they are both uncovered and if they lead to a clique cut.
  // If they do, we generate the cut.

  // We declare several variables here to avoid unnecessary repeated
  // declarations later on.
  Formulation &f = bac.getFormulation();
  double *soln = n.getSolutionVariableArray();
  int index, kp1index;
  double valuation, violation;
  int addindex;

  // vector to store the indices of the variables in the clique.
  vector< int > cliqueindices;
  cliqueindices.reserve(kp1);

  // The first thing we need to do is determine the number of fractional
  // variables, say m, and create a bijective mapping between Z_m and them.
  int startbound = n.getNumberFixedVariables() - n.getNumber0FixedVariables();
  int endbound = n.getNumberBranchingVariables() - n.getNumber0FixedVariables();

  // We need the variable to be between max(fracthreshhold, (1+vT)/(t+2)) and
  // 1-fracthreshhold. The reason for the 1/(t+2) is because each clique
  // equality will have k+1=t+2 variables in it, and in order to sum to
  // exceed 1+vT (leading to a violated inequality), we need that at least one
  // of the variables have value higher than (1+vT)/(t+2).
  double lowerbound = max(fracthreshhold, (((double)1)+violationTolerance)/((double)(kp1)));
  double upperbound = 1 - fracthreshhold;

  for (int var=startbound; var < endbound; ++var) {
    // We are iterating over the free section of the base, so we need to
    // get the index of the variable in this position.
    index = (g ? g->getBaseElement(var) : var);
    if (soln[index] >= lowerbound && soln[index] <= upperbound) {
      // We unrank this variable into a k-set, and then record in the flags
      // array which points appear in this k-set.
      duper(v, k, index, kset);

      memset(flags, 0, sizeof(int) * v);
      for (int i=0; i < k; ++i)
	flags[kset[i]] = 1;

      // Now we attempt to create all k+1-sets from this k-set.
      for (int i=0; i < v; ++i) {
	// If this point already appears in the original k-set, it obviously
	// won't give us a k+1-set, so we simply skip over it.
	if (flags[i])
	  continue;

	// We now have a k+1-set; create it in sorted order.
	addindex = 0;
	for (int j=0; j < k;)
	  if (addindex==0 && i < kset[j]) {
	    kp1set[j] = i;
	    addindex = 1;
	  }
	  else {
	    kp1set[j+addindex] = kset[j];
	    ++j;
	  }
	if (addindex == 0)
	  kp1set[k] = i;

	// Rank it and check if it has already been considered. If not, mark
	// it considered.
	kp1index = super(v, kp1, kp1set);
	if (kp1flags[kp1index])
	  continue;
	kp1flags[kp1index] = 1;

	// We have a new k+1-set. We need all k-sets from this k+1 set.
	// We get these simply by omitting an element from the k+1-set.
	valuation = 0;
	cliqueindices.clear();
	for (int j=0; j < kp1; ++j) {
	  // Omit position j from the kp1 set.
	  for (int m=0; m < k; ++m)
	    kset[m] = (m < j ? kp1set[m] : kp1set[m+1]);

	  // Rank, increment the violation, and push back.
	  index = super(v, k, kset);
	  valuation += soln[index];
	  cliqueindices.push_back(index);
	}

	// We now have a clique inequality. Determine if it matches our violation
	// requirements.
	violation = valuation - 1;
	if (isunviolated(valuation, 0, 1 + violationTolerance))
	  continue;

	// It is, so we add a constraint to our node.
	Constraint *c = Constraint::createConstraint(f, cliqueindices, LESSTHAN, 1);
	n.addCut(c);
	++numberOfCuts;
	if (violation > maximumViolation)
	  maximumViolation = violation;
      }
    }
  }
}


// *** SpecialCLIQUECUTPRODUCERCREATOR METHODS ***
SpecialCliqueCutProducerCreator::SpecialCliqueCutProducerCreator()
  : v(-1),
    k(-1),
    CC_FRACTIONAL_THRESHOLD(CliqueCutProducer::CC_DEFAULT_FRACTIONAL_THRESHOLD)
{
}


SpecialCliqueCutProducerCreator::~SpecialCliqueCutProducerCreator()
{
}


std::map< std::string, std::pair< std::string, std::string > > SpecialCliqueCutProducerCreator::getOptionsMap(void)
{
  std::map< std::string, std::pair< std::string, std::string > > optionsMap;

  std::ostringstream fractionalThresholdStream;
  fractionalThresholdStream << CliqueCutProducer::CC_DEFAULT_FRACTIONAL_THRESHOLD;
  optionsMap[std::string("T")] =
    std::pair< std::string, std::string >(std::string("Fractional threshold: a variable will be considered "
						      "for clique cuts if its value in the solution of the "
						      "LP relaxation exceeds this parameter."),
					  fractionalThresholdStream.str());

  return optionsMap;
}


bool SpecialCliqueCutProducerCreator::processOptionsString(const char *options)
{
  char ch, eqls;
  double dvalue;

  // We must explicitly check for empty string prior to processing, since an empty string does
  // not generate an EOF status.
  if (strlen(options) == 0)
    return true;

  std::istringstream stream(options);
  while (!stream.eof()) {
    stream >> ch;
    if (!stream)
      throw IllegalParameterException("SpecialCliqueCutProducer::ConfigurationString",
				      options,
				      "could not process string");
    stream >> eqls;
    if (!stream || eqls != '=')
      throw IllegalParameterException("SpecialCliqueCutProducer::ConfigurationString",
				      options,
				      "could not process string");

    switch (ch) {
    case 'T':
      stream >> dvalue;
      if (!stream)
	throw IllegalParameterException("SpecialCliqueCutProducer::FractionalThreshold",
					"undefined",
					"could not interpret value in configuration string");
      setFractionalThreshold(dvalue);
      break;

    default:
      std::ostringstream outputstream;
      outputstream << ch;
      throw IllegalParameterException("SpecialCliqueCutProducer::ConfigurationString",
				      outputstream.str().c_str(),
				      "not a supported option");
    }

    if (!stream.eof()) {
      stream >> ch;
      if (!stream || ch != ':')
	throw IllegalParameterException("SpecialCliqueCutProducer::ConfigurationString",
					options,
					"could not process string");
    }
  }

  return true;
}


CutProducer *SpecialCliqueCutProducerCreator::create(void) const
{
  if (v <= 0)
    throw MissingDataException("SpecialCliqueCutProducerCreator requires v to be populated.");
  if (k <= 0)
    throw MissingDataException("SpecialCliqueCutProducerCreator requires k to be populated.");
  return new SpecialCliqueCutProducer(v, k, CC_FRACTIONAL_THRESHOLD);
}
