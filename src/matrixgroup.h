/**
 * matrixgroup.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef MATRIXGROUP_H
#define MATRIXGROUP_H

#include "common.h"
#include "schreiersimsgroup.h"

namespace vorpal::nibac {
    /**
     * A general symmetry group based on the possible permutations over an incidence matrix,
     * namely all row and column permutations.
     */
    class MatrixGroup final : public SchreierSimsGroup {
    public:
        MatrixGroup(int, int, int **, bool, bool, int * = 0);
        virtual ~MatrixGroup() = default;
    };
};
#endif
