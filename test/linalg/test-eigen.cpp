#include <gtest/gtest.h>
#include <numeric>

#include <guanaqo/eigen/span.hpp>
#include <guanaqo/linalg/eigen/sparse.hpp>
#include <guanaqo/linalg/sparsity.hpp>

namespace sp = guanaqo::linalg::sparsity;

TEST(sparsity, eigen) {
    using index_t   = short;
    using SparseCSC = sp::SparseCSC<index_t, index_t>;
    Eigen::VectorX<index_t> row_idcs{7};
    Eigen::VectorX<index_t> col_ptrs{5};
    row_idcs << 0, 1, 1, 2, 0, 2, 0;
    col_ptrs << 0, 2, 4, 6, 7;
    SparseCSC sparsity{
        .rows      = 3,
        .cols      = 4,
        .symmetry  = sp::Symmetry::Unsymmetric,
        .inner_idx = guanaqo::as_span(row_idcs),
        .outer_ptr = guanaqo::as_span(col_ptrs),
        .order     = SparseCSC::SortedRows,
    };
    Eigen::VectorX<double> values{7};
    std::iota(values.begin(), values.end(), 1);

    // Convert from guanaqo to Eigen view
    auto eigen = as_eigen(sparsity, guanaqo::as_span(values));
    ASSERT_EQ(eigen.rows(), 3);
    ASSERT_EQ(eigen.cols(), 4);
    ASSERT_EQ(eigen.nonZeros(), 7);
    // Compare contents by iterating over the Eigen matrix
    std::vector<std::tuple<index_t, index_t, double>> result;
    std::vector<std::tuple<index_t, index_t, double>> expected{
        {0, 0, 1}, {1, 0, 2}, {1, 1, 3}, {2, 1, 4},
        {0, 2, 5}, {2, 2, 6}, {0, 3, 7},
    };
    for (decltype(eigen)::Index k = 0; k < eigen.outerSize(); ++k)
        for (decltype(eigen)::InnerIterator it(eigen, k); it; ++it)
            result.emplace_back(it.row(), it.col(), it.value());
    std::cout << eigen;
    EXPECT_EQ(result, expected);

    // Convert from Eigen view back to guanaqo
    auto sparsity_2 = sp::as_sparsity(eigen);
    ASSERT_EQ(sparsity_2.rows, sparsity.rows);
    ASSERT_EQ(sparsity_2.cols, sparsity.cols);
    ASSERT_EQ(sparsity_2.nnz(), sparsity.nnz());
    ASSERT_EQ(sparsity_2.inner_idx.size(), sparsity.inner_idx.size());
    ASSERT_EQ(sparsity_2.outer_ptr.size(), sparsity.outer_ptr.size());
    EXPECT_EQ(sparsity_2.inner_idx.data(), sparsity.inner_idx.data());
    EXPECT_EQ(sparsity_2.outer_ptr.data(), sparsity.outer_ptr.data());
    EXPECT_EQ(sparsity_2.symmetry, sp::Symmetry::Unsymmetric);
    EXPECT_EQ(sparsity_2.order, decltype(sparsity_2)::SortedRows);
}
