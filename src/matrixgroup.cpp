/**
 * matrixgroup.cpp
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#include "common.h"
#include "schreiersimsgroup.h"
#include "permutationpool.h"
#include "matrixgroup.h"

namespace vorpal::nibac {
    MatrixGroup::MatrixGroup(int rows, int cols, int **table, bool includerowperms, bool includecolperms, int *pbase)
            : SchreierSimsGroup(pbase) {
        int *perm = pool->newPermutation();
        getIdentityPermutation(perm);

        if (includerowperms) {
            // Construct all the row permutations; these are generated by permutations of the form:
            // (x_0,0 x_i,0)(x_0,1 x_i,1)...(x_0,cols-1 x_i,cols-1)
            for (int i = 1; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    perm[table[0][j]] = table[i][j];
                    perm[table[i][j]] = table[0][j];
                }
                enter(perm);

                // Reset the i row.
                for (int j = 0; j < cols; ++j)
                    perm[table[i][j]] = table[i][j];
            }

            // Reset the 0 row.
            for (int j = 0; j < cols; ++j)
                perm[table[0][j]] = table[0][j];
        }

        if (includecolperms) {
            // Construct all the column permutations; these are generated by permutations of the form:
            // (x_0,0 x_0,i)(x_1,0 x_1,i)...(x_rows-1,0 x_rows-1,i)
            for (int i = 1; i < cols; ++i) {
                for (int j = 0; j < rows; ++j) {
                    perm[table[j][0]] = table[j][i];
                    perm[table[j][i]] = table[j][0];
                }
                enter(perm);

                // Reset the i column.
                for (int j = 0; j < rows; ++j)
                    perm[table[j][i]] = table[j][i];
            }
        }

        // Release the permutation.
        pool->freePermutation(perm);
    }
};