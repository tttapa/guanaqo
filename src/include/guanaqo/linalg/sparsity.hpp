#pragma once

/// @file
/// @ingroup linalg_sparsity
/// Sparse and dense sparsity descriptors.

#include <guanaqo/linalg/config.hpp>

#include <cassert>
#include <cstdint>
#include <span>
#include <variant>

namespace guanaqo::linalg::sparsity {

/// @addtogroup linalg_sparsity
/// @{

/// Describes the symmetry of matrices.
enum class Symmetry : uint8_t {
    Unsymmetric = 0, ///< No symmetry.
    Upper       = 1, ///< Symmetric, upper-triangular part is stored.
    Lower       = 2, ///< Symmetric, lower-triangular part is stored.
};

inline const char *enum_name(Symmetry s) {
    switch (s) {
        case Symmetry::Unsymmetric: return "Unsymmetric";
        case Symmetry::Upper: return "Upper";
        case Symmetry::Lower: return "Lower";
        default: return "<invalid>";
    }
}

/// Dense matrix structure. Stores all elements in column-major storage.
/// Symmetric dense matrices always store all elements.
struct Dense {
    length_t rows = 0, cols = 0;
    Symmetry symmetry = Symmetry::Unsymmetric;
};

/// Sparse compressed-column structure (CCS or CSC).
template <class Index, class StorageIndex>
struct SparseCSC {
    using index_t         = Index;
    using storage_index_t = StorageIndex;
    length_t rows = 0, cols = 0;
    Symmetry symmetry = Symmetry::Unsymmetric;
    std::span<const index_t> inner_idx{};
    std::span<const storage_index_t> outer_ptr{};
    enum Order : uint8_t {
        /// The row indices are not sorted.
        Unsorted = 0,
        /// Within each column, all row indices are sorted in ascending order.
        SortedRows = 1,
    };
    Order order = Unsorted;

    /// Get the number of structurally nonzero elements.
    [[nodiscard]] length_t nnz() const {
        assert(static_cast<length_t>(outer_ptr.size()) == cols + 1);
        auto nnz = static_cast<length_t>(inner_idx.size());
        assert(static_cast<length_t>(outer_ptr.back()) == nnz);
        return nnz;
    }
};

/// Sparse coordinate list structure (COO).
template <class Index = index_t>
struct SparseCOO {
    using index_t = Index;
    length_t rows = 0, cols = 0;
    Symmetry symmetry = Symmetry::Unsymmetric;
    std::span<const index_t> row_indices{};
    std::span<const index_t> col_indices{};
    enum Order : uint8_t {
        /// The indices are not sorted.
        Unsorted = 0,
        /// The indices are sorted by column first, and within each column, the
        /// rows are sorted as well.
        SortedByColsAndRows = 1,
        /// The indices are sorted by column, but the rows within each column
        /// are not sorted.
        SortedByColsOnly = 2,
        /// The indices are sorted by row first, and within each row, the
        /// columns are sorted as well.
        SortedByRowsAndCols = 3,
        /// The indices are sorted by row, but the columns within each row are
        /// not sorted.
        SortedByRowsOnly = 4,
    };
    Order order         = Unsorted;
    index_t first_index = 0; ///< Zero for C/C++, one for Fortran.

    /// Get the number of structurally nonzero elements.
    [[nodiscard]] length_t nnz() const {
        assert(row_indices.size() == col_indices.size());
        return static_cast<length_t>(row_indices.size());
    }
};

/// Helper for @ref Sparsity.
using SparsityVariant = std::variant< //
    Dense,                            //
    SparseCSC<int, int>,              //
    SparseCSC<int, long>,             //
    SparseCSC<int, long long>,        //
    SparseCSC<long, long>,            //
    SparseCSC<long, long long>,       //
    SparseCSC<long long, long long>,  //
    SparseCOO<int>,                   //
    SparseCOO<long>,                  //
    SparseCOO<long long>              //
    >;

/// Stores any of the supported sparsity patterns.
/// @see @ref SparsityConverter<Sparsity<Conf>, To>
struct Sparsity {
    Sparsity(std::convertible_to<SparsityVariant> auto value)
        : value{std::move(value)} {}
    SparsityVariant value;
};

/// @}

namespace detail {
template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
} // namespace detail

/// @addtogroup linalg_sparsity
/// @{

/// Returns true if the sparsity pattern represents a dense matrix.
inline bool is_dense(const Sparsity &sp) {
    auto visitor = detail::overloaded{
        [](const Dense &) { return true; },
        [](const auto &) { return false; },
    };
    return std::visit(visitor, sp.value);
}

/// Get the number of structurally nonzero elements.
inline length_t get_nnz(const Sparsity &sp) {
    auto visitor = detail::overloaded{
        [](const Dense &d) { return d.rows * d.cols; },
        [](const auto &s) { return s.nnz(); },
    };
    return std::visit(visitor, sp.value);
}

/// Returns the symmetry of the sparsity pattern.
inline Symmetry get_symmetry(const Sparsity &sp) {
    return std::visit([](const auto &s) { return s.symmetry; }, sp.value);
}

/// Returns the number of rows of the sparsity pattern.
inline length_t num_rows(const Sparsity &sp) {
    return std::visit([](const auto &s) { return s.rows; }, sp.value);
}

/// Returns the number of rows of the sparsity pattern.
inline length_t num_cols(const Sparsity &sp) {
    return std::visit([](const auto &s) { return s.cols; }, sp.value);
}

/// @}

} // namespace guanaqo::linalg::sparsity

namespace guanaqo::linalg {
using sparsity::Sparsity;
} // namespace guanaqo::linalg
