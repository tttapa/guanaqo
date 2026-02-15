#pragma once

/// @file
/// @ingroup linalg_views
/// Conversion between guanaqo::MatrixView and NumPy arrays for nanobind bindings.

#include <guanaqo/mat-view.hpp>
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <type_traits>
#include <variant>

namespace nanobind::detail {

/// @addtogroup linalg_views
/// @{

/// Nanobind type caster for @ref guanaqo::MatrixView.
/// Supports only NumPy arrays on CPU. No conversions or copies are performed.
template <class T, class I, class S, guanaqo::StorageOrder O>
struct type_caster<guanaqo::MatrixView<T, I, S, O>> {
    using MatrixView = guanaqo::MatrixView<T, I, S, O>;

    static constexpr bool is_column_major         = MatrixView::is_column_major;
    static constexpr bool is_row_major            = MatrixView::is_row_major;
    static constexpr bool has_static_inner_stride = requires {
        { S::value } -> std::convertible_to<I>;
    };

    using NDArray       = ndarray<numpy, T, ndim<2>, device::cpu>;
    using NDArrayCaster = type_caster<NDArray>;
    using Contig        = std::conditional_t<is_row_major, c_contig, f_contig>;
    using NDArrayContig = ndarray<numpy, T, ndim<2>, Contig, device::cpu>;
    using NDArrayContigCaster = type_caster<NDArrayContig>;

    NB_TYPE_CASTER(MatrixView, NDArrayCaster::Name);

    type_caster() : value{{.data = nullptr, .rows = 0, .cols = 0}} {}
    type_caster(const type_caster &)     = default;
    type_caster(type_caster &&) noexcept = default;

  private:
    std::variant<NDArrayCaster, NDArrayContigCaster> caster;

    bool from_python_(auto &caster, handle src, uint8_t flags,
                      cleanup_list *cleanup) noexcept {
        if (!caster.from_python(src, flags, cleanup))
            return false;
        auto &arr            = caster.value;
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
        if constexpr (has_static_inner_stride) {
            this->value.reassign({{
                .data         = arr.data(),
                .rows         = rows,
                .cols         = cols,
                .outer_stride = outer_stride,
            }});
        } else {
            this->value.reassign({{
                .data         = arr.data(),
                .rows         = rows,
                .cols         = cols,
                .inner_stride = inner_stride,
                .outer_stride = outer_stride,
            }});
        }
        return true;
    }

  public:
    bool from_python(handle src, uint8_t flags,
                     cleanup_list *cleanup) noexcept {
        flags &= ~(uint8_t)cast_flags::accepts_none; // Don't accept None
        const auto flags_no_convert = flags & ~(uint8_t)cast_flags::convert;
        if (!std::is_const_v<T> || S{} > 1)
            // no conversions if mutable or non-unit stride
            flags = flags_no_convert;
        // Try without conversions first
        if (from_python_(caster.template emplace<NDArrayCaster>(), src,
                         flags_no_convert, cleanup))
            return true;
        if (flags == flags_no_convert)
            return false;
        // Try with conversions enabled
        return from_python_(caster.template emplace<NDArrayContigCaster>(), src,
                            flags, cleanup);
    }

    static handle from_cpp(const Value &view, rv_policy policy,
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

/// @}

} // namespace nanobind::detail
