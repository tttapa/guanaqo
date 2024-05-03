#include <linalg/test-sparsity.hpp>

// Dense to COO ----------------------------------------------------------------

template <class Index>
class SparsityDenseToCOOTest : public ::testing::Test {
  protected:
    using dst_index_t = Index;
    using Source      = sp::Dense;
    using Result      = sp::SparseCOO<dst_index_t>;
    using converter_t = sp::SparsityConverter<Sparsity, Result>;

    static converter_t
    get_converter(sp::SparsityConversionRequest<Result> req) {
        Source src{.rows = 3, .cols = 4, .symmetry = sp::Symmetry::Unsymmetric};
        return {Sparsity{src}, req};
    }
    static converter_t
    get_converter_upper(sp::SparsityConversionRequest<Result> req) {
        Source src{.rows = 4, .cols = 4, .symmetry = sp::Symmetry::Upper};
        return {Sparsity{src}, req};
    }
};

using DenseToCOOIndices = ::testing::Types<int, long, long long>;
TYPED_TEST_SUITE(SparsityDenseToCOOTest, DenseToCOOIndices);

TYPED_TEST(SparsityDenseToCOOTest, convertDenseToCOO) {
    using index_t      = TypeParam;
    auto converter     = this->get_converter({});
    const auto &result = converter.get_sparsity();
    ASSERT_EQ(result.nnz(), 12);
    Eigen::VectorX<index_t> expected_row_idcs(12), expected_col_idcs(12);
    expected_row_idcs << 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2;
    expected_col_idcs << 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3;
    EXPECT_THAT(egn(result.row_indices), EigenEqual(expected_row_idcs));
    EXPECT_THAT(egn(result.col_indices), EigenEqual(expected_col_idcs));
    mat m(3, 4);
    m << 11, 12, 13, 14, 21, 22, 23, 24, 31, 32, 33, 34;
    auto v = converter.convert_values_copy(spn(m));
    EXPECT_THAT(egn(v), EigenEqual(m.reshaped()));
}

TYPED_TEST(SparsityDenseToCOOTest, convertDenseToCOOfirstIndex) {
    using index_t      = TypeParam;
    auto converter     = this->get_converter({.first_index = 1});
    const auto &result = converter.get_sparsity();
    ASSERT_EQ(result.nnz(), 12);
    Eigen::VectorX<index_t> expected_row_idcs(12), expected_col_idcs(12);
    expected_row_idcs << 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3;
    expected_col_idcs << 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4;
    EXPECT_THAT(egn(result.row_indices), EigenEqual(expected_row_idcs));
    EXPECT_THAT(egn(result.col_indices), EigenEqual(expected_col_idcs));
    mat m(3, 4);
    m << 11, 12, 13, 14, 21, 22, 23, 24, 31, 32, 33, 34;
    auto v = converter.convert_values_copy(spn(m));
    EXPECT_THAT(egn(v), EigenEqual(m.reshaped()));
}

TYPED_TEST(SparsityDenseToCOOTest, convertDenseToCOOupper) {
    using index_t      = TypeParam;
    auto converter     = this->get_converter_upper({});
    const auto &result = converter.get_sparsity();
    Eigen::VectorX<index_t> expected_row_idcs(10), expected_col_idcs(10);
    expected_row_idcs << 0, 0, 1, 0, 1, 2, 0, 1, 2, 3;
    expected_col_idcs << 0, 1, 1, 2, 2, 2, 3, 3, 3, 3;
    EXPECT_THAT(egn(result.row_indices), EigenEqual(expected_row_idcs));
    EXPECT_THAT(egn(result.col_indices), EigenEqual(expected_col_idcs));
    mat m(4, 4);
    m << 11, 12, 13, 14, 21, 22, 23, 24, 31, 32, 33, 34, 41, 42, 43, 44;
    auto v = converter.convert_values_copy(spn(m));
    vec expected_v(10);
    expected_v << 11, 12, 22, 13, 23, 33, 14, 24, 34, 44;
    EXPECT_THAT(egn(v), EigenEqual(expected_v));
}

// Dense to CSC ----------------------------------------------------------------

template <class ResultT>
class SparsityDenseToCSCTest : public ::testing::Test {
  protected:
    using Source      = sp::Dense;
    using Result      = ResultT;
    using converter_t = sp::SparsityConverter<Sparsity, Result>;

    static converter_t
    get_converter(sp::SparsityConversionRequest<Result> req) {
        Source src{.rows = 3, .cols = 4, .symmetry = sp::Symmetry::Unsymmetric};
        return {Sparsity{src}, req};
    }
    static converter_t
    get_converter_upper(sp::SparsityConversionRequest<Result> req) {
        Source src{.rows = 4, .cols = 4, .symmetry = sp::Symmetry::Upper};
        return {Sparsity{src}, req};
    }
};

using DenseToCSCIndices = ::testing::Types< //
    sp::SparseCSC<int, int>,                //
    sp::SparseCSC<int, long>,               //
    sp::SparseCSC<int, long long>,          //
    sp::SparseCSC<long, long>,              //
    sp::SparseCSC<long, long long>,         //
    sp::SparseCSC<long long, long long>     //
    >;
TYPED_TEST_SUITE(SparsityDenseToCSCTest, DenseToCSCIndices);

/// @test   unsymmetric Dense to CSC
TYPED_TEST(SparsityDenseToCSCTest, convertDenseToCSC) {
    using Result       = TypeParam;
    auto converter     = this->get_converter({});
    const auto &result = converter.get_sparsity();
    Eigen::VectorX<typename Result::index_t> expected_row_idcs(12);
    Eigen::VectorX<typename Result::storage_index_t> expected_col_ptrs(5);
    expected_row_idcs << 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2;
    expected_col_ptrs << 0, 3, 6, 9, 12;
    EXPECT_EQ(result.rows, 3);
    ASSERT_EQ(result.cols, 4);
    ASSERT_EQ(result.nnz(), 12);
    EXPECT_THAT(egn(result.inner_idx), EigenEqual(expected_row_idcs));
    EXPECT_THAT(egn(result.outer_ptr), EigenEqual(expected_col_ptrs));
    EXPECT_EQ(result.symmetry, sp::Symmetry::Unsymmetric);
    EXPECT_EQ(result.order, Result::SortedRows);
    EXPECT_THAT(egn(result.inner_idx), EigenEqual(expected_row_idcs));
    EXPECT_THAT(egn(result.outer_ptr), EigenEqual(expected_col_ptrs));
    vec m(12);
    m << 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12;
    auto v = converter.convert_values_copy(spn(m));
    EXPECT_THAT(egn(v), EigenEqual(m));
}

/// @test   symmetric Dense to CSC
TYPED_TEST(SparsityDenseToCSCTest, convertDenseToCSCupper) {
    using Result       = TypeParam;
    auto converter     = this->get_converter_upper({});
    const auto &result = converter.get_sparsity();
    Eigen::VectorX<typename Result::index_t> expected_row_idcs(10);
    Eigen::VectorX<typename Result::storage_index_t> expected_col_ptrs(5);
    expected_row_idcs << 0, 0, 1, 0, 1, 2, 0, 1, 2, 3;
    expected_col_ptrs << 0, 1, 3, 6, 10;
    EXPECT_EQ(result.rows, 4);
    ASSERT_EQ(result.cols, 4);
    ASSERT_EQ(result.nnz(), 10);
    EXPECT_THAT(egn(result.inner_idx), EigenEqual(expected_row_idcs));
    EXPECT_THAT(egn(result.outer_ptr), EigenEqual(expected_col_ptrs));
    EXPECT_EQ(result.symmetry, sp::Symmetry::Upper);
    EXPECT_EQ(result.order, Result::SortedRows);
    mat m(4, 4);
    m << 11, 12, 13, 14, 21, 22, 23, 24, 31, 32, 33, 34, 41, 42, 43, 44;
    auto v = converter.convert_values_copy(spn(m));
    vec expected_v(10);
    expected_v << 11, 12, 22, 13, 23, 33, 14, 24, 34, 44;
    EXPECT_THAT(egn(v), EigenEqual(expected_v));
}

// Dense to Dense --------------------------------------------------------------

/// @test   dense to dense, trivial
TEST(Sparsity, convertDenseToDense) {
    using Source      = sp::Dense;
    using Result      = sp::Dense;
    using converter_t = sp::SparsityConverter<Sparsity, Result>;
    Source dense{
        .rows     = 3,
        .cols     = 4,
        .symmetry = sp::Symmetry::Unsymmetric,
    };
    converter_t converter = Sparsity{dense};
    const auto &result    = converter.get_sparsity();
    ASSERT_EQ(result.rows, 3);
    ASSERT_EQ(result.cols, 4);
    ASSERT_EQ(get_nnz(Sparsity{result}), 12);
    EXPECT_EQ(result.symmetry, sp::Symmetry::Unsymmetric);
    mat m(3, 4);
    m << 11, 12, 13, 14, 21, 22, 23, 24, 31, 32, 33, 34;
    auto v = converter.template convert_values_copy<double>(spn(m));
    EXPECT_THAT(egn(v), EigenEqual(m.reshaped()));
}
