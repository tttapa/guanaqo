#pragma once

#include <guanaqo/linalg/sparsity.hpp>

#include <Eigen/Sparse>

namespace guanaqo::linalg::sparsity {

/// Convert a guanaqo::MatrixView to an Eigen::Matrix view.
template <class I, class T>
auto as_eigen(const SparseCSC<I, I> sparsity, std::span<T> values) {
    using Scalar = std::remove_const_t<T>;
    using Mat    = Eigen::SparseMatrix<Scalar, 0, I>;
    using Index  = typename Mat::Index;
    return Eigen::Map<const Mat>{
        static_cast<Index>(sparsity.rows),
        static_cast<Index>(sparsity.cols),
        static_cast<Index>(sparsity.nnz()),
        sparsity.outer_ptr.data(),
        sparsity.inner_idx.data(),
        values.data(),
        nullptr,
    };
}

template <class Derived>
    requires(!Derived::IsRowMajor)
auto as_sparsity(const Eigen::SparseMatrixBase<Derived> &M,
                 Symmetry symmetry = Symmetry::Unsymmetric) {
    using I     = typename Derived::StorageIndex;
    using SpCSC = SparseCSC<I, I>;
    std::span<const I> inner{M.derived().innerIndexPtr(),
                             static_cast<size_t>(M.derived().nonZeros())};
    std::span<const I> outer{M.derived().outerIndexPtr(),
                             static_cast<size_t>(M.derived().outerSize()) + 1};
    return SpCSC{
        .rows      = static_cast<length_t>(M.derived().rows()),
        .cols      = static_cast<length_t>(M.derived().cols()),
        .symmetry  = symmetry,
        .inner_idx = inner,
        .outer_ptr = outer,
        .order     = SpCSC::SortedRows,
    };
}

} // namespace guanaqo::linalg::sparsity
