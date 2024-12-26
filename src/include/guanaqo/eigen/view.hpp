#pragma once

#include <guanaqo/mat-view.hpp>

#include <Eigen/Core>
#include <type_traits>

namespace guanaqo {

template <class I>
struct with_index_type_t {};
template <class I>
constexpr with_index_type_t<I> with_index_type;

/// Convert an Eigen matrix view to a guanaqo::MatrixView.
template <class Derived, class I = typename Derived::Index>
auto as_view(Eigen::DenseBase<Derived> &M,
             with_index_type_t<I> = with_index_type<typename Derived::Index>) {
    static_assert(M.InnerStrideAtCompileTime == 1,
                  "Only unit inner stride is supported");
    using T = std::remove_pointer_t<decltype(M.derived().data())>;
    return MatrixView<T, I>{{
        .data         = M.derived().data(),
        .rows         = static_cast<I>(M.derived().rows()),
        .cols         = static_cast<I>(M.derived().cols()),
        .outer_stride = static_cast<I>(M.derived().outerStride()),
    }};
}

/// Convert an Eigen matrix view to a guanaqo::MatrixView.
template <class Derived, class I = typename Derived::Index>
auto as_view(Eigen::DenseBase<Derived> &&M,
             with_index_type_t<I> = with_index_type<typename Derived::Index>) {
    using PlainObjectBase = Eigen::PlainObjectBase<std::decay_t<Derived>>;
    static_assert(M.InnerStrideAtCompileTime == 1,
                  "Only unit inner stride is supported");
    static_assert(!std::is_base_of_v<PlainObjectBase, std::decay_t<Derived>>,
                  "Refusing to return a view to a temporary Eigen matrix with "
                  "its own storage");
    using T = std::remove_pointer_t<decltype(M.derived().data())>;
    return MatrixView<T, I>{{
        .data         = M.derived().data(),
        .rows         = static_cast<I>(M.derived().rows()),
        .cols         = static_cast<I>(M.derived().cols()),
        .outer_stride = static_cast<I>(M.derived().outerStride()),
    }};
}

/// Convert an Eigen matrix view to a guanaqo::MatrixView.
template <class Derived, class I = typename Derived::Index>
auto as_view(const Eigen::DenseBase<Derived> &M,
             with_index_type_t<I> = with_index_type<typename Derived::Index>) {
    static_assert(M.InnerStrideAtCompileTime == 1,
                  "Only unit inner stride is supported");
    using T = std::remove_pointer_t<decltype(M.derived().data())>;
    return MatrixView<T, I>{{
        .data         = M.derived().data(),
        .rows         = static_cast<I>(M.derived().rows()),
        .cols         = static_cast<I>(M.derived().cols()),
        .outer_stride = static_cast<I>(M.derived().outerStride()),
    }};
}

/// Convert a guanaqo::MatrixView to an Eigen::Matrix view.
template <class T, class I>
auto as_eigen(MatrixView<T, I> M) {
    using Scalar = std::remove_const_t<T>;
    using Mat    = Eigen::MatrixX<Scalar>;
    using CMat   = std::conditional_t<std::is_const_v<T>, const Mat, Mat>;
    using Map    = Eigen::Map<CMat, 0, Eigen::OuterStride<>>;
    return Map{
        M.data,
        static_cast<typename Mat::Index>(M.rows),
        static_cast<typename Mat::Index>(M.cols),
        {static_cast<typename Mat::Index>(M.outer_stride)},
    };
}

} // namespace guanaqo
