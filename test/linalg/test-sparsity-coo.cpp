#include <linalg/test-sparsity.hpp>

// COO to COO ------------------------------------------------------------------

template <class Indices>
class SparsityCOOToCOOTest : public ::testing::Test {
  protected:
    using Source      = std::tuple_element_t<0, Indices>;
    using Result      = std::tuple_element_t<1, Indices>;
    using converter_t = sp::SparsityConverter<Sparsity, Result>;

    Eigen::VectorX<typename Source::index_t> row_idcs{7}, col_idcs{7};

    [[nodiscard]] converter_t
    get_converter(sp::SparsityConversionRequest<Result> req) {
        row_idcs << 0, 1, 1, 2, 0, 2, 0;
        col_idcs << 0, 0, 1, 1, 2, 2, 3;
        Source src{
            .rows        = 3,
            .cols        = 4,
            .symmetry    = sp::Symmetry::Unsymmetric,
            .row_indices = spn(row_idcs),
            .col_indices = spn(col_idcs),
            .order       = Source::Unsorted,
        };
        return {Sparsity{src}, req};
    }
    [[nodiscard]] converter_t
    get_converter_2(sp::SparsityConversionRequest<Result> req) {
        row_idcs << 10, 11, 11, 12, 10, 12, 10;
        col_idcs << 10, 10, 11, 11, 12, 12, 13;
        Source src{
            .rows        = 3,
            .cols        = 4,
            .symmetry    = sp::Symmetry::Unsymmetric,
            .row_indices = spn(row_idcs),
            .col_indices = spn(col_idcs),
            .order       = Source::Unsorted,
            .first_index = 10,
        };
        return {Sparsity{src}, req};
    }
};

using COOToCOOIndices = ::testing::Types<                          //
    std::tuple<sp::SparseCOO<int>, sp::SparseCOO<int>>,            //
    std::tuple<sp::SparseCOO<int>, sp::SparseCOO<long>>,           //
    std::tuple<sp::SparseCOO<int>, sp::SparseCOO<long long>>,      //
    std::tuple<sp::SparseCOO<long>, sp::SparseCOO<long>>,          //
    std::tuple<sp::SparseCOO<long>, sp::SparseCOO<long long>>,     //
    std::tuple<sp::SparseCOO<long long>, sp::SparseCOO<long long>> //
    >;
TYPED_TEST_SUITE(SparsityCOOToCOOTest, COOToCOOIndices);

TYPED_TEST(SparsityCOOToCOOTest, convertCOOToCOO) {
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

TYPED_TEST(SparsityCOOToCOOTest, convertCOOToCOOfirstIndex) {
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

TYPED_TEST(SparsityCOOToCOOTest, convertCOOToCOOfirstIndex2) {
    using dst_index_t  = typename std::tuple_element_t<1, TypeParam>::index_t;
    auto converter     = this->get_converter_2({.first_index = 1});
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

// COO to CSC ------------------------------------------------------------------

template <class Indices>
class SparsityCOOToCSCTest : public ::testing::Test {
  protected:
    using Source      = std::tuple_element_t<0, Indices>;
    using Result      = std::tuple_element_t<1, Indices>;
    using converter_t = sp::SparsityConverter<Sparsity, Result>;

    Eigen::VectorX<typename Source::index_t> row_idcs{7}, col_idcs{7};

    [[nodiscard]] Source get_source_unsorted() {
        row_idcs << 0, 1, 1, 2, 0, 2, 0;
        col_idcs << 3, 0, 1, 2, 0, 1, 2;
        return {
            .rows        = 3,
            .cols        = 4,
            .symmetry    = sp::Symmetry::Unsymmetric,
            .row_indices = spn(row_idcs),
            .col_indices = spn(col_idcs),
            .order       = Source::Unsorted,
        };
    }
    [[nodiscard]] Source get_source_silently_sorted() {
        row_idcs << 0, 1, 1, 2, 0, 2, 0;
        col_idcs << 0, 0, 1, 1, 2, 2, 3;
        return {
            .rows        = 3,
            .cols        = 4,
            .symmetry    = sp::Symmetry::Unsymmetric,
            .row_indices = spn(row_idcs),
            .col_indices = spn(col_idcs),
            .order       = Source::Unsorted,
        };
    }
    [[nodiscard]] Source get_source_sorted_cols() {
        row_idcs << 1, 0, 2, 1, 2, 0, 0;
        col_idcs << 0, 0, 1, 1, 2, 2, 3;
        return {
            .rows        = 3,
            .cols        = 4,
            .symmetry    = sp::Symmetry::Unsymmetric,
            .row_indices = spn(row_idcs),
            .col_indices = spn(col_idcs),
            .order       = Source::SortedByColsOnly,
        };
    }
    [[nodiscard]] Source get_source_sorted() {
        row_idcs << 0, 1, 1, 2, 0, 2, 0;
        col_idcs << 0, 0, 1, 1, 2, 2, 3;
        return {
            .rows        = 3,
            .cols        = 4,
            .symmetry    = sp::Symmetry::Unsymmetric,
            .row_indices = spn(row_idcs),
            .col_indices = spn(col_idcs),
            .order       = Source::SortedByColsAndRows,
        };
    }
    [[nodiscard]] Source get_source_sorted_first_index() {
        row_idcs << 10, 11, 11, 12, 10, 12, 10;
        col_idcs << 10, 10, 11, 11, 12, 12, 13;
        return {
            .rows        = 3,
            .cols        = 4,
            .symmetry    = sp::Symmetry::Unsymmetric,
            .row_indices = spn(row_idcs),
            .col_indices = spn(col_idcs),
            .order       = Source::SortedByColsAndRows,
            .first_index = 10,
        };
    }

    [[nodiscard]] converter_t
    get_converter(Source src, sp::SparsityConversionRequest<Result> req) const {
        return {Sparsity{src}, req};
    }
};

using COOToCSCIndices = ::testing::Types<                                     //
    std::tuple<sp::SparseCOO<int>, sp::SparseCSC<int, int>>,                  //
    std::tuple<sp::SparseCOO<long>, sp::SparseCSC<int, int>>,                 //
    std::tuple<sp::SparseCOO<long long>, sp::SparseCSC<int, int>>,            //
    std::tuple<sp::SparseCOO<int>, sp::SparseCSC<int, long>>,                 //
    std::tuple<sp::SparseCOO<long>, sp::SparseCSC<int, long>>,                //
    std::tuple<sp::SparseCOO<long long>, sp::SparseCSC<int, long>>,           //
    std::tuple<sp::SparseCOO<int>, sp::SparseCSC<int, long long>>,            //
    std::tuple<sp::SparseCOO<long>, sp::SparseCSC<int, long long>>,           //
    std::tuple<sp::SparseCOO<long long>, sp::SparseCSC<int, long long>>,      //
    std::tuple<sp::SparseCOO<int>, sp::SparseCSC<long, long>>,                //
    std::tuple<sp::SparseCOO<long>, sp::SparseCSC<long, long>>,               //
    std::tuple<sp::SparseCOO<long long>, sp::SparseCSC<long, long>>,          //
    std::tuple<sp::SparseCOO<int>, sp::SparseCSC<long, long long>>,           //
    std::tuple<sp::SparseCOO<long>, sp::SparseCSC<long, long long>>,          //
    std::tuple<sp::SparseCOO<long long>, sp::SparseCSC<long, long long>>,     //
    std::tuple<sp::SparseCOO<int>, sp::SparseCSC<long long, long long>>,      //
    std::tuple<sp::SparseCOO<long>, sp::SparseCSC<long long, long long>>,     //
    std::tuple<sp::SparseCOO<long long>, sp::SparseCSC<long long, long long>> //
    >;
TYPED_TEST_SUITE(SparsityCOOToCSCTest, COOToCSCIndices);

/// @test   unsorted COO to unsorted CSC, requires sorting columns
TYPED_TEST(SparsityCOOToCSCTest, convertCOOToCSCunsorted) {
    using Result       = std::tuple_element_t<1, TypeParam>;
    auto src           = this->get_source_unsorted();
    auto converter     = this->get_converter(src, {});
    const auto &result = converter.get_sparsity();
    EXPECT_EQ(result.rows, 3);
    ASSERT_EQ(result.cols, 4);
    ASSERT_EQ(result.nnz(), 7);
    EXPECT_EQ(result.symmetry, sp::Symmetry::Unsymmetric);
    EXPECT_EQ(result.order, Result::Unsorted);
    // The order is indeterminate (since we don't use a stable sort), so
    // compare the matrices using Eigen triplets.
    std::vector<Eigen::Triplet<real_t, index_t>> a_trip, b_trip;
    vec m(7);
    for (index_t i = 0; i < 7; ++i) {
        b_trip.emplace_back(src.row_indices[cast_sz(i)],
                            src.col_indices[cast_sz(i)], real_t(i));
        m(i) = real_t(i);
    }
    auto v = converter.convert_values_copy(spn(m));
    for (index_t c = 0; c < 4; ++c)
        for (index_t i = result.outer_ptr[cast_sz(c)];
             i < result.outer_ptr[cast_sz(c) + 1]; ++i)
            a_trip.emplace_back(result.inner_idx[cast_sz(i)], c, v[cast_sz(i)]);
    Eigen::SparseMatrix<real_t, 0, index_t> a(3, 4), b(3, 4);
    a.setFromTriplets(a_trip.begin(), a_trip.end());
    b.setFromTriplets(b_trip.begin(), b_trip.end());
    EXPECT_THAT(a.toDense(), EigenEqual(b.toDense()));
}

/// @test   COO which is sorted but reported as unsorted, to unsorted CSC
TYPED_TEST(SparsityCOOToCSCTest, convertCOOToCSCsilentlySorted2unsorted) {
    using Result       = std::tuple_element_t<1, TypeParam>;
    auto src           = this->get_source_silently_sorted();
    auto converter     = this->get_converter(src, {});
    const auto &result = converter.get_sparsity();
    EXPECT_EQ(result.rows, 3);
    ASSERT_EQ(result.cols, 4);
    ASSERT_EQ(result.nnz(), 7);
    EXPECT_EQ(result.symmetry, sp::Symmetry::Unsymmetric);
    EXPECT_EQ(result.order, Result::Unsorted);
    // The order is indeterminate (since we don't use a stable sort), so
    // compare the matrices using Eigen triplets.
    std::vector<Eigen::Triplet<real_t, index_t>> a_trip, b_trip;
    vec m(7);
    for (index_t i = 0; i < 7; ++i) {
        b_trip.emplace_back(src.row_indices[cast_sz(i)],
                            src.col_indices[cast_sz(i)], real_t(i));
        m(i) = real_t(i);
    }
    auto v = converter.convert_values_copy(spn(m));
    for (index_t c = 0; c < 4; ++c)
        for (index_t i = result.outer_ptr[cast_sz(c)];
             i < result.outer_ptr[cast_sz(c) + 1]; ++i)
            a_trip.emplace_back(result.inner_idx[cast_sz(i)], c, v[cast_sz(i)]);
    Eigen::SparseMatrix<real_t, 0, index_t> a(3, 4), b(3, 4);
    a.setFromTriplets(a_trip.begin(), a_trip.end());
    b.setFromTriplets(b_trip.begin(), b_trip.end());
    EXPECT_THAT(a.toDense(), EigenEqual(b.toDense()));
}

/// @test   COO which is sorted by columns only, to unsorted CSC,
///         no sorting required
TYPED_TEST(SparsityCOOToCSCTest, convertCOOToCSCsortedCols2unsorted) {
    using Result       = std::tuple_element_t<1, TypeParam>;
    auto src           = this->get_source_sorted_cols();
    auto converter     = this->get_converter(src, {});
    const auto &result = converter.get_sparsity();
    EXPECT_EQ(result.rows, 3);
    ASSERT_EQ(result.cols, 4);
    ASSERT_EQ(result.nnz(), 7);
    EXPECT_EQ(result.symmetry, sp::Symmetry::Unsymmetric);
    EXPECT_EQ(result.order, Result::Unsorted);
    // The order is indeterminate (since we don't use a stable sort), so
    // compare the matrices using Eigen triplets.
    std::vector<Eigen::Triplet<real_t, index_t>> a_trip, b_trip;
    vec m(7);
    for (index_t i = 0; i < 7; ++i) {
        b_trip.emplace_back(src.row_indices[cast_sz(i)],
                            src.col_indices[cast_sz(i)], real_t(i));
        m(i) = real_t(i);
    }
    auto v = converter.convert_values_copy(spn(m));
    for (index_t c = 0; c < 4; ++c)
        for (index_t i = result.outer_ptr[cast_sz(c)];
             i < result.outer_ptr[cast_sz(c) + 1]; ++i)
            a_trip.emplace_back(result.inner_idx[cast_sz(i)], c, v[cast_sz(i)]);
    Eigen::SparseMatrix<real_t, 0, index_t> a(3, 4), b(3, 4);
    a.setFromTriplets(a_trip.begin(), a_trip.end());
    b.setFromTriplets(b_trip.begin(), b_trip.end());
    EXPECT_THAT(a.toDense(), EigenEqual(b.toDense()));
}

/// @test   COO which is sorted but reported as unsorted, to sorted CSC
TYPED_TEST(SparsityCOOToCSCTest, convertCOOToCSCsilentlySorted2sorted) {
    using Result   = std::tuple_element_t<1, TypeParam>;
    auto src       = this->get_source_silently_sorted();
    auto converter = this->get_converter(src, {.order = Result::SortedRows});
    const auto &result = converter.get_sparsity();
    Eigen::VectorX<typename Result::index_t> expected_row_idcs(7);
    Eigen::VectorX<typename Result::storage_index_t> expected_col_ptrs(5);
    expected_row_idcs << 0, 1, 1, 2, 0, 2, 0;
    expected_col_ptrs << 0, 2, 4, 6, 7;
    EXPECT_EQ(result.rows, 3);
    ASSERT_EQ(result.cols, 4);
    ASSERT_EQ(result.nnz(), 7);
    EXPECT_EQ(result.symmetry, sp::Symmetry::Unsymmetric);
    EXPECT_EQ(result.order, Result::SortedRows);
    EXPECT_THAT(egn(result.inner_idx), EigenEqual(expected_row_idcs));
    EXPECT_THAT(egn(result.outer_ptr), EigenEqual(expected_col_ptrs));
    vec m(7);
    m << 1, 2, 3, 4, 5, 6, 7;
    auto v = converter.convert_values_copy(spn(m));
    EXPECT_THAT(egn(v), EigenEqual(m));
}

/// @test   unsorted COO to sorted CSC
TYPED_TEST(SparsityCOOToCSCTest, convertCOOToCSCunsorted2sorted) {
    using Result   = std::tuple_element_t<1, TypeParam>;
    auto src       = this->get_source_unsorted();
    auto converter = this->get_converter(src, {.order = Result::SortedRows});
    const auto &result = converter.get_sparsity();
    Eigen::VectorX<typename Result::index_t> expected_row_idcs(7);
    Eigen::VectorX<typename Result::storage_index_t> expected_col_ptrs(5);
    expected_row_idcs << 0, 1, 1, 2, 0, 2, 0;
    expected_col_ptrs << 0, 2, 4, 6, 7;
    EXPECT_EQ(result.rows, 3);
    ASSERT_EQ(result.cols, 4);
    ASSERT_EQ(result.nnz(), 7);
    EXPECT_EQ(result.symmetry, sp::Symmetry::Unsymmetric);
    EXPECT_EQ(result.order, Result::SortedRows);
    EXPECT_THAT(egn(result.inner_idx), EigenEqual(expected_row_idcs));
    EXPECT_THAT(egn(result.outer_ptr), EigenEqual(expected_col_ptrs));
    vec m(7);
    m << 1, 2, 3, 4, 5, 6, 7;
    auto v = converter.convert_values_copy(spn(m));
    vec expected_v(result.nnz());
    expected_v << 5, 2, 3, 6, 7, 4, 1;
    EXPECT_THAT(egn(v), EigenEqual(expected_v));
}

/// @test sorted COO to sorted CSC, no sorting required
TYPED_TEST(SparsityCOOToCSCTest, convertCOOToCSCsorted2sorted) {
    using Result   = std::tuple_element_t<1, TypeParam>;
    auto src       = this->get_source_sorted();
    auto converter = this->get_converter(src, {.order = Result::SortedRows});
    const auto &result = converter.get_sparsity();
    Eigen::VectorX<typename Result::index_t> expected_row_idcs(7);
    Eigen::VectorX<typename Result::storage_index_t> expected_col_ptrs(5);
    expected_row_idcs << 0, 1, 1, 2, 0, 2, 0;
    expected_col_ptrs << 0, 2, 4, 6, 7;
    EXPECT_EQ(result.rows, 3);
    ASSERT_EQ(result.cols, 4);
    ASSERT_EQ(result.nnz(), 7);
    EXPECT_EQ(result.symmetry, sp::Symmetry::Unsymmetric);
    EXPECT_EQ(result.order, Result::SortedRows);
    EXPECT_THAT(egn(result.inner_idx), EigenEqual(expected_row_idcs));
    EXPECT_THAT(egn(result.outer_ptr), EigenEqual(expected_col_ptrs));
    vec m(7);
    m << 1, 2, 3, 4, 5, 6, 7;
    auto v = converter.convert_values_copy(spn(m));
    EXPECT_THAT(egn(v), EigenEqual(m));
}

/// @test sorted COO to sorted CSC, with different first index for COO
TYPED_TEST(SparsityCOOToCSCTest, convertCOOToCSCfirstIndex) {
    using Result   = std::tuple_element_t<1, TypeParam>;
    auto src       = this->get_source_sorted_first_index();
    auto converter = this->get_converter(src, {.order = Result::SortedRows});
    const auto &result = converter.get_sparsity();
    Eigen::VectorX<typename Result::index_t> expected_row_idcs(7);
    Eigen::VectorX<typename Result::storage_index_t> expected_col_ptrs(5);
    expected_row_idcs << 0, 1, 1, 2, 0, 2, 0;
    expected_col_ptrs << 0, 2, 4, 6, 7;
    EXPECT_EQ(result.rows, 3);
    ASSERT_EQ(result.cols, 4);
    ASSERT_EQ(result.nnz(), 7);
    EXPECT_EQ(result.symmetry, sp::Symmetry::Unsymmetric);
    EXPECT_EQ(result.order, Result::SortedRows);
    EXPECT_THAT(egn(result.inner_idx), EigenEqual(expected_row_idcs));
    EXPECT_THAT(egn(result.outer_ptr), EigenEqual(expected_col_ptrs));
    vec m(7);
    m << 1, 2, 3, 4, 5, 6, 7;
    auto v = converter.convert_values_copy(spn(m));
    EXPECT_THAT(egn(v), EigenEqual(m));
}

/// @test   sorted COO to unsorted CSC, no sorting required
TYPED_TEST(SparsityCOOToCSCTest, convertCOOToCSCsorted2unsorted) {
    using Result       = std::tuple_element_t<1, TypeParam>;
    auto src           = this->get_source_sorted();
    auto converter     = this->get_converter(src, {});
    const auto &result = converter.get_sparsity();
    Eigen::VectorX<typename Result::index_t> expected_row_idcs(7);
    Eigen::VectorX<typename Result::storage_index_t> expected_col_ptrs(5);
    expected_row_idcs << 0, 1, 1, 2, 0, 2, 0;
    expected_col_ptrs << 0, 2, 4, 6, 7;
    EXPECT_EQ(result.rows, 3);
    ASSERT_EQ(result.cols, 4);
    ASSERT_EQ(result.nnz(), 7);
    EXPECT_EQ(result.symmetry, sp::Symmetry::Unsymmetric);
    EXPECT_EQ(result.order, Result::SortedRows);
    EXPECT_THAT(egn(result.inner_idx), EigenEqual(expected_row_idcs));
    EXPECT_THAT(egn(result.outer_ptr), EigenEqual(expected_col_ptrs));
    vec m(7);
    m << 1, 2, 3, 4, 5, 6, 7;
    auto v = converter.convert_values_copy(spn(m));
    EXPECT_THAT(egn(v), EigenEqual(m));
}

// COO to Dense ----------------------------------------------------------------

template <class SourceT>
class SparsityCOOToDenseTest : public ::testing::Test {
  protected:
    using Source      = SourceT;
    using Result      = sp::Dense;
    using converter_t = sp::SparsityConverter<Sparsity, Result>;
    using src_index_t = typename Source::index_t;

    Eigen::VectorX<src_index_t> row_idcs{7}, col_idcs{7};

    [[nodiscard]] converter_t
    get_converter_sorted(sp::SparsityConversionRequest<Result> req) {
        row_idcs << 0, 1, 1, 2, 0, 2, 0;
        col_idcs << 0, 0, 1, 1, 2, 2, 3;
        Source src{
            .rows        = 3,
            .cols        = 4,
            .symmetry    = sp::Symmetry::Unsymmetric,
            .row_indices = spn(row_idcs),
            .col_indices = spn(col_idcs),
            .order       = Source::SortedByColsAndRows,
        };
        return {Sparsity{src}, req};
    }
    [[nodiscard]] converter_t
    get_converter_sorted_upper(sp::SparsityConversionRequest<Result> req) {
        row_idcs << 0, 0, 1, 0, 2, 1, 3;
        col_idcs << 0, 1, 1, 2, 2, 3, 3;
        Source src{
            .rows        = 4,
            .cols        = 4,
            .symmetry    = sp::Symmetry::Upper,
            .row_indices = spn(row_idcs),
            .col_indices = spn(col_idcs),
            .order       = Source::SortedByColsAndRows,
        };
        return {Sparsity{src}, req};
    }
    [[nodiscard]] converter_t get_converter_sorted_upper_invalid(
        sp::SparsityConversionRequest<Result> req) {
        row_idcs << 0, 0, 2, 0, 2, 1, 3;
        col_idcs << 0, 1, 1, 2, 2, 3, 3;
        Source src{
            .rows        = 4,
            .cols        = 4,
            .symmetry    = sp::Symmetry::Upper,
            .row_indices = spn(row_idcs),
            .col_indices = spn(col_idcs),
            .order       = Source::SortedByColsAndRows,
        };
        return {Sparsity{src}, req};
    }
    [[nodiscard]] converter_t
    get_converter_sorted_lower(sp::SparsityConversionRequest<Result> req) {
        row_idcs << 0, 1, 2, 1, 3, 2, 3;
        col_idcs << 0, 0, 0, 1, 1, 2, 3;
        Source src{
            .rows        = 4,
            .cols        = 4,
            .symmetry    = sp::Symmetry::Lower,
            .row_indices = spn(row_idcs),
            .col_indices = spn(col_idcs),
            .order       = Source::SortedByColsAndRows,
        };
        return {Sparsity{src}, req};
    }
};

using COOToDenseIndices = ::testing::Types< //
    sp::SparseCOO<int>,                     //
    sp::SparseCOO<long>,                    //
    sp::SparseCOO<long long>                //
    >;
TYPED_TEST_SUITE(SparsityCOOToDenseTest, COOToDenseIndices);

/// @test   COO to dense
TYPED_TEST(SparsityCOOToDenseTest, convertCOOToDense) {
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

/// @test   COO upper symmetric to dense
TYPED_TEST(SparsityCOOToDenseTest, convertCOOToDenseUpper) {
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

/// @test   COO upper symmetric invalid
TYPED_TEST(SparsityCOOToDenseTest, convertCOOToDenseUpperInvalid) {
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

/// @test   COO lower symmetric to dense
TYPED_TEST(SparsityCOOToDenseTest, convertCOOToDenseLower) {
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
