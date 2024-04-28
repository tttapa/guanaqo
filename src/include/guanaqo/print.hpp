#pragma once

#include <guanaqo/export.h>
#include <guanaqo/float.hpp>
#include <guanaqo/mat-view.hpp>

#include <iosfwd>
#include <limits>
#include <string>
#include <string_view>

namespace guanaqo {

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
GUANAQO_EXPORT std::string
float_to_str(F value, int precision = std::numeric_limits<F>::max_digits10);

namespace detail {

template <class T>
GUANAQO_EXPORT std::ostream &
print_csv_impl(std::ostream &os, MatrixView<const T> M, PrintOpts opts = {});

template <class T>
GUANAQO_EXPORT std::ostream &print_matlab_impl(std::ostream &os,
                                               MatrixView<const T>,
                                               std::string_view end = ";\n");

template <class T>
GUANAQO_EXPORT std::ostream &
print_python_impl(std::ostream &os, MatrixView<const T>,
                  std::string_view end = "\n", bool squeeze = true);

} // namespace detail

} // namespace guanaqo