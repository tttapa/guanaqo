#pragma once

/// @file
/// @ingroup io
/// Matrix/vector printing helpers and float formatting.

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

/// @addtogroup io
/// @{

template <class T>
class PrintMatrixView {
  public:
    using value_type        = T;
    using index_type        = ptrdiff_t;
    using inner_stride_type = index_type;

  public:
    PrintMatrixView() = default;
    template <class U, class I, class S>
    PrintMatrixView(MatrixView<U, I, S, StorageOrder::ColMajor> v) noexcept
        : PrintMatrixView{v, false} {}
    template <class U, class I, class S>
    PrintMatrixView(MatrixView<U, I, S, StorageOrder::RowMajor> v) noexcept
        : PrintMatrixView{v.transposed(), true} {}
    template <class U, std::size_t E>
    PrintMatrixView(std::span<U, E> x) noexcept
        : view{{
              .data = x.data(),
              .rows = static_cast<index_type>(x.size()),
          }} {}

    [[nodiscard]] index_type rows() const noexcept {
        return transposed ? view.cols : view.rows;
    }
    [[nodiscard]] index_type cols() const noexcept {
        return transposed ? view.rows : view.cols;
    }
    [[nodiscard]] value_type &operator()(index_type i,
                                         index_type j) const noexcept {
        return transposed ? view(j, i) : view(i, j);
    }

  private:
    MatrixView<T, index_type, index_type, StorageOrder::ColMajor> view;
    bool transposed = false;

    template <class U, class I, class S>
    PrintMatrixView(MatrixView<U, I, S, StorageOrder::ColMajor> v,
                    bool transposed) noexcept
        : view{{
              .data         = v.data,
              .rows         = static_cast<index_type>(v.rows),
              .cols         = static_cast<index_type>(v.cols),
              .inner_stride = static_cast<inner_stride_type>(v.inner_stride),
              .outer_stride = static_cast<index_type>(v.outer_stride),
          }},
          transposed{transposed} {}
};

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

/// @}

namespace detail {

template <class T>
GUANAQO_EXPORT std::ostream &print_csv_impl(std::ostream &os,
                                            PrintMatrixView<const T> M,
                                            PrintOpts opts = {});

template <class T>
GUANAQO_EXPORT std::ostream &print_matlab_impl(std::ostream &os,
                                               PrintMatrixView<const T> M,
                                               std::string_view end = ";\n");

template <class T>
GUANAQO_EXPORT std::ostream &
print_python_impl(std::ostream &os, PrintMatrixView<const T> M,
                  std::string_view end = "\n", bool squeeze = true);

} // namespace detail

/// @addtogroup io
/// @{

template <class T, std::size_t E>
std::ostream &print_csv(std::ostream &os, std::span<T, E> x,
                        PrintOpts opts = {}) {
    return guanaqo::detail::print_csv_impl(os, PrintMatrixView<const T>{x},
                                           opts);
}

template <class T, class I, class S, StorageOrder O>
std::ostream &print_csv(std::ostream &os, MatrixView<T, I, S, O> X,
                        PrintOpts opts = {}) {
    return guanaqo::detail::print_csv_impl(os, PrintMatrixView<const T>{X},
                                           opts);
}

template <class T, std::size_t E>
std::ostream &print_matlab(std::ostream &os, std::span<T, E> x,
                           std::string_view end = ";\n") {
    return guanaqo::detail::print_matlab_impl(os, PrintMatrixView<const T>{x},
                                              end);
}

template <class T, class I, class S, StorageOrder O>
std::ostream &print_matlab(std::ostream &os, MatrixView<T, I, S, O> X,
                           std::string_view end = ";\n") {
    return guanaqo::detail::print_matlab_impl(os, PrintMatrixView<const T>{X},
                                              end);
}

template <class T, std::size_t E>
std::ostream &print_python(std::ostream &os, std::span<T, E> x,
                           std::string_view end = "\n", bool squeeze = true) {
    return guanaqo::detail::print_python_impl(os, PrintMatrixView<const T>{x},
                                              end, squeeze);
}

template <class T, class I, class S, StorageOrder O>
std::ostream &print_python(std::ostream &os, MatrixView<T, I, S, O> X,
                           std::string_view end = "\n", bool squeeze = true) {
    return guanaqo::detail::print_python_impl(os, PrintMatrixView<const T>{X},
                                              end, squeeze);
}

/// @}

} // namespace guanaqo
