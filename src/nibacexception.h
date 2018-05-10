/**
 * nibacexception.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef NIBACEXCEPTION_H
#define NIBACEXCEPTION_H

#include <ostream>
#include <sstream>
#include <string>
#include <exception>
#include "common.h"

namespace vorpal::nibac {
    class NIBACException : public std::exception {
    public:
        NIBACException() = default;
    };

    std::ostream &operator<<(std::ostream &, const NIBACException &);

    class DetailedException : public NIBACException {
    private:
        const std::string details;

    protected:
        DetailedException(const std::string pdetails)
                : details(pdetails) {
        }

    public:
        const char *what() const noexcept final {
            return details.c_str();
        }
    };

    class FileException : public DetailedException {
    protected:
        FileException(const char *filename, const char *desc)
                : DetailedException(createDescription(filename, desc)) {
        }

    private:
        static const std::string createDescription(const char *filename, const char *desc) {
            std::ostringstream stream;
            stream << "File \"" << filename << "\" not available for " << desc << ".";
            return stream.str();
        }
    };


    class FileInputException final : public FileException {
    public:
        FileInputException(const char *filename) : FileException(filename, "input") {}
    };


    class FileOutputException final : public FileException {
    public:
        FileOutputException(const char *filename) : FileException(filename, "output") {}
    };


    class IllegalOperationException : public DetailedException {
    public:
        IllegalOperationException(const char *description)
                : DetailedException(createDescription(description)) {
        }

    private:
        static const std::string createDescription(const char *description) {
            std::ostringstream stream;
            stream << "Illegal operation attempted: " << description;
            return stream.str();
        }
    };


    template<typename T>
    class IllegalParameterException final : public DetailedException {
    public:
        IllegalParameterException(const char *paramname, T &&paramvalue, const char *description = nullptr)
                : DetailedException(createDescription(paramname, paramvalue, description)) {
        }

    private:
        template<typename T>
        static const std::string createDescription(const char *paramname, T &&paramvalue, const char *description) {
            std::ostringstream stream;
            stream << "Illegal parameter specified (name: \"" << paramname << "\", value: \"" << paramvalue << "\")";
            if (description)
                stream << ": " << description;
            return stream.str();
        }
    };


    class MissingDataException : public DetailedException {
    public:
        MissingDataException(const char *description)
                : DetailedException(createDescription(description)) {
        }

    private:
        static const std::string createDescription(const char *description) {
            std::ostringstream stream;
            stream << "Mandatory data missing: " << description;
            return stream.str();
        }
    };


    class NoBranchingSchemeException : public DetailedException {
    public:
        NoBranchingSchemeException()
                : DetailedException("No branching scheme was selected.") {
        };
    };


    class NoSolutionManagerException : public DetailedException {
    public:
        NoSolutionManagerException()
                : DetailedException("No solution manager was selected.") {
        }
    };


    class OutOfMemoryException : public DetailedException {
    public:
        OutOfMemoryException()
                : DetailedException("Out of memory") {
        }
    };

    class UnexpectedResultException : public DetailedException {
    public:
        UnexpectedResultException(const char *description)
                : DetailedException(createDescription(description)) {
        }

    private:
        static const std::string createDescription(const char *description) {
            std::ostringstream stream;
            stream << "Unexpected result: " << description;
            return stream.str();
        }
    };
};
#endif
