/**
 * lpsolver.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include "common.h"
#include "node.h"
#include "lpsolver.h"

// Initialize the static instance to null
LPSolver *LPSolver::instance = 0;


LPSolver::LPSolver()
{
  instance = this;
}

