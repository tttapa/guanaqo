#pragma once

#include <guanaqo/export.h>
#include <concepts>
#include <iosfwd>
#include <span>
#include <stdexcept>
#include <vector>

namespace guanaqo::io {

struct GUANAQO_EXPORT csv_read_error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

namespace detail {

template <class F>
    requires(std::floating_point<F> || std::integral<F>)
void GUANAQO_EXPORT csv_read_row_impl(std::istream &is, std::span<F> v,
                                      char sep = ',');

} // namespace detail

template <class F>
    requires(std::floating_point<F> || std::integral<F>)
std::vector<F> GUANAQO_EXPORT csv_read_row_std_vector(std::istream &is,
                                                      char sep = ',');

} // namespace guanaqo::io