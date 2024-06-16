#include <guanaqo/export.h>
#include <guanaqo/implementation/io/csv.tpp>

namespace guanaqo::io {

// clang-format off
template void GUANAQO_EXPORT
csv_read_row(std::istream &, std::span<int>, char);
template void GUANAQO_EXPORT
csv_read_row(std::istream &, std::span<ptrdiff_t>, char);
template void GUANAQO_EXPORT
csv_read_row(std::istream &, std::span<float>, char);
template void GUANAQO_EXPORT
csv_read_row(std::istream &, std::span<double>, char);
template void GUANAQO_EXPORT
csv_read_row(std::istream &, std::span<long double>, char);
#ifdef GUANAQO_WITH_QUAD_PRECISION
template void GUANAQO_EXPORT
csv_read_row(std::istream &, std::span<__float128>, char);
#endif

template void GUANAQO_EXPORT
csv_read(std::istream &, MatrixView<int, ptrdiff_t, ptrdiff_t>, char);
template void GUANAQO_EXPORT
csv_read(std::istream &, MatrixView<ptrdiff_t, ptrdiff_t, ptrdiff_t>, char);
template void GUANAQO_EXPORT
csv_read(std::istream &, MatrixView<float, ptrdiff_t, ptrdiff_t>, char);
template void GUANAQO_EXPORT
csv_read(std::istream &, MatrixView<double, ptrdiff_t, ptrdiff_t>, char);
template void GUANAQO_EXPORT
csv_read(std::istream &, MatrixView<long double, ptrdiff_t, ptrdiff_t>, char);
#ifdef GUANAQO_WITH_QUAD_PRECISION
template void GUANAQO_EXPORT
csv_read(std::istream &, MatrixView<__float128, ptrdiff_t, ptrdiff_t>, char);
#endif

template std::vector<int> GUANAQO_EXPORT
csv_read_row_std_vector(std::istream &, char);
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
