// cliquecutproducer.cpp
//
// By Sebastian Raaphorst, 2004.

#include <map>
#include <string>
#include <sstream>
#include <vector>
#include "common.h"
#include "cliquecutproducer.h"
#include "bac.h"
#include "column.h"
#include "graph.h"
#include "group.h"
#include "nibacexception.h"
#include "node.h"



const double CliqueCutProducer::CC_DEFAULT_FRACTIONAL_THRESHOLD = 0.15;
const double CliqueCutProducer::CC_DEFAULT_FRACTIONAL_PREFERENCE = 0.5;
const int    CliqueCutProducer::CC_DEFAULT_ENUMERATION_VALUE = 20;


CliqueCutProducer::CliqueCutProducer(double pCC_FRACTIONAL_THRESHOLD,
				     double pCC_FRACTIONAL_PREFERENCE,
				     int pCC_ENUMERATION_VALUE)
  : CC_FRACTIONAL_THRESHOLD(pCC_FRACTIONAL_THRESHOLD),
    CC_FRACTIONAL_PREFERENCE(pCC_FRACTIONAL_PREFERENCE),
    CC_ENUMERATION_VALUE(pCC_ENUMERATION_VALUE)
{
  if (pCC_FRACTIONAL_THRESHOLD < 0 || pCC_FRACTIONAL_THRESHOLD > 1)
    throw IllegalParameterException("CliqueCutProducer::FractionalThreshold",
				    pCC_FRACTIONAL_THRESHOLD,
				    "must be in range [0,1]");
  if (pCC_FRACTIONAL_PREFERENCE < 0 || pCC_FRACTIONAL_PREFERENCE > 1)
    throw IllegalParameterException("CliqueCutProducer::FractionalPreference",
				    pCC_FRACTIONAL_PREFERENCE,
				    "must be in range [0,1]");
  if (pCC_ENUMERATION_VALUE < 0)
    throw IllegalParameterException("CliqueCutProducer::EnumerationValue",
				    pCC_ENUMERATION_VALUE,
				    "must be nonnegative");
}


CliqueCutProducer::~CliqueCutProducer()
{
}


void CliqueCutProducer::generateCuts(BAC &bac, Node &node, double violationTolerance,
				     int &numberOfCuts, double &maximumViolation)
{
  numberOfCuts = 0;
  maximumViolation = 0;

  Group *group = node.getSymmetryGroup();

  // The first thing we need to do is create the graph. To do so, we need to
  // determine the number of fractional variables, say m, and create a bijective
  // mapping between them and Z_m.
  int numberVariables = node.getNumberBranchingVariables();
  int startIndex = node.getNumberFixedVariables() - node.getNumber0FixedVariables();
  int endBound = numberVariables - node.getNumber0FixedVariables();
  double *solutionVariableArray = node.getSolutionVariableArray();

  int baseElement;
  std::vector< int > fractionals;
  double upperBound = 1 - CC_FRACTIONAL_THRESHOLD;
  for (int i=startIndex; i < endBound; ++i) {
    baseElement = (group ? group->getBaseElement(i) : i);
    assert(baseElement >= 0 && baseElement < numberVariables);
    if (solutionVariableArray[baseElement] >= CC_FRACTIONAL_THRESHOLD && solutionVariableArray[baseElement] <= upperBound)
      fractionals.push_back(baseElement);
  }
  int numberFractionals = fractionals.size();

  // Now we have determined the number of fractional variables, so
  // we create the bijection.
  int *indexToVariable = new int[numberFractionals];

  std::vector< int >::iterator beginIter = fractionals.begin();
  std::vector< int >::iterator endIter   = fractionals.end();
  int k=0;
  for (std::vector< int >::iterator iter = fractionals.begin();
       iter != fractionals.end();
       ++iter, ++k) {
    assert(*iter >= 0 && *iter < numberVariables);
    assert(k >= 0 && k < numberFractionals);
    indexToVariable[k] = *iter;
  }

  // We now create the graph. This involves investigating columns of
  // the incidence matrix and seeing if they intersect. Intersecting
  // columns correspond to edges in our graph.
  Graph graph(numberFractionals);
  const std::vector< Column > &columns = bac.getFormulation().getColumns();

  for (int i=0; i < numberFractionals; ++i)
    for (int j=i+1; j < numberFractionals; ++j) {
      assert(i >= 0 && i < numberFractionals);
      assert(indexToVariable[i] >= 0 && indexToVariable[i] < numberVariables);
      assert(j >= 0 && j < numberFractionals);
      assert(indexToVariable[j] >= 0 && indexToVariable[j] < numberVariables);
      if (columns[indexToVariable[i]].intersects(columns[indexToVariable[j]]))
	graph.addEdge(i, j);
    }

#ifdef DEBUG
  std::cerr << "*** GRAPH FORMULATION ***" << std::endl;
  std::cerr << "LPrlxsol:";
  for (int i=0; i < numberVariables; ++i)
    std::cerr << " x" << i << "=" << solutionVariableArray[i];
  std::cerr << std::endl;
  std::cerr << "Fracs:";
  for (int i=0; i < numberFractionals; ++i)
    std::cerr << " " << indexToVariable[i] << "(" << i << ")";
  std::cerr << std::endl;
  graph.printGraph();
#endif

  // We now attempt to find the maximal cliques, using different techniques
  // depending on the value of numberFractionals.
  // Determine the components of the graph.
  graph.determineComponents();
  std::vector< std::vector < int > > &components = graph.getComponents();

  // Iterate over the components, collecting cliques
  std::vector< std::vector< int > > cliques;
  for (std::vector< std::vector< int > >::iterator iter = components.begin();
       iter != components.end();
       ++iter) {
    if ((*iter).size() <= CC_ENUMERATION_VALUE)
      determineMaximalCliques(graph, *iter, cliques);
    else
      cliqueHeuristic(graph, *iter, solutionVariableArray, numberVariables,
		      numberFractionals, indexToVariable, CC_FRACTIONAL_PREFERENCE, cliques);
#ifdef DEBUG
    std::cerr << "Called " << ((*iter).size() <= CC_ENUMERATION_VALUE ? "enumerator" : "heuristic")
	 << " and produced " << cliques.size() << " cliques." << std::endl;
#endif
  }

  // Now we test the cliques to see if they generate violated inequalities, and if they do,
  // we add them to the ILP.
  double value;
  double violation;
  Formulation &formulation = bac.getFormulation();
  for (std::vector< std::vector< int > >::iterator iter = cliques.begin();
       iter != cliques.end();
       ++iter) {
    value = 0;

    std::vector< int > positions;
    for (std::vector< int >::iterator iter2 = (*iter).begin();
	 iter2 != (*iter).end();
	 ++iter2) {
      assert(*iter2 >= 0 && *iter2 < numberFractionals);
      assert(indexToVariable[*iter2] >= 0 && indexToVariable[*iter2] < numberVariables);
      value += solutionVariableArray[indexToVariable[*iter2]];
      positions.push_back(indexToVariable[*iter2]);
    }
    violation = value - 1;
    if (isunviolated(violation, 0, 1+violationTolerance))
      continue;

    // Add a constaint to the node, as it is valid in the subtree rooted here.
    Constraint *constraint = Constraint::createConstraint(formulation, positions, LESSTHAN, 1);
    node.addCut(constraint);
    ++numberOfCuts;
    if (violation > maximumViolation)
      maximumViolation = violation;
  }
#ifdef DEBUG
  std::cerr << "Total number of inequalities in this pass: " << numberOfCuts << std::endl;
#endif

  delete[] indexToVariable;
}


void CliqueCutProducer::cliqueHeuristic(Graph &graph,
					std::vector< int > &vertices,
					double *solutionVariableArray,
					int numberVariables,
					int numberFractionals,
					int *indexToVariable,
					float priority,
					std::vector< std::vector< int > > &cliques)
{
  // We sort the vertices in vertices according to how close they are to priority.
  std::vector< int > sortedVertices;
  std::vector< double > values;

  // Create an array of priorities, indexed by vertex. We must first find the largest
  // vertex.
  int largest = -1;
  for (std::vector< int >::iterator iter = vertices.begin();
       iter != vertices.end();
       ++iter)
    if (*iter > largest)
      largest = *iter;
  assert(largest >= 0);

  // Calculate the priorities array
  int prioritiesSize = largest+1;
  double *priorities = new double[prioritiesSize];
  for (std::vector< int >::iterator iter = vertices.begin();
       iter != vertices.end();
       ++iter) {
    assert(*iter >= 0 && *iter < prioritiesSize);
    assert(*iter >= 0 && *iter < numberFractionals);
    assert(indexToVariable[*iter] >= 0 && indexToVariable[*iter] < numberVariables);
    priorities[*iter] = fabs(priority - solutionVariableArray[indexToVariable[*iter]]);
  }

  // Now we sort the vertices according to priority using a quicksort
  quicksort(vertices, priorities, 0, vertices.size()-1);

  // We want to mark the nodes, so create a boolean array to do so
  bool *markedNodes = new bool[prioritiesSize];
  for (int i=0; i < prioritiesSize; ++i)
    markedNodes[i] = false;

  // Now we iterate over the vertices, creating cliques and marking nodes as
  // we go. We continue until all relevant nodes are marked.
  for (std::vector< int >::iterator iter = vertices.begin();
       iter != vertices.end();
       ++iter) {
    // Check if this node is marked; if it is, we have found a clique containing
    // it and will gain nothing new from processing it.
    assert(*iter >= 0 && *iter < prioritiesSize);
    if (markedNodes[*iter])
      continue;

    // We begin searching for a new clique at this node.
    std::vector< int > clique;
    cliqueHeuristicAux(graph, *iter, priorities, clique);

    // Mark all the nodes in the clique
    for (std::vector< int >::iterator citer = clique.begin();
	 citer != clique.end();
	 ++citer) {
      assert(*citer >= 0 && *citer < prioritiesSize);
      markedNodes[*citer] = true;
    }

    // Add the clique to our list
    cliques.push_back(clique);
  }

  delete[] markedNodes;
  delete[] priorities;
}


void CliqueCutProducer::cliqueHeuristicAux(Graph &graph,
					   int startVertex,
					   double *priorities,
					   std::vector< int > &clique)
{
  // Create a list of candidates and sort by priority
  clique.push_back(startVertex);
  assert(startVertex >= 0 && startVertex < graph.getNumberVertices());
  std::set< int > &adjacencyList = (graph.getAdjacencyLists())[startVertex];

  // Instead of using a vector or anything of that sort, due to problems with erase,
  // we will instead use an array of candidates. In previous implementations, removing
  // things from the end of a vector would return an iterator to the thing that was
  // removed instead of to the end of the vector as expected and as indicated in the STL
  // documentation (an invalid iterator). This is not acceptable.
  int *candidates = new int[adjacencyList.size()];

  int k=0;
  for (std::set< int >::iterator iter = adjacencyList.begin();
       iter != adjacencyList.end();
       ++iter, ++k) {
    assert(k >= 0 && k < adjacencyList.size());
    candidates[k] = *iter;
  }

  // highPosition is used to indicate the highest position of a valid
  // entry in the candidates array. Originally, it is the last element in
  // the candidates array, as everything in candidates represents a valid
  // clique candidate.
  int highPosition = adjacencyList.size()-1;
  quicksort(candidates, priorities, 0, highPosition);

  // If there are no more candidates, then we are done.
  while (highPosition >= 0) {
    // Pick the next candidate and find the adjacency intersection
    int candidate = candidates[0];
    assert(candidate >= 0 && candidate < graph.getNumberVertices());
    std::set< int > &neighbours = (graph.getAdjacencyLists())[candidate];

    // Remove all the candidates that are not neighbours, modifying
    // the array.
    int subtractFactor = 1;
    for (int i=1; i <= highPosition; ++i) {
      // Check to see if the current candidate is a neighbour
      if (neighbours.find(candidates[i]) != neighbours.end()) {
	// It is, so copy this guy back and proceed.
	candidates[i-subtractFactor] = candidates[i];
	continue;
      }

      // It is not, so remove it.
      ++subtractFactor;
    }

    // We now have sub less elements in candidates.
    highPosition -= subtractFactor;

    // Add this candidate to the clique and call recursively
    clique.push_back(candidate);
  }

  delete[] candidates;
}


void CliqueCutProducer::determineMaximalCliques(Graph &graph,
						std::vector< int > &vertices,
						std::vector< std::vector< int > > &cliques)
{
  // Use a backtracking approach to do this, only ever adding vertices in strictly
  // increasing numbered order. At every point in the backtrack, we keep track of
  // the set of all neighbours of our vertices. When we can add no strictly larger
  // numbered vertex, we are done, and if the list of neighbours is empty, then
  // we have a maximal clique.

  // The first thing that we will do is create an adjacency matrix for the subgraph
  // over vertices; although this will take awhile to set up depending on edge density,
  // it will allow constant time lookup of edges.
  int numberVertices = vertices.size();
  int totalNumberVertices = graph.getNumberVertices();
  int **adjacencyMatrix = new int*[numberVertices];
  for (int i=0; i < numberVertices; ++i) {
    adjacencyMatrix[i] = new int[numberVertices];
    memset(adjacencyMatrix[i], 0, sizeof(int) * numberVertices);
  }

  // We also need arrays to convert between the vertices in vertices and the
  // indices in the adjacency matrix.
  int *vertexToMatrix = new int[totalNumberVertices];
  int *matrixToVertex = new int[numberVertices];
  for (int i=0; i < totalNumberVertices; ++i)
    vertexToMatrix[i] = -1;

  // Initialize the adjacency matrix.
  std::set< int > *adjacencyLists = graph.getAdjacencyLists();

  // We start by initializing the vertexToMatrix array.
  int k=0;
  for (std::vector< int >::iterator iter = vertices.begin();
       iter != vertices.end();
       ++iter, ++k) {
    int vertex = *iter;
    assert(vertex >= 0 && vertex < totalNumberVertices);
    vertexToMatrix[vertex] = k;
    assert(k >= 0 && k < numberVertices);
    matrixToVertex[k] = vertex;
  }

  // Now we are ready to create the adjacency matrix.
  std::set< int >::iterator beginIter2, endIter2;
  for (std::vector< int >::iterator iter = vertices.begin();
       iter != vertices.end();
       ++iter) {
    assert(*iter >= 0 && *iter < totalNumberVertices);
    int vertex = vertexToMatrix[*iter];
    std::set< int > &adjacencyList = adjacencyLists[*iter];

    for (std::set< int >::iterator aiter = adjacencyList.begin();
	 aiter != adjacencyList.end();
	 ++aiter)
      if (vertexToMatrix[*aiter] >= 0) {
	assert(vertex >= 0 && vertex < numberVertices);
	assert(vertexToMatrix[*aiter] >= 0 && vertexToMatrix[*aiter] < numberVertices);
	adjacencyMatrix[vertex][vertexToMatrix[*aiter]] = 1;
      }
  }

  // We now have the adjacency matrix, and we are ready to backtrack.
  // We need an array to represent the clique so far, and a two dimensional
  // array to hold the neighbours of all the elements in the clique so far.
  int bound = numberVertices+1;
  int **neighbours = new int*[bound];
  for (int i=0; i < bound; ++i)
    neighbours[i] = new int[numberVertices];
  int *neighbourCount = new int[bound];
  int *nextValidElement = new int[bound];
  int *clique = new int[numberVertices];
  int cliqueSize = 0;

  // We initialize the first entry in the neighbours array to hold all vertices,
  // since we may start with any vertex to generate a maximal clique.
  for (int i=0; i < numberVertices; ++i)
    neighbours[0][i] = i;
  neighbourCount[0] = numberVertices;
  nextValidElement[0] = 0;

  while (cliqueSize >= 0) {
    // If we have a clique that cannot be extended, we process it.
    assert(cliqueSize >= 0 && cliqueSize < bound);
    if (nextValidElement[cliqueSize] >= neighbourCount[cliqueSize]
	|| nextValidElement[cliqueSize] == -1) {
      // Check to see if this clique is maximal.
      if (!neighbourCount[cliqueSize]) {
	// It is, so we output it.
	std::vector< int > vclique;
	for (int i=0; i < cliqueSize; ++i) {
	  assert(clique[i] >= 0 && clique[i] <= numberVertices);
	  vclique.push_back(matrixToVertex[clique[i]]);
	}
	cliques.push_back(vclique);
      }

      // We have nothing more to do with this clique, so we backtrack.
      --cliqueSize;
      continue;
    }

    // We now have a clique that can be extended, so we extend it.
    assert(nextValidElement[cliqueSize] >= 0 && nextValidElement[cliqueSize] < numberVertices);
    assert(cliqueSize >= 0 && cliqueSize < numberVertices);
    clique[cliqueSize] = neighbours[cliqueSize][nextValidElement[cliqueSize]];

    // We now advance nextValidElement[cliqueSize] so that when we backtrack, we
    // will try the next possibility.
    ++nextValidElement[cliqueSize];

    // We now initialize the neighbours, neighbourCount, and nextValidElement for
    // the extension.
    int nextindex = cliqueSize+1;
    neighbourCount[nextindex] = 0;
    nextValidElement[nextindex] = -1;
    assert(neighbourCount[cliqueSize] <= numberVertices);
    for (int i=0; i < neighbourCount[cliqueSize]; ++i) {
      assert (clique[cliqueSize] >= 0 && clique[cliqueSize] < numberVertices);
      assert(neighbours[cliqueSize][i] >= 0 && neighbours[cliqueSize][i] < numberVertices);
      if (adjacencyMatrix[clique[cliqueSize]][neighbours[cliqueSize][i]]) {
	assert(nextindex >=0 && nextindex < bound);
	assert(neighbourCount[nextindex] >= 0 && neighbourCount[nextindex] < numberVertices);
	neighbours[nextindex][neighbourCount[nextindex]] = neighbours[cliqueSize][i];

	// Check to see if this is a valid element.
	if (nextValidElement[nextindex] == -1 && neighbours[cliqueSize][i] > clique[cliqueSize])
	  nextValidElement[nextindex] = neighbourCount[nextindex];

	++neighbourCount[nextindex];
      }
    }

    // We have now extended the clique and updated the neighbours, so we can loop.
    ++cliqueSize;
  }

  // Free up the memory structures.
  delete[] clique;
  delete[] nextValidElement;
  delete[] neighbourCount;
  for (int i=0; i < bound; ++i)
    delete[] neighbours[i];
  delete[] neighbours;
  delete[] vertexToMatrix;
  delete[] matrixToVertex;
  for (int i=0; i < numberVertices; ++i)
    delete[] adjacencyMatrix[i];
  delete[] adjacencyMatrix;
}



// *** CLIQUECUTPRODUCERCREATOR METHODS ***
CliqueCutProducerCreator::CliqueCutProducerCreator()
  : CC_FRACTIONAL_THRESHOLD(CliqueCutProducer::CC_DEFAULT_FRACTIONAL_THRESHOLD),
    CC_FRACTIONAL_PREFERENCE(CliqueCutProducer::CC_DEFAULT_FRACTIONAL_PREFERENCE),
    CC_ENUMERATION_VALUE(CliqueCutProducer::CC_DEFAULT_ENUMERATION_VALUE)
{
}


CliqueCutProducerCreator::~CliqueCutProducerCreator()
{
}


std::map< std::string, std::pair< std::string, std::string > > CliqueCutProducerCreator::getOptionsMap(void)
{
  std::map< std::string, std::pair< std::string, std::string > > optionsMap;

  std::ostringstream fractionalThresholdStream;
  fractionalThresholdStream << CliqueCutProducer::CC_DEFAULT_FRACTIONAL_THRESHOLD;
  optionsMap[std::string("T")] =
    std::pair< std::string, std::string >(std::string("Fractional threshold: a variable will be considered "
						      "for clique cuts if its value in the solution of the "
						      "LP relaxation exceeds this parameter."),
					  fractionalThresholdStream.str());

  std::ostringstream fractionalPreferenceStream;
  fractionalPreferenceStream << CliqueCutProducer::CC_DEFAULT_FRACTIONAL_PREFERENCE;
  optionsMap[std::string("P")] =
    std::pair< std::string, std::string >(std::string("Fractional preference: in constructing cliques, preference "
						      "is given to the variables closest to this value."),
					  fractionalPreferenceStream.str());

  std::ostringstream enumerationValueStream;
  enumerationValueStream << CliqueCutProducer::CC_DEFAULT_ENUMERATION_VALUE;
  optionsMap[std::string("E")] =
    std::pair< std::string, std::string >(std::string("Enumeration value: if a component is this size or smaller, "
						      "all cliques in the component are explicitly enumerated "
						      "instead of using a heuristic."),
					  enumerationValueStream.str());

  return optionsMap;
}


bool CliqueCutProducerCreator::processOptionsString(const char *options)
{
  char ch, eqls;
  double dvalue;
  int ivalue;

  // We must explicitly check for empty string prior to processing, since an empty string does
  // not generate an EOF status.
  if (strlen(options) == 0)
    return true;

  std::istringstream stream(options);
  while (!stream.eof()) {
    stream >> ch;
    if (!stream)
      throw IllegalParameterException("CliqueCutProducer::ConfigurationString",
				      options,
				      "could not process string");
    stream >> eqls;
    if (!stream || eqls != '=')
      throw IllegalParameterException("CliqueCutProducer::ConfigurationString",
				      options,
				      "could not process string");

    switch (ch) {
    case 'T':
      stream >> dvalue;
      if (!stream)
	throw IllegalParameterException("CliqueCutProducer::FractionalThreshold",
					"undefined",
					"could not interpret value in configuration string");
      setFractionalThreshold(dvalue);
      break;

    case 'P':
      stream >> dvalue;
      if (!stream)
	throw IllegalParameterException("CliqueCutProducer::FractionalPreference",
					"undefined",
					"could not interpret value in configuration string");
      setFractionalPreference(dvalue);
      break;

    case 'E':
      stream >> ivalue;
      if (!stream)
	throw IllegalParameterException("CliqueCutProducer::EnumerationValue",
					"undefined",
					"could not interpret value in configuration string");
      setEnumerationValue(ivalue);
      break;

    default:
      std::ostringstream outputstream;
      outputstream << ch;
      throw IllegalParameterException("CliqueCutProducer::ConfigurationString",
				      outputstream.str().c_str(),
				      "not a supported option");
    }

    if (!stream.eof()) {
      stream >> ch;
      if (!stream || ch != ':')
	throw IllegalParameterException("CliqueCutProducer::ConfigurationString",
					options,
					"could not process string");
    }
  }

  return true;
}


CutProducer *CliqueCutProducerCreator::create(void) const
{
  return new CliqueCutProducer(CC_FRACTIONAL_THRESHOLD,
			       CC_FRACTIONAL_PREFERENCE,
			       CC_ENUMERATION_VALUE);
}
