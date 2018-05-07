/**
 * lowestindexbranchingscheme.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef LOWESTINDEXBRANCHINGSCHEME_H
#define LOWESTINDEXBRANCHINGSCHEME_H

#include <map>
#include <string>
#include "common.h"
#include "branchingscheme.h"
#include "node.h"

namespace vorpal::nibac {
    /**
     * An implementation of BranchingScheme that simply branches on the free variable of lowest index
     * (with respect to some variable ordering). This ensures consistent behaviour compatible with 0-fixing.
     */
    class LowestIndexBranchingScheme final : public BranchingScheme {
    public:
        LowestIndexBranchingScheme() = default;
        virtual ~LowestIndexBranchingScheme() = default;

        int getBranchingVariableIndex(Node &) override;
    };


    // This is a way of creating branching schemes through CommandLineProcessing.
    // If you do not wish to use CommandLineProcessing, this will be of no value to you.
    class LowestIndexBranchingSchemeCreator final : public BranchingSchemeCreator {
    public:
        LowestIndexBranchingSchemeCreator() = default;
        virtual ~LowestIndexBranchingSchemeCreator() = default;

    private:
        inline std::string getBranchingSchemeName(void) override {
            return std::string("Lowest index branching scheme");
        }

        std::map <std::string, std::pair<std::string, std::string>> getOptionsMap() override;

        bool processOptionsString(const char *) override;

        /**
         * Make create private so that users do not accidentally call this, which would
         * result in memory leakage.
         */
        BranchingScheme *create(void) const override;
    };
};
#endif
