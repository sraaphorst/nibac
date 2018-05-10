/**
 * lexicographicvariableorder.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include <string.h>
#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include "common.h"
#include "lexicographicvariableorder.h"
#include "nibacexception.h"
#include "variableorder.h"


void LexicographicVariableOrder::sort(int len, const int *src, int *dst)
{
  memcpy(dst, src, len * sizeof(int));
  std::sort(dst, dst+len);
}

std::map< std::string, std::pair< std::string, std::string > > LexicographicVariableOrderCreator::getOptionsMap(void)
{
  std::map< std::string, std::pair< std::string, std::string > > optionsMap;
  return optionsMap;
}


bool LexicographicVariableOrderCreator::processOptionsString(const char *options)
{
  char ch, eqls;

  // We must explicitly check for empty string prior to processing, since an empty string does
  // not generate an EOF status.
  if (strlen(options) == 0)
    return true;

  std::istringstream stream(options);
  while (!stream.eof()) {
    stream >> ch;
    if (!stream)
      throw IllegalParameterException("LexicographicVariableOrder::ConfigurationString",
				      options,
				      "could not process string");
    stream >> eqls;
    if (!stream || eqls != '=')
      throw IllegalParameterException("LexicographicVariableOrder::ConfigurationString",
				      options,
				      "could not process string");

    switch (ch) {
    default:
      std::ostringstream outputstream;
      outputstream << ch;
      throw IllegalParameterException("LexicographicVariableOrder::ConfigurationString",
				      outputstream.str().c_str(),
				      "not a supported option");
    }

    if (!stream.eof()) {
      stream >> ch;
      if (!stream || ch != ':')
	throw IllegalParameterException("LexicographicVariableOrder::ConfigurationString",
					options,
					"could not process string");
    }
  }

  return true;
}


VariableOrder *LexicographicVariableOrderCreator::create(void) const
{
  return new LexicographicVariableOrder();
}
