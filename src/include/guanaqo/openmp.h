#pragma once

#include <guanaqo/stringify.h>

#if GUANAQO_WITH_OPENMP
#include <omp.h>
#define GUANAQO_OMP(X) _Pragma(GUANAQO_STRINGIFY(omp X))
#define GUANAQO_OMP_IF_ELSE(X, Y) X
#define GUANAQO_OMP_IF(X) X
#else
#define GUANAQO_OMP(X)
#define GUANAQO_OMP_IF_ELSE(X, Y) Y
#define GUANAQO_OMP_IF(X)                                                      \
    do {                                                                       \
    } while (0)
#endif
