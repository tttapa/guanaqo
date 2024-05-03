#pragma once

#include <guanaqo/linalg/config.hpp>
#include <guanaqo/linalg/sparsity.hpp>

namespace guanaqo::linalg {

/// Non-owning view of dense and various formats of sparse matrix.
template <class T>
struct MatrixView {
    /// Description of sparsity format.
    using Sparsity = sparsity::Sparsity;

    /// Constructs an empty 0×0 dense matrix.
    MatrixView() : sparsity{sparsity::Dense{0, 0}} {}
    /// General constructor from flattened values and view of the sparsity
    /// pattern.
    /// @param  values      View of flattened array of (column-major) values.
    /// @param  sparsity    View of the sparsity pattern (dense, CSC or COO).
    MatrixView(std::span<const T> values, Sparsity sparsity)
        : values{values}, sparsity{sparsity} {}
    /// Dense matrix constructor (column-major).
    /// @param  values      View of flattened array of (column-major) values.
    /// @param  rows        Number of rows.
    /// @param  cols        Number of cols.
    /// @param  symmetry    Symmetry of the matrix.
    MatrixView(std::span<const T> values, length_t rows, length_t cols,
               sparsity::Symmetry symmetry = sparsity::Symmetry::Unsymmetric)
        : MatrixView{values, sparsity::Dense{rows, cols, symmetry}} {}

    /// Non-owning view of the flattened array of (column-major) values.
    std::span<const T> values;
    /// Non-owning view of the sparsity pattern of the matrix.
    Sparsity sparsity;
};

} // namespace guanaqo::linalg
