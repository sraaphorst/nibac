// nibacexception.h
//
// By Sebastian Raaphorst, 2006.
//
// The superclass of all NIBAC exceptions and all the exceptions
// that are subclasses of this.
//
// $Author$
// $Date$

#ifndef NIBACEXCEPTION_H
#define NIBACEXCEPTION_H

#include <ostream>
#include <sstream>
#include <string>
#include <exception>
#include "common.h"


class NIBACException : public std::exception
{
public:
  NIBACException();
};


std::ostream &operator<<(std::ostream&, const NIBACException&);


class FileException : public NIBACException
{
protected:
  char *outstring;

protected:
  FileException(const char *filename, const char *desc) {
    std::ostringstream stream;
    stream << "File \"" << filename << "\" not available for " << desc << ".";
    const char *cstr = stream.str().c_str();
    outstring = new char[strlen(cstr)+1];
    strcpy(outstring, cstr);
  }

public:
  virtual ~FileException() throw () {
    delete[] outstring;
    outstring = 0;
  }
  
  virtual const char *what() const throw() {
    return outstring;
  }
};


class FileInputException : public FileException
{
public:
  FileInputException(const char *filename) : FileException(filename, "input") { }
};


class FileOutputException : public FileException
{
public:
  FileOutputException(const char *filename) : FileException(filename, "output") { }
};


class IllegalOperationException : public NIBACException
{
private:
  char *outstring;

public:
  IllegalOperationException(const char *description) {
    std::ostringstream stream;
    stream << "Illegal operation attempted: " << description;
    const char *cstr = stream.str().c_str();
    outstring = new char[strlen(cstr)+1];
    strcpy(outstring, cstr);
  }

  virtual ~IllegalOperationException() throw() {
    delete[] outstring;
    outstring = 0;
  }

  virtual const char *what() const throw() {
    return outstring;
  }
};


class IllegalParameterException : public NIBACException
{
private:
  char *outstring;

public:
  IllegalParameterException(const char *paramname, const char *paramvalue, const char *description = 0) {
    createOutputString(paramname, paramvalue, description);
  }

  IllegalParameterException(const char *paramname, const long paramvalue, const char *description = 0) {
    std::ostringstream stream;
    stream << paramvalue;
    createOutputString(paramname, stream.str().c_str(), description);
  }

  IllegalParameterException(const char *paramname, const int paramvalue, const char *description = 0) {
    std::ostringstream stream;
    stream << paramvalue;
    createOutputString(paramname, stream.str().c_str(), description);
  }

  IllegalParameterException(const char *paramname, const float paramvalue, const char *description = 0) {
    std::ostringstream stream;
    stream << paramvalue;
    createOutputString(paramname, stream.str().c_str(), description);
  }

  IllegalParameterException(const char *paramname, const double paramvalue, const char *description = 0) {
    std::ostringstream stream;
    stream << paramvalue;
    createOutputString(paramname, stream.str().c_str(), description);
  }

  virtual ~IllegalParameterException() throw() {
    delete[] outstring;
    outstring = 0;
  }

  virtual const char *what() const throw() {
    return outstring;
  }

private:
  void createOutputString(const char *paramname, const char *paramvalue, const char *description) {
    std::ostringstream stream;
    stream << "Illegal parameter specified (name: \"" << paramname << "\", value: \"" << paramvalue << "\")";
    if (description)
      stream << ": " << description;
    const char *cstr = stream.str().c_str();
    outstring = new char[strlen(cstr)+1];
    strcpy(outstring, cstr);
  }
};


class MissingDataException : public NIBACException
{
private:
  char *outstring;

public:
  MissingDataException(const char *description) {
    std::ostringstream stream;
    stream << "Mandatory data missing: " << description;
    const char *cstr = stream.str().c_str();
    outstring = new char[strlen(cstr)+1];
    strcpy(outstring, cstr);
  }

  virtual ~MissingDataException() throw() {
    delete[] outstring;
    outstring = 0;
  }

  virtual const char *what() const throw() {
    return outstring;
  }
};

  
class NoBranchingSchemeException : public NIBACException
{
public:
  virtual const char *what() const throw() {
    return "No branching scheme was selected.";
  }
};


class NoSolutionManagerException : public NIBACException
{
public:
  virtual const char *what() const throw() {
    return "No solution manager was selected.";
  }
};


class OutOfMemoryException : public NIBACException
{
public:
  virtual const char *what() const throw() {
    return "Out of memory.";
  }
};

class UnexpectedResultException : public NIBACException
{
private:
  char *outstring;

public:
  UnexpectedResultException(const char *description) {
    std::ostringstream stream;
    stream << "Unexpected result: " << description;
    const char *cstr = stream.str().c_str();
    outstring = new char[strlen(cstr)+1];
    strcpy(outstring, cstr);
  }

  virtual ~UnexpectedResultException() throw() {
    delete[] outstring;
    outstring = 0;
  }

  virtual const char *what() const throw() {
    return outstring;
  }
};

#endif
