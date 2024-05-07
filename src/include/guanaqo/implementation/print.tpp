#pragma once

#include <guanaqo/print.hpp>

#include <array>
#include <cassert>
#include <cmath>
#include <ostream>
#include <span>
#include <string_view>

namespace guanaqo {

template <std::floating_point F>
std::string float_to_str(F value, int precision) {
    std::array<char, 64> buf;
    return std::string{float_to_str_vw(buf, value, precision)};
}

template <std::floating_point F>
void print_elem(std::span<char> buf, F value, std::ostream &os) {
    os << float_to_str_vw(buf, value);
}

template <std::integral I>
void print_elem(auto &, I value, std::ostream &os) {
    os << value;
}

template <std::floating_point F>
void print_elem(std::span<char> buf, std::complex<F> value, std::ostream &os) {
    os << float_to_str_vw(buf, value.real()) << " + "
       << float_to_str_vw(buf, value.imag()) << 'j';
}

namespace detail {

template <class T> // NOLINTNEXTLINE(*-cognitive-complexity)
std::ostream &print_csv_impl(std::ostream &os, MatrixView<const T> M,
                             PrintOpts opts) {
    using index_t = decltype(M)::index_t;
    auto indent   = std::string(opts.indent, opts.indent_char);
    std::array<char, 64> buf;
    if ((M.cols == 1 && opts.column_vector_as_1d) ||
        (M.rows == 1 && opts.row_vector_as_1d)) {
        os << indent << opts.start;
        for (index_t r = 0; r < M.rows; ++r) {
            for (index_t c = 0; c < M.cols; ++c) {
                print_elem(buf, M(r, c), os);
                if (r != M.rows - 1 || c != M.cols - 1)
                    os << opts.delimiter;
            }
        }
        return os << opts.end;
    } else {
        for (index_t r = 0; r < M.rows; ++r) {
            os << indent << (r == 0 ? opts.start : opts.line_start);
            for (index_t c = 0; c < M.cols; ++c) {
                print_elem(buf, M(r, c), os);
                if (c != M.cols - 1)
                    os << opts.delimiter;
            }
            if (r != M.rows - 1)
                os << opts.line_end;
        }
        return os << opts.end;
    }
}

template <class T>
std::ostream &print_matlab_impl(std::ostream &os, MatrixView<const T> M,
                                std::string_view end) {
    auto opts = [&] {
        if (M.cols == 1)
            return PrintOpts{
                .delimiter = "; ",
                .start     = "[",
                .end       = "]",
            };
        else
            return PrintOpts{
                .delimiter  = " ",
                .line_start = " ",
                .line_end   = ";\n",
                .start      = "[",
                .end        = "]",
            };
    }();
    return print_csv_impl<T>(os, M, opts) << end;
}

template <class T>
std::ostream &print_python_impl(std::ostream &os, MatrixView<const T> M,
                                std::string_view end, bool squeeze) {
    auto opts = [&] {
        if ((M.cols == 1 && squeeze) || (M.rows == 1 && squeeze))
            return PrintOpts{
                .delimiter = ", ",
                .start     = "[",
                .end       = "]",
            };
        else
            return PrintOpts{
                .delimiter           = ", ",
                .line_start          = " [",
                .line_end            = "],\n",
                .start               = "[[",
                .end                 = "]]",
                .column_vector_as_1d = false,
                .row_vector_as_1d    = false,
            };
    }();
    return print_csv_impl<T>(os, M, opts) << end;
}

} // namespace detail

} // namespace guanaqo
