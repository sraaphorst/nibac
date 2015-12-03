// commandlineprocessing.cpp
//
// By Sebastian Raaphorst, 2006.
//
// $Author$
// $Date$

#include <stdlib.h>
#include <map>
#include <string>
#include <vector>
#include "common.h"
#include "commandlineprocessing.h"
#include "bacoptions.h"
#include "branchingscheme.h"
#include "cutproducer.h"
#include "margotbacoptions.h"
#include "nibacexception.h"
#include "solutionmanager.h"
#include "util.h"
#include "variableorder.h"


// Static declarations
const int CommandLineProcessing::HELP = -1;


CommandLineProcessing::CommandLineProcessing(BACOptions &poptions)
  : options(poptions),
    solutionManagerCreator(0),
    defaultSolutionManagerCreator(0),
    solutionManager(0),
    variableOrderCreator(0),
    defaultVariableOrderCreator(0),
    variableOrder(0),
    branchingSchemeCreator(0),
    defaultBranchingSchemeCreator(0),
    branchingScheme(0)
{
}


CommandLineProcessing::~CommandLineProcessing()
{
  // Free all allocated memory.
  cutProducerCreatorDefaults.erase(cutProducerCreatorDefaults.begin(), cutProducerCreatorDefaults.end());
  cutProducerCreators.erase(cutProducerCreators.begin(), cutProducerCreators.end());
  while (!cutProducers.empty()) {
    CutProducer *cutProducer = cutProducers.back();
    cutProducers.pop_back();
    delete cutProducer;
  }

  defaultSolutionManagerCreator = 0;
  solutionManagerCreators.erase(solutionManagerCreators.begin(), solutionManagerCreators.end());
  if (solutionManager)
    delete solutionManager;
  solutionManager = 0;

  defaultVariableOrderCreator = 0;
  variableOrderCreators.erase(variableOrderCreators.begin(), variableOrderCreators.end());
  if (variableOrder)
    delete variableOrder;
  variableOrder = 0;

  defaultBranchingSchemeCreator = 0;
  branchingSchemeCreators.erase(branchingSchemeCreators.begin(), branchingSchemeCreators.end());
  if (branchingScheme)
    delete branchingScheme;
  branchingScheme = 0;
}


void CommandLineProcessing::registerCreator(CutProducerCreator &cutProducerCreator,
				     int id,
				     bool defaultFlag)
{
  if (id < 0)
    throw IllegalParameterException("CommandLineProcessing::registerCreator CutProducer",
				    id,
				    "ID must be nonnegative");

  if (cutProducerCreators.find(id) != cutProducerCreators.end())
    throw IllegalParameterException("CommandLineProcessing::registerCreator CutProducer",
				    id,
				    "ID already in use");

  cutProducerCreators[id] = &cutProducerCreator;
  cutProducerCreatorDefaults[&cutProducerCreator] = defaultFlag;
  cutProducerCreator.setActive(defaultFlag);
}


void CommandLineProcessing::registerCreator(SolutionManagerCreator &psolutionManagerCreator,
				     int id,
				     bool defaultFlag)
{
  if (id < 0)
    throw IllegalParameterException("CommandLineProcessing::registerCreator SolutionManager",
				    id,
				    "ID must be nonnegative");

  if (solutionManagerCreators.find(id) != solutionManagerCreators.end())
    throw IllegalParameterException("CommandLineProcessing::registerCreator SolutionManager",
				    id,
				    "ID already in use");

  if (defaultFlag && defaultSolutionManagerCreator)
    throw IllegalOperationException("CommandLineProcessing::registerSolutionManager can only register "
				    "one default solution manager");

  solutionManagerCreators[id] = &psolutionManagerCreator;
  if (defaultFlag) {
    defaultSolutionManagerCreatorID = id;
    defaultSolutionManagerCreator = &psolutionManagerCreator;
  }
}


void CommandLineProcessing::registerCreator(VariableOrderCreator &pvariableOrderCreator,
				     int id,
				     bool defaultFlag)
{
  if (id < 0)
    throw IllegalParameterException("CommandLineProcessing::registerCreator VariableOrder",
				    id,
				    "ID must be nonnegative");

  if (variableOrderCreators.find(id) != variableOrderCreators.end())
    throw IllegalParameterException("CommandLineProcessing::registerCreator VariableOrder",
				    id,
				    "ID already in use");

  if (defaultFlag && defaultVariableOrderCreator)
    throw IllegalOperationException("CommandLineProcessing::registerCreator VariableOrder can only register "
				    "one default variable order");

  variableOrderCreators[id] = &pvariableOrderCreator;
  if (defaultFlag) {
    defaultVariableOrderCreatorID = id;
    defaultVariableOrderCreator = &pvariableOrderCreator;
  }
}


void CommandLineProcessing::registerCreator(BranchingSchemeCreator &pbranchingSchemeCreator,
				     int id,
				     bool defaultFlag)
{
  if (id < 0)
    throw IllegalParameterException("CommandLineProcessing::registerCreator BranchingScheme",
				    id,
				    "ID must be nonnegative");

  if (branchingSchemeCreators.find(id) != branchingSchemeCreators.end())
    throw IllegalParameterException("CommandLineProcessing::registerCreator BranchingScheme",
				    id,
				    "ID already in use");

  if (defaultFlag && defaultBranchingSchemeCreator)
    throw IllegalOperationException("CommandLineProcessing::registerCreator BranchingScheme can only register "
				    "one default branching scheme");

  branchingSchemeCreators[id] = &pbranchingSchemeCreator;
  if (defaultFlag) {
    defaultBranchingSchemeCreatorID = id;
    defaultBranchingSchemeCreator = &pbranchingSchemeCreator;
  }
}


int CommandLineProcessing::populateBACOptions(int &argc, char **&argv)
{
  // Make sure at least one of each essential class is registered, and there is a default.
  if (solutionManagerCreators.size() == 0)
    throw IllegalOperationException("must have at least one solution manager registered");
  if (!defaultSolutionManagerCreator)
    throw IllegalOperationException("must have a default solution manager registered");
  if (variableOrderCreators.size() == 0)
    throw IllegalOperationException("must have at least one variable order registered");
  if (!defaultVariableOrderCreator)
    throw IllegalOperationException("must have a default variable order registered");
  if (branchingSchemeCreators.size() == 0)
    throw IllegalOperationException("must have at least one branching scheme registered");
  if (!defaultBranchingSchemeCreator)
    throw IllegalOperationException("must have a default branching scheme registered");

  // We iterate over the options, removing ones that correspond to settings in this class.
  int movebackindex = 0;

  for (int i=0; i < argc;) {
    // * HELP: -h *
    if (strcmp(argv[i], "-h") == 0) {
      // If the user requests help, immediately inform the calling code.
      // If the calling code desires, it can print the command line processing
      // options through outputOptions().
      return HELP;
    }

    // * DEPTH: -d # *
    if (strcmp(argv[i], "-d") == 0) {
      if (i == argc-1)
	throw IllegalParameterException("-d", "none", "-d requires a depth to be specified");
      int paramvalue = atoi(argv[i+1]);
      if (paramvalue < 0)
	throw IllegalParameterException("-d", paramvalue, "depth must be nonnegative");
      options.setDepth(paramvalue);
      movebackindex += 2;
      i += 2;
      continue;
    }

    // * MINNUMBEROFCUTS: -n # *
    if (strcmp(argv[i], "-n") == 0) {
      if (i == argc-1)
	throw IllegalParameterException("-n", "none", "-n requires a minimum number of cuts to be specified");
      int paramvalue = atoi(argv[i+1]);
      if (paramvalue < 0)
	throw IllegalParameterException("-n", paramvalue, "minimum number of cuts must be nonnegative");
      options.setMinimumNumberOfCuts(paramvalue);
      movebackindex += 2;
      i += 2;
      continue;
    }

    // * MINVIOLATIONL: -m # *
    if (strcmp(argv[i], "-m") == 0) {
      if (i == argc-1)
	throw IllegalParameterException("-m", "none", "-m requires a lower bound on violation to be specified");
      double paramvalue = atof(argv[i+1]);
      if (paramvalue < 0.0)
	throw IllegalParameterException("-m", paramvalue, "lower bound on violation must be nonnegative");
      options.setMinimumViolationL(paramvalue);
      movebackindex += 2;
      i += 2;
      continue;
    }

    // * MINVIOLATIONU: -m # *
    if (strcmp(argv[i], "-M") == 0) {
      if (i == argc-1)
	throw IllegalParameterException("-M", "none", "-M requires an upper bound on violation to be specified");
      double paramvalue = atof(argv[i+1]);
      if (paramvalue < 0.0)
	throw IllegalParameterException("-m", paramvalue, "upper bound on violation must be nonnegative");
      options.setMinimumViolationU(paramvalue);
      movebackindex += 2;
      i += 2;
      continue;
    }

    // * VIOLATIONTOLERANCEL: -v # *
    if (strcmp(argv[i], "-v") == 0) {
      if (i == argc-1)
	throw IllegalParameterException("-v", "none", "-v requires a lower bound on required violation to be specified");
      double paramvalue = atof(argv[i+1]);
      if (paramvalue < 0.0)
	throw IllegalParameterException("-v", paramvalue, "lower bound on required violation must be nonnegative");
      options.setViolationToleranceL(paramvalue);
      movebackindex += 2;
      i += 2;
      continue;
    }

    // * VIOLATIONTOLERANCEU: -V # *
    if (strcmp(argv[i], "-V") == 0) {
      if (i == argc-1)
	throw IllegalParameterException("-V", "none", "-V requires an upper bound on required violation to be specified");
      double paramvalue = atof(argv[i+1]);
      if (paramvalue < 0.0)
	throw IllegalParameterException("-V", paramvalue, "upper bound on required violation must be nonnegative");
      options.setViolationToleranceU(paramvalue);
      movebackindex += 2;
      i += 2;
      continue;
    }

    // * ACTIVITYTOLERANCE: -a # *
    if (strcmp(argv[i], "-a") == 0) {
      if (i == argc-1)
	throw IllegalParameterException("-a", "none", "-a requires an activity tolerance to be specified");
      double paramvalue = atof(argv[i+1]);
      if (paramvalue < 0.0)
	throw IllegalParameterException("-a", paramvalue, "activity tolerance must be nonnegative");
      options.setActivityTolerance(atof(argv[i+1]));
      movebackindex += 2;
      i += 2;
      continue;
    }

    // * LBOUND: -b # *
    if (strcmp(argv[i], "-b") == 0) {
      if (i == argc-1)
	throw IllegalParameterException("-b", "none", "-b requires a lower bound on objective function to be specified");
      options.setLowerBound(atoi(argv[i+1]));
      movebackindex += 2;
      i += 2;
      continue;
    }

    // * UBOUND: -B # *
    if (strcmp(argv[i], "-B") == 0) {
      if (i == argc-1)
	throw IllegalParameterException("-B", "none", "-B requires an upper bound on objective function to be specified");
      options.setUpperBound(atoi(argv[i+1]));
      movebackindex += 2;
      i += 2;
      continue;
    }

    // * KEEPCUTS: -k # *
    if (strcmp(argv[i], "-k") == 0) {
      if (i == argc-1)
	throw IllegalParameterException("-k", "none", "-k requires a 0/1 flag to be specified");
      int paramvalue = atoi(argv[i+1]);
      if (paramvalue < 0 || paramvalue > 1)
	throw IllegalParameterException("-k", paramvalue, "-k can only accept a 0/1 value");
      options.keepCuts(paramvalue == 1);
      movebackindex += 2;
      i += 2;
      continue;
    }

    // * EXPORTFILE: -e filename *
    if (strcmp(argv[i], "-e") == 0) {
      if (i == argc-1)
	throw IllegalParameterException("-e", "none", "-e requires a filename to be specified");
      options.setExportFileName(argv[i+1]);
      movebackindex += 2;
      i += 2;
      continue;
    }

    // * CUT PRODUCER STRING: -/+c#option=value:... *
    // This is only supported if there are cut producers.
    // Otherwise, allow this option to fall through.
    if (cutProducerCreators.size() > 0 && (argv[i][0] == '+' || argv[i][0] == '-') && argv[i][1] == 'c') {
      // Parse the number.
      int paramvalue;
      char *paramstring = argv[i]+2;
      std::istringstream stream(paramstring);
      stream >> paramvalue;
      if (!stream)
	if (argv[i][0] == '-')
	  throw IllegalParameterException("-c", "none", "-c must have the form -c#, where # specifies the cut type number");
	else
	  throw IllegalParameterException("+c", "none", "+c must have the form +c#option=value:option=value:..., where # "
					  "specifies the cut type number");

      std::map< int, CutProducerCreator* >::iterator iter = cutProducerCreators.find(paramvalue);
      if (iter == cutProducerCreators.end())
	if (argv[i][0] == '-')
	  throw IllegalParameterException("-c", paramvalue, "-c requires a valid cut type number");
	else
	  throw IllegalParameterException("+c", paramvalue, "+c requires a valid cut type number");

      // Find the start of the options string, if any.
      while (*paramstring && *paramstring >= '0' && *paramstring <= '9')
	++paramstring;

      // If there is a param string, and -c was specified, this is illegal; there can be no options
      // specified when one is removing a cut.
      if (*paramstring && argv[i][0] == '-')
	throw IllegalParameterException("-c", "none", "-c must have the form -c#, where # specifies the cut type number");

      CutProducerCreator *cutProducerCreator = (*iter).second;
      if (argv[i][0] == '+')
	cutProducerCreator->processOptionsString(paramstring);
      cutProducerCreator->setActive(argv[i][0] == '+');

      movebackindex += 1;
      i += 1;
      continue;
    }

    // * SOLUTION MANAGER: -S#option=value:... *
    // This is only supported if there are multiple solution managers.
    // Otherwise, allow this option to fall through.
    if (solutionManagerCreators.size() > 1 && argv[i][0] == '-' && argv[i][1] == 'S') {
      // If there is already a solution manager, we cannot create another.
      if (solutionManagerCreator)
	throw IllegalOperationException("-S can only be specified once");

      // Parse the number.
      int paramvalue;
      char *paramstring = argv[i]+2;
      std::istringstream stream(paramstring);
      stream >> paramvalue;
      if (!stream)
	throw IllegalParameterException("-S", "none", "-S must have the form -S#option=value:option=value:..., where # "
					"specifies the solution manager number");
      std::map< int, SolutionManagerCreator* >::iterator iter = solutionManagerCreators.find(paramvalue);
      if (iter == solutionManagerCreators.end())
	throw IllegalParameterException("-S", paramvalue, "-S requires a valid solution manager number");

      // Find the start of the options string, if any.
      while (*paramstring && *paramstring >= '0' && *paramstring <= '9')
	++paramstring;
      
      solutionManagerCreator = (*iter).second;
      solutionManagerCreator->processOptionsString(paramstring);

      movebackindex += 1;
      i += 1;
      continue;
    }

    // * VARIABLE ORDER: -o#option=value:... *
    // This is only supported if there are multiple variable orders.
    // Otherwise, allow this option to fall through.
    if (variableOrderCreators.size() > 1 && argv[i][0] == '-' && argv[i][1] == 'o') {
      // If there is already a variable order, we cannot create another.
      if (variableOrderCreator)
	throw IllegalOperationException("-o can only be specified once");

      // Parse the number.
      int paramvalue;
      char *paramstring = argv[i]+2;
      std::istringstream stream(paramstring);
      stream >> paramvalue;
      if (!stream)
	throw IllegalParameterException("-o", "none", "-o must have the form -o#option=value:option=value:..., where # "
					"specifies the variable order number");
      std::map< int, VariableOrderCreator* >::iterator iter = variableOrderCreators.find(paramvalue);
      if (iter == variableOrderCreators.end())
	throw IllegalParameterException("-o", paramvalue, "-o requires a valid variable order number");

      // Find the start of the options string, if any.
      while (*paramstring && *paramstring >= '0' && *paramstring <= '9')
	++paramstring;
      
      variableOrderCreator = (*iter).second;
      variableOrderCreator->processOptionsString(paramstring);

      movebackindex += 1;
      i += 1;
      continue;
    }

    // * BRANCHING SCHEME: -R#option=value:... *
    // This is only supported if there are multiple branching schemes.
    // Otherwise, allow this option to fall through.
    if (branchingSchemeCreators.size() > 1 && argv[i][0] == '-' && argv[i][1] == 'R') {
      // If there is already a branching scheme, we cannot create another.
      if (branchingSchemeCreator)
	throw IllegalOperationException("-R can only be specified once");

      // Parse the number.
      int paramvalue;
      char *paramstring = argv[i]+2;
      std::istringstream stream(paramstring);
      stream >> paramvalue;
      if (!stream)
	throw IllegalParameterException("-R", "none", "-R must have the form -s#option=value:option=value:..., where # "
					"specifies the branching scheme number");
      std::map< int, BranchingSchemeCreator* >::iterator iter = branchingSchemeCreators.find(paramvalue);
      if (iter == branchingSchemeCreators.end())
	throw IllegalParameterException("-R", paramvalue, "-R requires a valid branching scheme number");

      // Find the start of the options string, if any.
      while (*paramstring && *paramstring >= '0' && *paramstring <= '9')
	++paramstring;
      
      branchingSchemeCreator = (*iter).second;
      branchingSchemeCreator->processOptionsString(paramstring);

      movebackindex += 1;
      i += 1;
      continue;
    }

    // * 0-FIXINGS: -f string *
    if (strcmp(argv[i], "-f") == 0) {
      if (i == argc-1)
	throw IllegalParameterException("-f", "none", "-f requires a string list of 0-fixings to be specified");
      options.setManualFixings(true);
      if (!parseIntSetFromString(argv[i+1], options.getInitial0Fixings()))
	throw IllegalParameterException("-f", argv[i+1], "illegal string list of fixings");
      movebackindex += 2;
      i += 2;
      continue;
    }

    // * 1-FIXINGS: -F string *
    if (strcmp(argv[i], "-F") == 0) {
      if (i == argc-1)
	throw IllegalParameterException("-F", "none", "-F requires a string list of 1-fixings to be specified");
      options.setManualFixings(true);
      if (!parseIntSetFromString(argv[i+1], options.getInitial1Fixings()))
	throw IllegalParameterException("-F", argv[i+1], "illegal string list of fixings");
      movebackindex += 2;
      i += 2;
      continue;
    }

    // This does not correspond to an option, so we preserve it and shuffle it back as needed.
    argv[i-movebackindex] = argv[i];
    ++i;
  }

  // If any of the objects have not been chosen, set up the defaults.
  if (!solutionManagerCreator)
    solutionManagerCreator = defaultSolutionManagerCreator;
  if (!variableOrderCreator)
    variableOrderCreator = defaultVariableOrderCreator;
  if (!branchingSchemeCreator)
    branchingSchemeCreator = defaultBranchingSchemeCreator;

  // Modify argc to reflect what we've done.
  argc -= movebackindex;
  return TRUE;
}


void CommandLineProcessing::finishBACOptionsConfiguration(void)
{
  // We have processed all the command line arguments that we can. It's time to actually populate the
  // cutProducers vector and give them to BACOptions.
  for (std::map< int, CutProducerCreator* >::iterator iter = cutProducerCreators.begin();
       iter != cutProducerCreators.end();
       ++iter) {
    CutProducerCreator *cutProducerCreator = (*iter).second;
    if (cutProducerCreator->isActive()) {
      CutProducer *cutProducer = cutProducerCreator->create();
      cutProducers.push_back(cutProducer);
      options.addCutProducer(cutProducer);
    }
  }

  // Now create the objects we need that were chosen and set them up with the options.
  solutionManager = solutionManagerCreator->create();
  options.setSolutionManager(solutionManager);
  variableOrder = variableOrderCreator->create();
  options.setVariableOrder(variableOrder);
  branchingScheme = branchingSchemeCreator->create();
  options.setBranchingScheme(branchingScheme);
}


int CommandLineProcessing::populateMargotBACOptions(int &argc, char **&argv)
{
  MargotBACOptions &moptions = dynamic_cast< MargotBACOptions& >(options);

  // Call the method on BACOptions to populate the superclass information.
  int status = populateBACOptions(argc, argv);
  if (status == HELP || status == FALSE)
    return status;

  // We iterate over the options, removing ones that correspond to settings in this class.
  int movebackindex = 0;

  for (int i=0; i < argc;) {  
    // * CANONICITY DEPTHS: -C ranges *
    if (strcmp(argv[i], "-f") == 0) {
      if (i == argc-1)
	throw IllegalParameterException("-C", "none", "-C requires a string list of ranges of tree depths to test for canonicity");
      moptions.setCanonicityDepthFlagsString(argv[i+1]);
      movebackindex += 2;
      i += 2;
      continue;
    }

    // * ORBIT DEPTHS: -O ranges *
    if (strcmp(argv[i], "-O") == 0) {
      if (i == argc-1)
	throw IllegalParameterException("-O", "none", "-O requires a string list of ranges of tree depths to 0-fix");
      moptions.setOrbitDepthFlagsString(argv[i+1]);
      movebackindex += 2;
      i += 2;
      continue;
    }

    // * TEST FINAL SOLUTIONS: -t 0/1 *
    if (strcmp(argv[i], "-t") == 0) {
      if (i == argc-1)
	throw IllegalParameterException("-t", "none", "-t requires a 0/1 flag to be specified");
      int paramvalue = atoi(argv[i+1]);
      if (paramvalue < 0 || paramvalue > 1)
	throw IllegalParameterException("-t", paramvalue, "-t can only accept a 0/1 value");
      moptions.setTestFinalSolutions(paramvalue == 1);
      movebackindex += 2;
      i += 2;
      continue;
    }

    // * ORBIT THRESHOLD: -T # *
    if (strcmp(argv[i], "-T") == 0) {
      if (i == argc-1)
	throw IllegalParameterException("-T", "none", "-T requires an orbit threshold to be specified");
      int paramvalue = atoi(argv[i+1]);
      if (paramvalue < 0)
	throw IllegalParameterException("-T", paramvalue, "orbit threshold must be nonnegative");
      moptions.setOrbitThreshold(paramvalue);
      movebackindex += 2;
      i += 2;
      continue;
    }
      
    // This does not correspond to an option, so we preserve it and shuffle it back as needed.
    argv[i-movebackindex] = argv[i];
    ++i;
  }

  // Modify argc to reflect what we've done.
  argc -= movebackindex;
  return TRUE;
}


void CommandLineProcessing::finishMargotBACOptionsConfiguration(void)
{
  finishBACOptionsConfiguration();
}


void CommandLineProcessing::outputOptionsMap(std::ostream &out,
					     const std::map< std::string, std::pair< std::string, std::string > > &optionsMap)
{
  for (std::map< std::string, std::pair< std::string, std::string > >::const_iterator iter = optionsMap.begin();
       iter != optionsMap.end();
       ++iter) {
    std::string optionName = (*iter).first;
    std::string optionDescription = (*iter).second.first;
    std::string optionDefault = (*iter).second.second;
    out << "\t" << optionName << " \t " << optionDescription
	<< " (default: " << optionDefault << ")" << std::endl;
  }
}


void CommandLineProcessing::outputOptions(std::ostream &out)
{
  // Determine if this is a margot object.
  bool margotFlag;
  MargotBACOptions *moptions = dynamic_cast< MargotBACOptions* >(&options);
  margotFlag = (moptions != 0);

  out << "BRANCH-AND-BOUND OPTIONS" << std::endl;
  out << "-b # \t\t lower bound on objective value of an acceptable solution "
    "(default: " << BACOptions::getLowerBoundDefault() << ")" << std::endl;
  out << "-B # \t\t upper bound on objective value of an acceptable solution "
    "(default: " << BACOptions::getUpperBoundDefault() << ")" << std::endl;
  out << "-d # \t\t depth to switch from B&C library to pure ILP "
    "(default: " << BACOptions::getDepthDefault() << ")" << std::endl;
  out << "-f list \t\t override default 0-fixings with comma separated list of indices of "
    "variables to initially fix to 0" << std::endl;
  out << "-F list \t\t override default 1-fixings with comma separated list of indices of "
    "variables to initially fix to 1" << std::endl;

  if (solutionManagerCreators.size() > 1)
    out << "-S#option=value:option=value:... \t type of solution manager to use, see below "
      "(default: " << defaultSolutionManagerCreatorID << ")" << std::endl;
  
  if (variableOrderCreators.size() > 1)
    out << "-o#option=value:option=value... \t type of variable order to use, see below "
      "(default: " << defaultVariableOrderCreatorID << ")" << std::endl;
  
  if (branchingSchemeCreators.size() > 1)
    out << "-R#option=value:option=value:... \t type of branching scheme to use, see below "
      "(default: " << defaultBranchingSchemeCreatorID << ")" << std::endl;

  out << std::endl;


  out << "CUTTING PLANE OPTIONS" << std::endl;
  out << "-n #: \t minimum number of cuts required to be generated in one iteration of the cutting plane "
    "algorithm in order to continue with another iteration "
    "(default: " << BACOptions::getMinimumNumberOfCutsDefault() << ")" << std::endl;
  out << "-m #: \t lower bound on violation required in one iteration of the cutting plane "
    "algorithm in order to continue with another iteration "
    "(default: " << BACOptions::getMinimumViolationLDefault() << ")" << std::endl;
  out << "-M #: \t upper bound on violation required in one iteration of the cutting plane "
    "algorithm in order to continue with another iteration "
    "(default: " << BACOptions::getMinimumViolationUDefault() << ")" << std::endl;
  out << "-v #: \t lower bound required on violation of cut generated during the cutting plane to add "
    "them to the LP formulation "
    "(default: " <<  BACOptions::getViolationToleranceLDefault() << ")" << std::endl;
  out << "-V #: \t upper bound required on violation of cut generated during the cutting plane to add "
    "them to the LP formulation "
    "(default: " <<  BACOptions::getViolationToleranceUDefault() << ")" << std::endl;
  out << "-a #: \t activity tolerance for a cut to be considered active at a node "
    "(default: " << BACOptions::getActivityToleranceDefault() << ")" << std::endl;
  out << "-k 0/1: \t flag indicating whether or not inactive cuts should be readded when backtracking "
    "on nodes "
    "(default: " <<  (BACOptions::keepCutsDefault() ? '1' : '0') << ")" << std::endl;

  if (cutProducerCreators.size() > 0)
    out << "-c# / +c#option=value:option=value:... \t turn off (-) or on (+) a type of cut, "
      "see below" << std::endl;

  out << std::endl;


  if (margotFlag) {
    out << "ISOMORPHISM OPTIONS" << std::endl;
    out << "-t 0/1    \t\t flag indicating whether or not final solutions will always be tested "
      "for canonicity; if not, isomorphic solutions may be output "
      "(default: " << (MargotBACOptions::getTestFinalSolutionsDefault() ? '1' : '0') << ")" << std::endl;
    out << "-T #      \t\t terminate 0-fixing if orbits are smaller than the indicated number, "
      "where 0 indicates to never terminate "
      "(default: " << MargotBACOptions::getOrbitThresholdDefault() << ")" << std::endl;
    out << "-C ranges \t\t list of ranges of depths at which to test nodes for canonicity "
      "(default: always)" << std::endl;
    out << "-O ranges \t\t list of ranges of depths at which to perform 0-fixing calculations "
      "(default: always)" << std::endl;
    out << "where a list of ranges is specified as a comma-separated list of strings of the form:" << std::endl;
    out << "\ta   \t test at depth a" << std::endl;
    out << "\ta-b \t test at depths a, ..., b" << std::endl;
    out << "\t-b  \t test at depths 0, ..., b" << std::endl;
    out << "\ta-  \t test at depths a onwards" << std::endl;
    out << std::endl;
  }


  out << "GENERAL OPTIONS" << std::endl;
  out << "-e name: \t name of file to export the ILP formulation if supported by LP solver "
    "(default: none, i.e. do not export)" << std::endl;
  out << std::endl;


  // If there are multiple solution managers, provide a guide to them and their configuration.
  if (solutionManagerCreators.size() > 1) {
    out << "SOLUTION MANAGERS" << std::endl;
    for (std::map< int, SolutionManagerCreator* >::iterator iter = solutionManagerCreators.begin();
	 iter != solutionManagerCreators.end();
	 ++iter) {
      int id = (*iter).first;
      SolutionManagerCreator *psolutionManagerCreator = (*iter).second;

      out << id << ": " << psolutionManagerCreator->getSolutionManagerName();
      if (psolutionManagerCreator == defaultSolutionManagerCreator)
	out << " (default)";
      out << std::endl;
      const std::map< std::string, std::pair< std::string, std::string > > &optionsMap = psolutionManagerCreator->getOptionsMap();
      out << "Options: " << (optionsMap.empty() ? "None" : "") << std::endl;
      outputOptionsMap(out, optionsMap);
      out << std::endl;
    }
  }


  if (variableOrderCreators.size() > 1) {
    out << "VARIABLE ORDERS" << std::endl;
    for (std::map< int, VariableOrderCreator* >::iterator iter = variableOrderCreators.begin();
	 iter != variableOrderCreators.end();
	 ++iter) {
      int id = (*iter).first;
      VariableOrderCreator *pvariableOrderCreator = (*iter).second;

      out << id << ": " << pvariableOrderCreator->getVariableOrderName();
      if (pvariableOrderCreator == defaultVariableOrderCreator)
	out << " (default)";
      out << std::endl;
      const std::map< std::string, std::pair< std::string, std::string > > &optionsMap = pvariableOrderCreator->getOptionsMap();
      out << "Options: " << (optionsMap.empty() ? "None" : "") << std::endl;
      outputOptionsMap(out, optionsMap);
      out << std::endl;
    }
  }


  if (branchingSchemeCreators.size() > 1) {
    out << "BRANCHING SCHEMES" << std::endl;
    for (std::map< int, BranchingSchemeCreator* >::iterator iter = branchingSchemeCreators.begin();
	 iter != branchingSchemeCreators.end();
	 ++iter) {
      int id = (*iter).first;
      BranchingSchemeCreator *pbranchingSchemeCreator = (*iter).second;

      out << id << ": " << pbranchingSchemeCreator->getBranchingSchemeName();
      if (pbranchingSchemeCreator == defaultBranchingSchemeCreator)
	out << " (default)";
      out << std::endl;
      const std::map< std::string, std::pair< std::string, std::string > > &optionsMap = pbranchingSchemeCreator->getOptionsMap();
      out << "Options: " << (optionsMap.empty() ? "None" : "") << std::endl;
      outputOptionsMap(out, optionsMap);
      out << std::endl;
    }
  }


  if (cutProducerCreators.size() > 1) {
    out << "CUTS" << std::endl;
    for (std::map< int, CutProducerCreator* >::iterator iter = cutProducerCreators.begin();
	 iter != cutProducerCreators.end();
	 ++iter) {
      int id = (*iter).first;
      CutProducerCreator *cutProducerCreator = (*iter).second;

      std::map< CutProducerCreator*, bool >::iterator diter = cutProducerCreatorDefaults.find(cutProducerCreator);
      if (diter == cutProducerCreatorDefaults.end())
	throw UnexpectedResultException("default status of cut producer not found");	

      out << id << ": " << cutProducerCreator->getCutProducerName()
	  << ", default: " << ((*diter).second ? "on" : "off") << std::endl;
      const std::map< std::string, std::pair< std::string, std::string > > &optionsMap = cutProducerCreator->getOptionsMap();
      out << "Options: " << (optionsMap.empty() ? "None" : "") << std::endl;
      outputOptionsMap(out, optionsMap);
      out << std::endl;
    }
  }  
}
