#pragma once

/// @file
/// @ingroup macros
/// OpenMP helpers.

#include <guanaqo/preprocessor.h>
#include <guanaqo/stringify.h>

/// @def GUANAQO_OMP(X)
/// @ingroup macros
/// Emit the OpenMP pragma @p X if OpenMP is enabled.

/// @def GUANAQO_OMP_IF_ELSE(X, Y)
/// @ingroup macros
/// Emit @p X if OpenMP is enabled, otherwise emit @p Y.

/// @def GUANAQO_OMP_IF(X)
/// @ingroup macros
/// Emit @p X if OpenMP is enabled, otherwise emit nothing.

#if GUANAQO_WITH_OPENMP
#include <omp.h>
#define GUANAQO_OMP(X) _Pragma(GUANAQO_STRINGIFY(omp X))
#define GUANAQO_OMP_IF_ELSE(X, Y) X
#define GUANAQO_OMP_IF(X) X
#else
#define GUANAQO_OMP(X)
#define GUANAQO_OMP_IF_ELSE(X, Y) Y
#define GUANAQO_OMP_IF(X) GUANAQO_NOOP()
#endif
