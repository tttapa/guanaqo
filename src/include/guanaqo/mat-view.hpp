#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <span>
#include <type_traits>

namespace guanaqo {

enum class Triangular {
    Lower,
    StrictlyLower,
    Upper,
    StrictlyUpper,
};

template <class S>
struct unit_stride {
    static constexpr S value{1};
};

template <class I, I V>
struct unit_stride<std::integral_constant<I, V>> {
    static constexpr std::integral_constant<I, V> value{};
};

template <class T, class I = ptrdiff_t, class S = std::integral_constant<I, 1>>
struct MatrixView {
    using value_t        = T;
    using index_t        = I;
    using inner_stride_t = S;
    value_t *data        = nullptr;
    index_t rows         = 0;
    index_t cols         = 1;
    index_t stride       = rows;
    [[no_unique_address]] inner_stride_t inner_stride =
        unit_stride<inner_stride_t>::value;

    value_t &operator()(index_t r, index_t c) const {
        assert(0 <= r && r < rows);
        assert(0 <= c && c < cols);
        return data[r * inner_stride + c * stride];
    }
#if __cpp_multidimensional_subscript >= 202110L
    value_t &operator[](index_t r, index_t c) const { return operator()(r, c); }
#endif
    [[nodiscard]] bool empty() const { return rows == 0 || cols == 0; }

    MatrixView top_rows(index_t n) const {
        assert(0 <= n && n <= rows);
        return {
            .data         = data,
            .rows         = n,
            .cols         = cols,
            .stride       = stride,
            .inner_stride = inner_stride,
        };
    }
    MatrixView left_cols(index_t n) const {
        assert(0 <= n && n <= cols);
        return {
            .data         = data,
            .rows         = rows,
            .cols         = n,
            .stride       = stride,
            .inner_stride = inner_stride,
        };
    }
    MatrixView bottom_rows(index_t n) const {
        assert(0 <= n && n <= rows);
        return {
            .data         = data + inner_stride * (rows - n),
            .rows         = n,
            .cols         = cols,
            .stride       = stride,
            .inner_stride = inner_stride,
        };
    }
    MatrixView right_cols(index_t n) const {
        assert(0 <= n && n <= cols);
        return {
            .data         = data + stride * (cols - n),
            .rows         = rows,
            .cols         = n,
            .stride       = stride,
            .inner_stride = inner_stride,
        };
    }
    MatrixView middle_rows(index_t r, index_t n) const {
        return bottom_rows(rows - r).top_rows(n);
    }
    MatrixView middle_cols(index_t c, index_t n) const {
        return right_cols(cols - c).left_cols(n);
    }
    MatrixView top_left(index_t nr, index_t nc) const {
        return top_rows(nr).left_cols(nc);
    }
    MatrixView top_right(index_t nr, index_t nc) const {
        return top_rows(nr).right_cols(nc);
    }
    MatrixView bottom_left(index_t nr, index_t nc) const {
        return bottom_rows(nr).left_cols(nc);
    }
    MatrixView bottom_right(index_t nr, index_t nc) const {
        return bottom_rows(nr).right_cols(nc);
    }
    MatrixView block(index_t r, index_t c, index_t nr, index_t nc) const {
        return middle_rows(r, nr).middle_cols(c, nc);
    }

    static MatrixView as_column(std::span<T> v) {
        return {
            .data = v.data(),
            .rows = static_cast<index_t>(v.size()),
            .cols = 1,
        };
    }

    operator MatrixView<const T, I>() const {
        return {
            .data         = data,
            .rows         = rows,
            .cols         = cols,
            .stride       = stride,
            .inner_stride = inner_stride,
        };
    }

    void set_constant(const value_t &t) {
        if (inner_stride == 1)
            for (index_t c = 0; c < cols; ++c)
                std::fill_n(data + c * stride, rows, t);
        else
            for (index_t c = 0; c < cols; ++c)
                for (index_t r = 0; r < rows; ++r)
                    (*this)(r, c) = t;
    }
    void set_constant(const value_t &t, Triangular tr)
        requires(inner_stride_t::value == 1)
    {
        auto n = std::max(rows, cols);
        switch (tr) {
            case Triangular::Lower:
                for (index_t c = 0; c < n; ++c)
                    std::fill_n(data + c + c * stride, rows - c, t);
                break;
            case Triangular::StrictlyLower:
                for (index_t c = 0; c < n - 1; ++c)
                    std::fill_n(data + c + 1 + c * stride, rows - c - 1, t);
                break;
            case Triangular::Upper:
                for (index_t c = 0; c < n; ++c)
                    std::fill_n(data + c * stride, 1 + c, t);
                break;
            case Triangular::StrictlyUpper:
                for (index_t c = 1; c < n; ++c)
                    std::fill_n(data + c * stride, c, t);
                break;
            default: assert(!"Unexpected value for guanaqo::Triangular");
        }
    }
    void set_diagonal(const value_t &t) {
        auto *p = data;
        auto n  = std::max(rows, cols);
        for (index_t i = 0; i < n; ++i) {
            *p = t;
            p += stride + inner_stride;
        }
    }
    template <class U, class J, class R>
        requires(!std::is_const_v<T> &&
                 std::convertible_to<U, std::remove_cv_t<T>> &&
                 std::equality_comparable_with<I, J>)
    MatrixView &operator=(MatrixView<U, J, R> other) {
        assert(other.rows == this->rows);
        assert(other.cols == this->cols);
        const auto *src = other.data;
        auto *dst       = this->data;
        for (index_t c = 0; c < this->cols; ++c) {
            if (other.inner_stride == 1 && this->inner_stride == 1)
                std::copy_n(src, this->rows, dst);
            else {
                const auto *src_ = src;
                auto *dst_       = dst;
                for (index_t r = 0; r < rows; ++r) {
                    *dst_ = *src_;
                    src_ += other.inner_stride;
                    dst_ += this->inner_stride;
                }
            }
            src += other.stride;
            dst += this->stride;
        }
        return *this;
    }
    MatrixView &reassign(MatrixView other) {
        this->data         = other.data;
        this->rows         = other.rows;
        this->cols         = other.cols;
        this->stride       = other.stride;
        this->inner_stride = other.inner_stride;
        return *this;
    }
};

} // namespace guanaqo
