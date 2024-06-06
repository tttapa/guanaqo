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
    using T = std::remove_pointer_t<decltype(M.derived().data())>;
    return MatrixView<T, I>{
        .data   = M.derived().data(),
        .rows   = static_cast<I>(M.derived().rows()),
        .cols   = static_cast<I>(M.derived().cols()),
        .stride = static_cast<I>(M.derived().outerStride()),
    };
}

/// Convert an Eigen matrix view to a guanaqo::MatrixView.
template <class Derived, class I = typename Derived::Index>
auto as_view(Eigen::DenseBase<Derived> &&M,
             with_index_type_t<I> = with_index_type<typename Derived::Index>) {
    using PlainObjectBase = Eigen::PlainObjectBase<std::decay_t<Derived>>;
    static_assert(!std::is_base_of_v<PlainObjectBase, std::decay_t<Derived>>,
                  "Refusing to return a view to a temporary Eigen matrix with "
                  "its own storage");
    using T = std::remove_pointer_t<decltype(M.derived().data())>;
    return MatrixView<T, I>{
        .data   = M.derived().data(),
        .rows   = static_cast<I>(M.derived().rows()),
        .cols   = static_cast<I>(M.derived().cols()),
        .stride = static_cast<I>(M.derived().outerStride()),
    };
}

/// Convert an Eigen matrix view to a guanaqo::MatrixView.
template <class Derived, class I = typename Derived::Index>
auto as_view(const Eigen::DenseBase<Derived> &M,
             with_index_type_t<I> = with_index_type<typename Derived::Index>) {
    using T = std::remove_pointer_t<decltype(M.derived().data())>;
    return MatrixView<T, I>{
        .data   = M.derived().data(),
        .rows   = static_cast<I>(M.derived().rows()),
        .cols   = static_cast<I>(M.derived().cols()),
        .stride = static_cast<I>(M.derived().outerStride()),
    };
}

/// Convert a guanaqo::MatrixView to an Eigen::Matrix view.
template <class T, class I>
auto as_eigen(MatrixView<T, I> M) {
    using S   = std::remove_const_t<T>;
    using V   = Eigen::MatrixX<S>;
    using Map = Eigen::Map<std::conditional_t<std::is_const_v<T>, const V, V>>;
    return Map{
        M.data,
        static_cast<typename V::Index>(M.rows),
        static_cast<typename V::Index>(M.cols),
        static_cast<typename V::Index>(M.stride),
    };
}

} // namespace guanaqo
