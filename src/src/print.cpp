#include <guanaqo/export.h>
#include <guanaqo/implementation/print.tpp>

#include <charconv>
#include <concepts>
#ifdef __has_include
#if __has_include(<stdfloat>)
#include <stdfloat>
#endif
#endif

namespace guanaqo {

namespace {

std::string_view float_to_str_vw_snprintf(auto &&print, auto &buf,
                                          std::floating_point auto value,
                                          int precision, const char *fmt) {
    int n = print(buf.data(), buf.size(), fmt, precision, value);
    assert((size_t)n < buf.size());
    return {buf.data(), (size_t)n};
}
#if __cpp_lib_to_chars
template <std::floating_point F>
#ifdef GUANAQO_WITH_QUAD_PRECISION
    requires(!std::same_as<F, __float128>)
#endif
std::string_view float_to_str_vw_impl(std::span<char> buf, F value,
                                      int precision) {
    auto begin = buf.data(), buf_end = begin + buf.size();
    if (!std::signbit(value) && !std::isnan(value))
        *begin++ = '+';
    auto fmt = std::chars_format::scientific;
    // GCC to_chars support for std::float128_t depends on glibc version,
    // so first check if to_chars is available at all.
    constexpr bool print_chars_available =
        requires { std::to_chars(begin, buf_end, value, fmt, precision); };
    if constexpr (print_chars_available) {
        auto [end, _] = std::to_chars(begin, buf_end, value, fmt, precision);
        return std::string_view{buf.data(), end};
    } else {
        auto value_ld = static_cast<long double>(value);
        auto [end, _] = std::to_chars(begin, buf_end, value_ld, fmt, precision);
        return std::string_view{buf.data(), end};
    }
}
#else
#pragma message("Using std::snprintf as a fallback to replace std::to_chars")

std::string_view float_to_str_vw_impl(std::span<char> buf, double value,
                                      int precision) {
    return float_to_str_vw_snprintf(std::snprintf, buf, value, precision,
                                    "%+-#.*e");
}
std::string_view float_to_str_vw_impl(std::span<char> buf, float value,
                                      int precision) {
    return float_to_str_vw_impl(buf, static_cast<double>(value), precision);
}
std::string_view float_to_str_vw_impl(std::span<char> buf, long double value,
                                      int precision) {
    return float_to_str_vw_snprintf(std::snprintf, buf, value, precision,
                                    "%+-#.*Le");
}
#endif

#ifdef GUANAQO_WITH_QUAD_PRECISION
std::string_view float_to_str_vw_impl(std::span<char> buf, __float128 value,
                                      int precision) {
    return float_to_str_vw_snprintf(quadmath_snprintf, buf, value, precision,
                                    "%+-#.*Qe");
}
#endif

} // namespace

template <std::floating_point F>
std::string_view float_to_str_vw(std::span<char> buf, F value, int precision) {
    return float_to_str_vw_impl(buf, value, precision);
}

#ifndef DOXYGEN
// clang-format off
// NOLINTBEGIN(*-parentheses)
#define GUANAQO_PRINT_INSTANTIATE_F(type) \
    template GUANAQO_EXPORT std::string_view float_to_str_vw(std::span<char>, type value, int precision); \
    template GUANAQO_EXPORT std::string float_to_str(type value, int precision); \
    template GUANAQO_EXPORT std::ostream &detail::print_csv_impl(std::ostream &os, PrintMatrixView<const type>, PrintOpts); \
    template GUANAQO_EXPORT std::ostream &detail::print_matlab_impl(std::ostream &os, PrintMatrixView<const type>, std::string_view); \
    template GUANAQO_EXPORT std::ostream &detail::print_python_impl(std::ostream &os, PrintMatrixView<const type>, std::string_view, bool); \
    template GUANAQO_EXPORT std::ostream &detail::print_csv_impl(std::ostream &os, PrintMatrixView<const std::complex<type>>, PrintOpts); \
    template GUANAQO_EXPORT std::ostream &detail::print_matlab_impl(std::ostream &os, PrintMatrixView<const std::complex<type>>, std::string_view); \
    template GUANAQO_EXPORT std::ostream &detail::print_python_impl(std::ostream &os, PrintMatrixView<const std::complex<type>>, std::string_view, bool)
// NOLINTEND(*-parentheses)
// clang-format on

GUANAQO_PRINT_INSTANTIATE_F(float);
GUANAQO_PRINT_INSTANTIATE_F(double);
GUANAQO_PRINT_INSTANTIATE_F(long double);
#if defined(__STDCPP_FLOAT16_T__) && __STDCPP_FLOAT16_T__ == 1
GUANAQO_PRINT_INSTANTIATE_F(std::float16_t);
#endif
#if defined(__STDCPP_FLOAT32_T__) && __STDCPP_FLOAT32_T__ == 1
GUANAQO_PRINT_INSTANTIATE_F(std::float32_t);
#endif
#if defined(__STDCPP_FLOAT64_T__) && __STDCPP_FLOAT64_T__ == 1
GUANAQO_PRINT_INSTANTIATE_F(std::float64_t);
#endif
#if defined(__STDCPP_FLOAT128_T__) && __STDCPP_FLOAT128_T__ == 1
GUANAQO_PRINT_INSTANTIATE_F(std::float128_t);
#endif
#if defined(__STDCPP_BFLOAT16_T__) && __STDCPP_BFLOAT16_T__ == 1
GUANAQO_PRINT_INSTANTIATE_F(std::bfloat16_t);
#endif
#ifdef GUANAQO_WITH_QUAD_PRECISION
GUANAQO_PRINT_INSTANTIATE_F(__float128);
#endif

// clang-format off
#define GUANAQO_PRINT_INSTANTIATE_I(type) \
    template GUANAQO_EXPORT std::ostream &detail::print_csv_impl(std::ostream &os, PrintMatrixView<const type>, PrintOpts); \
    template GUANAQO_EXPORT std::ostream &detail::print_matlab_impl(std::ostream &os, PrintMatrixView<const type>, std::string_view); \
    template GUANAQO_EXPORT std::ostream &detail::print_python_impl(std::ostream &os, PrintMatrixView<const type>, std::string_view, bool)
// clang-format on

GUANAQO_PRINT_INSTANTIATE_I(char);
GUANAQO_PRINT_INSTANTIATE_I(signed char);
GUANAQO_PRINT_INSTANTIATE_I(short);
GUANAQO_PRINT_INSTANTIATE_I(int);
GUANAQO_PRINT_INSTANTIATE_I(long);
GUANAQO_PRINT_INSTANTIATE_I(long long);
GUANAQO_PRINT_INSTANTIATE_I(unsigned char);
GUANAQO_PRINT_INSTANTIATE_I(unsigned short);
GUANAQO_PRINT_INSTANTIATE_I(unsigned int);
GUANAQO_PRINT_INSTANTIATE_I(unsigned long);
GUANAQO_PRINT_INSTANTIATE_I(unsigned long long);

#endif // DOXYGEN

} // namespace guanaqo
