#pragma once

#include <algorithm>
#include <cassert>
#include <numeric>
#include <ranges>

#if __cpp_lib_ranges_zip >= 202110L
#define GUANAQO_SPARSE_SUPPORTS_SORTING 1
#else
#define GUANAQO_SPARSE_SUPPORTS_SORTING 0
#endif

namespace guanaqo::linalg {

void convert_triplets_to_ccs(const auto &rows, const auto &cols,
                             auto &&inner_idx, auto &&outer_ptr,
                             auto idx_0 = 0) {
    using Index        = std::remove_cvref_t<decltype(*std::begin(inner_idx))>;
    using IndexFrom    = std::remove_cvref_t<decltype(*std::begin(cols))>;
    using StorageIndex = std::remove_cvref_t<decltype(*std::begin(outer_ptr))>;
    static_assert(std::is_same_v<decltype(idx_0), IndexFrom>);
    // Inner indices: simply copy the row indices
    assert(std::size(rows) == std::size(inner_idx));
    auto cvt_rows = [&](auto r) {
        return static_cast<Index>(r) - static_cast<Index>(idx_0);
    };
    std::ranges::ref_view rows_vw = rows;
    std::ranges::transform(rows_vw, std::begin(inner_idx), cvt_rows);
    // Outer indices: need to count the number of nonzeros per column
    auto num_cols  = static_cast<IndexFrom>(std::size(outer_ptr));
    auto cols_iter = std::begin(cols);
    for (IndexFrom c = 0; c < num_cols; ++c) {
        cols_iter     = std::lower_bound(cols_iter, std::end(cols), c + idx_0);
        auto cz       = static_cast<size_t>(c);
        outer_ptr[cz] = static_cast<StorageIndex>(cols_iter - std::begin(cols));
    }
}

#if GUANAQO_SPARSE_SUPPORTS_SORTING

namespace detail {
template <auto cmp, class... Ts>
void sort_triplets_impl(Ts &&...triplets) {
    auto indices = std::views::zip(std::ranges::ref_view{triplets}...);
    std::ranges::sort(indices, cmp);
}
} // namespace detail

/// Sort the (row, column, value) triplets, by column index first, then row.
template <class... Ts>
void sort_triplets(Ts &&...triplets) {
    // Sort the indices (by column first, then row)
    static constexpr auto cmp = [](const auto &a, const auto &b) {
        return std::tie(std::get<1>(a), std::get<0>(a)) <
               std::tie(std::get<1>(b), std::get<0>(b));
    };
    detail::sort_triplets_impl<cmp>(std::forward<Ts>(triplets)...);
}

/// Sort the (row, column, value) triplets by column index.
template <class... Ts>
void sort_triplets_col(Ts &&...triplets) {
    // Sort the indices (by column)
    static constexpr auto cmp = [](const auto &a, const auto &b) {
        return std::get<1>(a) < std::get<1>(b);
    };
    detail::sort_triplets_impl<cmp>(std::forward<Ts>(triplets)...);
}

/// Given a sparse compressed-column storage matrix, sort all row indices
/// within each column.
template <class Outer, class... Inners>
void sort_rows_csc(const Outer &outer_ptr, Inners &&...inners) {
    if (outer_ptr.size() == 0)
        return;
    // Sort the indices (by row)
    static constexpr auto cmp = [](const auto &a, const auto &b) {
        return std::get<0>(a) < std::get<0>(b);
    };
    // TODO: could be parallelized
    for (decltype(outer_ptr.size()) c = 0; c < outer_ptr.size() - 1; ++c) {
        auto inner_start = outer_ptr[c];
        auto inner_end   = outer_ptr[c + 1];
        auto indices     = std::views::zip(
            std::ranges::subrange{std::ranges::begin(inners) + inner_start,
                                  std::ranges::begin(inners) + inner_end}...);
        std::ranges::sort(indices, cmp);
    }
}

/// Check that no two entries with the same row and column index exist in
/// the given sparse coordinate list matrix. Assumes sorted indices.
template <class... Ts>
bool check_uniqueness_triplets(Ts &&...triplets) {
    auto indices = std::views::zip(std::ranges::ref_view{triplets}...);
    return std::ranges::adjacent_find(indices, std::equal_to<>{}) ==
           std::ranges::end(indices);
}

#endif // GUANAQO_SPARSE_SUPPORTS_SORTING

/// Check that no two entries with the same row and column index exist in
/// the given sparse compressed-column storage matrix. Assumes sorted indices.
template <class Outer, class Inner>
bool check_uniqueness_csc(const Outer &outer_ptr, const Inner inner) {
    if (outer_ptr.size() == 0)
        return true;
    auto is_unique_col = [&](auto &inner_start) {
        auto inner_end = (&inner_start)[1];
        auto indices =
            std::ranges::subrange{std::ranges::begin(inner) + inner_start,
                                  std::ranges::begin(inner) + inner_end};
        return std::ranges::adjacent_find(indices, std::equal_to<>{}) ==
               std::ranges::end(indices);
    };
    return std::transform_reduce(std::begin(outer_ptr), std::end(outer_ptr) - 1,
                                 true, std::logical_and<>{}, is_unique_col);
}

} // namespace guanaqo::linalg
