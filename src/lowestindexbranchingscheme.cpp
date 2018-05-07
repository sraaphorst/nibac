/**
 * lowestindexbranchingscheme.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include <map>
#include <set>
#include <sstream>
#include <string>
#include "common.h"
#include "lowestindexbranchingscheme.h"
#include "nibacexception.h"
#include "node.h"

namespace vorpal::nibac {
    int LowestIndexBranchingScheme::getBranchingVariableIndex(Node &n) {
        return n.getLowestFreeVariableIndex();
    }

    std::map <std::string, std::pair<std::string, std::string>> LowestIndexBranchingSchemeCreator::getOptionsMap(void) {
        std::map <std::string, std::pair<std::string, std::string>> optionsMap;
        return optionsMap;
    }


    bool LowestIndexBranchingSchemeCreator::processOptionsString(const char *options) {
        char ch, eqls;

        // We must explicitly check for empty string prior to processing, since an empty string does
        // not generate an EOF status.
        if (strlen(options) == 0)
            return true;

        std::istringstream stream(options);
        while (!stream.eof()) {
            stream >> ch;
            if (stream.fail())
                throw IllegalParameterException("LowestIndexBranchingScheme::ConfigurationString",
                                                options,
                                                "could not process string");
            stream >> eqls;
            if (stream.fail() || eqls != '=')
                throw IllegalParameterException("LowestIndexBranchingScheme::ConfigurationString",
                                                options,
                                                "could not process string");

            // TODO: Again, what was I thinking here?!?
            switch (ch) {
                default:
                    std::ostringstream outputstream;
                    outputstream << ch;
                    throw IllegalParameterException("LowestIndexBranchingScheme::ConfigurationString",
                                                    outputstream.str().c_str(),
                                                    "not a supported option");
            }

            if (!stream.eof()) {
                stream >> ch;
                if (!stream || ch != ':')
                    throw IllegalParameterException("LowestIndexBranchingScheme::ConfigurationString",
                                                    options,
                                                    "could not process string");
            }
        }

        return true;
    }


    BranchingScheme *LowestIndexBranchingSchemeCreator::create(void) const {
        return new LowestIndexBranchingScheme();
    }
};