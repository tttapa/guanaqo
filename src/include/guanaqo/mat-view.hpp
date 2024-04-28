#pragma once

#include <cstddef>

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
};

} // namespace guanaqo