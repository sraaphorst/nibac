/**
 * isomorphismcutproducer.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include <string>
#include <map>
#include <sstream>
#include <vector>
#include "common.h"
#include "isomorphismcutproducer.h"
#include "bac.h"
#include "cutproducer.h"
#include "group.h"
#include "isomorphismcut.h"
#include "lpsolver.h"
#include "margotbac.h"
#include "node.h"
#include "permutationpool.h"


namespace vorpal::nibac {
    const double IsomorphismCutProducer::IC_DEFAULT_MIN_FRACTIONAL_VALUE = 0.5;


    IsomorphismCutProducer::IsomorphismCutProducer(int pnumberVariables,
                                                   double pIC_MIN_FRACTIONAL_VALUE)
            : numberVariables(pnumberVariables),
              IC_MIN_FRACTIONAL_VALUE(pIC_MIN_FRACTIONAL_VALUE) {
        used = new bool[numberVariables];
        for (int i = 0; i < numberVariables; ++i)
            used[i] = false;
        remain = new int[numberVariables];
        pos = new int[numberVariables + 1];
        pos[0] = -1;
        selected = new int[numberVariables];
        hperms = new int *[numberVariables + 1];
        for (int i = 0; i <= numberVariables; ++i)
            hperms[i] = new int[numberVariables];
        for (int i = 0; i < numberVariables; ++i)
            hperms[0][i] = i;
        tmpperm = new int[numberVariables];;
        sum_x = new double[numberVariables];
    }


    IsomorphismCutProducer::~IsomorphismCutProducer() {
        delete[] sum_x;
        for (int i = 0; i <= numberVariables; ++i)
            delete[] hperms[i];
        delete[] tmpperm;
        delete[] hperms;
        delete[] selected;
        delete[] pos;
        delete[] remain;
        delete[] used;
    }


    void IsomorphismCutProducer::generateCuts(BAC &bac, Node &node, double violationTolerance,
                                              int &numberOfCuts, double &maximumViolation) {
        numberOfCuts = 0;
        maximumViolation = 0;

        // If the ancestors haven't been 0-fixed, we cannot generate iso cuts reliably.
        if (!node.ancestorsCanonical())
            return;

        Group *g = node.getSymmetryGroup();
        int *part_zero = ((MargotBAC &) bac).getPartZero();

        // Some variables we need for later on.
        IsomorphismCut *cuts = 0;
        IsomorphismCut *cut, *iter, *tmpcut;
        bool addflag;
        int *h;
        bool flag;
        int permelem, elem;
        int fixed_one = node.getNumberFixedVariables() - node.getNumber0FixedVariables();

        // Determine the elements that belong in remain. These are the elements of our
        // solution in F_1 and F with solutions greater than or equal to IC_MIN_FRACTIONAL_VALUE.
        int bound = node.getNumberBranchingVariables() - node.getNumber0FixedVariables();
        double *x = node.getSolutionVariableArray();
        int k = 0;
        for (int i = 0; i < bound; ++i)
            if (x[g->getBaseElement(i)] >= IC_MIN_FRACTIONAL_VALUE) {
                remain[k] = g->getBaseElement(i);
                ++k;
            }

        int index = 0;
        while (index >= 0) {
            // If we've already selected an element for this position, return it to the list of remaining elements.
            if (pos[index] >= 0)
                used[pos[index]] = false;

            // Get the next valid element.
            flag = true;
            bound = index + 1;
            //pos[index] = (pos[index] == -1 ? (index == 0 ? 0 : pos[index-1]+1) : pos[index]+1);
            for (++pos[index]; pos[index] < k; ++pos[index]) {
                // If it has already been used, skip it.
                if (used[pos[index]])
                    continue;

                permelem = remain[pos[index]];
                elem = hperms[index][permelem];
                sum_x[index] = x[permelem] + (index == 0 ? 0 : sum_x[index - 1]);
                selected[index] = permelem;

                // If the sum is not sufficient, we can immediately backtrack.
                if (isunviolated(sum_x[index], 0, index + violationTolerance))
                    break;

                // Check canonicity, and if not canonical, we make a cut if we can.
                if (g->getPosition(elem) >= part_zero[index]) {
                    cut = new IsomorphismCut(g->getBaseSetSize(), index + 1, selected, sum_x[index] - index);
                    addflag = true;

                    // Check for duplicates and containment.
                    for (iter = cuts; iter;) {
                        if (*cut == *iter) {
                            addflag = false;
                            delete cut;
                            break;
                        }

                        if (*cut < *iter) {
                            // We want to delete iter, as cut is a subcut.
                            tmpcut = iter;
                            iter = iter->deleteCut();
                            if (tmpcut == cuts)
                                cuts = iter;
                            delete tmpcut;
                            continue;
                        }

                        // These cuts are unrelated, so we simply loop.
                        iter = iter->getNext();
                    }

                    // Add the cut if we've deemed it appropriate to do so.
                    if (addflag) {
                        cut->setNext(cuts);
                        cuts = cut;
                    }

                    // As per Margot's algorithms, in this case, we do not extend (we want
                    // minimal cuts) and simply loop on the next possible elem of remain.
                    continue;
                }

                // If index+1 >= fixed_one, by Margot's algorithms, we're not going to extend,
                // so we can simply loop.
                if (bound >= fixed_one)
                    continue;

                // Attempt to get a permutation mapping base[index] to elem.
                h = g->getPermutation(g->getBaseElement(index), elem);
                if (!h)
                    continue;

                // We have found a way to extend our selected list which doesn't yet lead to
                // a minimal cut, so proceed.
                flag = false;
                break;
            }

            // If we couldn't extend, we backtrack.
            if (flag) {
                pos[index] = -1;
                --index;
                continue;
            }

            // We were able to extend properly.
            used[pos[index]] = true;
            ++index;
            g->invert(h, tmpperm);
            g->multiply(tmpperm, hperms[index - 1], hperms[index]);
            pos[index] = -1;
        }

        // We now have a list of cuts. We add them.
        // We now have a linked list of minimal cuts without repetition; iterate over the list, deleting nodes and
        // creating formulation cuts.
        Formulation &formulation = bac.getFormulation();

        for (iter = cuts; iter;) {
            Constraint *constraint = Constraint::createConstraint(formulation, iter->getNumberIndices(),
                                                                  iter->getIndices(), LT,
                                                                  iter->getNumberIndices() - 1);
            node.addCut(constraint);

            ++numberOfCuts;
            if (iter->getViolation() > maximumViolation)
                maximumViolation = iter->getViolation();

            // Now delete this cut.
            tmpcut = iter;
            iter = iter->deleteCut();
            delete tmpcut;
        }
    }


    IsomorphismCutProducerCreator::IsomorphismCutProducerCreator()
            : numberVariables(-1),
              IC_MIN_FRACTIONAL_VALUE(IsomorphismCutProducer::IC_DEFAULT_MIN_FRACTIONAL_VALUE) {
    }


    std::map <std::string, std::pair<std::string, std::string>> IsomorphismCutProducerCreator::getOptionsMap(void) {
        std::map <std::string, std::pair<std::string, std::string>> optionsMap;

        std::ostringstream minimumFractionalValueStream;
        minimumFractionalValueStream << IsomorphismCutProducer::IC_DEFAULT_MIN_FRACTIONAL_VALUE;
        optionsMap[std::string("M")] =
                std::pair<std::string, std::string>(
                        std::string("Minimum fractional value: only consider variables for isomorphism "
                                    "cuts if their value in the LP relaxation exceeds this."),
                        minimumFractionalValueStream.str());

        return optionsMap;
    }


    bool IsomorphismCutProducerCreator::processOptionsString(const char *options) {
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
                throw IllegalParameterException("IsomorphismCutProducer::ConfigurationString",
                                                options,
                                                "could not process string");
            stream >> eqls;
            if (stream.fail() || eqls != '=')
                throw IllegalParameterException("IsomorphismCutProducer::ConfigurationString",
                                                options,
                                                "could not process string");

            switch (ch) {
                case 'M':
                    stream >> dvalue;
                    if (stream.fail())
                        throw IllegalParameterException("IsomorphismCutProducer::MinimumFractionalValue",
                                                        "undefined",
                                                        "could not interpret value in configuration string");
                    setMinimumFractionalValue(dvalue);
                    break;

                default:
                    std::ostringstream outputstream;
                    outputstream << ch;
                    throw IllegalParameterException("IsomorphismCutProducer::ConfigurationString",
                                                    outputstream.str().c_str(),
                                                    "not a supported option");
            }

            if (!stream.eof()) {
                stream >> ch;
                if (stream.fail() || ch != ':')
                    throw IllegalParameterException("IsomorphismCutProducer::ConfigurationString",
                                                    options,
                                                    "could not process string");
            }
        }

        return true;
    }


    CutProducer *IsomorphismCutProducerCreator::create(void) const {
        if (numberVariables <= 0)
            throw MissingDataException("IsomorphismCutProducerCreator requires numberVariables to be populated.");
        return new IsomorphismCutProducer(numberVariables, IC_MIN_FRACTIONAL_VALUE);
    }
};