// cutproducer.h
//
// By Sebastian Raaphorst, 2004.
//
// This is the superclass of cut-generators, for instance, isomorphism
// cuts, clique inequalities, etc...
//
// $Author$
// $Date$

#ifndef CUTPRODUCER_H
#define CUTPRODUCER_H

#include <map>
#include <set>
#include <string>
#include <vector>
#include "common.h"
#include "bac.h"
#include "node.h"

class CutProducer
{
protected:
  // Constructor; uses defaults for algorithm specifics.
  CutProducer();

public:
  virtual ~CutProducer();

  // This is the only method that must be overridden in subclasses; it performs
  // the actual cut-generation. The node is passed in for any node-specific
  // information that may be required. Similarly, the BAC algorithm is given.
  // Cuts may be either added globally (to the cutpool) or locally (to the node)
  // via methods in Formulation and Node. The number of cuts and the maximum violation are
  // returned via the referenced parameters.
  virtual void generateCuts(BAC&, Node&, double, int&, double&) = 0;

protected:
  // These are convenience method sthat we provide. Many of the cuts we will produce
  // require sorting a list of things according to some numerical value associated
  // with each item. Given such a list, and a set of associated values (doubles) indexed by
  // the items, we use quicksort to sort the list. 0 and the size of the list-1 should be
  // passed in as the last two parameters (or whatever range of the vector you wish to sort).
  static void quicksort(std::vector< int >&, double*, int, int);
  static void quicksort(int*, double*, int, int);
};



// Class forward for CutProducerCreator.
class CommandLineProcessing;

// Basic class to create / destroy cut producers. This allows users
// to populate the possible options in an instance of this class at
// will and then request that the class be created. This is really
// only intended for use with CommandLineProcessing and is not
// necessary for alternative NIBAC use.
//
// How this works is that you create an instance of a Creator class
// for each cut producer you want to make available to the user, and
// register it with the command line processing. The command line
// processing is then performed, and then if this cut is selected,
// pass the corresponding options string to it to have it configure
// itself. Pending that, the user can finalize any configuration
// needed (e.g. number of variables), and then the command line
// processing finalize method is called to finish configuration.
// This will give a BACOptions / MargotBACOptions object completely
// ready for use with NIBAC, requiring little - if any - programmer
// intervention, and the command line params will be standardized
// from application to application. Of course, it is not required
// that this be used and one could opt to configure manually.
//
// The ID identifies the cut in the sequence of cuts. The user then
// selects it by invoking -c# "configstring", where # is the ID.
//
// If any data is missing from the class, a NIBAC MissingDataException
// should be thrown when create is called.
class CutProducerCreator
{
  // Friend declarations.
  friend class CommandLineProcessing;

private:
  // A flag to indicate whether or not this cut is turned on.
  // This allows programmers to turn on and off cuts prior to
  // initialization (i.e. default cuts) and following initialization
  // (i.e. cuts that cannot be used by the problem parameters).
  bool activeFlag;

protected:
  CutProducerCreator();
  virtual ~CutProducerCreator();

  virtual std::string getCutProducerName(void) = 0;
  virtual std::map< std::string, std::pair< std::string, std::string > > getOptionsMap(void) = 0;

  virtual bool processOptionsString(const char*) = 0;

  // Make create protected so that users do not accidentally call
  // this, which would result in memory leakage.
  virtual CutProducer *create(void) const = 0;

public:
  inline bool isActive(void) const { return activeFlag; }
  inline void setActive(bool pactiveFlag) { activeFlag = pactiveFlag; }
};

#endif
