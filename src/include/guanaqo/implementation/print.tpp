#pragma once

#include <guanaqo/print.hpp>

#include <array>
#include <cassert>
#include <charconv>
#include <cmath>
#include <limits>
#include <ostream>
#include <string_view>

namespace guanaqo {

inline std::string_view float_to_str_vw_snprintf(auto &&print, auto &buf,
                                                 std::floating_point auto value,
                                                 int precision,
                                                 const char *fmt) {
    int n = print(buf.data(), buf.size(), fmt, precision, value);
    assert((size_t)n < buf.size());
    return {buf.data(), (size_t)n};
}

#if __cpp_lib_to_chars
template <std::floating_point F>
std::string_view
float_to_str_vw(auto &buf, F value,
                int precision = std::numeric_limits<F>::max_digits10) {
    auto begin = buf.data();
    if (!std::signbit(value) && !std::isnan(value))
        *begin++ = '+';
    auto [end, _] = std::to_chars(begin, buf.data() + buf.size(), value,
                                  std::chars_format::scientific, precision);
    return std::string_view{buf.data(), end};
}
#else
#pragma message("Using std::snprintf as a fallback to replace std::to_chars")

inline std::string_view
float_to_str_vw(auto &buf, double value,
                int precision = std::numeric_limits<double>::max_digits10) {
    return float_to_str_vw_snprintf(std::snprintf, buf, value, precision,
                                    "%+-#.*e");
}
inline std::string_view
float_to_str_vw(auto &buf, float value,
                int precision = std::numeric_limits<float>::max_digits10) {
    return float_to_str_vw(buf, static_cast<double>(value), precision);
}
inline std::string_view float_to_str_vw(
    auto &buf, long double value,
    int precision = std::numeric_limits<long double>::max_digits10) {
    return float_to_str_vw_snprintf(std::snprintf, buf, value, precision,
                                    "%+-#.*Le");
}
#endif

#ifdef GUANAQO_WITH_QUAD_PRECISION
std::string_view
float_to_str_vw(auto &buf, __float128 value,
                int precision = std::numeric_limits<__float128>::max_digits10) {
    return float_to_str_vw_snprintf(quadmath_snprintf, buf, value, precision,
                                    "%+-#.*Qe");
}
#endif

template <std::floating_point F>
std::string float_to_str(F value, int precision) {
    std::array<char, 64> buf;
    return std::string{float_to_str_vw(buf, value, precision)};
}

template <std::floating_point F>
void print_elem(auto &buf, F value, std::ostream &os) {
    os << float_to_str_vw(buf, value);
}

template <std::integral I>
void print_elem(auto &, I value, std::ostream &os) {
    os << value;
}

template <std::floating_point F>
void print_elem(auto &buf, std::complex<F> value, std::ostream &os) {
    os << float_to_str_vw(buf, value.real()) << " + "
       << float_to_str_vw(buf, value.imag()) << 'j';
}

namespace detail {

template <class T>
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
