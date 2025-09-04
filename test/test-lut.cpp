#include <gtest/gtest.h>

#include <guanaqo/compiler.h>
#include <guanaqo/lut.hpp>

#include <tuple>
#include <utility>

TEST(lut, lut1D) {
    constexpr auto lut = guanaqo::make_1d_lut<short, 4>([]<class I>(I i) {
        static_assert(std::is_same_v<typename I::value_type, short>);
        return i();
    });
    std::array<short, 4> expected{{0, 1, 2, 3}};
    EXPECT_EQ(lut, expected);
}

TEST(lut, lut2D) {
    constexpr auto lut =
        guanaqo::make_2d_lut<short, 3, 4>([]<class I, class J>(I i, J j) {
            static_assert(std::is_same_v<typename I::value_type, short>);
            static_assert(std::is_same_v<typename J::value_type, short>);
            return std::pair{i(), j()};
        });
    std::array<std::array<std::pair<short, short>, 4>, 3> expected{{
        {{{0, 0}, {0, 1}, {0, 2}, {0, 3}}},
        {{{1, 0}, {1, 1}, {1, 2}, {1, 3}}},
        {{{2, 0}, {2, 1}, {2, 2}, {2, 3}}},
    }};
    EXPECT_EQ(lut, expected);
}

TEST(lut, lut3D) {
    constexpr auto lut = guanaqo::make_lut<3l, 4ll, 5>(
        []<class I, class J, class K>(I i, J j, K k) {
            static_assert(std::is_same_v<typename I::value_type, long>);
            static_assert(std::is_same_v<typename J::value_type, long long>);
            static_assert(std::is_same_v<typename K::value_type, int>);
            return std::tuple{i(), j(), k()};
        });
    using tup = std::tuple<long, long long, int>;
    std::array<std::array<std::array<tup, 5>, 4>, 3> expected{{
        {{{{{0, 0, 0}, {0, 0, 1}, {0, 0, 2}, {0, 0, 3}, {0, 0, 4}}},
          {{{0, 1, 0}, {0, 1, 1}, {0, 1, 2}, {0, 1, 3}, {0, 1, 4}}},
          {{{0, 2, 0}, {0, 2, 1}, {0, 2, 2}, {0, 2, 3}, {0, 2, 4}}},
          {{{0, 3, 0}, {0, 3, 1}, {0, 3, 2}, {0, 3, 3}, {0, 3, 4}}}}},
        {{{{{1, 0, 0}, {1, 0, 1}, {1, 0, 2}, {1, 0, 3}, {1, 0, 4}}},
          {{{1, 1, 0}, {1, 1, 1}, {1, 1, 2}, {1, 1, 3}, {1, 1, 4}}},
          {{{1, 2, 0}, {1, 2, 1}, {1, 2, 2}, {1, 2, 3}, {1, 2, 4}}},
          {{{1, 3, 0}, {1, 3, 1}, {1, 3, 2}, {1, 3, 3}, {1, 3, 4}}}}},
        {{{{{2, 0, 0}, {2, 0, 1}, {2, 0, 2}, {2, 0, 3}, {2, 0, 4}}},
          {{{2, 1, 0}, {2, 1, 1}, {2, 1, 2}, {2, 1, 3}, {2, 1, 4}}},
          {{{2, 2, 0}, {2, 2, 1}, {2, 2, 2}, {2, 2, 3}, {2, 2, 4}}},
          {{{2, 3, 0}, {2, 3, 1}, {2, 3, 2}, {2, 3, 3}, {2, 3, 4}}}}},
    }};
    EXPECT_EQ(lut, expected);
}

TEST(lut, lut3Darray) {
#if GUANAQO_GCC_OLDER_THAN(12) || GUANAQ_MSVC_OLDER_THAN(19, 42)
    GTEST_SKIP() << "Advanced lookup tables require GCC 12 or later, "
                    "or MSVC 19.42 or later";
#else
    using guanaqo::make_lut;
    constexpr auto lut = make_lut<3l, std::array{5ll, 6ll, 7ll, 8ll}, 5>(
        []<class I, class J, class K>(I i, J j, K k) {
            static_assert(std::is_same_v<typename I::value_type, long>);
            static_assert(std::is_same_v<typename J::value_type, long long>);
            static_assert(std::is_same_v<typename K::value_type, int>);
            return std::tuple{i(), j(), k()};
        });
    using tup = std::tuple<long, long long, int>;
    std::array<std::array<std::array<tup, 5>, 4>, 3> expected{{
        {{{{{0, 5, 0}, {0, 5, 1}, {0, 5, 2}, {0, 5, 3}, {0, 5, 4}}},
          {{{0, 6, 0}, {0, 6, 1}, {0, 6, 2}, {0, 6, 3}, {0, 6, 4}}},
          {{{0, 7, 0}, {0, 7, 1}, {0, 7, 2}, {0, 7, 3}, {0, 7, 4}}},
          {{{0, 8, 0}, {0, 8, 1}, {0, 8, 2}, {0, 8, 3}, {0, 8, 4}}}}},
        {{{{{1, 5, 0}, {1, 5, 1}, {1, 5, 2}, {1, 5, 3}, {1, 5, 4}}},
          {{{1, 6, 0}, {1, 6, 1}, {1, 6, 2}, {1, 6, 3}, {1, 6, 4}}},
          {{{1, 7, 0}, {1, 7, 1}, {1, 7, 2}, {1, 7, 3}, {1, 7, 4}}},
          {{{1, 8, 0}, {1, 8, 1}, {1, 8, 2}, {1, 8, 3}, {1, 8, 4}}}}},
        {{{{{2, 5, 0}, {2, 5, 1}, {2, 5, 2}, {2, 5, 3}, {2, 5, 4}}},
          {{{2, 6, 0}, {2, 6, 1}, {2, 6, 2}, {2, 6, 3}, {2, 6, 4}}},
          {{{2, 7, 0}, {2, 7, 1}, {2, 7, 2}, {2, 7, 3}, {2, 7, 4}}},
          {{{2, 8, 0}, {2, 8, 1}, {2, 8, 2}, {2, 8, 3}, {2, 8, 4}}}}},
    }};
    EXPECT_EQ(lut, expected);
#endif
}
