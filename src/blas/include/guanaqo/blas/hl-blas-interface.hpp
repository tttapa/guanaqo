#pragma once

/// @file
/// This file provides simple overloaded wrappers around standard BLAS functions.
/// @ingroup linalg_blas_hl

#include <guanaqo/assume.hpp>
#include <guanaqo/blas/blas-interface.hpp>
#include <guanaqo/blas/blas.hpp>
#include <guanaqo/blas/lapack.hpp>
#include <guanaqo/mat-view.hpp>
#include <guanaqo/trace.hpp>

#if GUANAQO_WITH_HL_BLAS_TRACING
#define GUANAQO_TRACE_HL_BLAS(...) GUANAQO_TRACE(__VA_ARGS__)
#else
#define GUANAQO_TRACE_HL_BLAS(...) GUANAQO_NOOP()
#endif

namespace guanaqo::blas {

/// @addtogroup linalg_blas_hl
/// @{

template <class I>
using UnitStride = std::integral_constant<I, 1>;

template <class T, class I>
void xgemv_N(T alpha, std::type_identity_t<MatrixView<const T, I>> A,
             std::type_identity_t<MatrixView<const T, I>> x, T beta,
             MatrixView<T, I> y) {
    GUANAQO_ASSUME(A.rows == y.rows);
    GUANAQO_ASSUME(A.cols == x.rows);
    GUANAQO_ASSUME(x.cols == 1);
    GUANAQO_ASSUME(y.cols == 1);
    GUANAQO_TRACE_HL_BLAS("gemv", 0, A.rows * A.cols);
    xgemv<T, I>(CblasColMajor, CblasNoTrans, A.rows, A.cols, alpha, A.data,
                A.outer_stride, x.data, I{1}, beta, y.data, I{1});
}

template <class T, class I>
void xgemv_T(T alpha, std::type_identity_t<MatrixView<const T, I>> A,
             std::type_identity_t<MatrixView<const T, I>> x, T beta,
             MatrixView<T, I> y) {
    GUANAQO_ASSUME(A.cols == y.rows);
    GUANAQO_ASSUME(A.rows == x.rows);
    GUANAQO_ASSUME(x.cols == 1);
    GUANAQO_ASSUME(y.cols == 1);
    GUANAQO_TRACE_HL_BLAS("gemv", 0, A.rows * A.cols);
    xgemv<T, I>(CblasColMajor, CblasTrans, A.rows, A.cols, alpha, A.data,
                A.outer_stride, x.data, I{1}, beta, y.data, I{1});
}

template <class T, class I>
void xgemv_N(T alpha, std::type_identity_t<MatrixViewRM<const T, I>> A,
             std::type_identity_t<MatrixView<const T, I>> x, T beta,
             MatrixView<T, I> y) {
    GUANAQO_ASSUME(A.rows == y.rows);
    GUANAQO_ASSUME(A.cols == x.rows);
    GUANAQO_ASSUME(x.cols == 1);
    GUANAQO_ASSUME(y.cols == 1);
    GUANAQO_TRACE_HL_BLAS("gemv", 0, A.rows * A.cols);
    xgemv<T, I>(CblasRowMajor, CblasNoTrans, A.rows, A.cols, alpha, A.data,
                A.outer_stride, x.data, I{1}, beta, y.data, I{1});
}

template <class T, class I>
void xgemv_T(T alpha, std::type_identity_t<MatrixViewRM<const T, I>> A,
             std::type_identity_t<MatrixView<const T, I>> x, T beta,
             MatrixView<T, I> y) {
    GUANAQO_ASSUME(A.cols == y.rows);
    GUANAQO_ASSUME(A.rows == x.rows);
    GUANAQO_ASSUME(x.cols == 1);
    GUANAQO_ASSUME(y.cols == 1);
    GUANAQO_TRACE_HL_BLAS("gemv", 0, A.rows * A.cols);
    xgemv<T, I>(CblasRowMajor, CblasTrans, A.rows, A.cols, alpha, A.data,
                A.outer_stride, x.data, I{1}, beta, y.data, I{1});
}

template <class T, class I, StorageOrder O>
void xgemm_NN(T alpha,
              std::type_identity_t<MatrixView<const T, I, UnitStride<I>, O>> A,
              std::type_identity_t<MatrixView<const T, I, UnitStride<I>, O>> B,
              T beta, MatrixView<T, I, UnitStride<I>, O> C) {
    GUANAQO_ASSUME(A.rows == C.rows);
    GUANAQO_ASSUME(A.cols == B.rows);
    GUANAQO_ASSUME(B.cols == C.cols);
    GUANAQO_TRACE_HL_BLAS("gemm", 0, C.rows * C.cols * A.cols);
    xgemm<T, I>(O == StorageOrder::RowMajor ? CblasRowMajor : CblasColMajor,
                CblasNoTrans, CblasNoTrans, C.rows, C.cols, A.cols, alpha,
                A.data, A.outer_stride, B.data, B.outer_stride, beta, C.data,
                C.outer_stride);
}

template <class T, class I, StorageOrder O>
void xgemm_TN(T alpha,
              std::type_identity_t<MatrixView<const T, I, UnitStride<I>, O>> A,
              std::type_identity_t<MatrixView<const T, I, UnitStride<I>, O>> B,
              T beta, MatrixView<T, I, UnitStride<I>, O> C) {
    GUANAQO_ASSUME(A.cols == C.rows);
    GUANAQO_ASSUME(A.rows == B.rows);
    GUANAQO_ASSUME(B.cols == C.cols);
    GUANAQO_TRACE_HL_BLAS("gemm", 0, C.rows * C.cols * A.rows);
    xgemm<T, I>(O == StorageOrder::RowMajor ? CblasRowMajor : CblasColMajor,
                CblasTrans, CblasNoTrans, C.rows, C.cols, A.rows, alpha, A.data,
                A.outer_stride, B.data, B.outer_stride, beta, C.data,
                C.outer_stride);
}

template <class T, class I, StorageOrder O>
void xgemm_TT(T alpha,
              std::type_identity_t<MatrixView<const T, I, UnitStride<I>, O>> A,
              std::type_identity_t<MatrixView<const T, I, UnitStride<I>, O>> B,
              T beta, MatrixView<T, I, UnitStride<I>, O> C) {
    GUANAQO_ASSUME(A.cols == C.rows);
    GUANAQO_ASSUME(A.rows == B.cols);
    GUANAQO_ASSUME(B.rows == C.cols);
    GUANAQO_TRACE_HL_BLAS("gemm", 0, C.rows * C.cols * A.rows);
    xgemm<T, I>(O == StorageOrder::RowMajor ? CblasRowMajor : CblasColMajor,
                CblasTrans, CblasTrans, C.rows, C.cols, A.rows, alpha, A.data,
                A.outer_stride, B.data, B.outer_stride, beta, C.data,
                C.outer_stride);
}

template <class T, class I, StorageOrder O>
void xgemm_NT(T alpha,
              std::type_identity_t<MatrixView<const T, I, UnitStride<I>, O>> A,
              std::type_identity_t<MatrixView<const T, I, UnitStride<I>, O>> B,
              T beta, MatrixView<T, I, UnitStride<I>, O> C) {
    GUANAQO_ASSUME(A.rows == C.rows);
    GUANAQO_ASSUME(A.cols == B.cols);
    GUANAQO_ASSUME(B.rows == C.cols);
    GUANAQO_TRACE_HL_BLAS("gemm", 0, C.rows * C.cols * A.cols);
    xgemm<T, I>(O == StorageOrder::RowMajor ? CblasRowMajor : CblasColMajor,
                CblasNoTrans, CblasTrans, C.rows, C.cols, A.cols, alpha, A.data,
                A.outer_stride, B.data, B.outer_stride, beta, C.data,
                C.outer_stride);
}

template <class T, class I>
void xgemmt_LNN(T alpha, std::type_identity_t<MatrixView<const T, I>> A,
                std::type_identity_t<MatrixView<const T, I>> B, T beta,
                MatrixView<T, I> C) {
    GUANAQO_ASSUME(A.rows == C.rows);
    GUANAQO_ASSUME(A.cols == B.rows);
    GUANAQO_ASSUME(B.cols == C.cols);
    GUANAQO_ASSUME(C.rows == C.cols);
    GUANAQO_TRACE_HL_BLAS("gemmt", 0, C.rows * (C.rows + 1) * A.cols / 2);
    xgemmt<T, I>(CblasColMajor, CblasLower, CblasNoTrans, CblasNoTrans, C.rows,
                 A.cols, alpha, A.data, A.outer_stride, B.data, B.outer_stride,
                 beta, C.data, C.outer_stride);
}

template <class T, class I>
void xgemmt_LTN(T alpha, std::type_identity_t<MatrixView<const T, I>> A,
                std::type_identity_t<MatrixView<const T, I>> B, T beta,
                MatrixView<T, I> C) {
    GUANAQO_ASSUME(A.cols == C.rows);
    GUANAQO_ASSUME(A.rows == B.rows);
    GUANAQO_ASSUME(B.cols == C.cols);
    GUANAQO_ASSUME(C.rows == C.cols);
    GUANAQO_TRACE_HL_BLAS("gemmt", 0, C.rows * (C.rows + 1) * A.rows / 2);
    xgemmt<T, I>(CblasColMajor, CblasLower, CblasTrans, CblasNoTrans, C.rows,
                 A.rows, alpha, A.data, A.outer_stride, B.data, B.outer_stride,
                 beta, C.data, C.outer_stride);
}

template <class T, class I>
void xgemmt_LTT(T alpha, std::type_identity_t<MatrixView<const T, I>> A,
                std::type_identity_t<MatrixView<const T, I>> B, T beta,
                MatrixView<T, I> C) {
    GUANAQO_ASSUME(A.cols == C.rows);
    GUANAQO_ASSUME(A.rows == B.cols);
    GUANAQO_ASSUME(B.rows == C.cols);
    GUANAQO_ASSUME(C.rows == C.cols);
    GUANAQO_TRACE_HL_BLAS("gemmt", 0, C.rows * (C.rows + 1) * A.rows / 2);
    xgemmt<T, I>(CblasColMajor, CblasLower, CblasTrans, CblasTrans, C.rows,
                 A.rows, alpha, A.data, A.outer_stride, B.data, B.outer_stride,
                 beta, C.data, C.outer_stride);
}

template <class T, class I>
void xgemmt_LNT(T alpha, std::type_identity_t<MatrixView<const T, I>> A,
                std::type_identity_t<MatrixView<const T, I>> B, T beta,
                MatrixView<T, I> C) {
    GUANAQO_ASSUME(A.rows == C.rows);
    GUANAQO_ASSUME(A.cols == B.cols);
    GUANAQO_ASSUME(B.rows == C.cols);
    GUANAQO_ASSUME(C.rows == C.cols);
    GUANAQO_TRACE_HL_BLAS("gemmt", 0, C.rows * (C.rows + 1) * A.cols / 2);
    xgemmt<T, I>(CblasColMajor, CblasLower, CblasNoTrans, CblasTrans, C.rows,
                 A.cols, alpha, A.data, A.outer_stride, B.data, B.outer_stride,
                 beta, C.data, C.outer_stride);
}

template <class T, class I, StorageOrder O>
void xsymv_L(T alpha, MatrixView<const T, I, UnitStride<I>, O> A,
             std::type_identity_t<MatrixView<const T, I>> x, T beta,
             MatrixView<T, I> y) {
    GUANAQO_ASSUME(A.cols == A.rows);
    GUANAQO_ASSUME(A.rows == y.rows);
    GUANAQO_ASSUME(A.cols == x.rows);
    GUANAQO_TRACE_HL_BLAS("symv", 0, A.rows * (A.cols + 1));
    xsymv<T, I>(O == StorageOrder::RowMajor ? CblasRowMajor : CblasColMajor,
                CblasLower, A.rows, alpha, A.data, A.outer_stride, x.data, I{1},
                beta, y.data, I{1});
}

template <class T, class I>
void xtrmv_LNN(std::type_identity_t<MatrixView<const T, I>> A,
               MatrixView<T, I> x) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.cols == x.rows);
    GUANAQO_ASSUME(x.cols == 1);
    GUANAQO_TRACE_HL_BLAS("trmv", 0, A.rows * (A.rows + 1) / 2);
    xtrmv<T, I>(CblasColMajor, CblasLower, CblasNoTrans, CblasNonUnit, A.rows,
                A.data, A.outer_stride, x.data, I{1});
}

template <class T, class I>
void xtrmv_LTN(std::type_identity_t<MatrixView<const T, I>> A,
               MatrixView<T, I> x) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.cols == x.rows);
    GUANAQO_ASSUME(x.cols == 1);
    GUANAQO_TRACE_HL_BLAS("trmv", 0, A.rows * (A.rows + 1) / 2);
    xtrmv<T, I>(CblasColMajor, CblasLower, CblasTrans, CblasNonUnit, A.rows,
                A.data, A.outer_stride, x.data, I{1});
}

template <class T, class I>
void xtrsv_LNN(std::type_identity_t<MatrixView<const T, I>> A,
               MatrixView<T, I> x) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.cols == x.rows);
    GUANAQO_ASSUME(x.cols == 1);
    GUANAQO_TRACE_HL_BLAS("trsv", 0, A.rows * (A.rows + 1) / 2);
    xtrsv<T, I>(CblasColMajor, CblasLower, CblasNoTrans, CblasNonUnit, A.rows,
                A.data, A.outer_stride, x.data, I{1});
}

template <class T, class I>
void xtrsv_LTN(std::type_identity_t<MatrixView<const T, I>> A,
               MatrixView<T, I> x) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.cols == x.rows);
    GUANAQO_ASSUME(x.cols == 1);
    GUANAQO_TRACE_HL_BLAS("trsv", 0, A.rows * (A.rows + 1) / 2);
    xtrsv<T, I>(CblasColMajor, CblasLower, CblasTrans, CblasNonUnit, A.rows,
                A.data, A.outer_stride, x.data, I{1});
}

template <class T, class I>
void xtrmm_LLNN(T alpha, std::type_identity_t<MatrixView<const T, I>> A,
                MatrixView<T, I> B) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.cols == B.rows);
    GUANAQO_TRACE_HL_BLAS("trmm", 0, A.rows * (A.rows + 1) * B.cols / 2);
    xtrmm<T, I>(CblasColMajor, CblasLeft, CblasLower, CblasNoTrans,
                CblasNonUnit, B.rows, B.cols, alpha, A.data, A.outer_stride,
                B.data, B.outer_stride);
}

template <class T, class I>
void xtrmm_LLTN(T alpha, std::type_identity_t<MatrixView<const T, I>> A,
                MatrixView<T, I> B) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.cols == B.rows);
    GUANAQO_TRACE_HL_BLAS("trmm", 0, A.rows * (A.rows + 1) * B.cols / 2);
    xtrmm<T, I>(CblasColMajor, CblasLeft, CblasLower, CblasTrans, CblasNonUnit,
                B.rows, B.cols, alpha, A.data, A.outer_stride, B.data,
                B.outer_stride);
}

template <class T, class I>
void xtrmm_RLNN(T alpha, std::type_identity_t<MatrixView<const T, I>> A,
                MatrixView<T, I> B) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.rows == B.cols);
    GUANAQO_TRACE_HL_BLAS("trmm", 0, A.rows * (A.rows + 1) * B.rows / 2);
    xtrmm<T, I>(CblasColMajor, CblasRight, CblasLower, CblasNoTrans,
                CblasNonUnit, B.rows, B.cols, alpha, A.data, A.outer_stride,
                B.data, B.outer_stride);
}

template <class T, class I>
void xtrmm_RLTN(T alpha, std::type_identity_t<MatrixView<const T, I>> A,
                MatrixView<T, I> B) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.rows == B.cols);
    GUANAQO_TRACE_HL_BLAS("trmm", 0, A.rows * (A.rows + 1) * B.rows / 2);
    xtrmm<T, I>(CblasColMajor, CblasRight, CblasLower, CblasTrans, CblasNonUnit,
                B.rows, B.cols, alpha, A.data, A.outer_stride, B.data,
                B.outer_stride);
}

template <class T, class I>
void xsyrk_LN(T alpha, std::type_identity_t<MatrixView<const T, I>> A, T beta,
              MatrixView<T, I> C) {
    GUANAQO_ASSUME(C.rows == C.cols);
    GUANAQO_ASSUME(A.rows == C.rows);
    GUANAQO_TRACE_HL_BLAS("syrk", 0, C.rows * (C.rows + 1) * A.cols / 2);
    xsyrk<T, I>(CblasColMajor, CblasLower, CblasNoTrans, C.rows, A.cols, alpha,
                A.data, A.outer_stride, beta, C.data, C.outer_stride);
}

template <class T, class I>
void xsyrk_LT(T alpha, std::type_identity_t<MatrixView<const T, I>> A, T beta,
              MatrixView<T, I> C) {
    GUANAQO_ASSUME(C.rows == C.cols);
    GUANAQO_ASSUME(A.cols == C.rows);
    GUANAQO_TRACE_HL_BLAS("syrk", 0, C.rows * (C.rows + 1) * A.rows / 2);
    xsyrk<T, I>(CblasColMajor, CblasLower, CblasTrans, C.rows, A.rows, alpha,
                A.data, A.outer_stride, beta, C.data, C.outer_stride);
}

template <class T, class I>
void xtrsm_LLNN(T alpha, std::type_identity_t<MatrixView<const T, I>> A,
                MatrixView<T, I> B) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.cols == B.rows);
    GUANAQO_TRACE_HL_BLAS("trsm", 0,
                          A.rows * (A.rows + 1) * B.cols / 2 + A.rows);
    xtrsm<T, I>(CblasColMajor, CblasLeft, CblasLower, CblasNoTrans,
                CblasNonUnit, B.rows, B.cols, alpha, A.data, A.outer_stride,
                B.data, B.outer_stride);
}

template <class T, class I>
void xtrsm_LLTN(T alpha, std::type_identity_t<MatrixView<const T, I>> A,
                MatrixView<T, I> B) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.rows == B.rows);
    GUANAQO_TRACE_HL_BLAS("trsm", 0,
                          A.rows * (A.rows + 1) * B.cols / 2 + A.rows);
    xtrsm<T, I>(CblasColMajor, CblasLeft, CblasLower, CblasTrans, CblasNonUnit,
                B.rows, B.cols, alpha, A.data, A.outer_stride, B.data,
                B.outer_stride);
}

template <class T, class I>
void xtrsm_RLNN(T alpha, std::type_identity_t<MatrixView<const T, I>> A,
                MatrixView<T, I> B) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.cols == B.cols);
    GUANAQO_TRACE_HL_BLAS("trsm", 0,
                          A.rows * (A.rows + 1) * B.rows / 2 + A.rows);
    xtrsm<T, I>(CblasColMajor, CblasRight, CblasLower, CblasNoTrans,
                CblasNonUnit, B.rows, B.cols, alpha, A.data, A.outer_stride,
                B.data, B.outer_stride);
}

template <class T, class I>
void xtrsm_RLTN(T alpha, std::type_identity_t<MatrixView<const T, I>> A,
                MatrixView<T, I> B) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.cols == B.cols);
    GUANAQO_TRACE_HL_BLAS("trsm", 0,
                          A.rows * (A.rows + 1) * B.rows / 2 + A.rows);
    xtrsm<T, I>(CblasColMajor, CblasRight, CblasLower, CblasTrans, CblasNonUnit,
                B.rows, B.cols, alpha, A.data, A.outer_stride, B.data,
                B.outer_stride);
}

// TODO
template <class T, class I>
void xsytrf_rk(const char *uplo, const I *n, T *a, const I *lda, T *e, I *ipiv,
               T *work, const I *lwork, I *info);

// TODO
template <class T, class I>
void xtrtrs(const char *uplo, const char *trans, const char *diag, const I *n,
            const I *nrhs, const T *A, const I *ldA, T *B, const I *ldB,
            I *info);

/// @throw lapack_error
template <class T, class I>
void xpotrf_L(MatrixView<T, I> A) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_TRACE_HL_BLAS("potrf", 0,
                          (A.rows - 1) * A.rows * (A.rows + 1) / 6 +
                              (A.rows - 1) * A.rows / 2 + 2 * A.rows);
    I info = 0;
    xpotrf<T, I>("L", A.rows, A.data, A.outer_stride, &info);
    lapack_throw_on_err("xpotrf_L", info);
}

/// @throw lapack_error
template <class T, class I>
void xlauum_L(MatrixView<T, I> A) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_TRACE_HL_BLAS("trtri", 0,
                          (A.rows - 2) * (A.rows - 1) * A.rows / 6 +
                              (A.rows - 1) * (A.rows + 2) / 2 + A.rows);
    I info = 0;
    xlauum<T, I>("L", A.rows, A.data, A.outer_stride, &info);
    lapack_throw_on_err("xlauum_L", info);
}

/// @throw lapack_error
template <class T, class I>
void xtrtri_LN(MatrixView<T, I> A) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_TRACE_HL_BLAS("trtri", 0,
                          (A.rows - 2) * (A.rows - 1) * A.rows / 6 +
                              (A.rows - 1) * A.rows / 2 + A.rows);
    I info = 0;
    xtrtri<T, I>("L", "N", A.rows, A.data, A.outer_stride, &info);
    lapack_throw_on_err("xtrtri_LN", info);
}

/// @}

} // namespace guanaqo::blas
