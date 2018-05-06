// util.cpp
//
// By Sebastian Raaphorst, 2006.
//
// $Author$
// $Date$

#include <stdlib.h>
#include <string.h>
#include <set>
#include <sstream>
#include "common.h"
#include "util.h"


int parseFlagsFromString(int length, int *array, const char *str)
{
  int i;
  int first, last;
  char *copiedstring;
  char *firstptr, *lastptr;
  char tmpchar;

  assert(str);

  // Make a copy of the string.
  copiedstring = new char[strlen(str)+1];
  strcpy(copiedstring, str);
 
  bool correctflag = true;
  firstptr = lastptr = copiedstring;
  while (*firstptr) {
    // We want to parse up to a comma or end of string, and we have four possibilities.
    // 1. A single number, e.g. 1
    // 2. A range, e.g. 2-5
    // 3. A range open at the front, e.g. -4
    // 4. A range open at the end, e.g. 7-

    // Begin by assuming that this entry covers everything, and narrow it down.
    first = 0;
    last = length-1;

    // Starting with a comma is invalid.
    if (*firstptr == ',') {
      correctflag = false;
      break;
    }

    // Compute the front of the range if it exists.
    if (*firstptr != '-') {
      for (lastptr = firstptr; *lastptr && (*lastptr >= '0' && *lastptr <= '9'); ++lastptr);
      // We now have a number between firstptr and lastptr. Decipher this as first.
      tmpchar = *lastptr;
      *lastptr = 0;
      first = atoi(firstptr);
      if (first < 0 || first >= length) {
	correctflag = false;
	break;
      }
      *lastptr = tmpchar;
      firstptr = lastptr;
    }

    // Now, to be valid, firstptr must be either at the end of the string, a comma,
    // or a dash. Nothing else is acceptable here. If we have a comma or EOS, this is
    // easy.
    if (*firstptr == 0 || *firstptr == ',')
      last = first;
    else if (*firstptr == '-') {
      // Advance firstptr and parse through to the end of a possible numerical string.
      ++firstptr;
      for (lastptr = firstptr; *lastptr && (*lastptr >= '0' && *lastptr <= '9'); ++lastptr);

      // If there was no numerical string, then this was an open on the right interval.
      // We've already set last to the end, as expected. If not, we must decipher this
      // number as last.
      if (lastptr != firstptr) {
	tmpchar = *lastptr;
	*lastptr = 0;
	last = atoi(firstptr);
	if (last < 0 || last >= length) {
	  correctflag = false;
	  break;
	}
	*lastptr = tmpchar;
	firstptr = lastptr;
      }
    }
    else {
      // There was an illegal character here.
      correctflag = false;
      break;
    }

    // Do we have a valid range?
    if (first > last) {
      correctflag = false;
      break;
    }

    // We'd better be at the end of string or at a comma at this point.
    if (*firstptr) {
      if (*firstptr != ',') {
	correctflag = false;
	break;
      }
      ++firstptr;
    }

    // At this point, we have the range first-last that we want set in the array.
    for (i=first; i <= last; ++i)
      array[i] = TRUE;
  }

  delete[] copiedstring;
  return (correctflag ? TRUE : FALSE);
}



bool parseIntSetFromString(const char *str, std::set< int > &fset)
{
  if (strlen(str) == 0)
    return true;

  std::istringstream stream;
  stream.str(str);
  int variable;
  char ch;

  while (!stream.eof()) {
    stream >> variable;

    // If this failed, we have a good stream gone bad influenced by a string from the wrong side
    // of the tracks, so terminate.
    if (!stream)
      return false;

    // These entries need to be comma-separated.
    stream >> ch;
    if (!stream || ch != ',')
      return false;

    // Otherwise, we're good. Insert the variable.
    fset.insert(variable);
  }

  return true;
}
