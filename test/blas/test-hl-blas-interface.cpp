#include <gtest/gtest.h>
#include <algorithm>
#include <random>
#include <tuple>
#include <utility>

#include <guanaqo/blas/hl-blas-interface.hpp>
#include <guanaqo/eigen/view.hpp>
#include "test-util/eigen-matchers.hpp"

constexpr double eps = 1e-12;

using eindex_t = Eigen::Index;
using guanaqo::blas::index_t;

template <class T>
auto as_view(T &&t) {
    return guanaqo::as_view(std::forward<T>(t),
                            guanaqo::with_index_type<index_t>);
}

using sizes_tuple = std::tuple<eindex_t, eindex_t, eindex_t>;
class BlasTest : public testing::TestWithParam<sizes_tuple> {
  protected:
    std::mt19937 rng{12345}; // Fixed seed for reproducibility.
    std::uniform_real_distribution<double> uniform{-1.0, 1.0};

    // Generates a random matrix of the given size with values in [-1, 1].
    template <typename T>
    Eigen::MatrixX<T> random_matrix(eindex_t rows, eindex_t cols,
                                    bool trans = false) {
        Eigen::MatrixX<T> mat(trans ? cols : rows, trans ? rows : cols);
        std::ranges::generate(mat.reshaped(), [&] { return uniform(rng); });
        return mat;
    }

    // Generic test function for different GEMM variants
    template <typename T, typename I>
    void TestGemm(char transA, char transB, eindex_t m, eindex_t n, eindex_t k,
                  T alpha, T beta) {
        const Eigen::MatrixX<T> A = random_matrix<T>(m, k, transA == 'T');
        const Eigen::MatrixX<T> B = random_matrix<T>(k, n, transB == 'T');
        Eigen::MatrixX<T> C = random_matrix<T>(m, n), C_ref = C;

        // Compute the expected result
        using namespace guanaqo::blas;
        if (transA == 'T' && transB == 'T') {
            C_ref = alpha * (A.transpose() * B.transpose()) + beta * C_ref;
            xgemm_TT(alpha, as_view(A), as_view(B), beta, as_view(C));
        } else if (transA == 'T' && transB == 'N') {
            C_ref = alpha * (A.transpose() * B) + beta * C_ref;
            xgemm_TN(alpha, as_view(A), as_view(B), beta, as_view(C));
        } else if (transA == 'N' && transB == 'T') {
            C_ref = alpha * (A * B.transpose()) + beta * C_ref;
            xgemm_NT(alpha, as_view(A), as_view(B), beta, as_view(C));
        } else { // (transA == 'N' && transB == 'N')
            C_ref = alpha * (A * B) + beta * C_ref;
            xgemm_NN(alpha, as_view(A), as_view(B), beta, as_view(C));
        }
        EXPECT_THAT(C, EigenAlmostEqualRel(C_ref, eps));
    }

    template <typename T, typename I>
    void TestGemmt(char transA, char transB, eindex_t n, eindex_t k, T alpha,
                   T beta) {
        const Eigen::MatrixX<T> A = random_matrix<T>(n, k, transA == 'T');
        const Eigen::MatrixX<T> B = random_matrix<T>(k, n, transB == 'T');
        Eigen::MatrixX<T> C = random_matrix<T>(n, n), C_ref = C;

        // Compute the expected result, modifying only the lower-triangular part
        using namespace guanaqo::blas;
        if (transA == 'T' && transB == 'T') {
            C_ref.template triangularView<Eigen::Lower>() =
                alpha * (A.transpose() * B.transpose()) + beta * C_ref;
            xgemmt_LTT(alpha, as_view(A), as_view(B), beta, as_view(C));
        } else if (transA == 'T' && transB == 'N') {
            C_ref.template triangularView<Eigen::Lower>() =
                alpha * (A.transpose() * B) + beta * C_ref;
            xgemmt_LTN(alpha, as_view(A), as_view(B), beta, as_view(C));
        } else if (transA == 'N' && transB == 'T') {
            C_ref.template triangularView<Eigen::Lower>() =
                alpha * (A * B.transpose()) + beta * C_ref;
            xgemmt_LNT(alpha, as_view(A), as_view(B), beta, as_view(C));
        } else { // (transA == 'N' && transB == 'N')
            C_ref.template triangularView<Eigen::Lower>() =
                alpha * (A * B) + beta * C_ref;
            xgemmt_LNN(alpha, as_view(A), as_view(B), beta, as_view(C));
        }
        EXPECT_THAT(C, EigenAlmostEqualRel(C_ref, eps));
    }

    template <typename T, typename I>
    void TestSyrk(char transA, eindex_t n, eindex_t k, T alpha, T beta) {
        const Eigen::MatrixX<T> A = random_matrix<T>(n, k, transA == 'T');
        Eigen::MatrixX<T> C = random_matrix<T>(n, n), C_ref = C;

        // Compute the expected result, modifying only the lower-triangular part
        using namespace guanaqo::blas;
        if (transA == 'T') {
            C_ref.template triangularView<Eigen::Lower>() =
                alpha * (A.transpose() * A) + beta * C_ref;
            xsyrk_LT(alpha, as_view(A), beta, as_view(C));
        } else { // transA == 'N'
            C_ref.template triangularView<Eigen::Lower>() =
                alpha * (A * A.transpose()) + beta * C_ref;
            xsyrk_LN(alpha, as_view(A), beta, as_view(C));
        }
        EXPECT_THAT(C, EigenAlmostEqualRel(C_ref, eps));
    }
};

// Test cases for all GEMM variants using parameterized tests.
TEST_P(BlasTest, TestGemmNN) {
    auto [m, n, k] = GetParam();
    TestGemm<double, int>('N', 'N', m, n, k, 1.5, 0.5);
}

TEST_P(BlasTest, TestGemmNT) {
    auto [m, n, k] = GetParam();
    TestGemm<double, int>('N', 'T', m, n, k, 1.2, 0.8);
}

TEST_P(BlasTest, TestGemmTN) {
    auto [m, n, k] = GetParam();
    TestGemm<double, int>('T', 'N', m, n, k, 2.0, 1.0);
}

TEST_P(BlasTest, TestGemmTT) {
    auto [m, n, k] = GetParam();
    TestGemm<double, int>('T', 'T', m, n, k, -1.0, 0.3);
}

// Test cases for all xgemmt variants (symmetric matrix multiplication)
TEST_P(BlasTest, TestGemmtNN) {
    auto [n, _, k] = GetParam();
    TestGemmt<double, int>('N', 'N', n, k, 1.1, 0.7);
}

TEST_P(BlasTest, TestGemmtNT) {
    auto [n, _, k] = GetParam();
    TestGemmt<double, int>('N', 'T', n, k, 1.3, 0.9);
}

TEST_P(BlasTest, TestGemmtTN) {
    auto [n, _, k] = GetParam();
    TestGemmt<double, int>('T', 'N', n, k, 1.4, 0.6);
}

TEST_P(BlasTest, TestGemmtTT) {
    auto [n, _, k] = GetParam();
    TestGemmt<double, int>('T', 'T', n, k, 1.2, 0.8);
}

// Define test cases with different matrix sizes.
INSTANTIATE_TEST_SUITE_P(GemmTests, BlasTest,
                         testing::Values(sizes_tuple{2, 3, 4},
                                         sizes_tuple{7, 5, 3},
                                         sizes_tuple{10, 8, 12},
                                         sizes_tuple{6, 5, 4}));
