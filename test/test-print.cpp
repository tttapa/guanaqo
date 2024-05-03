#include <gtest/gtest.h>

#include <guanaqo/print.hpp>
#include <span>
#include <sstream>

TEST(Print, pythonVector) {
    std::ostringstream ss;
    std::vector<double> v{1, 2, 3, 4, 5, 0.1};
    guanaqo::print_python(ss, std::span{v});
    EXPECT_EQ(ss.str(),
              "[+1.00000000000000000e+00, +2.00000000000000000e+00, "
              "+3.00000000000000000e+00, +4.00000000000000000e+00, "
              "+5.00000000000000000e+00, +1.00000000000000006e-01]\n");
}

TEST(Print, matlabVector) {
    std::ostringstream ss;
    std::vector<double> v{1, 2, 3, 4, 5, 0.1};
    guanaqo::print_matlab(ss, std::span{v});
    EXPECT_EQ(ss.str(),
              "[+1.00000000000000000e+00; +2.00000000000000000e+00; "
              "+3.00000000000000000e+00; +4.00000000000000000e+00; "
              "+5.00000000000000000e+00; +1.00000000000000006e-01];\n");
}

TEST(Print, pythonMatrix) {
    std::ostringstream ss;
    std::vector<double> m{1, 3, 5, 2, 4, 0.1};
    guanaqo::detail::print_python_impl<double>(
        ss, {.data = m.data(), .rows = 3, .cols = 2});
    EXPECT_EQ(ss.str(),
              "[[+1.00000000000000000e+00, +2.00000000000000000e+00],\n"
              " [+3.00000000000000000e+00, +4.00000000000000000e+00],\n"
              " [+5.00000000000000000e+00, +1.00000000000000006e-01]]\n");
}

TEST(Print, matlabMatrix) {
    std::ostringstream ss;
    std::vector<double> m{1, 3, 5, 2, 4, 0.1};
    guanaqo::detail::print_matlab_impl<double>(
        ss, {.data = m.data(), .rows = 3, .cols = 2});
    EXPECT_EQ(ss.str(),
              "[+1.00000000000000000e+00 +2.00000000000000000e+00;\n"
              " +3.00000000000000000e+00 +4.00000000000000000e+00;\n"
              " +5.00000000000000000e+00 +1.00000000000000006e-01];\n");
}

TEST(Print, pythonVectorInt) {
    std::ostringstream ss;
    std::vector<int> v{1, 2, 3, 4, 5, -100};
    guanaqo::print_python(ss, std::span{v});
    EXPECT_EQ(ss.str(), "[1, 2, 3, 4, 5, -100]\n");
}

TEST(Print, matlabVectorInt) {
    std::ostringstream ss;
    std::vector<int> v{1, 2, 3, 4, 5, -100};
    guanaqo::print_matlab(ss, std::span{v});
    EXPECT_EQ(ss.str(), "[1; 2; 3; 4; 5; -100];\n");
}

TEST(Print, pythonVectorIndex) {
    std::ostringstream ss;
    std::vector<ptrdiff_t> v{1, 2, 3, 4, 5, -100};
    guanaqo::print_python(ss, std::span{v});
    EXPECT_EQ(ss.str(), "[1, 2, 3, 4, 5, -100]\n");
}

TEST(Print, matlabVectorIndex) {
    std::ostringstream ss;
    std::vector<ptrdiff_t> v{1, 2, 3, 4, 5, -100};
    guanaqo::print_matlab(ss, std::span{v});
    EXPECT_EQ(ss.str(), "[1; 2; 3; 4; 5; -100];\n");
}
