/**
 * @file
 * This file provides simple overloaded wrappers around standard BLAS functions.
 */

#pragma once

#include <guanaqo/blas/blas.hpp>

namespace guanaqo::blas {

template <class T, class I>
void xgemv(CBLAS_LAYOUT Layout, CBLAS_TRANSPOSE TransA, I M, I N,
           std::type_identity_t<T> alpha, std::type_identity_t<const T *> A,
           I lda, std::type_identity_t<const T *> X, I incX,
           std::type_identity_t<T> beta, T *Y, I incY);

template <class T, class I>
void xgemm(CBLAS_LAYOUT Layout, CBLAS_TRANSPOSE TransA, CBLAS_TRANSPOSE TransB,
           I M, I N, I K, std::type_identity_t<T> alpha,
           std::type_identity_t<const T *> A, I lda,
           std::type_identity_t<const T *> B, I ldb,
           std::type_identity_t<T> beta, T *C, I ldc);

template <class T, class I>
void xgemmt(CBLAS_LAYOUT Layout, CBLAS_UPLO uplo, CBLAS_TRANSPOSE TransA,
            CBLAS_TRANSPOSE TransB, I N, I K, std::type_identity_t<T> alpha,
            std::type_identity_t<const T *> A, I lda,
            std::type_identity_t<const T *> B, I ldb,
            std::type_identity_t<T> beta, T *C, I ldc);

template <class T, class I>
void xsymv(CBLAS_LAYOUT Layout, CBLAS_UPLO uplo, I N,
           std::type_identity_t<T> alpha, std::type_identity_t<const T *> A,
           I lda, std::type_identity_t<const T *> X, I incX,
           std::type_identity_t<T> beta, T *Y, I incY);

template <class T, class I>
void xtrmv(CBLAS_LAYOUT Layout, CBLAS_UPLO Uplo, CBLAS_TRANSPOSE TransA,
           CBLAS_DIAG Diag, I N, std::type_identity_t<const T *> A, I lda, T *X,
           I incX);

template <class T, class I>
void xtrsv(CBLAS_LAYOUT Layout, CBLAS_UPLO Uplo, CBLAS_TRANSPOSE TransA,
           CBLAS_DIAG Diag, I N, std::type_identity_t<const T *> A, I lda, T *X,
           I incX);

template <class T, class I>
void xtrmm(CBLAS_LAYOUT Layout, CBLAS_SIDE Side, CBLAS_UPLO Uplo,
           CBLAS_TRANSPOSE TransA, CBLAS_DIAG Diag, I M, I N,
           std::type_identity_t<T> alpha, std::type_identity_t<const T *> A,
           I lda, T *B, I ldb);

template <class T, class I>
void xsyrk(CBLAS_LAYOUT Layout, CBLAS_UPLO Uplo, CBLAS_TRANSPOSE Trans, I N,
           I K, std::type_identity_t<T> alpha,
           std::type_identity_t<const T *> A, I lda,
           std::type_identity_t<T> beta, T *C, I ldc);

template <class T, class I>
void xtrsm(CBLAS_LAYOUT Layout, CBLAS_SIDE Side, CBLAS_UPLO Uplo,
           CBLAS_TRANSPOSE TransA, CBLAS_DIAG Diag, I M, I N,
           std::type_identity_t<T> alpha, std::type_identity_t<const T *> A,
           I lda, T *B, I ldb);

template <class T, class I>
void xsytrf_rk(const char *uplo, const I *n, T *a, const I *lda, T *e, I *ipiv,
               T *work, const I *lwork, I *info);

template <class T, class I>
void xtrtrs(const char *uplo, const char *trans, const char *diag, const I *n,
            const I *nrhs, std::type_identity_t<const T *> A, const I *ldA,
            T *B, const I *ldB, I *info);

template <class T, class I>
void xscal(I N, std::type_identity_t<T> alpha, T *X, I incX);

template <class T, class I>
void xpotrf(const char *uplo, I n, T *a, I lda, I *info);

template <class T, class I>
void xlauum(const char *uplo, I n, T *a, I lda, I *info);

template <class T, class I>
void xtrtri(const char *uplo, const char *diag, I n, T *a, I lda, I *info);

template <class T, class I>
void xgemv_batch_strided(CBLAS_LAYOUT layout, CBLAS_TRANSPOSE trans, I m, I n,
                         std::type_identity_t<T> alpha,
                         std::type_identity_t<const T *> a, I lda, I stridea,
                         std::type_identity_t<const T *> x, I incx, I stridex,
                         std::type_identity_t<T> beta, T *y, I incy, I stridey,
                         I batch_size);

template <class T, class I>
void xgemm_batch_strided(CBLAS_LAYOUT Layout, CBLAS_TRANSPOSE TransA,
                         CBLAS_TRANSPOSE TransB, I M, I N, I K,
                         std::type_identity_t<T> alpha,
                         std::type_identity_t<const T *> A, I lda, I stridea,
                         std::type_identity_t<const T *> B, I ldb, I strideb,
                         std::type_identity_t<T> beta, T *C, I ldc, I stridec,
                         I batch_size);

template <class T, class I>
void xsyrk_batch_strided(CBLAS_LAYOUT Layout, CBLAS_UPLO Uplo,
                         CBLAS_TRANSPOSE Trans, I N, I K,
                         std::type_identity_t<T> alpha,
                         std::type_identity_t<const T *> A, I lda, I stridea,
                         std::type_identity_t<T> beta, T *C, I ldc, I stridec,
                         I batch_size);

template <class T, class I>
void xtrsm_batch_strided(CBLAS_LAYOUT Layout, CBLAS_SIDE Side, CBLAS_UPLO Uplo,
                         CBLAS_TRANSPOSE TransA, CBLAS_DIAG Diag, I M, I N,
                         std::type_identity_t<T> alpha,
                         std::type_identity_t<const T *> A, I lda, I stridea,
                         T *B, I ldb, I strideb, I batch_size);

template <class T, class I>
void xpotrf_batch_strided(const char *Uplo, I N, T *A, I lda, I stridea,
                          I batch_size);

} // namespace guanaqo::blas
