#include <gtest/gtest.h>

#include <guanaqo/eigen/view.hpp>
#include <guanaqo/mat-view.hpp>

#include <numeric>
#include <tuple>
#include <vector>

TEST(MatrixView, access) {
    std::vector<double> storage(35);
    std::iota(storage.begin(), storage.end(), 0);
    guanaqo::MatrixView<double> view{{
        .data = storage.data(),
        .rows = 7,
        .cols = 5,
    }};

    for (ptrdiff_t c = 0; c < 5; ++c)
        for (ptrdiff_t r = 0; r < 7; ++r)
            EXPECT_EQ(view(r, c), static_cast<double>(r + c * 7))
                << r << ", " << c;
}

TEST(MatrixView, accessStride) {
    std::vector<double> storage(35);
    std::iota(storage.begin(), storage.end(), 0);
    guanaqo::MatrixView<double> view{{
        .data         = storage.data(),
        .rows         = 6,
        .cols         = 5,
        .outer_stride = 7,
    }};

    for (ptrdiff_t c = 0; c < 5; ++c)
        for (ptrdiff_t r = 0; r < 6; ++r)
            EXPECT_EQ(view(r, c), static_cast<double>(r + c * 7))
                << r << ", " << c;
}

TEST(MatrixView, accessInnerStride) {
    std::vector<double> storage(70);
    std::iota(storage.begin(), storage.end(), 0);
    guanaqo::MatrixView<double, ptrdiff_t, ptrdiff_t> view{{
        .data         = storage.data(),
        .rows         = 6,
        .cols         = 5,
        .inner_stride = 2,
        .outer_stride = 14,
    }};

    for (ptrdiff_t c = 0; c < 5; ++c)
        for (ptrdiff_t r = 0; r < 6; ++r)
            EXPECT_EQ(view(r, c), static_cast<double>(2 * r + c * 14))
                << r << ", " << c;
}

TEST(MatrixView, asEigen) {
    std::vector<double> storage(35);
    std::iota(storage.begin(), storage.end(), 0);
    guanaqo::MatrixView<double> view{{
        .data = storage.data(),
        .rows = 7,
        .cols = 5,
    }};
    auto eigen_view = as_eigen(view);
    auto view_2     = guanaqo::as_view(eigen_view);

    static_assert(eigen_view.InnerStrideAtCompileTime == 1);
    static_assert(decltype(view_2)::inner_stride_type{} == 1);

    for (Eigen::Index c = 0; c < 5; ++c)
        for (Eigen::Index r = 0; r < 7; ++r)
            EXPECT_EQ(eigen_view(r, c), static_cast<double>(r + c * 7))
                << r << ", " << c;

    for (ptrdiff_t c = 0; c < 5; ++c)
        for (ptrdiff_t r = 0; r < 7; ++r)
            EXPECT_EQ(view_2(r, c), static_cast<double>(r + c * 7))
                << r << ", " << c;
}

TEST(MatrixView, asEigenStride) {
    std::vector<double> storage(35);
    std::iota(storage.begin(), storage.end(), 0);
    guanaqo::MatrixView<double> view{{
        .data         = storage.data(),
        .rows         = 6,
        .cols         = 5,
        .outer_stride = 7,
    }};
    auto eigen_view = as_eigen(view);
    auto view_2     = guanaqo::as_view(eigen_view);

    static_assert(eigen_view.InnerStrideAtCompileTime == 1);
    static_assert(decltype(view_2)::inner_stride_type{} == 1);

    for (Eigen::Index c = 0; c < 5; ++c)
        for (Eigen::Index r = 0; r < 6; ++r)
            EXPECT_EQ(eigen_view(r, c), static_cast<double>(r + c * 7))
                << r << ", " << c;

    for (ptrdiff_t c = 0; c < 5; ++c)
        for (ptrdiff_t r = 0; r < 6; ++r)
            EXPECT_EQ(view_2(r, c), static_cast<double>(r + c * 7))
                << r << ", " << c;
}

TEST(MatrixView, asEigenInnerStride) {
    std::vector<double> storage(70);
    std::iota(storage.begin(), storage.end(), 0);
    guanaqo::MatrixView<double, ptrdiff_t, ptrdiff_t> view{{
        .data         = storage.data(),
        .rows         = 6,
        .cols         = 5,
        .inner_stride = 2,
        .outer_stride = 14,
    }};
    auto eigen_view = as_eigen(view);
    auto view_2     = guanaqo::as_view(eigen_view);

    for (Eigen::Index c = 0; c < 5; ++c)
        for (Eigen::Index r = 0; r < 6; ++r)
            EXPECT_EQ(eigen_view(r, c), static_cast<double>(2 * r + c * 14))
                << r << ", " << c;

    for (ptrdiff_t c = 0; c < 5; ++c)
        for (ptrdiff_t r = 0; r < 6; ++r)
            EXPECT_EQ(view_2(r, c), static_cast<double>(2 * r + c * 14))
                << r << ", " << c;
}

TEST(MatrixView, asViewIndexType) {
    std::vector<double> storage(70);
    std::iota(storage.begin(), storage.end(), 0);
    guanaqo::MatrixView<double, ptrdiff_t, ptrdiff_t> view{{
        .data         = storage.data(),
        .rows         = 6,
        .cols         = 5,
        .inner_stride = 2,
        .outer_stride = 14,
    }};
    auto eigen_view = as_eigen(view);
    auto view_2     = as_view(eigen_view, guanaqo::with_index_type<short>);

    static_assert(std::is_same_v<decltype(view_2)::index_type, short>);

    for (Eigen::Index c = 0; c < 5; ++c)
        for (Eigen::Index r = 0; r < 6; ++r)
            EXPECT_EQ(eigen_view(r, c), static_cast<double>(2 * r + c * 14))
                << r << ", " << c;

    for (short c = 0; c < 5; ++c)
        for (short r = 0; r < 6; ++r)
            EXPECT_EQ(view_2(r, c), static_cast<double>(2 * r + c * 14))
                << r << ", " << c;
}

TEST(MatrixView, asEigenCompTimeInnerStride) {
    std::vector<double> storage(70);
    std::iota(storage.begin(), storage.end(), 0);
    using stride_t = std::integral_constant<ptrdiff_t, 2>;
    guanaqo::MatrixView<double, ptrdiff_t, stride_t> view{{
        .data         = storage.data(),
        .rows         = 6,
        .cols         = 5,
        .outer_stride = 14,
    }};
    auto eigen_view = as_eigen(view);
    auto view_2     = guanaqo::as_view(eigen_view);

    static_assert(eigen_view.InnerStrideAtCompileTime == 2);
    static_assert(decltype(view_2)::inner_stride_type{} == 2);

    for (Eigen::Index c = 0; c < 5; ++c)
        for (Eigen::Index r = 0; r < 6; ++r)
            EXPECT_EQ(eigen_view(r, c), static_cast<double>(2 * r + c * 14))
                << r << ", " << c;

    for (ptrdiff_t c = 0; c < 5; ++c)
        for (ptrdiff_t r = 0; r < 6; ++r)
            EXPECT_EQ(view_2(r, c), static_cast<double>(2 * r + c * 14))
                << r << ", " << c;
}

TEST(MatrixView, asViewTemporaryRef) {
    // Temporaries of Eigen::Ref are okay, but not temporaries of Eigen::Matrix.
    Eigen::MatrixXd matrix(7, 5);
    std::ignore = guanaqo::as_view(Eigen::Ref<Eigen::MatrixXd>(matrix));
    // Note: Eigen::Ref<const Eigen::Matrix> could still contain its own
    // storage, so there's still a danger of dangling, but we leave it in as an
    // escape hatch, and also because the storage is optional, we cannot know at
    // compile-time whether it is in use or not.
    std::ignore = guanaqo::as_view(Eigen::Ref<const Eigen::MatrixXd>(matrix));
}

#if 0
// Should fail to compile:
// Refusing to return a view to a temporary Eigen matrix with its own storage
TEST(MatrixView, asViewTemporaryMatrix) {
    guanaqo::as_view(Eigen::MatrixXd::Zero(7, 5).eval());
}
#endif
