// ASC.cpp
//
// By Sebastian Raaphorst, 2010.

#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>
#include "common.h"
#include "ASC.h"
using std::includes;
using std::insert_iterator;
using std::istream_iterator;
using std::istringstream;
using std::string;


sr::ASC::ASC(istream &istr)
{
	// Read the ASC from the istream. The first thing to read is the number of points.
	istr >> numPoints;
	facetsByPoint.resize(numPoints);

	while (istr) {
		// Read in the edge.
		Edge e;

		string nextline;
		getline(istr, nextline);
		istringstream sstr(nextline);
		copy(istream_iterator< int >(sstr), istream_iterator< int >(),
						insert_iterator< Edge >(e, e.begin()));

		// Print the edge.
		if (!(e.empty()))
			addEdge(e);
	}
}


sr::ASC::ASC(int np)
	: numPoints(np)
{
	facetsByPoint.resize(numPoints);
}


sr::ASC::ASC(const ASC &other)
	: numPoints(other.numPoints),
	  facets(other.facets),
	  facetsByPoint(other.facetsByPoint)
{
}


sr::ASC::~ASC()
{
}


void sr::ASC::addEdge(const Edge &e)
{
	// Determine if this edge is a subset of any existing edge, or vice versa.
	for (unsigned int i=0; i < facets.size();) {
		if (includes(facets[i].begin(), facets[i].end(), e.begin(), e.end()))
			// The edge at i is or contains e.
			return;
		if (includes(e.begin(), e.end(), facets[i].begin(), facets[i].end())) {
			// The ith facet is contained in e, so it is no longer a facet.
			// We must remove this facet from all the point lists containing it.
			for (Edge::iterator iter = facets[i].begin();
					iter != facets[i].end();
					++iter)
				facetsByPoint[*iter].erase(find(facetsByPoint[*iter].begin(),
												facetsByPoint[*iter].end(),
												facets[i]));
			// We must also remove it from the list of facets.
			facets.erase(facets.begin()+i);
		}
		else
			// The new edge and this facet are incomparable, so advance.
			++i;
	}

	// We are ready for the new facet.
	facets.push_back(e);
	for (sr::Edge::iterator iter = e.begin();
			iter != e.end();
			++iter)
		facetsByPoint[*iter].push_back(e);
}


sr::EdgeList sr::ASC::createAllEdges() const
{
	// We begin by creating a set so that everything is unique, but we
	// return a vector of edges, ultimately, as we will likely want to
	// take indices into it.
	std::set< sr::Edge > edgeSet;

	// Iterate over all the facets.
	for (sr::EdgeList::const_iterator iter = facets.begin(); iter != facets.end(); ++iter) {
		const sr::Edge &facet = *iter;

		// We convert to a vector, and then we take all integers between 1..2^{n-1}, where n
		// is the size of the list, with the integer representing the characteristic vector
		// of the subset.
		std::vector< int > facetVector(facet.begin(), facet.end());

		unsigned long maxvec = 1 << facetVector.size();
		for (unsigned long charvec = 1; charvec < maxvec; ++charvec) {
			std::set< int > edge;
			for (unsigned int idx=0; idx < facetVector.size(); ++idx)
				if ((1 << idx) & charvec)
					edge.insert(edge.end(), facetVector[idx]);
			edgeSet.insert(edge);
		}
	}

	// Now convert to a vector.
	return sr::EdgeList(edgeSet.begin(), edgeSet.end());
}


std::vector< std::vector< int > > sr::ASC::createEdgeIndex(const sr::EdgeList &edgeList) const
{
	std::vector< std::vector< int > > edgeIndex(numPoints);

	// Now iterate over the edges.
	for (unsigned int edgeIdx=0; edgeIdx < edgeList.size(); ++edgeIdx) {
		const sr::Edge &edge = edgeList[edgeIdx];

		// Iterate over the edge and add the edge index to all the lists of vertices
		// appearing in the edge.
		for (sr::Edge::const_iterator iter = edge.begin(); iter != edge.end(); ++iter)
			edgeIndex[*iter].push_back(edgeIdx);
	}

	return edgeIndex;
}



//#define ASC_H_TEST
#ifdef ASC_H_TEST
#include <iostream>
#include <iterator>
#include <sstream>
using std::istringstream;
using std::ostream_iterator;
using std::istream_iterator;
using std::insert_iterator;
using std::copy;
using std::cout;
using std::endl;
using sr::ASC;
using sr::Edge;
using sr::EdgeList;

int main(int argc, char *argv[])
{
	istringstream sstr(argv[1]);
	int numPoints;
	sstr >> numPoints;
	cout << "Number of points: " << numPoints << endl;
	ASC asc(numPoints);

	for (int i=2; i < argc; ++i) {
		// Read in the edge.
		Edge e;
		istringstream sstr(argv[i]);
		copy(istream_iterator< int >(sstr), istream_iterator< int >(),
						insert_iterator< Edge >(e, e.begin()));

		// Print the edge.
		cout << "Adding edge: ";
		copy(e.begin(), e.end(), ostream_iterator< int >(cout)); // (cout, " ")
		cout << endl;
		asc.addEdge(e);
	}

	// Print all the facets of the ASC.
	cout << endl << "*** Facets of ASC: ***" << endl;
	//BOOST_FOREACH(const Edge &e, std::make_pair(asc.begin(), asc.end())) {
	for (EdgeList::const_iterator iter = asc.begin(); iter != asc.end(); ++iter) {
		cout << "Facet: ";
		//copy(e.begin(), e.end(), ostream_iterator< int >(cout)); // (cout, " ")
		copy(iter->begin(), iter->end(), ostream_iterator< int >(cout)); // (cout, " ")
		cout << endl;
	}

	// Print the facets per point.
	for (unsigned int i=0; i < asc.getNumPoints(); ++i) {
		cout << "Point " << i << " (size " << asc[i].size() << "): ";
		//BOOST_FOREACH(const Edge &e, asc[i]) {
		for (EdgeList::const_iterator iter = asc[i].begin(); iter != asc[i].end(); ++iter) {
			//copy(e.begin(), e.end(), ostream_iterator< int >(cout)); // (cout, " ")
			copy(iter->begin(), iter->end(), ostream_iterator< int >(cout));
			cout << " ";
		}
		cout << endl;
	}

	// Now try to construct all of the edges of the ASC.
	EdgeList edgeList = asc.createAllEdges();
	cout << endl << "*** All Edges of ASC: ***" << endl;
	for (EdgeList::const_iterator iter = edgeList.begin(); iter != edgeList.end(); ++iter) {
		cout << "Edge: ";
		copy(iter->begin(), iter->end(), ostream_iterator< int >(cout));
		cout << endl;
	}

	// Now print the edges per point.
	vector< vector< int > > edgesPerPoint = asc.createEdgeIndex(edgeList);
	for (unsigned int i=0; i < edgesPerPoint.size(); ++i) {
		cout << "Point " << i << " (size " << edgesPerPoint[i].size() << "): ";
		for (vector< int >::const_iterator iter = edgesPerPoint[i].begin();
				iter != edgesPerPoint[i].end(); ++iter) {
			copy(edgeList[*iter].begin(), edgeList[*iter].end(), ostream_iterator< int >(cout));
			cout << " ";
		}
		cout << endl;
	}
}
#endif // ASC_H_TEST
