/**
 * randomvariableorder.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include <stdlib.h>
#include <map>
#include <sstream>
#include <string>
#include "common.h"
#include "nibacexception.h"
#include "randomvariableorder.h"
#include "variableorder.h"

namespace vorpal::nibac {
// Static initializer.
    void RandomVariableOrder::initializeRNG(void) {
        // Seed using the time since 1970.
        srandom(time(0));
    }


    RandomVariableOrder::RandomVariableOrder(int pnumberVariables)
            : numberVariables(pnumberVariables) {
        indexList = new int[numberVariables];
        variableList = new int[numberVariables];

        // We use a multimap to establish the initial random order.
        std::multimap<long, int> order;
        for (int i = 0; i < numberVariables; ++i)
            order.insert(std::pair<const long, int>(random(), i));

        // Now iterate over the map and pick the elements off one by one.
        int pos = 0;
        for (std::multimap<long, int>::iterator iter = order.begin();
             iter != order.end();
             ++iter, ++pos) {
            variableList[pos] = (*iter).second;
            indexList[(*iter).second] = pos;
        }
    }


    RandomVariableOrder::~RandomVariableOrder() {
        delete[] variableList;
        variableList = 0;

        delete[] indexList;
        indexList = 0;
    }


    void RandomVariableOrder::sort(int len, const int *src, int *dst) {
        // Insert the elements into a multimap by index and simply read it back.
        std::multimap<int, int> sorted;
        for (int i = 0; i < len; ++i)
            sorted.insert(std::pair<const int, int>(indexList[src[i]], src[i]));

        int pos = 0;
        for (std::multimap<int, int>::iterator iter = sorted.begin();
             iter != sorted.end();
             ++iter, ++pos)
            dst[pos] = (*iter).second;
    }


    RandomVariableOrderCreator::RandomVariableOrderCreator()
            : numberVariables(-1) {
    }


    std::map <std::string, std::pair<std::string, std::string>> RandomVariableOrderCreator::getOptionsMap(void) {
        std::map <std::string, std::pair<std::string, std::string>> optionsMap;
        return optionsMap;
    }


    bool RandomVariableOrderCreator::processOptionsString(const char *options) {
        char ch, eqls;

        // We must explicitly check for empty string prior to processing, since an empty string does
        // not generate an EOF status.
        if (strlen(options) == 0)
            return true;

        std::istringstream stream(options);
        while (!stream.eof()) {
            stream >> ch;
            if (stream.fail())
                throw IllegalParameterException("RandomVariableOrder::ConfigurationString",
                                                options,
                                                "could not process string");
            stream >> eqls;
            if (stream.fail() || eqls != '=')
                throw IllegalParameterException("RandomVariableOrder::ConfigurationString",
                                                options,
                                                "could not process string");

            // TODO: Again, what was I thinking here?!?
            switch (ch) {
                default:
                    std::ostringstream outputstream;
                    outputstream << ch;
                    throw IllegalParameterException("RandomVariableOrder::ConfigurationString",
                                                    outputstream.str().c_str(),
                                                    "not a supported option");
            }

            if (!stream.eof()) {
                stream >> ch;
                if (!stream || ch != ':')
                    throw IllegalParameterException("RandomVariableOrder::ConfigurationString",
                                                    options,
                                                    "could not process string");
            }
        }

        return true;
    }


    VariableOrder *RandomVariableOrderCreator::create(void) const {
        if (numberVariables <= 0)
            throw MissingDataException("RandomVariableOrderCreator requires numberVariables to be populated.");
        return new RandomVariableOrder(numberVariables);
    }
};