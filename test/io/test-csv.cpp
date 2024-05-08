#include <guanaqo/io/csv.hpp>
#include <gtest/gtest.h>
#include <array>
#include <cstring>
#include <limits>
#include <sstream>

TEST(csv, read) {
    std::istringstream is{"1.00000000000000000,"
                          "+2.00000000000000000,"
                          "-3.00000000000000000,"
                          "4.00000000000000000,"
                          "5.00000000000000000\n"};
    std::vector<double> v(5);
    guanaqo::io::csv_read_row(is, std::span{v});
    std::vector<double> expected{1, +2, -3, 4, 5};
    EXPECT_EQ(v, expected);
}

TEST(csv, readStaticExtent) {
    std::istringstream is{"1.00000000000000000,"
                          "+2.00000000000000000,"
                          "-3.00000000000000000,"
                          "4.00000000000000000,"
                          "5.00000000000000000\n"};
    std::array<double, 5> v;
    guanaqo::io::csv_read_row(is, std::span{v});
    std::array<double, 5> expected{1, +2, -3, 4, 5};
    EXPECT_EQ(v, expected);
}

TEST(csv, readMultiplePerBuffer) {
    std::istringstream is{"1.00,"
                          "+2.0,"
                          "-3.00000000000000000,"
                          "4.00,"
                          "5.00000000000000000\n"};
    std::vector<double> v(5);
    guanaqo::io::csv_read_row(is, std::span{v});
    std::vector<double> expected{1, +2, -3, 4, 5};
    EXPECT_EQ(v, expected);
}

TEST(csv, readCharsPastEnd) {
    std::istringstream is{"1.00000000000000000,"
                          "+2.00000000000000000,"
                          "-3.00000000000000000,"
                          "4.00000000000000000,"
                          "5.00000000000000000\nfoobar"};
    std::vector<double> v(5);
    guanaqo::io::csv_read_row(is, std::span{v});
    std::vector<double> expected{1, +2, -3, 4, 5};
    EXPECT_EQ(v, expected);
}

TEST(csv, readComment1) {
    std::istringstream is{"# This is a comment\n"
                          "1.00000000000000000,"
                          "+2.00000000000000000,"
                          "-3.00000000000000000,"
                          "4.00000000000000000,"
                          "5.00000000000000000\nfoobar"};
    std::vector<double> v(5);
    guanaqo::io::csv_read_row(is, std::span{v});
    std::vector<double> expected{1, +2, -3, 4, 5};
    EXPECT_EQ(v, expected);
}

TEST(csv, readComment2) {
    std::istringstream is{"# This is a comment\n"
                          "# This is a second comment\n"
                          "1.00000000000000000,"
                          "+2.00000000000000000,"
                          "-3.00000000000000000,"
                          "4.00000000000000000,"
                          "5.00000000000000000\nfoobar"};
    std::vector<double> v(5);
    guanaqo::io::csv_read_row(is, std::span{v});
    std::vector<double> expected{1, +2, -3, 4, 5};
    EXPECT_EQ(v, expected);
}

TEST(csv, readCommentLong) {
    std::istringstream is{
        "# This is a very long comment "
        "1234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890"
        "1234567890123456789012345678901234567890123456789012345678901234567890"
        "12345678901234567890\n"
        "# This is a second comment\n"
        "1.00000000000000000,"
        "+2.00000000000000000,"
        "-3.00000000000000000,"
        "4.00000000000000000,"
        "5.00000000000000000\nfoobar"};
    std::vector<double> v(5);
    guanaqo::io::csv_read_row(is, std::span{v});
    std::vector<double> expected{1, +2, -3, 4, 5};
    EXPECT_EQ(v, expected);
}

TEST(csv, readEOF) {
    std::istringstream is{"1.00000000000000000,"
                          "2.00000000000000000,"
                          "3.00000000000000000,"
                          "4.00000000000000000,"
                          "5.00000000000000000"};
    std::vector<double> v(5);
    guanaqo::io::csv_read_row(is, std::span{v});
    std::vector<double> expected{1, 2, 3, 4, 5};
    EXPECT_EQ(v, expected);
}

TEST(csv, readEndWithSep) {
    std::istringstream is{"1.00000000000000000,"
                          "2.00000000000000000,"
                          "3.00000000000000000,"
                          "4.00000000000000000,"
                          "5.00000000000000000,\n"};
    std::vector<double> v(5);
    guanaqo::io::csv_read_row(is, std::span{v});
    std::vector<double> expected{1, 2, 3, 4, 5};
    EXPECT_EQ(v, expected);
}

TEST(csv, readEndWithSepEOF) {
    std::istringstream is{"1.00000000000000000,"
                          "2.00000000000000000,"
                          "3.00000000000000000,"
                          "4.00000000000000000,"
                          "5.00000000000000000,"};
    std::vector<double> v(5);
    guanaqo::io::csv_read_row(is, std::span{v});
    std::vector<double> expected{1, 2, 3, 4, 5};
    EXPECT_EQ(v, expected);
}

TEST(csv, readLong) {
    std::istringstream is{"1.000000000000000000000000000000000000000000000000,"
                          "2.000000000000000000000000000000000000000000000000,"
                          "3.000000000000000000000000000000000000000000000000,"
                          "4.000000000000000000000000000000000000000000000000,"
                          "5.000000000000000000000000000000000000000000000000"};
    std::vector<double> v(5);
    guanaqo::io::csv_read_row(is, std::span{v});
    std::vector<double> expected{1, 2, 3, 4, 5};
    EXPECT_EQ(v, expected);
}

TEST(csv, readShort) {
    std::istringstream is{"1.0,"
                          "2.0,"
                          "3.0,"
                          "4.0,"
                          "5.0"};
    std::vector<double> v(5);
    guanaqo::io::csv_read_row(is, std::span{v});
    std::vector<double> expected{1, 2, 3, 4, 5};
    EXPECT_EQ(v, expected);
}

constexpr auto inf = std::numeric_limits<double>::infinity();
constexpr auto NaN = std::numeric_limits<double>::quiet_NaN();

TEST(csv, readNaNInf) {
    std::istringstream is{"inf,"
                          "+inf,"
                          "-inf,"
                          "nan,"
                          "-nan"};
    std::vector<double> v(5);
    guanaqo::io::csv_read_row(is, std::span{v});
    std::vector<double> expected{inf, +inf, -inf, NaN, -NaN};
    // NOLINTNEXTLINE(*-suspicious-memory-comparison)
    EXPECT_EQ(std::memcmp(v.data(), expected.data(), 5 * sizeof(*v.data())), 0);
}

TEST(csv, readEmpty) {
    std::istringstream is{"\n"};
    std::vector<double> v;
    guanaqo::io::csv_read_row(is, std::span{v});
    std::vector<double> expected;
    EXPECT_EQ(v, expected);
}

TEST(csv, readEmptyEOF) {
    std::istringstream is{""};
    std::vector<double> v;
    guanaqo::io::csv_read_row(is, std::span{v});
    std::vector<double> expected;
    EXPECT_EQ(v, expected);
}

TEST(csv, readTooMany) {
    std::istringstream is{"1.0,"
                          "2.0,"
                          "3.0,"
                          "4.0,"
                          "5.0,"
                          "6.0"};
    std::vector<double> v(5);
    EXPECT_THROW(guanaqo::io::csv_read_row(is, std::span{v}),
                 guanaqo::io::csv_read_error);
}

TEST(csv, readInvalidCharPrefix) {
    std::istringstream is{"1.0,"
                          "2.0,"
                          "k3.0,"
                          "4.0,"
                          "5.0"};
    std::vector<double> v(5);
    EXPECT_THROW(guanaqo::io::csv_read_row(is, std::span{v}),
                 guanaqo::io::csv_read_error);
}

TEST(csv, readInvalidCharInfix) {
    std::istringstream is{"1.0,"
                          "2.0,"
                          "3k.0,"
                          "4.0,"
                          "5.0"};
    std::vector<double> v(5);
    EXPECT_THROW(guanaqo::io::csv_read_row(is, std::span{v}),
                 guanaqo::io::csv_read_error);
}

TEST(csv, readInvalidCharSuffix) {
    std::istringstream is{"1.0,"
                          "2.0,"
                          "3.0k,"
                          "4.0,"
                          "5.0"};
    std::vector<double> v(5);
    EXPECT_THROW(guanaqo::io::csv_read_row(is, std::span{v}),
                 guanaqo::io::csv_read_error);
}

TEST(csv, readTooFew) {
    std::istringstream is{"1.0,"
                          "2.0,"
                          "3.0,"
                          "4.0"};
    std::vector<double> v(5);
    EXPECT_THROW(guanaqo::io::csv_read_row(is, std::span{v}),
                 guanaqo::io::csv_read_error);
}

// ---

TEST(csv, stdvecRead) {
    std::istringstream is{"1.00000000000000000,"
                          "2.00000000000000000,"
                          "3.00000000000000000,"
                          "4.00000000000000000,"
                          "5.00000000000000000\n"};
    auto v = guanaqo::io::csv_read_row_std_vector<double>(is);
    std::vector<double> expected{1, 2, 3, 4, 5};
    EXPECT_EQ(v, expected);
}

TEST(csv, stdvecReadEOF) {
    std::istringstream is{"1.00000000000000000,"
                          "2.00000000000000000,"
                          "3.00000000000000000,"
                          "4.00000000000000000,"
                          "5.00000000000000000"};
    auto v = guanaqo::io::csv_read_row_std_vector<double>(is);
    std::vector<double> expected{1, 2, 3, 4, 5};
    EXPECT_EQ(v, expected);
}

TEST(csv, stdvecReadEndWithSep) {
    std::istringstream is{"1.00000000000000000,"
                          "2.00000000000000000,"
                          "3.00000000000000000,"
                          "4.00000000000000000,"
                          "5.00000000000000000,\n"};
    auto v = guanaqo::io::csv_read_row_std_vector<double>(is);
    std::vector<double> expected{1, 2, 3, 4, 5};
    EXPECT_EQ(v, expected);
}

TEST(csv, stdvecReadEndWithSepEOF) {
    std::istringstream is{"1.00000000000000000,"
                          "2.00000000000000000,"
                          "3.00000000000000000,"
                          "4.00000000000000000,"
                          "5.00000000000000000,"};
    auto v = guanaqo::io::csv_read_row_std_vector<double>(is);
    std::vector<double> expected{1, 2, 3, 4, 5};
    EXPECT_EQ(v, expected);
}

TEST(csv, stdvecReadLong) {
    std::istringstream is{"1.000000000000000000000000000000000000000000000000,"
                          "2.000000000000000000000000000000000000000000000000,"
                          "3.000000000000000000000000000000000000000000000000,"
                          "4.000000000000000000000000000000000000000000000000,"
                          "5.000000000000000000000000000000000000000000000000"};
    auto v = guanaqo::io::csv_read_row_std_vector<double>(is);
    std::vector<double> expected{1, 2, 3, 4, 5};
    EXPECT_EQ(v, expected);
}

TEST(csv, stdvecReadShort) {
    std::istringstream is{"1.0,"
                          "2.0,"
                          "3.0,"
                          "4.0,"
                          "5.0"};
    auto v = guanaqo::io::csv_read_row_std_vector<double>(is);
    std::vector<double> expected{1, 2, 3, 4, 5};
    EXPECT_EQ(v, expected);
}

TEST(csv, stdvecReadNaNInf) {
    std::istringstream is{"inf,"
                          "-inf,"
                          "nan,"
                          "-nan"};
    auto v = guanaqo::io::csv_read_row_std_vector<double>(is);
    std::vector<double> expected{inf, -inf, NaN, -NaN};
    // NOLINTNEXTLINE(*-suspicious-memory-comparison)
    EXPECT_EQ(std::memcmp(v.data(), expected.data(), 4 * sizeof(*v.data())), 0);
}

TEST(csv, stdvecReadEmpty) {
    std::istringstream is{"\n"};
    auto v = guanaqo::io::csv_read_row_std_vector<double>(is);
    std::vector<double> expected;
    EXPECT_EQ(v, expected);
}

TEST(csv, stdvecReadEmptyEOF) {
    std::istringstream is{""};
    auto v = guanaqo::io::csv_read_row_std_vector<double>(is);
    std::vector<double> expected;
    EXPECT_EQ(v, expected);
}

TEST(csv, stdvecReadInvalidCharPrefix) {
    std::istringstream is{"1.0,"
                          "2.0,"
                          "k3.0,"
                          "4.0,"
                          "5.0"};
    EXPECT_THROW(guanaqo::io::csv_read_row_std_vector<double>(is),
                 guanaqo::io::csv_read_error);
}

TEST(csv, stdvecReadInvalidCharInfix) {
    std::istringstream is{"1.0,"
                          "2.0,"
                          "3k.0,"
                          "4.0,"
                          "5.0"};
    EXPECT_THROW(guanaqo::io::csv_read_row_std_vector<double>(is),
                 guanaqo::io::csv_read_error);
}

TEST(csv, stdvecReadInvalidCharSuffix) {
    std::istringstream is{"1.0,"
                          "2.0,"
                          "3.0k,"
                          "4.0,"
                          "5.0"};
    EXPECT_THROW(guanaqo::io::csv_read_row_std_vector<double>(is),
                 guanaqo::io::csv_read_error);
}

TEST(csv, readIndex) {
    std::istringstream is{"1,"
                          "+2,"
                          "-3,"
                          "4,"
                          "5\n"};
    std::vector<ptrdiff_t> v(5);
    guanaqo::io::csv_read_row(is, std::span{v});
    std::vector<ptrdiff_t> expected{1, +2, -3, 4, 5};
    EXPECT_EQ(v, expected);
}

TEST(csv, readIndexInvalid) {
    std::istringstream is{"1.23,"
                          "+2,"
                          "-3,"
                          "4,"
                          "5\n"};
    std::vector<ptrdiff_t> v(5);
    EXPECT_THROW(guanaqo::io::csv_read_row(is, std::span{v}),
                 guanaqo::io::csv_read_error);
}
