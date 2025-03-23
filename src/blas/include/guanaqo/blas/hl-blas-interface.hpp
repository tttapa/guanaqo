/**
 * @file
 * This file provides simple overloaded wrappers around standard BLAS functions.
 */

#pragma once

#include <guanaqo/assume.hpp>
#include <guanaqo/blas/blas-interface.hpp>
#include <guanaqo/blas/blas.hpp>
#include <guanaqo/mat-view.hpp>
#include "guanaqo/blas/lapack.hpp"

namespace guanaqo::blas {

template <class T, class I>
void xgemv_N(T alpha, MatrixView<const T, I> A, MatrixView<const T, I> x,
             T beta, MatrixView<T, I> y) {
    GUANAQO_ASSUME(A.rows == y.rows);
    GUANAQO_ASSUME(A.cols == x.rows);
    GUANAQO_ASSUME(x.cols == 1);
    GUANAQO_ASSUME(y.cols == 1);
    xgemv<T, I>(CblasColMajor, CblasNoTrans, A.rows, A.cols, alpha, A.data,
                A.outer_stride, x.data, I{1}, beta, y.data, I{1});
}

template <class T, class I>
void xgemv_T(T alpha, MatrixView<const T, I> A, MatrixView<const T, I> x,
             T beta, MatrixView<T, I> y) {
    GUANAQO_ASSUME(A.cols == y.rows);
    GUANAQO_ASSUME(A.rows == x.rows);
    GUANAQO_ASSUME(x.cols == 1);
    GUANAQO_ASSUME(y.cols == 1);
    xgemv<T, I>(CblasColMajor, CblasTrans, A.rows, A.cols, alpha, A.data,
                A.outer_stride, x.data, I{1}, beta, y.data, I{1});
}

template <class T, class I>
void xgemm_NN(T alpha, MatrixView<const T, I> A, MatrixView<const T, I> B,
              T beta, MatrixView<T, I> C) {
    GUANAQO_ASSUME(A.rows == C.rows);
    GUANAQO_ASSUME(A.cols == B.rows);
    GUANAQO_ASSUME(B.cols == C.cols);
    xgemm<T, I>(CblasColMajor, CblasNoTrans, CblasNoTrans, C.rows, C.cols,
                A.cols, alpha, A.data, A.outer_stride, B.data, B.outer_stride,
                beta, C.data, C.outer_stride);
}

template <class T, class I>
void xgemm_TN(T alpha, MatrixView<const T, I> A, MatrixView<const T, I> B,
              T beta, MatrixView<T, I> C) {
    GUANAQO_ASSUME(A.cols == C.rows);
    GUANAQO_ASSUME(A.rows == B.rows);
    GUANAQO_ASSUME(B.cols == C.cols);
    xgemm<T, I>(CblasColMajor, CblasTrans, CblasNoTrans, C.rows, C.cols, A.rows,
                alpha, A.data, A.outer_stride, B.data, B.outer_stride, beta,
                C.data, C.outer_stride);
}

template <class T, class I>
void xgemm_TT(T alpha, MatrixView<const T, I> A, MatrixView<const T, I> B,
              T beta, MatrixView<T, I> C) {
    GUANAQO_ASSUME(A.cols == C.rows);
    GUANAQO_ASSUME(A.rows == B.cols);
    GUANAQO_ASSUME(B.rows == C.cols);
    xgemm<T, I>(CblasColMajor, CblasTrans, CblasTrans, C.rows, C.cols, A.rows,
                alpha, A.data, A.outer_stride, B.data, B.outer_stride, beta,
                C.data, C.outer_stride);
}

template <class T, class I>
void xgemm_NT(T alpha, MatrixView<const T, I> A, MatrixView<const T, I> B,
              T beta, MatrixView<T, I> C) {
    GUANAQO_ASSUME(A.rows == C.rows);
    GUANAQO_ASSUME(A.cols == B.cols);
    GUANAQO_ASSUME(B.rows == C.cols);
    xgemm<T, I>(CblasColMajor, CblasNoTrans, CblasTrans, C.rows, C.cols, A.cols,
                alpha, A.data, A.outer_stride, B.data, B.outer_stride, beta,
                C.data, C.outer_stride);
}

template <class T, class I>
void xgemmt_LNN(T alpha, MatrixView<const T, I> A, MatrixView<const T, I> B,
                T beta, MatrixView<T, I> C) {
    GUANAQO_ASSUME(A.rows == C.rows);
    GUANAQO_ASSUME(A.cols == B.rows);
    GUANAQO_ASSUME(B.cols == C.cols);
    GUANAQO_ASSUME(C.rows == C.cols);
    xgemmt<T, I>(CblasColMajor, CblasLower, CblasNoTrans, CblasNoTrans, C.rows,
                 A.cols, alpha, A.data, A.outer_stride, B.data, B.outer_stride,
                 beta, C.data, C.outer_stride);
}

template <class T, class I>
void xgemmt_LTN(T alpha, MatrixView<const T, I> A, MatrixView<const T, I> B,
                T beta, MatrixView<T, I> C) {
    GUANAQO_ASSUME(A.cols == C.rows);
    GUANAQO_ASSUME(A.rows == B.rows);
    GUANAQO_ASSUME(B.cols == C.cols);
    GUANAQO_ASSUME(C.rows == C.cols);
    xgemmt<T, I>(CblasColMajor, CblasLower, CblasTrans, CblasNoTrans, C.rows,
                 A.rows, alpha, A.data, A.outer_stride, B.data, B.outer_stride,
                 beta, C.data, C.outer_stride);
}

template <class T, class I>
void xgemmt_LTT(T alpha, MatrixView<const T, I> A, MatrixView<const T, I> B,
                T beta, MatrixView<T, I> C) {
    GUANAQO_ASSUME(A.cols == C.rows);
    GUANAQO_ASSUME(A.rows == B.cols);
    GUANAQO_ASSUME(B.rows == C.cols);
    GUANAQO_ASSUME(C.rows == C.cols);
    xgemmt<T, I>(CblasColMajor, CblasLower, CblasTrans, CblasTrans, C.rows,
                 A.rows, alpha, A.data, A.outer_stride, B.data, B.outer_stride,
                 beta, C.data, C.outer_stride);
}

template <class T, class I>
void xgemmt_LNT(T alpha, MatrixView<const T, I> A, MatrixView<const T, I> B,
                T beta, MatrixView<T, I> C) {
    GUANAQO_ASSUME(A.rows == C.rows);
    GUANAQO_ASSUME(A.cols == B.cols);
    GUANAQO_ASSUME(B.rows == C.cols);
    GUANAQO_ASSUME(C.rows == C.cols);
    xgemmt<T, I>(CblasColMajor, CblasLower, CblasNoTrans, CblasTrans, C.rows,
                 A.cols, alpha, A.data, A.outer_stride, B.data, B.outer_stride,
                 beta, C.data, C.outer_stride);
}

template <class T, class I>
void xtrmv_LNN(MatrixView<const T, I> A, MatrixView<T, I> x) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.cols == x.rows);
    GUANAQO_ASSUME(x.cols == 1);
    xtrmv<T, I>(CblasColMajor, CblasLower, CblasNoTrans, CblasNonUnit, A.rows,
                A.data, A.outer_stride, x.data, I{1});
}

template <class T, class I>
void xtrmv_LTN(MatrixView<const T, I> A, MatrixView<T, I> x) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.cols == x.rows);
    GUANAQO_ASSUME(x.cols == 1);
    xtrmv<T, I>(CblasColMajor, CblasLower, CblasTrans, CblasNonUnit, A.rows,
                A.data, A.outer_stride, x.data, I{1});
}

template <class T, class I>
void xtrsv_LNN(MatrixView<const T, I> A, MatrixView<T, I> x) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.cols == x.rows);
    GUANAQO_ASSUME(x.cols == 1);
    xtrsv<T, I>(CblasColMajor, CblasLower, CblasNoTrans, CblasNonUnit, A.rows,
                A.data, A.outer_stride, x.data, I{1});
}

template <class T, class I>
void xtrsv_LTN(MatrixView<const T, I> A, MatrixView<T, I> x) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.cols == x.rows);
    GUANAQO_ASSUME(x.cols == 1);
    xtrsv<T, I>(CblasColMajor, CblasLower, CblasTrans, CblasNonUnit, A.rows,
                A.data, A.outer_stride, x.data, I{1});
}

template <class T, class I>
void xtrmm_RLNN(T alpha, MatrixView<const T, I> A, MatrixView<T, I> B) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.rows == B.cols);
    xtrmm<T, I>(CblasColMajor, CblasRight, CblasLower, CblasNoTrans,
                CblasNonUnit, B.rows, B.cols, alpha, A.data, A.outer_stride,
                B.data, B.outer_stride);
}

template <class T, class I>
void xsyrk_LN(T alpha, MatrixView<const T, I> A, T beta, MatrixView<T, I> C) {
    GUANAQO_ASSUME(C.rows == C.cols);
    GUANAQO_ASSUME(A.rows == C.rows);
    xsyrk<T, I>(CblasColMajor, CblasLower, CblasNoTrans, C.rows, A.cols, alpha,
                A.data, A.outer_stride, beta, C.data, C.outer_stride);
}

template <class T, class I>
void xsyrk_LT(T alpha, MatrixView<const T, I> A, T beta, MatrixView<T, I> C) {
    GUANAQO_ASSUME(C.rows == C.cols);
    GUANAQO_ASSUME(A.cols == C.rows);
    xsyrk<T, I>(CblasColMajor, CblasLower, CblasTrans, C.rows, A.rows, alpha,
                A.data, A.outer_stride, beta, C.data, C.outer_stride);
}

template <class T, class I>
void xtrsm_LLNN(T alpha, MatrixView<const T, I> A, MatrixView<T, I> B) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.cols == B.rows);
    xtrsm<T, I>(CblasColMajor, CblasLeft, CblasLower, CblasNoTrans,
                CblasNonUnit, B.rows, B.cols, alpha, A.data, A.outer_stride,
                B.data, B.outer_stride);
}

template <class T, class I>
void xtrsm_LLTN(T alpha, MatrixView<const T, I> A, MatrixView<T, I> B) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.rows == B.rows);
    xtrsm<T, I>(CblasColMajor, CblasLeft, CblasLower, CblasTrans, CblasNonUnit,
                B.rows, B.cols, alpha, A.data, A.outer_stride, B.data,
                B.outer_stride);
}

template <class T, class I>
void xtrsm_RLNN(T alpha, MatrixView<const T, I> A, MatrixView<T, I> B) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.cols == B.cols);
    xtrsm<T, I>(CblasColMajor, CblasRight, CblasLower, CblasNoTrans,
                CblasNonUnit, B.rows, B.cols, alpha, A.data, A.outer_stride,
                B.data, B.outer_stride);
}

template <class T, class I>
void xtrsm_RLTN(T alpha, MatrixView<const T, I> A, MatrixView<T, I> B) {
    GUANAQO_ASSUME(A.rows == A.cols);
    GUANAQO_ASSUME(A.cols == B.cols);
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
    I info = 0;
    xpotrf<T, I>("L", A.rows, A.data, A.outer_stride, &info);
    lapack_throw_on_err("xpotrf_L", info);
}

/// @throw lapack_error
template <class T, class I>
void xtrtri_LN(MatrixView<T, I> A) {
    GUANAQO_ASSUME(A.rows == A.cols);
    I info = 0;
    xtrtri<T, I>("L", "N", A.rows, A.data, A.outer_stride, &info);
    lapack_throw_on_err("xtrtri_LN", info);
}

} // namespace guanaqo::blas
