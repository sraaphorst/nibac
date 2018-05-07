/**
 * graph.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include <set>
#include <vector>
#include "common.h"
#include "graph.h"

namespace vorpal::nibac {
    Graph::Graph(int pnumberVertices)
            : numberVertices(pnumberVertices) {
        adjacencyLists = new std::set<int>[numberVertices];
    }


    Graph::~Graph() {
        delete[] adjacencyLists;
    }


    void Graph::addEdge(int v1, int v2) {
        assert(v1 >= 0 && v1 < numberVertices);
        adjacencyLists[v1].insert(v2);

        assert(v2 >= 0 && v2 < numberVertices);
        adjacencyLists[v2].insert(v1);
    }


    void Graph::determineComponents(void) {
        // Data structures to hold the stack representing the DFS,
        // and the next positon to try in adjacency lists.
        std::vector<int> stack;
        std::vector <std::set<int>::iterator> positions;

        // List indicating what we've visited.
        // We mark everything unvisited to begin.
        bool *visited = new bool[numberVertices];
        for (int i = 0; i < numberVertices; ++i)
            visited[i] = false;

        // Set indicating what's left over; used to start the
        // next component. We give hints to the insertion positionsition
        // to speed up the insert method.
        std::set<int> remain;
        for (int i = 0; i < numberVertices; ++i)
            remain.insert(remain.end(), i);

        // Some variable declarations that we do here for efficiency
        // to avoid doing them repeatedly in the loops.
        int vertex;
        std::set<int>::iterator iter;
        std::set<int>::iterator beginIter;
        std::set<int>::iterator endIter;
        bool flag;

        while (remain.size() > 0) {
            // Create a new component
            components.resize(components.size() + 1);
            std::vector<int> &component = components.back();

            // Set up the stack and list of positionsitions
            // This should be unnecessary, due to the DFS technique,
            // but we do it all the same to ensure correctness.
            stack.clear();
            positions.clear();

            // Get an element from the set and remove it.
            // This element will be the start of our new component.
            iter = remain.begin();
            vertex = *iter;
            assert(vertex >= 0 && vertex < numberVertices);
            remain.erase(iter);

            // Add to the stack and initialize.
            stack.push_back(vertex);
            positions.push_back(adjacencyLists[vertex].begin());
            component.push_back(vertex);
            visited[vertex] = true;

            while (stack.size() > 0) {
                // We are extending from the previous element, so get its
                // adjacency list.
                vertex = stack.back();
                assert(vertex >= 0 && vertex < numberVertices);
                std::set<int> &adjacencyList = adjacencyLists[vertex];
                beginIter = positions.back();
                endIter = adjacencyList.end();
                flag = false;

                // Start iterating on positions, determining if there is an
                // unvisited neighbour of this vertex.
                for (; beginIter != endIter; ++beginIter) {
                    assert(*beginIter >= 0 && *beginIter < numberVertices);
                    if (!visited[*beginIter]) {
                        // We have found a vertex with which to extend
                        flag = true;
                        break;
                    }
                }

                // If we could not extend, we backtrack
                if (!flag) {
                    stack.pop_back();
                    positions.pop_back();
                    continue;
                }

                // Extend
                vertex = *beginIter;
                assert(vertex >= 0 && vertex < numberVertices);
                remain.erase(vertex);
                stack.push_back(vertex);
                positions.push_back(adjacencyLists[vertex].begin());
                component.push_back(vertex);
                visited[vertex] = true;
            }
        }

#ifdef DEBUG
        for (int i=0; i < components.size(); ++i) {
          std::cerr << "Component " << i << ":";
          std::vector< int > &component = components[i];
          for (int j=0; j < component.size(); ++j)
            std::cerr << " " << component[j];
          std::cerr << std::endl;
        }
#endif
    }


#ifdef DEBUG
    void Graph::printGraph() const
    {
      std::set< int >::iterator beginIter;
      std::set< int >::iterator endIter;

      for (int i=0; i < numberVertices; ++i) {
        beginIter = adjacencyLists[i].begin();
        endIter   = adjacencyLists[i].end();
        std::cerr << i << ":";
        for (; beginIter != endIter; ++beginIter)
          std::cerr << " " << *beginIter;
        std::cerr << std::endl;
      }
    }
#endif
};