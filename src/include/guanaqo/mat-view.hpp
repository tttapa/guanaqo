#pragma once

#include <cstddef>
#include <span>

namespace guanaqo {

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
        return {
            .data   = data,
            .rows   = n,
            .cols   = cols,
            .stride = stride,
        };
    }
    MatrixView left_cols(index_t n) const {
        return {
            .data   = data,
            .rows   = rows,
            .cols   = n,
            .stride = stride,
        };
    }
    MatrixView bottom_rows(index_t n) const {
        return {
            .data   = data + rows - n,
            .rows   = n,
            .cols   = cols,
            .stride = stride,
        };
    }
    MatrixView right_cols(index_t n) const {
        return {
            .data   = data + stride * (cols - n),
            .rows   = rows,
            .cols   = n,
            .stride = stride,
        };
    }

    static MatrixView as_column(std::span<T> v) {
        return {
            .data = v.data(),
            .rows = static_cast<index_t>(v.size()),
            .cols = 1,
        };
    }
};

} // namespace guanaqo
