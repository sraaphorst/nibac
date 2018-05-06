// group.cpp
//
// By Sebastian Raaphorst, 2003.

#include <string.h>
#include "common.h"
#include "permutationpool.h"
#include "group.h"


// Static member declarations.
int Group::x = 0;
int Group::memsize = 0;
PermutationPool *Group::pool = 0;
int *Group::idperm = 0;


void Group::initialize(int basesetsize)
{
  x = basesetsize;
  memsize = x * sizeof(int);
  PermutationPool::createPool(x);
  pool = PermutationPool::getPool();
  idperm = pool->newPermutation();
  for (int i=0; i < x; ++i)
    idperm[i] = i;
}


void Group::destroy()
{
  pool->freePermutation(idperm);
  PermutationPool::deletePool();
}



Group::Group()
{
}


Group::~Group()
{
}


void Group::down(int r, int s)
{
}


int Group::getPosition(int var)
{
  return var;
}


int Group::getBaseElement(int pos)
{
  return pos;
}


void Group::invert(int *p, int *target)
{
  for (int i=0; i < x; ++i)
    target[p[i]] = i;
}
