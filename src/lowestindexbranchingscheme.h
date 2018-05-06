// lowestindexbranchingscheme.h
//
// By Sebastian Raaphorst, 2006.
//
// An implementation of BranchingScheme that simply
// branches on the free variable of lowest index (with respect
// to some variable ordering). This ensures consistent behaviour
// compatible with 0-fixing.
//
// $Author$
// $Date$


#ifndef LOWESTINDEXBRANCHINGSCHEME_H
#define LOWESTINDEXBRANCHINGSCHEME_H

#include <map>
#include <string>
#include "common.h"
#include "branchingscheme.h"
#include "node.h"


class LowestIndexBranchingScheme : public BranchingScheme
{
public:
  LowestIndexBranchingScheme();
  virtual ~LowestIndexBranchingScheme();
  virtual int getBranchingVariableIndex(Node&);
};


// This is a way of creating branching schemes through CommandLineProcessing.
// If you do not wish to use CommandLineProcessing, this will be of no value to you.
class LowestIndexBranchingSchemeCreator : public BranchingSchemeCreator
{
public:
  LowestIndexBranchingSchemeCreator();
  virtual ~LowestIndexBranchingSchemeCreator();

protected:
  inline virtual std::string getBranchingSchemeName(void) {
    return std::string("Lowest index branching scheme");
  }
  virtual std::map< std::string, std::pair< std::string, std::string > > getOptionsMap(void);

  virtual bool processOptionsString(const char*);

  // Make create protected so that users do not accidentally call
  // this, which would result in memory leakage.
  virtual BranchingScheme *create(void) const;
};

#endif
