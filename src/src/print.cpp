#include <guanaqo/export.h>
#include <guanaqo/implementation/print.tpp>

namespace guanaqo {

using std::complex;
using std::ostream;

#ifndef DOXYGEN
// clang-format off

template GUANAQO_EXPORT std::string float_to_str(float value, int precision);
template GUANAQO_EXPORT std::string float_to_str(double value, int precision);
template GUANAQO_EXPORT std::string float_to_str(long double value, int precision);

namespace detail {
template GUANAQO_EXPORT ostream &print_csv_impl(ostream &os, MatrixView<const int>, PrintOpts);
template GUANAQO_EXPORT ostream &print_csv_impl(ostream &os, MatrixView<const long>, PrintOpts);
template GUANAQO_EXPORT ostream &print_csv_impl(ostream &os, MatrixView<const long long>, PrintOpts);
template GUANAQO_EXPORT ostream &print_csv_impl(ostream &os, MatrixView<const float>, PrintOpts);
template GUANAQO_EXPORT ostream &print_csv_impl(ostream &os, MatrixView<const double>, PrintOpts);
template GUANAQO_EXPORT ostream &print_csv_impl(ostream &os, MatrixView<const long double>, PrintOpts);
template GUANAQO_EXPORT ostream &print_csv_impl(ostream &os, MatrixView<const complex<float>>, PrintOpts);
template GUANAQO_EXPORT ostream &print_csv_impl(ostream &os, MatrixView<const complex<double>>, PrintOpts);
template GUANAQO_EXPORT ostream &print_csv_impl(ostream &os, MatrixView<const complex<long double>>, PrintOpts);

template GUANAQO_EXPORT ostream &print_matlab_impl(ostream &os, MatrixView<const int>, std::string_view);
template GUANAQO_EXPORT ostream &print_matlab_impl(ostream &os, MatrixView<const long>, std::string_view);
template GUANAQO_EXPORT ostream &print_matlab_impl(ostream &os, MatrixView<const long long>, std::string_view);
template GUANAQO_EXPORT ostream &print_matlab_impl(ostream &os, MatrixView<const float>, std::string_view);
template GUANAQO_EXPORT ostream &print_matlab_impl(ostream &os, MatrixView<const double>, std::string_view);
template GUANAQO_EXPORT ostream &print_matlab_impl(ostream &os, MatrixView<const long double>, std::string_view);
template GUANAQO_EXPORT ostream &print_matlab_impl(ostream &os, MatrixView<const complex<float>>, std::string_view);
template GUANAQO_EXPORT ostream &print_matlab_impl(ostream &os, MatrixView<const complex<double>>, std::string_view);
template GUANAQO_EXPORT ostream &print_matlab_impl(ostream &os, MatrixView<const complex<long double>>, std::string_view);

template GUANAQO_EXPORT ostream &print_python_impl(ostream &os, MatrixView<const int>, std::string_view, bool);
template GUANAQO_EXPORT ostream &print_python_impl(ostream &os, MatrixView<const long>, std::string_view, bool);
template GUANAQO_EXPORT ostream &print_python_impl(ostream &os, MatrixView<const long long>, std::string_view, bool);
template GUANAQO_EXPORT ostream &print_python_impl(ostream &os, MatrixView<const float>, std::string_view, bool);
template GUANAQO_EXPORT ostream &print_python_impl(ostream &os, MatrixView<const double>, std::string_view, bool);
template GUANAQO_EXPORT ostream &print_python_impl(ostream &os, MatrixView<const long double>, std::string_view, bool);
template GUANAQO_EXPORT ostream &print_python_impl(ostream &os, MatrixView<const complex<float>>, std::string_view, bool);
template GUANAQO_EXPORT ostream &print_python_impl(ostream &os, MatrixView<const complex<double>>, std::string_view, bool);
template GUANAQO_EXPORT ostream &print_python_impl(ostream &os, MatrixView<const complex<long double>>, std::string_view, bool);
}

#ifdef GUANAQO_WITH_QUAD_PRECISION
template GUANAQO_EXPORT std::string float_to_str(__float128 value, int precision);

namespace detail {
template GUANAQO_EXPORT ostream &print_csv_impl(ostream &os, MatrixView<const __float128>, PrintOpts);
template GUANAQO_EXPORT ostream &print_csv_impl(ostream &os, MatrixView<const complex<__float128>>, PrintOpts);

template GUANAQO_EXPORT ostream &print_matlab_impl(ostream &os, MatrixView<const __float128>, std::string_view);
template GUANAQO_EXPORT ostream &print_matlab_impl(ostream &os, MatrixView<const complex<__float128>>, std::string_view);

template GUANAQO_EXPORT ostream &print_python_impl(ostream &os, MatrixView<const __float128>, std::string_view, bool);
template GUANAQO_EXPORT ostream &print_python_impl(ostream &os, MatrixView<const complex<__float128>>, std::string_view, bool);
}
#endif

// clang-format on
#endif // DOXYGEN

} // namespace guanaqo