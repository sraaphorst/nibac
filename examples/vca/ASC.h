// ASC.h
//
// Abstract simplicial complex.
//
// By Sebastian Raaphorst, 2010.

#ifndef SR_ASC_H
#define SR_ASC_H

#include <set>
#include <vector>
#include <istream>
#include "common.h"
using std::set;
using std::vector;
using std::istream;

namespace sr
{
typedef set< int > Edge;
typedef vector< Edge > EdgeList;

class ASC
{
private:
	int numPoints;
	EdgeList facets;
	vector< EdgeList > facetsByPoint;

public:
	// Read an ASC from an istream with the following format:
	// numpoints
	// e^0_0 e^0_1 ... e^0_{n_0-1}
	// e^1_0 e^1_1 ... e^1_{n_1-1}
	// ...
	// e^{m-1}_0 e^{m-1}_1 ... e^{m-1}_{n_{m-1}-1}
	// where for all i in [0,m), e_i = {e^i_0, ..., e^i_{n_i-1}} is an edge.
	ASC(istream&);

	// Empty ASC over the specified number of points.
	ASC(int);

	// Copy constructor.
	ASC(const ASC&);

	virtual ~ASC();

	// Add an edge to the ASC. If it is a facet, we remove all
	// subedges. If it is a subedge of a facet, ignore it.
	// NOTE: we do not provide removeEdge, as due to the fact
	// that we only maintain a set of facets, this could result
	// in unexpected behaviour (e.g. add 123, add 1234, remove 1234
	// will result in no edge 123 as it would have been dropped in
	// the addition of 1234).
	void addEdge(const Edge&);

	// Number of points.
	unsigned int getNumPoints(void) const { return numPoints; }

	// Const iterators to traverse the edge list.
	EdgeList::const_iterator begin() const { return facets.begin(); }
	EdgeList::const_iterator end() const { return facets.end(); }
	const EdgeList &getFacets() const { return facets; }

	// Const vector to edgelist.
	const EdgeList &operator[](int idx) const { return facetsByPoint[idx]; }

	// Create explicitly a list of all the edges in the ASC.
	// As only the facets are explicitly stored, this can be a fairly
	// costly operation, as we have to iterate over all facets and create
	// all of the subfacets explicitly.
	EdgeList createAllEdges() const;

	// Given a list of edges, create an index vector into the list.
	// The structure V returned consists of a vector of length numPoints, where
	// V[i] is a vector containing the indices of all edges in edgeList that
	// contain the vertex i.
	vector< vector< int > > createEdgeIndex(const EdgeList&) const;
};
}

#endif // SR_ASC_H
