#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <span>

namespace guanaqo {

enum class Triangular {
    Lower,
    StrictlyLower,
    Upper,
    StrictlyUpper,
};

template <class T, class I = ptrdiff_t>
struct MatrixView {
    using value_t  = T;
    using index_t  = I;
    value_t *data  = nullptr;
    index_t rows   = 0;
    index_t cols   = 1;
    index_t stride = rows;

    value_t &operator()(index_t r, index_t c) { return data[r + c * stride]; }
    const value_t &operator()(index_t r, index_t c) const {
        return data[r + c * stride];
    }
#if __cpp_multidimensional_subscript >= 202110L
    value_t &operator[](index_t r, index_t c) { return operator()(r, c); }
    const value_t &operator[](index_t r, index_t c) const {
        return operator()(r, c);
    }
#endif
    [[nodiscard]] bool empty() const { return rows == 0 || cols == 0; }

    MatrixView top_rows(index_t n) const {
        assert(0 <= n && n <= rows);
        return {
            .data   = data,
            .rows   = n,
            .cols   = cols,
            .stride = stride,
        };
    }
    MatrixView left_cols(index_t n) const {
        assert(0 <= n && n <= cols);
        return {
            .data   = data,
            .rows   = rows,
            .cols   = n,
            .stride = stride,
        };
    }
    MatrixView bottom_rows(index_t n) const {
        assert(0 <= n && n <= rows);
        return {
            .data   = data + rows - n,
            .rows   = n,
            .cols   = cols,
            .stride = stride,
        };
    }
    MatrixView right_cols(index_t n) const {
        assert(0 <= n && n <= cols);
        return {
            .data   = data + stride * (cols - n),
            .rows   = rows,
            .cols   = n,
            .stride = stride,
        };
    }
    MatrixView middle_rows(index_t r, index_t n) const {
        return bottom_rows(rows - r).top_rows(n);
    }
    MatrixView middle_cols(index_t c, index_t n) const {
        return right_cols(rows - c).left_cols(n);
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
        return {.data = data, .rows = rows, .cols = cols, .stride = stride};
    }

    void set_constant(const value_t &t) {
        for (index_t c = 0; c < cols; ++c)
            std::fill_n(data + c * stride, rows, t);
    }
    void set_constant(const value_t &t, Triangular tr) {
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
            p += stride + 1;
        }
    }
};

} // namespace guanaqo
