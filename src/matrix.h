// matrix.h
//
// By Sebastian Raaphorst, 2004.
//
// Functions to create and destroy 2 and 3 dimensional matrices.

#ifndef MATRIX_H
#define MATRIX_H

#include "common.h"

template <class T> void matrix_2d(T***, int, int);
template <class T> void matrix_3d(T****, int, int, int);

template <class T> void matrix_free_2d(T***, int, int);
template <class T> void matrix_free_3d(T****, int, int, int);

#endif
