#pragma once

#include <guanaqo/export.h>
#include <guanaqo/linalg/config.hpp>
#include <guanaqo/linalg/sparse-ops.hpp>
#include <guanaqo/linalg/sparsity.hpp>

#include <algorithm>
#include <cassert>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace guanaqo::linalg::sparsity {

struct GUANAQO_EXPORT unsupported_conversion : std::logic_error {
    using std::logic_error::logic_error;
};

constexpr size_t cast_sz(auto i) {
    assert(i >= 0);
    return static_cast<size_t>(i);
}

/// Converts one matrix storage format to another.
/// @tparam From
///         The input sparsity pattern type.
/// @tparam To
///         The output sparsity pattern type.
template <class From, class To>
struct SparsityConverter;

/// Additional options for the conversion performed by @ref SparsityConverter.
template <class To>
struct SparsityConversionRequest;

template <>
struct SparsityConversionRequest<Dense> {};

template <>
struct SparsityConverter<Dense, Dense> {
    using from_sparsity_t = Dense;
    using to_sparsity_t   = Dense;
    using Request         = SparsityConversionRequest<to_sparsity_t>;
    SparsityConverter(from_sparsity_t from, Request = {}) : sparsity(from) {
        if (from.symmetry != Symmetry::Unsymmetric && from.rows != from.cols)
            throw std::invalid_argument("Nonsquare matrix cannot be symmetric");
    }
    to_sparsity_t sparsity;
    [[nodiscard]] operator const to_sparsity_t &() const & { return sparsity; }
    [[nodiscard]] const to_sparsity_t &get_sparsity() const { return *this; }
    [[nodiscard]] size_t work_size() const { return 0; }
    template <class T>
    [[nodiscard]] std::span<T>
    convert_values(std::span<T> from_values,
                   std::span<std::remove_const_t<T>>) const {
        return from_values;
    }
};

template <class Index, class StorageIndex>
struct SparsityConverter<SparseCSC<Index, StorageIndex>, Dense> {
    using from_sparsity_t = SparseCSC<Index, StorageIndex>;
    using to_sparsity_t   = Dense;
    using Request         = SparsityConversionRequest<to_sparsity_t>;
    to_sparsity_t convert_sparsity(from_sparsity_t from, Request) {
        assert(check_uniqueness_csc(from.outer_ptr, from.inner_idx));
        if (from.symmetry != Symmetry::Unsymmetric && from.rows != from.cols)
            throw std::invalid_argument("Nonsquare matrix cannot be symmetric");
        return {
            .rows     = from.rows,
            .cols     = from.cols,
            .symmetry = from.symmetry,
        };
    }
    SparsityConverter(from_sparsity_t from, Request request = {})
        : from_sparsity(from), sparsity(convert_sparsity(from, request)) {}
    from_sparsity_t from_sparsity;
    to_sparsity_t sparsity;
    [[nodiscard]] operator const to_sparsity_t &() const & { return sparsity; }
    [[nodiscard]] const to_sparsity_t &get_sparsity() const { return *this; }
    [[nodiscard]] size_t work_size() const {
        return cast_sz(sparsity.rows) * cast_sz(sparsity.cols);
    }
    template <class T>
    [[nodiscard]] std::span<T>
    convert_values(std::span<T> from_values,
                   std::span<std::remove_const_t<T>> work) const {
        assert(work.size() == work_size());
        std::ranges::fill(work, T{});
        auto W = [&](index_t r, index_t c) -> std::remove_const_t<T> & {
            return work[cast_sz(r) + cast_sz(c) * cast_sz(sparsity.rows)];
        };
        size_t l = 0;
        for (index_t c = 0; c < from_sparsity.cols; ++c) {
            auto inner_start = from_sparsity.outer_ptr[cast_sz(c)];
            auto inner_end   = from_sparsity.outer_ptr[cast_sz(c) + 1];
            for (auto i = inner_start; i < inner_end; ++i) {
                auto r = from_sparsity.inner_idx[cast_sz(i)];
                switch (from_sparsity.symmetry) {
                    case Symmetry::Unsymmetric: W(r, c) = from_values[l]; break;
                    case Symmetry::Upper:
                        if (r > c)
                            throw std::invalid_argument(
                                "Invalid symmetric CSC matrix: "
                                "upper-triangular matrix should not "
                                "have elements below the diagonal");
                        W(c, r) = W(r, c) = from_values[l];
                        break;
                    case Symmetry::Lower:
                        if (r < c)
                            throw std::invalid_argument(
                                "Invalid symmetric CSC matrix: "
                                "lower-triangular matrix should not "
                                "have elements above the diagonal");
                        W(c, r) = W(r, c) = from_values[l];
                        break;
                    default: throw std::invalid_argument("Invalid symmetry");
                }
                ++l;
            }
        }
        return work;
    }
};

template <class Index>
struct SparsityConverter<SparseCOO<Index>, Dense> {
    using from_sparsity_t = SparseCOO<Index>;
    using to_sparsity_t   = Dense;
    using Request         = SparsityConversionRequest<to_sparsity_t>;
    to_sparsity_t convert_sparsity(from_sparsity_t from, Request) {
        assert(check_uniqueness_triplets(from.row_indices, from.col_indices));
        if (from.symmetry != Symmetry::Unsymmetric && from.rows != from.cols)
            throw std::invalid_argument("Nonsquare matrix cannot be symmetric");
        return {
            .rows     = from.rows,
            .cols     = from.cols,
            .symmetry = from.symmetry,
        };
    }
    SparsityConverter(from_sparsity_t from, Request request = {})
        : from_sparsity(from), sparsity(convert_sparsity(from, request)) {}
    from_sparsity_t from_sparsity;
    to_sparsity_t sparsity;
    [[nodiscard]] operator const to_sparsity_t &() const & { return sparsity; }
    [[nodiscard]] const to_sparsity_t &get_sparsity() const { return *this; }
    [[nodiscard]] size_t work_size() const {
        return cast_sz(sparsity.rows) * cast_sz(sparsity.cols);
    }
    template <class T>
    [[nodiscard]] std::span<T>
    convert_values(std::span<T> from_values,
                   std::span<std::remove_const_t<T>> work) const {
        assert(work.size() == work_size());
        std::ranges::fill(work, T{});
        auto W = [&](Index r, Index c) -> std::remove_const_t<T> & {
            return work[cast_sz(r) + cast_sz(c) * cast_sz(sparsity.rows)];
        };
        for (size_t l = 0; l < cast_sz(from_sparsity.nnz()); ++l) {
            auto r = from_sparsity.row_indices[l] - from_sparsity.first_index;
            auto c = from_sparsity.col_indices[l] - from_sparsity.first_index;
            switch (from_sparsity.symmetry) {
                case Symmetry::Unsymmetric: W(r, c) = from_values[l]; break;
                case Symmetry::Upper:
                    if (r > c)
                        throw std::invalid_argument(
                            "Invalid symmetric COO matrix: upper-triangular "
                            "matrix should not "
                            "have elements below the diagonal");
                    W(c, r) = W(r, c) = from_values[l];
                    break;
                case Symmetry::Lower:
                    if (r < c)
                        throw std::invalid_argument(
                            "Invalid symmetric COO matrix: lower-triangular "
                            "matrix should not "
                            "have elements above the diagonal");
                    W(c, r) = W(r, c) = from_values[l];
                    break;
                default: throw std::invalid_argument("Invalid symmetry");
            }
        }
        return work;
    }
};

template <class Index>
struct SparsityConversionRequest<SparseCOO<Index>> {
    /// Convert the index offset (zero for C/C++, one for Fortran).
    std::optional<Index> first_index = std::nullopt;
};

template <class Index>
struct SparsityConverter<Dense, SparseCOO<Index>> {
    using from_sparsity_t = Dense;
    using to_sparsity_t   = SparseCOO<Index>;
    using Request         = SparsityConversionRequest<to_sparsity_t>;
    to_sparsity_t convert_sparsity(from_sparsity_t from, Request request) {
        Index Δ = 0;
        if (request.first_index)
            Δ = *request.first_index;
        switch (from.symmetry) {
            case Symmetry::Unsymmetric: {
                auto nnz = cast_sz(from.rows) * cast_sz(from.cols);
                row_indices.resize(nnz);
                col_indices.resize(nnz);
                size_t l = 0;
                for (index_t c = 0; c < from.cols; ++c) {
                    for (index_t r = 0; r < from.rows; ++r) {
                        row_indices[l] = static_cast<Index>(r) + Δ;
                        col_indices[l] = static_cast<Index>(c) + Δ;
                        ++l;
                    }
                }
            } break;
            case Symmetry::Upper: {
                if (from.rows != from.cols)
                    throw std::invalid_argument(
                        "Nonsquare matrix cannot be symmetric");
                auto nnz = cast_sz(from.rows) * cast_sz(from.rows + 1) / 2;
                row_indices.resize(nnz);
                col_indices.resize(nnz);
                size_t l = 0;
                for (index_t c = 0; c < from.cols; ++c) {
                    for (index_t r = 0; r <= c; ++r) {
                        row_indices[l] = static_cast<Index>(r) + Δ;
                        col_indices[l] = static_cast<Index>(c) + Δ;
                        ++l;
                    }
                }
            } break;
            case Symmetry::Lower:
                throw std::invalid_argument(
                    "Lower-triangular symmetry currently not supported");
            default: throw std::invalid_argument("Invalid symmetry");
        }
        return {
            .rows        = from.rows,
            .cols        = from.cols,
            .symmetry    = from.symmetry,
            .row_indices = row_indices,
            .col_indices = col_indices,
            .order       = to_sparsity_t::SortedByColsAndRows,
            .first_index = request.first_index ? *request.first_index : 0,
        };
    }
    SparsityConverter(from_sparsity_t from, Request request = {})
        : sparsity(convert_sparsity(from, request)) {}
    std::vector<Index> row_indices, col_indices;
    to_sparsity_t sparsity;
    [[nodiscard]] operator const to_sparsity_t &() const & { return sparsity; }
    [[nodiscard]] const to_sparsity_t &get_sparsity() const { return *this; }
    [[nodiscard]] size_t work_size() const {
        if (sparsity.symmetry == Symmetry::Unsymmetric)
            return 0;
        return cast_sz(sparsity.nnz());
    }
    template <class T>
    [[nodiscard]] std::span<T>
    convert_values(std::span<T> from_values,
                   std::span<std::remove_const_t<T>> work) const {
        assert(work.size() == work_size());
        if (sparsity.symmetry == Symmetry::Unsymmetric) {
            return from_values;
        } else if (sparsity.symmetry == Symmetry::Upper) {
            auto f_col_above_diag = [&](index_t c) {
                return std::span{from_values}.subspan(
                    cast_sz(c) * cast_sz(sparsity.rows), cast_sz(c) + 1);
            };
            auto t = work.begin();
            for (index_t c = 0; c < sparsity.cols; ++c)
                std::ranges::copy_backward(f_col_above_diag(c), t += c + 1);
            return work;
        }
        std::unreachable();
    }
};

template <class IndexFrom, class StorageIndexFrom, class IndexTo>
struct SparsityConverter<SparseCSC<IndexFrom, StorageIndexFrom>,
                         SparseCOO<IndexTo>> {
    using from_sparsity_t = SparseCSC<IndexFrom, StorageIndexFrom>;
    using to_sparsity_t   = SparseCOO<IndexTo>;
    using Request         = SparsityConversionRequest<to_sparsity_t>;
    to_sparsity_t convert_sparsity(from_sparsity_t from, Request request) {
        IndexTo Δ = 0;
        if (request.first_index)
            Δ = *request.first_index;
        row_indices.resize(cast_sz(from.nnz()));
        col_indices.resize(cast_sz(from.nnz()));
        size_t l = 0;
        for (index_t c = 0; c < from.cols; ++c) {
            auto inner_start = from.outer_ptr[cast_sz(c)];
            auto inner_end   = from.outer_ptr[cast_sz(c) + 1];
            for (auto i = inner_start; i < inner_end; ++i) {
                auto r         = from.inner_idx[cast_sz(i)];
                row_indices[l] = static_cast<IndexTo>(r) + Δ;
                col_indices[l] = static_cast<IndexTo>(c) + Δ;
                ++l;
            }
        }
        return {
            .rows        = from.rows,
            .cols        = from.cols,
            .symmetry    = from.symmetry,
            .row_indices = row_indices,
            .col_indices = col_indices,
            .order       = from.order == from_sparsity_t::SortedRows
                               ? to_sparsity_t::SortedByColsAndRows
                               : to_sparsity_t::SortedByColsOnly,
            .first_index = request.first_index ? *request.first_index : 0,
        };
    }
    SparsityConverter(from_sparsity_t from, Request request = {})
        : sparsity(convert_sparsity(from, request)) {
        assert(check_uniqueness_triplets(sparsity.row_indices,
                                         sparsity.col_indices));
    }
    std::vector<IndexTo> row_indices, col_indices;
    to_sparsity_t sparsity;
    [[nodiscard]] operator const to_sparsity_t &() const & { return sparsity; }
    [[nodiscard]] const to_sparsity_t &get_sparsity() const { return *this; }
    [[nodiscard]] size_t work_size() const { return 0; }
    template <class T>
    [[nodiscard]] std::span<T>
    convert_values(std::span<T> from_values,
                   std::span<std::remove_const_t<T>>) const {
        return from_values;
    }
};

template <class IndexFrom, class IndexTo>
struct SparsityConverter<SparseCOO<IndexFrom>, SparseCOO<IndexTo>> {
    using from_sparsity_t = SparseCOO<IndexFrom>;
    using to_sparsity_t   = SparseCOO<IndexTo>;
    using Request         = SparsityConversionRequest<to_sparsity_t>;
    to_sparsity_t convert_sparsity(from_sparsity_t from, Request request) {
        IndexTo Δ = 0;
        if (request.first_index)
            Δ = *request.first_index - static_cast<IndexTo>(from.first_index);
        // Check if we can fully reuse the indices without changes
        if constexpr (std::is_same_v<IndexFrom, IndexTo>)
            if (Δ == 0)
                return from;
        // Otherwise, allocate space for shifted or converted indices
        row_indices.resize(cast_sz(from.nnz()));
        col_indices.resize(cast_sz(from.nnz()));
        auto cvt_idx = [Δ](auto i) { return static_cast<IndexTo>(i) + Δ; };
        std::ranges::transform(from.row_indices, row_indices.begin(), cvt_idx);
        std::ranges::transform(from.col_indices, col_indices.begin(), cvt_idx);
        return {
            .rows        = from.rows,
            .cols        = from.cols,
            .symmetry    = from.symmetry,
            .row_indices = row_indices,
            .col_indices = col_indices,
            .order = static_cast<typename to_sparsity_t::Order>(from.order),
            .first_index = request.first_index
                               ? *request.first_index
                               : static_cast<IndexTo>(from.first_index),
        };
    }
    SparsityConverter(from_sparsity_t from, Request request = {})
        : sparsity(convert_sparsity(from, request)) {
        assert(check_uniqueness_triplets(sparsity.row_indices,
                                         sparsity.col_indices));
    }
    std::vector<IndexTo> row_indices, col_indices;
    to_sparsity_t sparsity;
    [[nodiscard]] operator const to_sparsity_t &() const & { return sparsity; }
    [[nodiscard]] const to_sparsity_t &get_sparsity() const { return *this; }
    [[nodiscard]] size_t work_size() const { return 0; }
    template <class T>
    [[nodiscard]] std::span<T>
    convert_values(std::span<T> from_values,
                   std::span<std::remove_const_t<T>>) const {
        return from_values;
    }
};

template <class Index, class StorageIndex>
struct SparsityConversionRequest<SparseCSC<Index, StorageIndex>> {
    /// Sort the indices.
    std::optional<typename SparseCSC<Index, StorageIndex>::Order> order =
        std::nullopt;
};

template <class IndexFrom, class IndexTo, class StorageIndexTo>
struct SparsityConverter<SparseCOO<IndexFrom>,
                         SparseCSC<IndexTo, StorageIndexTo>> {
    using to_sparsity_t   = SparseCSC<IndexTo, StorageIndexTo>;
    using from_sparsity_t = SparseCOO<IndexFrom>;
    using Request         = SparsityConversionRequest<to_sparsity_t>;
    to_sparsity_t convert_sparsity([[maybe_unused]] from_sparsity_t from,
                                   [[maybe_unused]] Request request) {
        // Optional copy of the indices (needed for sorting)
        std::vector<IndexFrom> row_indices, col_indices;
        [[maybe_unused]] auto prepare_sort = [&] {
            row_indices.resize(cast_sz(from.nnz()));
            col_indices.resize(cast_sz(from.nnz()));
            permutation.resize(cast_sz(from.nnz()));
            std::ranges::copy(from.row_indices, row_indices.begin());
            std::ranges::copy(from.col_indices, col_indices.begin());
            std::iota(begin(permutation), end(permutation), StorageIndexTo{0});
        };
        // Sort the indices
        typename to_sparsity_t::Order order;
        if (request.order && *request.order == to_sparsity_t::SortedRows) {
            order = to_sparsity_t::SortedRows;
            switch (from.order) {
                case from_sparsity_t::SortedByColsAndRows:
                    // No sorting necessary
                    break;
                case from_sparsity_t::Unsorted: [[fallthrough]];
                case from_sparsity_t::SortedByColsOnly: [[fallthrough]];
                case from_sparsity_t::SortedByRowsAndCols: [[fallthrough]];
                case from_sparsity_t::SortedByRowsOnly:
#if __cpp_lib_ranges_zip >= 202110L
                    prepare_sort();
                    sort_triplets(row_indices, col_indices, permutation);
                    break;
#else
                    throw unsupported_conversion(
                        "Sorting is SparseCOO is not supported. Recompile with "
                        "C++23-compliant compiler");
#endif
                default: throw std::invalid_argument("Invalid order");
            }
        } else {
            switch (from.order) {
                case from_sparsity_t::SortedByColsAndRows:
                    order = to_sparsity_t::SortedRows;
                    // No sorting necessary
                    break;
                case from_sparsity_t::SortedByColsOnly:
                    order = to_sparsity_t::Unsorted;
                    // No sorting necessary
                    break;
                case from_sparsity_t::Unsorted: [[fallthrough]];
                case from_sparsity_t::SortedByRowsAndCols: [[fallthrough]];
                case from_sparsity_t::SortedByRowsOnly:
#if __cpp_lib_ranges_zip >= 202110L
                    order = to_sparsity_t::Unsorted;
                    prepare_sort();
                    sort_triplets_col(row_indices, col_indices, permutation);
                    break;
#else
                    throw unsupported_conversion(
                        "Sorting is SparseCOO is not supported. Recompile with "
                        "C++23-compliant compiler");
#endif
                default: throw std::invalid_argument("Invalid order");
            }
        }
        assert(!request.order || *request.order == order);
        if (std::ranges::is_sorted(permutation))
            permutation.clear();
        // Convert the sorted COO format to CSC
        inner_idx.resize(cast_sz(from.nnz()));
        outer_ptr.resize(cast_sz(from.cols) + 1);
        convert_triplets_to_ccs(
            row_indices.empty() ? from.row_indices : std::span{row_indices},
            col_indices.empty() ? from.col_indices : std::span{col_indices},
            inner_idx, outer_ptr, from.first_index);
        return {
            .rows      = from.rows,
            .cols      = from.cols,
            .symmetry  = from.symmetry,
            .inner_idx = inner_idx,
            .outer_ptr = outer_ptr,
            .order     = order,
        };
    }
    SparsityConverter(from_sparsity_t from, Request request = {})
        : sparsity(convert_sparsity(from, request)) {
        assert(check_uniqueness_csc(sparsity.outer_ptr, sparsity.inner_idx));
    }
    std::vector<IndexTo> inner_idx;
    std::vector<StorageIndexTo> outer_ptr;
    std::vector<StorageIndexTo> permutation;
    to_sparsity_t sparsity;
    [[nodiscard]] operator const to_sparsity_t &() const & { return sparsity; }
    [[nodiscard]] const to_sparsity_t &get_sparsity() const { return *this; }
    [[nodiscard]] size_t work_size() const { return permutation.size(); }
    template <class T>
    [[nodiscard]] std::span<T>
    convert_values(std::span<T> from_values,
                   std::span<std::remove_const_t<T>> work) const {
        assert(work.size() == work_size());
        if (permutation.size() > 0) {
            for (size_t i = 0; i < permutation.size(); ++i)
                work[i] = from_values[cast_sz(permutation[i])];
            return work;
        } else {
            return from_values;
        }
    }
};

template <class IndexFrom, class StorageIndexFrom, class IndexTo,
          class StorageIndexTo>
struct SparsityConverter<SparseCSC<IndexFrom, StorageIndexFrom>,
                         SparseCSC<IndexTo, StorageIndexTo>> {
    using to_sparsity_t   = SparseCSC<IndexTo, StorageIndexTo>;
    using from_sparsity_t = SparseCSC<IndexFrom, StorageIndexFrom>;
    using Request         = SparsityConversionRequest<to_sparsity_t>;
    to_sparsity_t convert_sparsity([[maybe_unused]] from_sparsity_t from,
                                   Request request) {
        auto cvt_in  = [](auto i) { return static_cast<IndexTo>(i); };
        auto cvt_out = [](auto i) { return static_cast<StorageIndexTo>(i); };
        auto convert_inner = [&] {
            inner_idx.resize(from.inner_idx.size());
            std::ranges::transform(from.inner_idx, inner_idx.begin(), cvt_in);
        };
        auto convert_outer = [&] {
            outer_ptr.resize(from.outer_ptr.size());
            std::ranges::transform(from.outer_ptr, outer_ptr.begin(), cvt_out);
        };
        auto sort_indices = [&] {
#if __cpp_lib_ranges_zip >= 202110L
            permutation.resize(from.inner_idx.size());
            std::iota(begin(permutation), end(permutation), StorageIndexTo{0});
            sort_rows_csc(outer_ptr, inner_idx, permutation);
#else
            throw unsupported_conversion(
                "Sorting is SparseCSC is not supported. Recompile with "
                "C++23-compliant compiler");
#endif
        };
        using Order       = typename to_sparsity_t::Order;
        bool need_sorting = false;
        auto order        = static_cast<Order>(from.order);
        // Check whether the user requested the indices to be sorted
        if (request.order && *request.order == to_sparsity_t::SortedRows) {
            order = to_sparsity_t::SortedRows;
            switch (from.order) {
                case from_sparsity_t::Unsorted: need_sorting = true; break;
                case from_sparsity_t::SortedRows: need_sorting = false; break;
                default: throw std::invalid_argument("Invalid order");
            }
        }
        // Convert and sort, or only convert the indices
        if (need_sorting) {
            convert_inner();
            convert_outer();
            sort_indices();
        } else {
            if constexpr (!std::is_same_v<IndexFrom, IndexTo>)
                convert_inner();
            if constexpr (!std::is_same_v<StorageIndexFrom, StorageIndexTo>)
                convert_outer();
        }
        // Remove unnecessary permutations
        if (std::ranges::is_sorted(permutation))
            permutation.clear();
        // Can we avoid copying?
        // If the index types are the same, we may be able to reuse the storage
        auto get_inner_idx = [&] {
            if constexpr (std::is_same_v<IndexFrom, IndexTo>)
                return inner_idx.empty() ? from.inner_idx
                                         : std::span{inner_idx};
            else
                return std::span{inner_idx};
        };
        auto get_outer_ptr = [&] {
            if constexpr (std::is_same_v<StorageIndexFrom, StorageIndexTo>)
                return outer_ptr.empty() ? from.outer_ptr
                                         : std::span{outer_ptr};
            else
                return std::span{outer_ptr};
        };
        return {
            .rows      = from.rows,
            .cols      = from.cols,
            .symmetry  = from.symmetry,
            .inner_idx = get_inner_idx(),
            .outer_ptr = get_outer_ptr(),
            .order     = order,
        };
    }
    SparsityConverter(from_sparsity_t from, Request request = {})
        : sparsity(convert_sparsity(from, request)) {
        assert(check_uniqueness_csc(sparsity.outer_ptr, sparsity.inner_idx));
    }
    std::vector<IndexTo> inner_idx;
    std::vector<StorageIndexTo> outer_ptr;
    std::vector<StorageIndexTo> permutation;
    to_sparsity_t sparsity;
    [[nodiscard]] operator const to_sparsity_t &() const & { return sparsity; }
    [[nodiscard]] const to_sparsity_t &get_sparsity() const { return *this; }
    [[nodiscard]] size_t work_size() const { return permutation.size(); }
    template <class T>
    [[nodiscard]] std::span<T>
    convert_values(std::span<T> from_values,
                   std::span<std::remove_const_t<T>> work) const {
        assert(work.size() == work_size());
        if (permutation.size() > 0) {
            for (size_t i = 0; i < permutation.size(); ++i)
                work[i] = from_values[cast_sz(permutation[i])];
            return work;
        } else {
            return from_values;
        }
    }
};

template <class Index, class StorageIndex>
struct SparsityConverter<Dense, SparseCSC<Index, StorageIndex>> {
    using to_sparsity_t   = SparseCSC<Index, StorageIndex>;
    using from_sparsity_t = Dense;
    using Request         = SparsityConversionRequest<to_sparsity_t>;
    to_sparsity_t convert_sparsity([[maybe_unused]] from_sparsity_t from,
                                   Request) {
        auto cvt_out = [](auto i) { return static_cast<StorageIndex>(i); };
        auto cvt_in  = [](auto i) { return static_cast<Index>(i); };
        switch (from.symmetry) {
            case Symmetry::Unsymmetric: {
                auto nnz = cast_sz(from.rows) * cast_sz(from.cols);
                inner_idx.resize(nnz);
                outer_ptr.resize(cast_sz(from.cols) + 1);
                size_t l = 0;
                for (index_t c = 0; c < from.cols; ++c) {
                    outer_ptr[cast_sz(c)] = cvt_out(l);
                    for (index_t r = 0; r < from.rows; ++r) {
                        inner_idx[l] = cvt_in(r);
                        ++l;
                    }
                }
                outer_ptr[cast_sz(from.cols)] = cvt_out(l);
            } break;
            case Symmetry::Upper: {
                if (from.rows != from.cols)
                    throw std::invalid_argument(
                        "Nonsquare matrix cannot be symmetric");
                auto nnz = cast_sz(from.rows) * (cast_sz(from.rows) + 1) / 2;
                inner_idx.resize(nnz);
                outer_ptr.resize(cast_sz(from.cols) + 1);
                size_t l = 0;
                for (index_t c = 0; c < from.cols; ++c) {
                    outer_ptr[cast_sz(c)] = cvt_out(l);
                    for (index_t r = 0; r <= c; ++r) {
                        inner_idx[l] = cvt_in(r);
                        ++l;
                    }
                }
                outer_ptr[cast_sz(from.cols)] = cvt_out(l);
            } break;
            case Symmetry::Lower:
                throw std::invalid_argument(
                    "Lower-triangular symmetry currently not supported");
            default: throw std::invalid_argument("Invalid symmetry");
        }
        return {
            .rows      = from.rows,
            .cols      = from.cols,
            .symmetry  = from.symmetry,
            .inner_idx = inner_idx,
            .outer_ptr = outer_ptr,
            .order     = to_sparsity_t::SortedRows,
        };
    }
    SparsityConverter(from_sparsity_t from, Request request = {})
        : sparsity(convert_sparsity(from, request)) {}
    std::vector<Index> inner_idx;
    std::vector<StorageIndex> outer_ptr;
    to_sparsity_t sparsity;
    [[nodiscard]] operator const to_sparsity_t &() const & { return sparsity; }
    [[nodiscard]] const to_sparsity_t &get_sparsity() const { return *this; }
    [[nodiscard]] size_t work_size() const {
        if (sparsity.symmetry == Symmetry::Unsymmetric)
            return 0;
        return cast_sz(sparsity.nnz());
    }
    template <class T>
    [[nodiscard]] std::span<T>
    convert_values(std::span<T> from_values,
                   std::span<std::remove_const_t<T>> work) const {
        assert(work.size() == work_size());
        if (sparsity.symmetry == Symmetry::Unsymmetric) {
            return from_values;
        } else if (sparsity.symmetry == Symmetry::Upper) {
            auto f_col_above_diag = [&](index_t c) {
                return std::span{from_values}.subspan(
                    cast_sz(c) * cast_sz(sparsity.rows), cast_sz(c) + 1);
            };
            auto t = work.begin();
            for (index_t c = 0; c < sparsity.cols; ++c)
                std::ranges::copy_backward(f_col_above_diag(c), t += c + 1);
            return work;
        } else {
            assert(false);
            return {};
        }
    }
};

namespace detail {
template <class To, class>
struct ConverterVariantHelper;

template <class To, class... Froms>
struct ConverterVariantHelper<To, std::variant<Froms...>> {
    using type = std::variant<SparsityConverter<Froms, To>...>;
};
} // namespace detail

template <class To>
using ConverterVariant =
    detail::ConverterVariantHelper<To, SparsityVariant>::type;

/// Converts any supported matrix storage format to the given format.
/// @see @ref Sparsity
template <class To>
struct SparsityConverter<Sparsity, To> {
    using from_sparsity_t = Sparsity;
    using to_sparsity_t   = To;
    using Request         = SparsityConversionRequest<to_sparsity_t>;
    SparsityConverter(Sparsity from, Request request = {})
        : converter{std::visit(wrap_converter(request), from.value)} {}
    ConverterVariant<To> converter;
    [[nodiscard]] operator const to_sparsity_t &() const {
        return std::visit(
            [](const auto &c) -> const to_sparsity_t & { return c; },
            converter);
    }
    [[nodiscard]] const to_sparsity_t &get_sparsity() const { return *this; }
    [[nodiscard]] size_t work_size() const {
        return std::visit([](const auto &c) { return c.work_size(); },
                          converter);
    }
    template <class T>
    [[nodiscard]] std::span<T>
    convert_values(std::span<T> from_values,
                   std::span<std::remove_const_t<T>> work) const {
        const auto visitor = [&](const auto &c) {
            return c.template convert_values<T>(from_values, work);
        };
        return std::visit(visitor, converter);
    }
    /// Call @p evaluator, then convert the values and write the result into
    /// @p result, while minimizing the number of copies.
    /// @return The allocated workspace that can be passed as the @p work
    ///         argument during the next invocation (to minimize allocations).
    /// @todo   Write tests.
    template <class T, class E>
    std::vector<T> convert_values_into(std::span<T> result, E &&evaluator,
                                       std::vector<T> work = {}) const {
        const auto eval_size = static_cast<size_t>(get_nnz(get_sparsity()));
        // If the work size is zero, conversion can be done in-place, so
        // evaluate directly into the result.
        if (work_size() == 0 && result.size() >= eval_size) {
            std::forward<E>(evaluator)(result.first(eval_size));
            const auto visitor = [&](const auto &c) {
                return c.template convert_values<T>(result, work);
            };
            [[maybe_unused]] auto r = std::visit(visitor, converter);
            assert(r.data() == result.data());
        }
        // If the conversion cannot be done in-place, allocate a workspace
        // vector, then evaluate into that vector, and then convert the values
        // into the result.
        else {
            work.resize(eval_size);
            std::forward<E>(evaluator)(std::span<T>{work});
            const auto visitor = [&](const auto &c) {
                return c.template convert_values<T>(work, result);
            };
            [[maybe_unused]] auto r = std::visit(visitor, converter);
            assert(r.data() == result.data());
        }
        return std::move(work);
    }
    template <class T>
    [[nodiscard]] std::vector<std::remove_const_t<T>>
    convert_values_copy(std::span<T> from_values) const {
        std::vector<T> work(work_size());
        auto result = convert_values<std::add_const_t<T>>(from_values, work);
        if (result.data() != work.data())
            work = {result.begin(), result.end()};
        return work;
    }

  private:
    static auto wrap_converter(Request request) {
        return [request]<class From>(const From &from) -> ConverterVariant<To> {
            return SparsityConverter<From, To>{from, request};
        };
    }
};

} // namespace guanaqo::linalg::sparsity
