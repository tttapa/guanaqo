#pragma once

#include <guanaqo/blas/config.hpp>

#if GUANAQO_WITH_MKL
#include <mkl.h>
#else
#include <cblas.h>
#endif

#include <type_traits>

namespace guanaqo::blas {

#if defined(MKL_INT)
using blas_index_t = MKL_INT;
#elif defined(CBLAS_INT)
using blas_index_t = CBLAS_INT;
#else
using blas_index_t = int;
#endif

static_assert(std::is_same_v<index_t, blas_index_t>, "Unsupported index type");
static_assert(std::is_signed_v<index_t>);

} // namespace guanaqo::blas
