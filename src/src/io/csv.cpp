#include <guanaqo/export.h>
#include <guanaqo/implementation/io/csv.tpp>

namespace guanaqo::io {

// clang-format off
namespace detail {
template void GUANAQO_EXPORT
csv_read_row_impl(std::istream &, std::span<ptrdiff_t>, char);
template void GUANAQO_EXPORT
csv_read_row_impl(std::istream &, std::span<float>, char);
template void GUANAQO_EXPORT
csv_read_row_impl(std::istream &, std::span<double>, char);
template void GUANAQO_EXPORT
csv_read_row_impl(std::istream &, std::span<long double>, char);
#ifdef GUANAQO_WITH_QUAD_PRECISION
template void GUANAQO_EXPORT
csv_read_row_impl(std::istream &, std::span<__float128>, char);
#endif
} // namespace detail

template std::vector<ptrdiff_t> GUANAQO_EXPORT
csv_read_row_std_vector(std::istream &, char);
template std::vector<float> GUANAQO_EXPORT
csv_read_row_std_vector(std::istream &, char);
template std::vector<double> GUANAQO_EXPORT
csv_read_row_std_vector(std::istream &, char);
template std::vector<long double> GUANAQO_EXPORT
csv_read_row_std_vector(std::istream &, char);
#ifdef GUANAQO_WITH_QUAD_PRECISION
template std::vector<__float128> GUANAQO_EXPORT
csv_read_row_std_vector(std::istream &, char);
#endif
// clang-format on

} // namespace guanaqo::io
