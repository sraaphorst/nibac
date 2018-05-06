// nodestack.h
//
// By Sebastian Raaphorst, 2003.
//
// This class represents a stack used for storing nodes in a branch-and-cut
// tree. It allows us to efficiently explore a tree in a depth-first fashion
// while maintaining all parent nodes of the currently examined node. This
// is important in the generation of maximal solutions.

#ifndef NODESTACK_H
#define NODESTACK_H

// We use an STL vector to implement the stack, as it allows for
// constant time access in adding and removing elements from the
// end (e.g. push / pop operations), and, unlike the STL stack,
// it allows us to traverse the elements (which allows us to set
// flags in a child-parent fashion with respect to maximality).
#include <vector>
#include "common.h"
#include "branchingscheme.h"
#include "node.h"
#include "statistics.h"


class NodeStack
{
 private:
  BranchingScheme &branchingScheme;
  std::vector< Node* > stack;
  Statistics &statistics;

 public:
  NodeStack(BranchingScheme&, Node*, Statistics&);
  virtual ~NodeStack();

  // Prune the most recently visited node from the tree.
  void pruneTop();

  // We have no need to manually allow users to push Nodes onto the stack.
  // We can automatically generate children and manage them as needed.
  // This method will return null when there are no further nodes to process.
  Node *getNextNode();
};


#endif
