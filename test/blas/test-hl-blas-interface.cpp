#include <gtest/gtest.h>
#include <algorithm>
#include <random>
#include <tuple>
#include <utility>

#include <Eigen/Cholesky>

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
    template <typename T>
    void TestGemm(char transA, char transB, eindex_t m, eindex_t n, eindex_t k,
                  T alpha, T beta) {
        const Eigen::MatrixX<T> A = random_matrix<T>(m, k, transA == 'T');
        const Eigen::MatrixX<T> B = random_matrix<T>(k, n, transB == 'T');
        Eigen::MatrixX<T> C = random_matrix<T>(m, n), C_ref = C;

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

    template <typename T>
    void TestGemmt(char transA, char transB, eindex_t n, eindex_t k, T alpha,
                   T beta) {
        const Eigen::MatrixX<T> A = random_matrix<T>(n, k, transA == 'T');
        const Eigen::MatrixX<T> B = random_matrix<T>(k, n, transB == 'T');
        Eigen::MatrixX<T> C = random_matrix<T>(n, n), C_ref = C;

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

    template <typename T>
    void TestSyrk(char transA, eindex_t n, eindex_t k, T alpha, T beta) {
        const Eigen::MatrixX<T> A = random_matrix<T>(n, k, transA == 'T');
        Eigen::MatrixX<T> C = random_matrix<T>(n, n), C_ref = C;

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

    template <typename T>
    void TestTrmmRLNN(eindex_t m, eindex_t n, T alpha) {
        const Eigen::MatrixX<T> A = random_matrix<T>(n, n);
        Eigen::MatrixX<T> B = random_matrix<T>(m, n), B_ref = B;
        B_ref *= A.template triangularView<Eigen::Lower>();
        B_ref *= alpha;
        using namespace guanaqo::blas;
        xtrmm_RLNN(alpha, as_view(A), as_view(B));
        EXPECT_THAT(B, EigenAlmostEqualRel(B_ref, eps));
    }

    template <typename T>
    void TestTrsm(char side, char transA, eindex_t m, eindex_t n, T alpha) {
        const Eigen::MatrixX<T> A = random_matrix<T>(n, n);
        Eigen::MatrixX<T> B = random_matrix<T>(n, m, side == 'R'), B_ref = B;

        using namespace guanaqo::blas;
        B_ref *= alpha;
        if (side == 'L' && transA == 'N') {
            B_ref = A.template triangularView<Eigen::Lower>().solve(B_ref);
            xtrsm_LLNN(alpha, as_view(A), as_view(B));
        } else if (side == 'L' && transA == 'T') {
            B_ref = A.transpose().template triangularView<Eigen::Upper>().solve(
                B_ref);
            xtrsm_LLTN(alpha, as_view(A), as_view(B));
        } else if (side == 'R' && transA == 'N') {
            B_ref = A.transpose()
                        .template triangularView<Eigen::Upper>()
                        .solve(B_ref.transpose())
                        .transpose();
            xtrsm_RLNN(alpha, as_view(A), as_view(B));
        } else if (side == 'R' && transA == 'T') {
            B_ref = A.template triangularView<Eigen::Lower>()
                        .solve(B_ref.transpose())
                        .transpose();
            xtrsm_RLTN(alpha, as_view(A), as_view(B));
        }
        EXPECT_THAT(B, EigenAlmostEqualRel(B_ref, eps));
    }

    template <typename T>
    void TestPotrfL(eindex_t n) {
        const auto nI = static_cast<T>(n) * Eigen::MatrixX<T>::Identity(n, n);
        Eigen::MatrixX<T> A     = random_matrix<T>(n, n) + nI;
        Eigen::MatrixX<T> A_ref = A;
        A.template triangularView<Eigen::StrictlyUpper>().setZero();
        A_ref = A_ref.template selfadjointView<Eigen::Lower>().llt().matrixL();
        using namespace guanaqo::blas;
        xpotrf_L(as_view(A));
        EXPECT_THAT(A, EigenAlmostEqualRel(A_ref, eps));
    }

    template <typename T>
    void TestTrtriLN(eindex_t n) {
        const auto I            = Eigen::MatrixX<T>::Identity(n, n);
        const auto nI           = static_cast<T>(n) * I;
        Eigen::MatrixX<T> A     = random_matrix<T>(n, n) + nI;
        Eigen::MatrixX<T> A_ref = A;
        A.template triangularView<Eigen::StrictlyUpper>().setZero();
        A_ref = A_ref.template triangularView<Eigen::Lower>().solve(I).eval();
        using namespace guanaqo::blas;
        xtrtri_LN(as_view(A));
        EXPECT_THAT(A, EigenAlmostEqualRel(A_ref, eps));
    }
};

// Test cases for all GEMM variants using parameterized tests.
TEST_P(BlasTest, TestGemmNN) {
    auto [m, n, k] = GetParam();
    TestGemm<double>('N', 'N', m, n, k, 1.5, 0.5);
}

TEST_P(BlasTest, TestGemmNT) {
    auto [m, n, k] = GetParam();
    TestGemm<double>('N', 'T', m, n, k, 1.2, 0.8);
}

TEST_P(BlasTest, TestGemmTN) {
    auto [m, n, k] = GetParam();
    TestGemm<double>('T', 'N', m, n, k, 2.0, 1.0);
}

TEST_P(BlasTest, TestGemmTT) {
    auto [m, n, k] = GetParam();
    TestGemm<double>('T', 'T', m, n, k, -1.0, 0.3);
}

// Test cases for all xgemmt variants (symmetric matrix multiplication)
TEST_P(BlasTest, TestGemmtNN) {
    auto [n, _, k] = GetParam();
    TestGemmt<double>('N', 'N', n, k, 1.1, 0.7);
}

TEST_P(BlasTest, TestGemmtNT) {
    auto [n, _, k] = GetParam();
    TestGemmt<double>('N', 'T', n, k, 1.3, 0.9);
}

TEST_P(BlasTest, TestGemmtTN) {
    auto [n, _, k] = GetParam();
    TestGemmt<double>('T', 'N', n, k, 1.4, 0.6);
}

TEST_P(BlasTest, TestGemmtTT) {
    auto [n, _, k] = GetParam();
    TestGemmt<double>('T', 'T', n, k, 1.2, 0.8);
}

// Test cases for all xtrmm variants (triangular matrix multiplication)
TEST_P(BlasTest, TestTrmmRLNN) {
    auto [m, n, _] = GetParam();
    TestTrmmRLNN<double>(m, n, 1.3);
}

// Test cases for all xtrsm variants (triangular matrix solve)
TEST_P(BlasTest, TestTrsmLLNN) {
    auto [m, n, _] = GetParam();
    TestTrsm<double>('L', 'N', m, n, 1.3);
}

TEST_P(BlasTest, TestTrsmLLTN) {
    auto [m, n, _] = GetParam();
    TestTrsm<double>('L', 'T', m, n, 1.3);
}

TEST_P(BlasTest, TestTrsmRLNN) {
    auto [m, n, _] = GetParam();
    TestTrsm<double>('R', 'N', m, n, 1.3);
}

TEST_P(BlasTest, TestTrsmRLTN) {
    auto [m, n, _] = GetParam();
    TestTrsm<double>('R', 'T', m, n, 1.3);
}

// Test cases for all xpotrf variants (Cholesky factorization)
TEST_P(BlasTest, TestPotrfL) {
    auto m = get<0>(GetParam());
    TestPotrfL<double>(m);
}

// Test cases for all xtrtri variants (triangular inverse)
TEST_P(BlasTest, TestTrtriL) {
    auto m = get<0>(GetParam());
    TestTrtriLN<double>(m);
}

// Define test cases with different matrix sizes.
INSTANTIATE_TEST_SUITE_P(BlasTests, BlasTest,
                         testing::Values(sizes_tuple{2, 3, 4},
                                         sizes_tuple{7, 5, 3},
                                         sizes_tuple{10, 8, 12},
                                         sizes_tuple{6, 5, 4}));
