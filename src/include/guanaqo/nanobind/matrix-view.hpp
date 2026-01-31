#pragma once

#include <guanaqo/mat-view.hpp>
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>

namespace nanobind::detail {

/// Type caster for @ref guanaqo::MatrixView.
/// Supports only NumPy arrays on CPU. No conversions or copies are performed.
template <class T, class I, class S, guanaqo::StorageOrder O>
struct type_caster<guanaqo::MatrixView<T, I, S, O>> {
    using MatView = guanaqo::MatrixView<T, I, S, O>;

    static constexpr bool is_column_major         = MatView::is_column_major;
    static constexpr bool is_row_major            = MatView::is_row_major;
    static constexpr bool has_static_inner_stride = requires {
        { S::value } -> std::convertible_to<I>;
    };

    using NDArray       = ndarray<numpy, T, ndim<2>, device::cpu>;
    using NDArrayCaster = type_caster<NDArray>;

    static constexpr auto Name = NDArrayCaster::Name;
    template <typename T_>
    using Cast = MatView;

    NDArrayCaster caster;

    bool from_python(handle src, uint8_t flags,
                     cleanup_list *cleanup) noexcept {
        flags &= ~(uint8_t)cast_flags::convert; // Disable implicit conversions
        flags &= ~(uint8_t)cast_flags::accepts_none; // Don't accept None
        return from_python_(src, flags, cleanup);
    }

  private:
    bool from_python_(handle src, uint8_t flags,
                      cleanup_list *cleanup) noexcept {
        // Delegate to ndarray caster
        if (!caster.from_python(src, flags, cleanup))
            return false;

        const NDArray &arr   = caster.value;
        const I rows         = static_cast<I>(arr.shape(0)),
                cols         = static_cast<I>(arr.shape(1));
        const I row_stride   = static_cast<I>(arr.stride(0)),
                col_stride   = static_cast<I>(arr.stride(1));
        const I inner_stride = is_column_major ? row_stride : col_stride,
                outer_stride = is_column_major ? col_stride : row_stride;

        // Validate inner stride against compile-time expectation
        if constexpr (has_static_inner_stride) {
            // For empty arrays, accept any reported stride
            if (rows > 0 && cols > 0 && inner_stride != S::value)
                return false;
        }
        return true;
    }

  public:
    // Conversion operator to construct MatrixView from the cached ndarray
    operator MatView() {
        const NDArray &arr   = caster.value;
        const I row_stride   = static_cast<I>(arr.stride(0)),
                col_stride   = static_cast<I>(arr.stride(1));
        const I inner_stride = is_column_major ? row_stride : col_stride,
                outer_stride = is_column_major ? col_stride : row_stride;
        if constexpr (has_static_inner_stride) {
            return MatView{{
                .data         = arr.data(),
                .rows         = static_cast<I>(arr.shape(0)),
                .cols         = static_cast<I>(arr.shape(1)),
                .outer_stride = outer_stride,
            }};
        } else {
            return MatView{{
                .data         = arr.data(),
                .rows         = static_cast<I>(arr.shape(0)),
                .cols         = static_cast<I>(arr.shape(1)),
                .inner_stride = inner_stride,
                .outer_stride = outer_stride,
            }};
        }
    }

    static handle from_cpp(const MatView &view, rv_policy policy,
                           cleanup_list *cleanup) noexcept {
        const size_t shape[2]{static_cast<size_t>(view.rows),
                              static_cast<size_t>(view.cols)};
        const int64_t strides[2]{static_cast<int64_t>(view.row_stride()),
                                 static_cast<int64_t>(view.col_stride())};
        if (policy == rv_policy::automatic ||
            policy == rv_policy::automatic_reference)
            policy = rv_policy::reference;
        return NDArrayCaster::from_cpp(
            NDArray(view.data(), 2, shape, handle(), strides), policy, cleanup);
    }
};

} // namespace nanobind::detail
