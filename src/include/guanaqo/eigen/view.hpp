#pragma once

/// @file
/// @ingroup linalg_views
/// Conversions between Eigen expressions, maps and refs, and guanaqo::MatrixView.

#include <guanaqo/mat-view.hpp>

#include <Eigen/Core>
#include <type_traits>

namespace guanaqo {

template <class I>
struct with_index_type_t {};
template <class I>
constexpr with_index_type_t<I> with_index_type;

namespace detail {

template <class Derived, class I>
auto as_view_impl(auto &M) {
    auto *const data = M.derived().data();
    using T          = std::remove_pointer_t<decltype(data)>;
    using Scalar     = typename Derived::Scalar;
    static_assert(std::is_same_v<Scalar, std::remove_const_t<T>>);
    static constexpr auto O =
        Derived::IsRowMajor ? StorageOrder::RowMajor : StorageOrder::ColMajor;
    const auto rows         = static_cast<I>(M.derived().rows()),
               cols         = static_cast<I>(M.derived().cols()),
               outer_stride = static_cast<I>(M.derived().outerStride());
    constexpr auto static_inner_stride = Derived::InnerStrideAtCompileTime;
    // Dynamic (run-time) inner stride
    if constexpr (static_inner_stride == Eigen::Dynamic) {
        const auto inner_stride = static_cast<I>(M.derived().innerStride());
        return MatrixView<T, I, I, O>{{.data         = data,
                                       .rows         = rows,
                                       .cols         = cols,
                                       .inner_stride = inner_stride,
                                       .outer_stride = outer_stride}};
    }
    // Compile-time inner stride
    else {
        using S = std::integral_constant<I, static_inner_stride>;
        return MatrixView<T, I, S, O>{{.data         = data,
                                       .rows         = rows,
                                       .cols         = cols,
                                       .outer_stride = outer_stride}};
    }
}

} // namespace detail

/// @addtogroup linalg_views
/// @{

// Conversions from Eigen expressions to MatrixView

// We need overloads for const and non-const references because some Eigen types
// have deep constness (i.e. if the matrix itself is const, the .data() function
// returns a pointer to const).
// Then we also need overloads for rvalues because we want to prevent dangling
// pointers to temporaries. We need the const rvalue reference overload because
// Eigen's .eval() member function returns a const Eigen::Matrix, so it does not
// match the normal rvalue reference overload.

/// Convert an Eigen matrix view to a guanaqo::MatrixView.
template <class Derived, class I = typename Derived::Index>
auto as_view(Eigen::DenseBase<Derived> &M, with_index_type_t<I> = {}) {
    return detail::as_view_impl<Derived, I>(M);
}

/// Convert an Eigen matrix view to a guanaqo::MatrixView.
template <class Derived, class I = typename Derived::Index>
auto as_view(const Eigen::DenseBase<Derived> &M, with_index_type_t<I> = {}) {
    return detail::as_view_impl<Derived, I>(M);
}

/// Convert an Eigen matrix view to a guanaqo::MatrixView.
template <class Derived, class I = typename Derived::Index>
auto as_view(Eigen::DenseBase<Derived> &&M, with_index_type_t<I> = {}) {
    return detail::as_view_impl<Derived, I>(M);
}

/// Convert an Eigen matrix view to a guanaqo::MatrixView.
template <class Derived, class I = typename Derived::Index>
auto as_view(const Eigen::DenseBase<Derived> &&M, with_index_type_t<I> = {}) {
    return detail::as_view_impl<Derived, I>(M);
}

template <class Derived, class I = typename Derived::Index>
    requires(std::is_base_of_v<Eigen::PlainObjectBase<std::decay_t<Derived>>,
                               std::decay_t<Derived>>)
auto as_view( // Refusing to return a view to a temporary Eigen matrix with its own storage
    Eigen::DenseBase<Derived> &&M, with_index_type_t<I> = {}) = delete;

template <class Derived, class I = typename Derived::Index>
    requires(std::is_base_of_v<Eigen::PlainObjectBase<std::decay_t<Derived>>,
                               std::decay_t<Derived>>)
auto as_view( // Refusing to return a view to a temporary Eigen matrix with its own storage
    const Eigen::DenseBase<Derived> &&M, with_index_type_t<I> = {}) = delete;

// Conversions from MatrixView to Eigen::Map

/// Convert a guanaqo::MatrixView to an Eigen::Matrix view.
template <class T, class I, class S, StorageOrder O>
auto as_eigen(MatrixView<T, I, S, O> M) {
    constexpr auto Opt = M.is_row_major ? Eigen::RowMajor : Eigen::ColMajor;
    constexpr auto X   = Eigen::Dynamic;
    using Scalar       = std::remove_const_t<T>;
    using Mat          = Eigen::Matrix<Scalar, X, X, Opt>;
    using CMat         = std::conditional_t<std::is_const_v<T>, const Mat, Mat>;
    using Index        = typename Mat::Index;
    const auto rows    = static_cast<Index>(M.rows),
               cols    = static_cast<Index>(M.cols),
               outer_stride = static_cast<Index>(M.outer_stride);
    // Case where inner stride is a compile-time constant
    if constexpr (requires { S::value; }) {
        static constexpr auto inner_stride = static_cast<Index>(M.inner_stride);
        // Eigen does not support zero stride at compile time (it is interpreted
        // as unit stride). The value -1 is reserved as well, so it cannot be
        // used as a compile-time stride either. We therefore use a dynamic
        // stride, and set the run-time value to 0 or -1.
        if constexpr (inner_stride == 0 || inner_stride == Eigen::Dynamic) {
            using Stride = Eigen::Stride<Eigen::Dynamic, Eigen::Dynamic>;
            using Map    = Eigen::Map<CMat, 0, Stride>;
            return Map{M.data, rows, cols, {outer_stride, inner_stride}};
        }
        // Unit inner stride
        else if constexpr (inner_stride == 1) {
            using Stride = Eigen::OuterStride<>;
            using Map    = Eigen::Map<CMat, 0, Stride>;
            return Map{M.data, rows, cols, {outer_stride}};
        }
        // Fixed inner stride >1
        else {
            using Stride = Eigen::Stride<Eigen::Dynamic, inner_stride>;
            using Map    = Eigen::Map<CMat, 0, Stride>;
            return Map{M.data, rows, cols, {outer_stride, inner_stride}};
        }
    }
    // Both inner and outer stride are dynamic
    else {
        const auto inner_stride = static_cast<Index>(M.inner_stride);
        using Stride            = Eigen::Stride<Eigen::Dynamic, Eigen::Dynamic>;
        using Map               = Eigen::Map<CMat, 0, Stride>;
        return Map{M.data, rows, cols, {outer_stride, inner_stride}};
    }
}

/// @}

} // namespace guanaqo
