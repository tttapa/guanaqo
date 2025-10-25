#pragma once

#include <guanaqo/export.h>
#include <guanaqo/mat-view.hpp>
#include <concepts>
#include <cstddef>
#include <iosfwd>
#include <span>
#include <stdexcept>
#include <vector>

namespace guanaqo::io {

struct GUANAQO_EXPORT csv_read_error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

template <class F>
    requires(std::floating_point<F> || std::integral<F>)
void GUANAQO_EXPORT csv_read_row(std::istream &is, std::span<F> v,
                                 char sep = ',');

template <class F, size_t E>
    requires(E != std::dynamic_extent)
void GUANAQO_EXPORT csv_read_row(std::istream &is, std::span<F, E> v,
                                 char sep = ',') {
    csv_read_row(is, std::span<F, std::dynamic_extent>{v}, sep);
}

template <class F>
    requires(std::floating_point<F> || std::integral<F>)
void GUANAQO_EXPORT csv_read(std::istream &is,
                             MatrixView<F, ptrdiff_t, ptrdiff_t> M,
                             char sep = ',');

template <class F, class I, class S>
    requires(!std::same_as<I, ptrdiff_t> || !std::same_as<S, ptrdiff_t>)
void GUANAQO_EXPORT csv_read(std::istream &is, MatrixView<F, I, S> M,
                             char sep = ',') {
    csv_read(is,
             MatrixView<F, ptrdiff_t, ptrdiff_t>{
                 {.data         = M.data,
                  .rows         = static_cast<ptrdiff_t>(M.rows),
                  .cols         = static_cast<ptrdiff_t>(M.cols),
                  .inner_stride = static_cast<ptrdiff_t>(M.inner_stride),
                  .outer_stride = static_cast<ptrdiff_t>(M.outer_stride)}},
             sep);
}

template <class F>
    requires(std::floating_point<F> || std::integral<F>)
std::vector<F> GUANAQO_EXPORT csv_read_row_std_vector(std::istream &is,
                                                      char sep = ',');

} // namespace guanaqo::io
