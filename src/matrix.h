/**
 * matrix.h
 *
 * By Sebastian Raaphorst, 2003 - 2018.
 */

#ifndef MATRIX_H
#define MATRIX_H

#include "common.h"

namespace vorpal::nibac {
    // Functions to create and destroy 2 and 3 dimensional matrices.
    template<class T>
    void matrix_2d(T ***, int, int);

    template<class T>
    void matrix_3d(T ****, int, int, int);

    template<class T>
    void matrix_free_2d(T ***, int, int);

    template<class T>
    void matrix_free_3d(T ****, int, int, int);
};
#endif
