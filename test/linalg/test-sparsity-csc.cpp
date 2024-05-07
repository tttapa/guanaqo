#include <linalg/test-sparsity.hpp>

// CSC to COO ------------------------------------------------------------------

template <class Indices>
class SparsityCSCToCOOTest : public ::testing::Test {
  protected:
    using Source      = std::tuple_element_t<0, Indices>;
    using Result      = std::tuple_element_t<1, Indices>;
    using converter_t = sp::SparsityConverter<Sparsity, Result>;

    Eigen::VectorX<typename Source::index_t> row_idcs{7};
    Eigen::VectorX<typename Source::storage_index_t> col_ptrs{5};

    [[nodiscard]] converter_t
    get_converter(sp::SparsityConversionRequest<Result> req) {
        row_idcs << 0, 1, 1, 2, 0, 2, 0;
        col_ptrs << 0, 2, 4, 6, 7;
        Source src{
            .rows      = 3,
            .cols      = 4,
            .symmetry  = sp::Symmetry::Unsymmetric,
            .inner_idx = spn(row_idcs),
            .outer_ptr = spn(col_ptrs),
            .order     = Source::Unsorted,
        };
        return {Sparsity{src}, req};
    }
};

using CSCToCOOIndices = ::testing::Types<                                     //
    std::tuple<sp::SparseCSC<int, int>, sp::SparseCOO<int>>,                  //
    std::tuple<sp::SparseCSC<int, int>, sp::SparseCOO<long>>,                 //
    std::tuple<sp::SparseCSC<int, int>, sp::SparseCOO<long long>>,            //
    std::tuple<sp::SparseCSC<int, long>, sp::SparseCOO<int>>,                 //
    std::tuple<sp::SparseCSC<int, long>, sp::SparseCOO<long>>,                //
    std::tuple<sp::SparseCSC<int, long>, sp::SparseCOO<long long>>,           //
    std::tuple<sp::SparseCSC<int, long long>, sp::SparseCOO<int>>,            //
    std::tuple<sp::SparseCSC<int, long long>, sp::SparseCOO<long>>,           //
    std::tuple<sp::SparseCSC<int, long long>, sp::SparseCOO<long long>>,      //
    std::tuple<sp::SparseCSC<long, long>, sp::SparseCOO<int>>,                //
    std::tuple<sp::SparseCSC<long, long>, sp::SparseCOO<long>>,               //
    std::tuple<sp::SparseCSC<long, long>, sp::SparseCOO<long long>>,          //
    std::tuple<sp::SparseCSC<long, long long>, sp::SparseCOO<int>>,           //
    std::tuple<sp::SparseCSC<long, long long>, sp::SparseCOO<long>>,          //
    std::tuple<sp::SparseCSC<long, long long>, sp::SparseCOO<long long>>,     //
    std::tuple<sp::SparseCSC<long long, long long>, sp::SparseCOO<int>>,      //
    std::tuple<sp::SparseCSC<long long, long long>, sp::SparseCOO<long>>,     //
    std::tuple<sp::SparseCSC<long long, long long>, sp::SparseCOO<long long>> //
    >;
TYPED_TEST_SUITE(SparsityCSCToCOOTest, CSCToCOOIndices);

TYPED_TEST(SparsityCSCToCOOTest, convertCSCToCOO) {
    using dst_index_t  = typename std::tuple_element_t<1, TypeParam>::index_t;
    auto converter     = this->get_converter({});
    const auto &result = converter.get_sparsity();
    Eigen::VectorX<dst_index_t> expected_row_idcs(7), expected_col_idcs(7);
    expected_row_idcs << 0, 1, 1, 2, 0, 2, 0;
    expected_col_idcs << 0, 0, 1, 1, 2, 2, 3;
    EXPECT_THAT(egn(result.row_indices), EigenEqual(expected_row_idcs));
    EXPECT_THAT(egn(result.col_indices), EigenEqual(expected_col_idcs));
    vec m(7);
    m << 1, 2, 3, 4, 5, 6, 7;
    auto v = converter.convert_values_copy(spn(m));
    EXPECT_THAT(egn(v), EigenEqual(m));
}

TYPED_TEST(SparsityCSCToCOOTest, convertCSCToCOOfirstIndex) {
    using dst_index_t  = typename std::tuple_element_t<1, TypeParam>::index_t;
    auto converter     = this->get_converter({.first_index = 1});
    const auto &result = converter.get_sparsity();
    Eigen::VectorX<dst_index_t> expected_row_idcs(7), expected_col_idcs(7);
    expected_row_idcs << 1, 2, 2, 3, 1, 3, 1;
    expected_col_idcs << 1, 1, 2, 2, 3, 3, 4;
    EXPECT_THAT(egn(result.row_indices), EigenEqual(expected_row_idcs));
    EXPECT_THAT(egn(result.col_indices), EigenEqual(expected_col_idcs));
    vec m(7);
    m << 1, 2, 3, 4, 5, 6, 7;
    auto v = converter.convert_values_copy(spn(m));
    EXPECT_THAT(egn(v), EigenEqual(m));
}

// CSC to CSC ------------------------------------------------------------------

template <class Types>
class SparsityCSCToCSCTest : public ::testing::Test {
  protected:
    using Source      = std::tuple_element_t<0, Types>;
    using Result      = std::tuple_element_t<1, Types>;
    using converter_t = sp::SparsityConverter<Sparsity, Result>;

    Eigen::VectorX<typename Source::index_t> row_idcs{7};
    Eigen::VectorX<typename Source::storage_index_t> col_ptrs{5};

    [[nodiscard]] converter_t
    get_converter_sorted(sp::SparsityConversionRequest<Result> req) {
        row_idcs << 0, 1, 1, 2, 0, 2, 0;
        col_ptrs << 0, 2, 4, 6, 7;
        Source src{
            .rows      = 3,
            .cols      = 4,
            .symmetry  = sp::Symmetry::Unsymmetric,
            .inner_idx = spn(row_idcs),
            .outer_ptr = spn(col_ptrs),
            .order     = Source::SortedRows,
        };
        return {Sparsity{src}, req};
    }

    [[nodiscard]] converter_t
    get_converter_unsorted(sp::SparsityConversionRequest<Result> req) {
        row_idcs << 1, 0, 2, 1, 2, 0, 0;
        col_ptrs << 0, 2, 4, 6, 7;
        Source src{
            .rows      = 3,
            .cols      = 4,
            .symmetry  = sp::Symmetry::Unsymmetric,
            .inner_idx = spn(row_idcs),
            .outer_ptr = spn(col_ptrs),
            .order     = Source::Unsorted,
        };
        return {Sparsity{src}, req};
    }
};

// clang-format off
using CSCToCSCIndices = ::testing::Types<
    std::tuple<sp::SparseCSC<int, int>, sp::SparseCSC<int, int>>,
    std::tuple<sp::SparseCSC<int, int>, sp::SparseCSC<int, long>>,
    std::tuple<sp::SparseCSC<int, int>, sp::SparseCSC<int, long long>>,
    std::tuple<sp::SparseCSC<int, int>, sp::SparseCSC<long, long>>,
    std::tuple<sp::SparseCSC<int, int>, sp::SparseCSC<long, long long>>,
    std::tuple<sp::SparseCSC<int, int>, sp::SparseCSC<long long, long long>>,
    std::tuple<sp::SparseCSC<int, long>, sp::SparseCSC<int, int>>,
    std::tuple<sp::SparseCSC<int, long>, sp::SparseCSC<int, long>>,
    std::tuple<sp::SparseCSC<int, long>, sp::SparseCSC<int, long long>>,
    std::tuple<sp::SparseCSC<int, long>, sp::SparseCSC<long, long>>,
    std::tuple<sp::SparseCSC<int, long>, sp::SparseCSC<long, long long>>,
    std::tuple<sp::SparseCSC<int, long>, sp::SparseCSC<long long, long long>>,
    std::tuple<sp::SparseCSC<int, long long>, sp::SparseCSC<int, int>>,
    std::tuple<sp::SparseCSC<int, long long>, sp::SparseCSC<int, long>>,
    std::tuple<sp::SparseCSC<int, long long>, sp::SparseCSC<int, long long>>,
    std::tuple<sp::SparseCSC<int, long long>, sp::SparseCSC<long, long>>,
    std::tuple<sp::SparseCSC<int, long long>, sp::SparseCSC<long, long long>>,
    std::tuple<sp::SparseCSC<int, long long>, sp::SparseCSC<long long, long long>>,
    std::tuple<sp::SparseCSC<long, long>, sp::SparseCSC<int, int>>,
    std::tuple<sp::SparseCSC<long, long>, sp::SparseCSC<int, long>>,
    std::tuple<sp::SparseCSC<long, long>, sp::SparseCSC<int, long long>>,
    std::tuple<sp::SparseCSC<long, long>, sp::SparseCSC<long, long>>,
    std::tuple<sp::SparseCSC<long, long>, sp::SparseCSC<long, long long>>,
    std::tuple<sp::SparseCSC<long, long>, sp::SparseCSC<long long, long long>>,
    std::tuple<sp::SparseCSC<long, long long>, sp::SparseCSC<int, int>>,
    std::tuple<sp::SparseCSC<long, long long>, sp::SparseCSC<int, long>>,
    std::tuple<sp::SparseCSC<long, long long>, sp::SparseCSC<int, long long>>,
    std::tuple<sp::SparseCSC<long, long long>, sp::SparseCSC<long, long>>,
    std::tuple<sp::SparseCSC<long, long long>, sp::SparseCSC<long, long long>>,
    std::tuple<sp::SparseCSC<long, long long>, sp::SparseCSC<long long, long long>>,
    std::tuple<sp::SparseCSC<long long, long long>, sp::SparseCSC<int, int>>,
    std::tuple<sp::SparseCSC<long long, long long>, sp::SparseCSC<int, long>>,
    std::tuple<sp::SparseCSC<long long, long long>, sp::SparseCSC<int, long long>>,
    std::tuple<sp::SparseCSC<long long, long long>, sp::SparseCSC<long, long>>,
    std::tuple<sp::SparseCSC<long long, long long>, sp::SparseCSC<long, long long>>,
    std::tuple<sp::SparseCSC<long long, long long>, sp::SparseCSC<long long, long long>>
    >;
// clang-format on
TYPED_TEST_SUITE(SparsityCSCToCSCTest, CSCToCSCIndices);

/// @test   sorted CSC to unsorted CSC, so should be no-op
TYPED_TEST(SparsityCSCToCSCTest, convertCSCToCSCsorted) {
    using Result   = std::tuple_element_t<1, TypeParam>;
    auto converter = this->get_converter_sorted({});
    auto result    = converter.get_sparsity();
    Eigen::VectorX<typename Result::index_t> expected_row_idcs(7);
    Eigen::VectorX<typename Result::storage_index_t> expected_col_ptrs(5);
    expected_row_idcs << 0, 1, 1, 2, 0, 2, 0;
    expected_col_ptrs << 0, 2, 4, 6, 7;
    EXPECT_EQ(result.rows, 3);
    ASSERT_EQ(result.cols, 4);
    ASSERT_EQ(result.nnz(), 7);
    EXPECT_THAT(egn(result.inner_idx), EigenEqual(expected_row_idcs));
    EXPECT_THAT(egn(result.outer_ptr), EigenEqual(expected_col_ptrs));
    EXPECT_EQ(result.symmetry, sp::Symmetry::Unsymmetric);
    EXPECT_EQ(result.order, result.SortedRows);
    vec m(7);
    m << 1, 2, 3, 4, 5, 6, 7;
    auto v = converter.convert_values_copy(spn(m));
    EXPECT_THAT(egn(v), EigenEqual(m));
}

/// @test   unsorted CSC to unsorted CSC, so should be no-op
TYPED_TEST(SparsityCSCToCSCTest, convertCSCToCSCunsorted) {
    using Result       = std::tuple_element_t<1, TypeParam>;
    auto converter     = this->get_converter_unsorted({});
    const auto &result = converter.get_sparsity();
    Eigen::VectorX<typename Result::index_t> expected_row_idcs(7);
    Eigen::VectorX<typename Result::storage_index_t> expected_col_ptrs(5);
    expected_row_idcs << 1, 0, 2, 1, 2, 0, 0;
    expected_col_ptrs << 0, 2, 4, 6, 7;
    EXPECT_EQ(result.rows, 3);
    ASSERT_EQ(result.cols, 4);
    ASSERT_EQ(result.nnz(), 7);
    EXPECT_THAT(egn(result.inner_idx), EigenEqual(expected_row_idcs));
    EXPECT_THAT(egn(result.outer_ptr), EigenEqual(expected_col_ptrs));
    EXPECT_EQ(result.symmetry, sp::Symmetry::Unsymmetric);
    EXPECT_EQ(result.order, result.Unsorted);
    vec m(7);
    m << 1, 2, 3, 4, 5, 6, 7;
    auto v = converter.convert_values_copy(spn(m));
    EXPECT_THAT(egn(v), EigenEqual(m));
}

/// @test   unsorted CSC to sorted CSC, requires sorting each column
TYPED_TEST(SparsityCSCToCSCTest, convertCSCToCSCunsorted2sorted) try {
    using Result = std::tuple_element_t<1, TypeParam>;
    auto converter =
        this->get_converter_unsorted({.order = Result::SortedRows});
    const auto &result = converter.get_sparsity();
    Eigen::VectorX<typename Result::index_t> expected_row_idcs(7);
    Eigen::VectorX<typename Result::storage_index_t> expected_col_ptrs(5);
    expected_row_idcs << 0, 1, 1, 2, 0, 2, 0;
    expected_col_ptrs << 0, 2, 4, 6, 7;
    EXPECT_EQ(result.rows, 3);
    ASSERT_EQ(result.cols, 4);
    ASSERT_EQ(result.nnz(), 7);
    EXPECT_THAT(egn(result.inner_idx), EigenEqual(expected_row_idcs));
    EXPECT_THAT(egn(result.outer_ptr), EigenEqual(expected_col_ptrs));
    EXPECT_EQ(result.symmetry, sp::Symmetry::Unsymmetric);
    EXPECT_EQ(result.order, Result::SortedRows);
    vec m(7);
    m << 1, 2, 3, 4, 5, 6, 7;
    auto v = converter.convert_values_copy(spn(m));
    vec expected_v(result.nnz());
    expected_v << 2, 1, 4, 3, 6, 5, 7;
    EXPECT_THAT(egn(v), EigenEqual(expected_v));
} catch (sp::unsupported_conversion &e) {
    GTEST_SKIP() << e.what();
}

/// @test   sorted CSC to sorted CSC, should be no-op
TYPED_TEST(SparsityCSCToCSCTest, convertCSCToCSCsorted2sorted) {
    using Result   = std::tuple_element_t<1, TypeParam>;
    auto converter = this->get_converter_sorted({.order = Result::SortedRows});
    const auto &result = converter.get_sparsity();
    Eigen::VectorX<typename Result::index_t> expected_row_idcs(7);
    Eigen::VectorX<typename Result::storage_index_t> expected_col_ptrs(5);
    expected_row_idcs << 0, 1, 1, 2, 0, 2, 0;
    expected_col_ptrs << 0, 2, 4, 6, 7;
    EXPECT_EQ(result.rows, 3);
    ASSERT_EQ(result.cols, 4);
    ASSERT_EQ(result.nnz(), 7);
    EXPECT_THAT(egn(result.inner_idx), EigenEqual(expected_row_idcs));
    EXPECT_THAT(egn(result.outer_ptr), EigenEqual(expected_col_ptrs));
    EXPECT_EQ(result.symmetry, sp::Symmetry::Unsymmetric);
    EXPECT_EQ(result.order, result.SortedRows);
    vec m(7);
    m << 1, 2, 3, 4, 5, 6, 7;
    auto v = converter.convert_values_copy(spn(m));
    EXPECT_THAT(egn(v), EigenEqual(m));
}

// CSC to Dense ----------------------------------------------------------------

template <class SourceT>
class SparsityCSCToDenseTest : public ::testing::Test {
  protected:
    using Source              = SourceT;
    using Result              = sp::Dense;
    using converter_t         = sp::SparsityConverter<Sparsity, Result>;
    using src_index_t         = typename Source::index_t;
    using src_storage_index_t = typename Source::storage_index_t;

    Eigen::VectorX<src_index_t> row_idcs{7};
    Eigen::VectorX<src_storage_index_t> col_ptrs{5};

    [[nodiscard]] converter_t
    get_converter_sorted(sp::SparsityConversionRequest<Result> req) {
        row_idcs << 0, 1, 1, 2, 0, 2, 0;
        col_ptrs << 0, 2, 4, 6, 7;
        Source src{
            .rows      = 3,
            .cols      = 4,
            .symmetry  = sp::Symmetry::Unsymmetric,
            .inner_idx = spn(row_idcs),
            .outer_ptr = spn(col_ptrs),
            .order     = Source::SortedRows,
        };
        return {Sparsity{src}, req};
    }
    [[nodiscard]] converter_t
    get_converter_sorted_upper(sp::SparsityConversionRequest<Result> req) {
        row_idcs << 0, 0, 1, 0, 2, 1, 3;
        col_ptrs << 0, 1, 3, 5, 7;
        Source src{
            .rows      = 4,
            .cols      = 4,
            .symmetry  = sp::Symmetry::Upper,
            .inner_idx = spn(row_idcs),
            .outer_ptr = spn(col_ptrs),
            .order     = Source::SortedRows,
        };
        return {Sparsity{src}, req};
    }
    [[nodiscard]] converter_t get_converter_sorted_upper_invalid(
        sp::SparsityConversionRequest<Result> req) {
        row_idcs << 0, 0, 2, 0, 2, 1, 3;
        col_ptrs << 0, 1, 3, 5, 7;
        Source src{
            .rows      = 4,
            .cols      = 4,
            .symmetry  = sp::Symmetry::Upper,
            .inner_idx = spn(row_idcs),
            .outer_ptr = spn(col_ptrs),
            .order     = Source::SortedRows,
        };
        return {Sparsity{src}, req};
    }
    [[nodiscard]] converter_t
    get_converter_sorted_lower(sp::SparsityConversionRequest<Result> req) {
        row_idcs << 0, 1, 2, 1, 3, 2, 3;
        col_ptrs << 0, 3, 5, 6, 7;
        Source src{
            .rows      = 4,
            .cols      = 4,
            .symmetry  = sp::Symmetry::Lower,
            .inner_idx = spn(row_idcs),
            .outer_ptr = spn(col_ptrs),
            .order     = Source::SortedRows,
        };
        return {Sparsity{src}, req};
    }
};

using CSCToDenseIndices = ::testing::Types< //
    sp::SparseCSC<int, int>,                //
    sp::SparseCSC<int, long>,               //
    sp::SparseCSC<int, long long>,          //
    sp::SparseCSC<long, long>,              //
    sp::SparseCSC<long, long long>,         //
    sp::SparseCSC<long long, long long>     //
    >;
TYPED_TEST_SUITE(SparsityCSCToDenseTest, CSCToDenseIndices);

/// @test   CSC to dense
TYPED_TEST(SparsityCSCToDenseTest, convertCSCToDense) {
    auto converter     = this->get_converter_sorted({});
    const auto &result = converter.get_sparsity();
    ASSERT_EQ(result.rows, 3);
    ASSERT_EQ(result.cols, 4);
    ASSERT_EQ(get_nnz(Sparsity{result}), 12);
    EXPECT_EQ(result.symmetry, sp::Symmetry::Unsymmetric);
    vec m(7);
    m << 11, 21, 22, 32, 13, 33, 14;
    auto v = converter.convert_values_copy(spn(m));
    mat expected_v(3, 4);
    expected_v << 11, 0, 13, 14, 21, 22, 0, 0, 0, 32, 33, 0;
    EXPECT_THAT(egn(v).reshaped(3, 4), EigenEqual(expected_v));
}

/// @test   CSC upper symmetric to dense
TYPED_TEST(SparsityCSCToDenseTest, convertCSCToDenseUpper) {
    auto converter     = this->get_converter_sorted_upper({});
    const auto &result = converter.get_sparsity();
    ASSERT_EQ(result.rows, 4);
    ASSERT_EQ(result.cols, 4);
    ASSERT_EQ(get_nnz(Sparsity{result}), 16);
    EXPECT_EQ(result.symmetry, sp::Symmetry::Upper);
    vec m(7);
    m << 11, 12, 22, 13, 33, 24, 44;
    auto v = converter.convert_values_copy(spn(m));
    mat expected_v(4, 4);
    expected_v << 11, 12, 13, 0, 12, 22, 0, 24, 13, 0, 33, 0, 0, 24, 0, 44;
    EXPECT_THAT(egn(v).reshaped(4, 4), EigenEqual(expected_v));
}

/// @test   CSC upper symmetric invalid
TYPED_TEST(SparsityCSCToDenseTest, convertCSCToDenseUpperInvalid) {
    auto converter     = this->get_converter_sorted_upper_invalid({});
    const auto &result = converter.get_sparsity();
    ASSERT_EQ(result.rows, 4);
    ASSERT_EQ(result.cols, 4);
    ASSERT_EQ(get_nnz(Sparsity{result}), 16);
    EXPECT_EQ(result.symmetry, sp::Symmetry::Upper);
    vec m(7);
    m << 11, 12, 32, 13, 33, 24, 44;
    EXPECT_THROW(std::ignore = converter.convert_values_copy(spn(m)),
                 std::invalid_argument);
}

/// @test   CSC lower symmetric to dense
TYPED_TEST(SparsityCSCToDenseTest, convertCSCToDenseLower) {
    auto converter     = this->get_converter_sorted_lower({});
    const auto &result = converter.get_sparsity();
    ASSERT_EQ(result.rows, 4);
    ASSERT_EQ(result.cols, 4);
    ASSERT_EQ(get_nnz(Sparsity{result}), 16);
    EXPECT_EQ(result.symmetry, sp::Symmetry::Lower);
    vec m(7);
    m << 11, 12, 13, 22, 24, 33, 44;
    auto v = converter.convert_values_copy(spn(m));
    mat expected_v(4, 4);
    expected_v << 11, 12, 13, 0, //
        12, 22, 0, 24,           //
        13, 0, 33, 0,            //
        0, 24, 0, 44;
    EXPECT_THAT(egn(v).reshaped(4, 4), EigenEqual(expected_v));
}
