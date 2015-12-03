// statistics.cpp
//
// By Sebastian Raaphorst, 2004.

#include <ostream>
#include <vector>
#include <map>
#include "common.h"
#include "timer.h"
#include "statistics.h"


Statistics::Statistics()
  : numberCanonicityCalls(0),
    numberCanonicityRejections(0),
    nonCanonicalMaximumDepth(0),
    numberNodesExplored(0),
    numberStackBacktracks(0),
    numberLPsSolved(0),
    treeDepth(0)
{
}


Statistics::~Statistics()
{
}


void Statistics::setNumberCutProducers(unsigned int p)
{
  numberCuts.resize(p);
  for (std::vector< unsigned long >::iterator iter = numberCuts.begin();
       iter != numberCuts.end();
       ++iter)
    *iter = 0;
}


std::ostream &operator<<(std::ostream &out, Statistics &statistics)
{
  std::map< int, int >::iterator beginIter, endIter;
  int count;

  out << "Problem formulation time: " << statistics.getFormulationTimer() << std::endl;
  out << "Symmetry group calculation time: " << statistics.getSymmetryGroupTimer() << std::endl;
  out << "Processed " << statistics.getNumberNodesExplored() << " nodes (tree depth " << statistics.getTreeDepth() << ")." << std::endl;
  out << "Total branch-and-cut time: " << statistics.getTotalTimer() << std::endl;
#ifdef NODEGROUPS
  out << "\tTotal time spent copying groups: " << statistics.getGroupCopyTimer() << std::endl;
#endif
  out << "\tTotal time spent solving LPs: " << statistics.getLPSolverTimer() << std::endl;
  out << "\tTotal time spent in separation: " << statistics.getSeparationTimer() << std::endl;
  out << "\tTotal time spent in Margot's algorithms: " << statistics.getMargotTimer() << std::endl;
  out << "Number of calls to canonicity tester: " << statistics.getNumberCanonicityCalls() << std::endl;
  out << "\tNumber of non-canonical rejections: " << statistics.getNumberCanonicityRejections() << std::endl;
  out << "\tHighest depth of non-canonical rejection: " << statistics.getNonCanonicalMaximumDepth() << std::endl;
  out << "Number of LPs solved: " << statistics.getNumberLPsSolved() << std::endl;
  out << "Number of stack backtracks: " << statistics.getNumberStackBacktracks() << std::endl;

  out << "Number of nodes by depth:";
  std::map< int, int > &nodesbydepth = statistics.getNodeCountByDepth();
  beginIter = nodesbydepth.begin();
  endIter   = nodesbydepth.end();
  for (count=0; beginIter != endIter; ++beginIter, ++count) {
    if (count % 5 == 0)
      out << std::endl;
    out << '\t' << (*beginIter).first << ": " << (*beginIter).second;
  }
  out << std::endl;

  out << "Number of fixings by depth:";
  std::map< int, int > & fixingsbydepth = statistics.getVariableFixingCountByDepth();
  beginIter = fixingsbydepth.begin();
  endIter   = fixingsbydepth.end();
  for (count=0; beginIter != endIter; ++beginIter, ++count) {
    if (count % 5 == 0)
      out << std::endl;
    out << '\t' << (*beginIter).first << ": " << (*beginIter).second;
  }
  out << std::endl;

  std::vector< unsigned long > &cutstatistics = statistics.getNumberCuts();
  if (cutstatistics.size() > 0) {
    out << "Cuts:";
    std::vector< unsigned long >::iterator vbeginIter = cutstatistics.begin();
    std::vector< unsigned long >::iterator vendIter   = cutstatistics.end();
    for (; vbeginIter != vendIter; ++vbeginIter)
      out << " " << *vbeginIter;
  }

  return out;
}
