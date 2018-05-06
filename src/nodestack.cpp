// stack.cpp
//
// By Sebastian Raaphorst, 2003.

#include <vector>
#include "common.h"
#include "branchingscheme.h"
#include "lpsolver.h"
#include "node.h"
#include "statistics.h"
#include "nodestack.h"


NodeStack::NodeStack(BranchingScheme &pbranchingScheme, Node *root, Statistics &pstatistics)
  : branchingScheme(pbranchingScheme), statistics(pstatistics)
{
  stack.push_back(root);
}


NodeStack::~NodeStack()
{
  // We empty out the vector if it isn't already empty
  Node *n;
  while (!stack.empty()) {
    n = stack.back();
    stack.pop_back();
    delete n;
  }
}


void NodeStack::pruneTop()
{
  // We pop the top node from the stack as it has been reported as
  // being invalid - this may happen in a variety of cases, for instance,
  // if a node is not found to be canonical. By popping this node,
  // we avoid exploring the subtree rooted here.
#ifdef DEBUG
  assert(!stack.empty());
#endif
  Node *top = stack.back();
  stack.pop_back();
  delete top;
}


Node *NodeStack::getNextNode()
{
  // We use a sort of backtracking approach in order to get the next
  // available node from the stack.
  for (;;) {
    // If there are no more elements, we're done processing.
    if (stack.empty())
      return (Node*)0;

    // Get the node on the top of the stack
    Node *top = stack.back();

    // If this node has not been processed, then it is the one we
    // return.
    if (!top->processedFlag) {
      top->processedFlag = true;
      return top;
    }

    // Check if we have fully explored the subtree rooted at this node,
    // and if so, remove it from the tree and continue.
    if (top->nextBranchingVariableValue == -1) {
      stack.pop_back();
      delete top;
      statistics.reportBacktrack();
      continue;
    }

    // We will try to branch on this node. Check to see if we have already
    // determined the branching variable, and if not, find the free variable
    // of lowest index.
    if (top->branchingVariableIndex == -1) {
      // Get the new branching variable index.
      top->branchingVariableIndex = branchingScheme.getBranchingVariableIndex(*top);

      // We now no longer need solution information associated with the
      // node. We delete it to regain memory.
      top->cleanup();

      // If the branching method returned -1, we could not branch.
      // This will happen if the node is a leaf. It may also
      // happen in various other user-defined circumstances.
      // We pop it, set the parent flags to non-maximal, and proceed.
      if (top->branchingVariableIndex == -1) {
	stack.pop_back();
	std::vector< Node* >::reverse_iterator rbeginIter = stack.rbegin();
	std::vector< Node* >::reverse_iterator rendIter   = stack.rend();
	for (; rbeginIter != rendIter; ++rbeginIter)
	  (*rbeginIter)->possiblyMaximalFlag = false;
	delete top;
	statistics.reportBacktrack();
	continue;
      }
    }

    // At this point, we can branch, so we do so, creating a child node with
    // the next branching value. We then loop and allow child to be returned
    // on the next iteration of this loop. We also set the possibility of
    // maximality on the node.
    Node *child = new Node(top, top->branchingVariableIndex, top->nextBranchingVariableValue);
    child->possiblyMaximalFlag = (top->nextBranchingVariableValue ?
			      true :
			      top->possiblyMaximalFlag);
    --(top->nextBranchingVariableValue);
    stack.push_back(child);
  }
}
