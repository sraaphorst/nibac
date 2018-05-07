/**
 * closestvaluebranchingscheme.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include <float.h>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include "common.h"
#include "closestvaluebranchingscheme.h"
#include "nibacexception.h"
#include "node.h"

namespace vorpal::nibac {
// Static declarations
    const double ClosestValueBranchingScheme::DEFAULT_VALUE = 0.5;


    ClosestValueBranchingScheme::ClosestValueBranchingScheme(int pv, const double pvalue)
            : RankedBranchingScheme(pv) {
        // Use the set method, to avoid having to throw IllegalParameterExceptions
        // in more than one place.
        setValue(pvalue);
    }


    ClosestValueBranchingScheme::~ClosestValueBranchingScheme() {
    }


    int ClosestValueBranchingScheme::chooseBranchingVariableIndex(Node &n) {
        int closestvariable = -1;
        double closestvalue = DBL_MAX;

        std::set<int> &freelist = n.getFreeVariables();
        std::set<int>::iterator beginIter = freelist.begin();
        std::set<int>::iterator endIter = freelist.end();
        double *values = n.getSolutionVariableArray();
        double newvalue;
        for (; beginIter != endIter; ++beginIter) {
            newvalue = fabs((values[*beginIter] - value));
            if (newvalue < closestvalue) {
                closestvariable = *beginIter;
                closestvalue = newvalue;
            }
        }

        return closestvariable;
    }


// *** CLOSESTVARIABLEBRANCHINGSCHEMECREATOR METHODS ***
    ClosestValueBranchingSchemeCreator::ClosestValueBranchingSchemeCreator()
            : numberVariables(-1),
              value(ClosestValueBranchingScheme::DEFAULT_VALUE) {
    }


    ClosestValueBranchingSchemeCreator::~ClosestValueBranchingSchemeCreator() {
    }


    std::map <std::string, std::pair<std::string, std::string>>
    ClosestValueBranchingSchemeCreator::getOptionsMap(void) {
        std::map <std::string, std::pair<std::string, std::string>> optionsMap;

        std::ostringstream valueStream;
        valueStream << ClosestValueBranchingScheme::DEFAULT_VALUE;
        optionsMap[std::string("V")] =
                std::pair<std::string, std::string>(
                        std::string("Value: the earliest variable in the selected variable order closest "
                                    "to this value will be the one used for branching."),
                        valueStream.str());

        return optionsMap;
    }


    bool ClosestValueBranchingSchemeCreator::processOptionsString(const char *options) {
        char ch, eqls;
        double dvalue;

        // We must explicitly check for empty string prior to processing, since an empty string does
        // not generate an EOF status.
        if (strlen(options) == 0)
            return true;

        std::istringstream stream(options);
        while (!stream.eof()) {
            stream >> ch;
            if (stream.fail())
                throw IllegalParameterException("ClosestValueBranchingScheme::ConfigurationString",
                                                options,
                                                "could not process string");
            stream >> eqls;
            if (stream.fail() || eqls != '=')
                throw IllegalParameterException("ClosestValueBranchingScheme::ConfigurationString",
                                                options,
                                                "could not process string");

            switch (ch) {
                case 'V':
                    stream >> dvalue;
                    if (stream.fail())
                        throw IllegalParameterException("ClosestValueBranchingScheme::Value",
                                                        "undefined",
                                                        "could not interpret value in configuration string");
                    setValue(dvalue);
                    break;

                default:
                    std::ostringstream outputstream;
                    outputstream << ch;
                    throw IllegalParameterException("ClosestValueBranchingScheme::ConfigurationString",
                                                    outputstream.str().c_str(),
                                                    "not a supported option");
            }

            if (!stream.eof()) {
                stream >> ch;
                if (stream.fail() || ch != ':')
                    throw IllegalParameterException("ClosestValueBranchingScheme::ConfigurationString",
                                                    options,
                                                    "could not process string");
            }
        }

        return true;
    }


    BranchingScheme *ClosestValueBranchingSchemeCreator::create(void) const {
        if (numberVariables <= 0)
            throw MissingDataException("ClosestValueBranchingSchemeCreator requires numberVariables to be populated.");
        return new ClosestValueBranchingScheme(numberVariables, value);
    }
};