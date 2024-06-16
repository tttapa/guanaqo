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
struct default_stride;

template <std::integral S>
struct default_stride<S> {
    static constexpr S value{1};
};

template <class S>
    requires(std::default_initializable<S> && !std::constructible_from<S, int>)
struct default_stride<S> {
    static constexpr S value{};
};

template <class T, class I = ptrdiff_t, class S = std::integral_constant<I, 1>>
struct MatrixView {
    using value_type        = T;
    using index_type        = I;
    using inner_stride_type = S;

    value_type *data;
    index_type rows;
    index_type cols;
    [[no_unique_address]] inner_stride_type inner_stride;
    index_type outer_stride;

    /// POD type for designated initializers
    struct PlainMatrixView {
        value_type *data = nullptr;
        index_type rows  = 0;
        index_type cols  = 1;
        [[no_unique_address]] inner_stride_type inner_stride =
            default_stride<inner_stride_type>::value;
        index_type outer_stride = inner_stride * rows;
    };

    MatrixView(PlainMatrixView p)
        : data{p.data}, rows{p.rows}, cols{p.cols},
          inner_stride{p.inner_stride}, outer_stride{p.outer_stride} {}

    value_type &operator()(index_type r, index_type c) const {
        assert(0 <= r && r < rows);
        assert(0 <= c && c < cols);
        return data[r * inner_stride + c * outer_stride];
    }
#if __cpp_multidimensional_subscript >= 202110L
    value_type &operator[](index_type r, index_type c) const {
        return operator()(r, c);
    }
#endif
    [[nodiscard]] bool empty() const { return rows == 0 || cols == 0; }

    MatrixView top_rows(index_type n) const {
        assert(0 <= n && n <= rows);
        return {{
            .data         = data,
            .rows         = n,
            .cols         = cols,
            .inner_stride = inner_stride,
            .outer_stride = outer_stride,
        }};
    }
    MatrixView left_cols(index_type n) const {
        assert(0 <= n && n <= cols);
        return {{
            .data         = data,
            .rows         = rows,
            .cols         = n,
            .inner_stride = inner_stride,
            .outer_stride = outer_stride,
        }};
    }
    MatrixView bottom_rows(index_type n) const {
        assert(0 <= n && n <= rows);
        return {{
            .data         = data + inner_stride * (rows - n),
            .rows         = n,
            .cols         = cols,
            .inner_stride = inner_stride,
            .outer_stride = outer_stride,
        }};
    }
    MatrixView right_cols(index_type n) const {
        assert(0 <= n && n <= cols);
        return {{
            .data         = data + outer_stride * (cols - n),
            .rows         = rows,
            .cols         = n,
            .inner_stride = inner_stride,
            .outer_stride = outer_stride,
        }};
    }
    MatrixView middle_rows(index_type r, index_type n) const {
        return bottom_rows(rows - r).top_rows(n);
    }
    MatrixView middle_cols(index_type c, index_type n) const {
        return right_cols(cols - c).left_cols(n);
    }
    MatrixView top_left(index_type nr, index_type nc) const {
        return top_rows(nr).left_cols(nc);
    }
    MatrixView top_right(index_type nr, index_type nc) const {
        return top_rows(nr).right_cols(nc);
    }
    MatrixView bottom_left(index_type nr, index_type nc) const {
        return bottom_rows(nr).left_cols(nc);
    }
    MatrixView bottom_right(index_type nr, index_type nc) const {
        return bottom_rows(nr).right_cols(nc);
    }
    MatrixView block(index_type r, index_type c, index_type nr,
                     index_type nc) const {
        return middle_rows(r, nr).middle_cols(c, nc);
    }

    static MatrixView as_column(std::span<T> v) {
        return {{
            .data = v.data(),
            .rows = static_cast<index_type>(v.size()),
            .cols = 1,
        }};
    }

    operator MatrixView<const T, I, S>() const {
        return {{
            .data         = data,
            .rows         = rows,
            .cols         = cols,
            .inner_stride = inner_stride,
            .outer_stride = outer_stride,
        }};
    }

    void set_constant(const value_type &t) {
        if (inner_stride == 1)
            for (index_type c = 0; c < cols; ++c)
                std::fill_n(data + c * outer_stride, rows, t);
        else
            for (index_type c = 0; c < cols; ++c)
                for (index_type r = 0; r < rows; ++r)
                    (*this)(r, c) = t;
    }
    void set_constant(const value_type &t, Triangular tr)
        requires(inner_stride_type::value == 1)
    {
        auto n = std::max(rows, cols);
        switch (tr) {
            case Triangular::Lower:
                for (index_type c = 0; c < n; ++c)
                    std::fill_n(data + c + c * outer_stride, rows - c, t);
                break;
            case Triangular::StrictlyLower:
                for (index_type c = 0; c < n - 1; ++c)
                    std::fill_n(data + c + 1 + c * outer_stride, rows - c - 1,
                                t);
                break;
            case Triangular::Upper:
                for (index_type c = 0; c < n; ++c)
                    std::fill_n(data + c * outer_stride, 1 + c, t);
                break;
            case Triangular::StrictlyUpper:
                for (index_type c = 1; c < n; ++c)
                    std::fill_n(data + c * outer_stride, c, t);
                break;
            default: assert(!"Unexpected value for guanaqo::Triangular");
        }
    }
    MatrixView<T, I, I> diagonal() {
        auto n = std::max(rows, cols);
        return {{
            .data         = data,
            .rows         = n,
            .cols         = 1,
            .inner_stride = outer_stride + inner_stride,
        }};
    }
    void set_diagonal(const value_type &t) {
        auto *p = data;
        auto n  = std::max(rows, cols);
        for (index_type i = 0; i < n; ++i) {
            *p = t;
            p += outer_stride + inner_stride;
        }
    }
    void add_to_diagonal(const value_type &t) {
        auto *p = data;
        auto n  = std::max(rows, cols);
        for (index_type i = 0; i < n; ++i) {
            *p += t;
            p += outer_stride + inner_stride;
        }
    }
    void copy_values(auto &other) {
        assert(other.rows == this->rows);
        assert(other.cols == this->cols);
        const auto *src = other.data;
        auto *dst       = this->data;
        for (index_type c = 0; c < this->cols; ++c) {
            if (other.inner_stride == 1 && this->inner_stride == 1)
                std::copy_n(src, this->rows, dst);
            else {
                const auto *src_ = src;
                auto *dst_       = dst;
                for (index_type r = 0; r < rows; ++r) {
                    *dst_ = *src_;
                    src_ += other.inner_stride;
                    dst_ += this->inner_stride;
                }
            }
            src += other.outer_stride;
            dst += this->outer_stride;
        }
    }
    MatrixView(const MatrixView &) = default;
    MatrixView &operator=(const MatrixView &other) {
        if (this != &other)
            copy_values(other);
        return *this;
    }
    template <class U, class J, class R>
        requires(!std::is_const_v<T> &&
                 std::convertible_to<U, std::remove_cv_t<T>> &&
                 std::equality_comparable_with<I, J>)
    MatrixView &operator=(MatrixView<U, J, R> other) {
        copy_values(other);
        return *this;
    }
    // TODO: abstract logic into generic function (and check performance)
    template <class U, class J, class R>
        requires(!std::is_const_v<T> &&
                 std::convertible_to<U, std::remove_cv_t<T>> &&
                 std::equality_comparable_with<I, J>)
    MatrixView &operator+=(MatrixView<U, J, R> other) {
        assert(other.rows == this->rows);
        assert(other.cols == this->cols);
        const auto *src = other.data;
        auto *dst       = this->data;
        for (index_type c = 0; c < this->cols; ++c) {
            const auto *src_ = src;
            auto *dst_       = dst;
            for (index_type r = 0; r < rows; ++r) {
                *dst_ += *src_;
                src_ += other.inner_stride;
                dst_ += this->inner_stride;
            }
            src += other.outer_stride;
            dst += this->outer_stride;
        }
        return *this;
    }
    template <class Generator>
    void generate(Generator gen) {
        for (index_type c = 0; c < cols; ++c)
            for (index_type r = 0; r < rows; ++r)
                (*this)(r, c) = gen();
    }
    MatrixView &reassign(MatrixView other) {
        this->data         = other.data;
        this->rows         = other.rows;
        this->cols         = other.cols;
        this->inner_stride = other.inner_stride;
        this->outer_stride = other.outer_stride;
        return *this;
    }
};

} // namespace guanaqo
