// graph.h
//
// By Sebastian Raaphorst, 2004.
//
// Generic graph implementation with adjacency lists, used
// to determine cliques for clique inequalities.
//
// $Author$
// $Date$

#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <set>
#include "common.h"


class Graph
{
 private:
  int numberVertices;
  std::set< int > *adjacencyLists;
  std::vector< std::vector< int > > components;

 public:
  Graph(int);
  virtual ~Graph();

  inline int getNumberVertices(void) const { return numberVertices; }
  void addEdge(int, int);
  inline std::set< int > *getAdjacencyLists(void) { return adjacencyLists; }

  void determineComponents(void);
  inline std::vector< std::vector < int > > &getComponents(void) { return components; }

#ifdef DEBUG
  void printGraph() const;
#endif
};

#endif
