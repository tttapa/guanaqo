#pragma once

#include <guanaqo/export.h>
#include <guanaqo/float.hpp>
#include <guanaqo/mat-view.hpp>

#include <cstdlib>
#include <iosfwd>
#include <limits>
#include <span>
#include <string>
#include <string_view>

namespace guanaqo {

template <class T>
using PrintMatrixView = MatrixView<const T, ptrdiff_t, ptrdiff_t>;

struct GUANAQO_EXPORT PrintOpts {
    int precision               = 0;
    std::string_view delimiter  = ",";
    std::string_view line_start = {};
    std::string_view line_end   = "\n";
    std::string_view start      = {};
    std::string_view end        = "\n";
    unsigned indent             = 0;
    char indent_char            = ' ';
    bool column_vector_as_1d    = true;
    bool row_vector_as_1d       = true;
};

template <std::floating_point F>
GUANAQO_EXPORT std::string_view
float_to_str_vw(std::span<char> buf, F value,
                int precision = std::numeric_limits<F>::max_digits10);

template <std::floating_point F>
GUANAQO_EXPORT std::string
float_to_str(F value, int precision = std::numeric_limits<F>::max_digits10);

namespace detail {

template <class T>
GUANAQO_EXPORT std::ostream &
print_csv_impl(std::ostream &os, PrintMatrixView<T> M, PrintOpts opts = {});

template <class T>
GUANAQO_EXPORT std::ostream &print_matlab_impl(std::ostream &os,
                                               PrintMatrixView<T>,
                                               std::string_view end = ";\n");

template <class T>
GUANAQO_EXPORT std::ostream &
print_python_impl(std::ostream &os, PrintMatrixView<T>,
                  std::string_view end = "\n", bool squeeze = true);

} // namespace detail

template <class T, std::size_t E>
std::ostream &print_csv(std::ostream &os, std::span<T, E> x,
                        PrintOpts opts = {}) {
    return guanaqo::detail::print_csv_impl(
        os,
        PrintMatrixView<T>{{
            .data = x.data(),
            .rows = static_cast<ptrdiff_t>(x.size()),
        }},
        opts);
}

template <class T, class I, class S>
std::ostream &print_csv(std::ostream &os, MatrixView<T, I, S> X,
                        PrintOpts opts = {}) {
    return guanaqo::detail::print_csv_impl(
        os,
        PrintMatrixView<T>{{
            .data         = X.data,
            .rows         = static_cast<ptrdiff_t>(X.rows),
            .cols         = static_cast<ptrdiff_t>(X.cols),
            .inner_stride = static_cast<ptrdiff_t>(X.inner_stride),
            .outer_stride = static_cast<ptrdiff_t>(X.outer_stride),
        }},
        opts);
}

template <class T, std::size_t E>
std::ostream &print_matlab(std::ostream &os, std::span<T, E> x,
                           std::string_view end = ";\n") {
    return guanaqo::detail::print_matlab_impl(
        os,
        PrintMatrixView<T>{{
            .data = x.data(),
            .rows = static_cast<ptrdiff_t>(x.size()),
        }},
        end);
}

template <class T, class I, class S>
std::ostream &print_matlab(std::ostream &os, MatrixView<T, I, S> X,
                           std::string_view end = ";\n") {
    return guanaqo::detail::print_matlab_impl(
        os,
        PrintMatrixView<T>{{
            .data         = X.data,
            .rows         = static_cast<ptrdiff_t>(X.rows),
            .cols         = static_cast<ptrdiff_t>(X.cols),
            .inner_stride = static_cast<ptrdiff_t>(X.inner_stride),
            .outer_stride = static_cast<ptrdiff_t>(X.outer_stride),
        }},
        end);
}

template <class T, std::size_t E>
std::ostream &print_python(std::ostream &os, std::span<T, E> x,
                           std::string_view end = "\n", bool squeeze = true) {
    return guanaqo::detail::print_python_impl(
        os,
        PrintMatrixView<T>{{
            .data = x.data(),
            .rows = static_cast<ptrdiff_t>(x.size()),
        }},
        end, squeeze);
}

template <class T, class I, class S>
std::ostream &print_python(std::ostream &os, MatrixView<T, I, S> X,
                           std::string_view end = "\n", bool squeeze = true) {
    return guanaqo::detail::print_python_impl(
        os,
        PrintMatrixView<T>{{
            .data         = X.data,
            .rows         = static_cast<ptrdiff_t>(X.rows),
            .cols         = static_cast<ptrdiff_t>(X.cols),
            .inner_stride = static_cast<ptrdiff_t>(X.inner_stride),
            .outer_stride = static_cast<ptrdiff_t>(X.outer_stride),
        }},
        end, squeeze);
}

} // namespace guanaqo
