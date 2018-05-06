// matrix.cpp
//
// By Sebastian Raaphorst, 2004.

#include "common.h"
#include "matrix.h"


template <class T>
void matrix_2d(T ***matrix, int n, int m)
{
  (*matrix) = new T*[n];
  for (int i=0; i < n; ++i)
    (*matrix)[i] = new T[m];
}

template <class T>
void matrix_3d(T ****matrix, int n, int m, int p)
{
  (*matrix) = new T**[n];
  for (int i=0; i < n; ++i)
    matrix_2d(&((*matrix)[i]), m, p);
}

template <class T>
void matrix_free_2d(T ***matrix, int n, int m)
{
  for (int i=0; i < n; ++i)
    delete[] (*matrix)[i];
  delete[] (*matrix);
  *matrix = 0;
}

template <class T>
void matrix_free_3d(T ****matrix, int n, int m, int p)
{
  for (int i=0; i < n; ++i)
    matrix_free_2d(&((*matrix)[i]), m, p);
  delete[] (*matrix);
  *matrix = 0;
}
