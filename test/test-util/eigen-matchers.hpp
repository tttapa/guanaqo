#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <guanaqo/demangled-typename.hpp>
#include <guanaqo/eigen/view.hpp>
#include <guanaqo/print.hpp>

#include <Eigen/Core>
#include <limits>

/// @file
/// @see https://google.github.io/googletest/reference/matchers.html#defining-matchers

namespace guanaqo_test {
template <class T>
void print(std::ostream &os, const T &arg) {
    using Scalar = typename T::Scalar;
    Eigen::MatrixX<Scalar> M{arg};
    guanaqo::print_python(os, guanaqo::as_view(M));
}
} // namespace guanaqo_test

// Suppress googletest's own output:
namespace Eigen {
inline void PrintTo(const Eigen::MatrixXd &mat, std::ostream *os) {
    *os << "Eigen::MatrixXd of shape (" << mat.rows() << "x" << mat.cols()
        << ") (output suppressed)";
}
inline void PrintTo(const Eigen::MatrixXf &mat, std::ostream *os) {
    *os << "Eigen::MatrixXf of shape (" << mat.rows() << "x" << mat.cols()
        << ") (output suppressed)";
}
template <typename Derived>
void PrintTo(const Derived &mat, std::ostream *os)
    requires std::is_base_of_v<DenseBase<Derived>, Derived>
{
    *os << "Eigen expression (" << guanaqo::demangled_typename(typeid(Derived))
        << ") of shape (" << mat.rows() << "x" << mat.cols()
        << ") (output suppressed)";
}
} // namespace Eigen

MATCHER_P(EigenEqual, expect, "") {
    auto diffnorm = (arg - expect).template lpNorm<Eigen::Infinity>();
    if (!arg.allFinite())
        diffnorm = std::numeric_limits<decltype(diffnorm)>::quiet_NaN();
    if (auto *os = result_listener->stream()) {
        *os << "\nactual = ...\n";
        guanaqo_test::print(*os, arg);
        *os << "and expected = ...\n";
        guanaqo_test::print(*os, expect);
        *os << "with difference = ...\n";
        guanaqo_test::print(*os, arg - expect);
        *os << "which has infinity norm " << diffnorm;
    }
    return diffnorm == 0;
}

MATCHER_P2(EigenAlmostEqual, expect, atol, "") {
    auto diffnorm = (arg - expect).template lpNorm<Eigen::Infinity>();
    if (!arg.allFinite())
        diffnorm = std::numeric_limits<decltype(diffnorm)>::quiet_NaN();
    if (auto *os = result_listener->stream()) {
        *os << "\nactual = ...\n";
        guanaqo_test::print(*os, arg);
        *os << "and expected = ...\n";
        guanaqo_test::print(*os, expect);
        *os << "with difference = ...\n";
        guanaqo_test::print(*os, arg - expect);
        *os << "which has infinity norm                      " << diffnorm;
        *os << ",\nwhich is greater than the absolute tolerance " << atol;
    }
    return diffnorm <= atol;
}

MATCHER_P2(EigenAlmostEqualRel, expect, rtol, "") {
    auto denom =
        expect.unaryExpr([](auto x) { return x == 0 ? decltype(x)(1) : x; });
    auto diffnorm =
        (arg - expect).cwiseQuotient(denom).template lpNorm<Eigen::Infinity>();
    if (!arg.allFinite())
        diffnorm = std::numeric_limits<decltype(diffnorm)>::quiet_NaN();
    if (auto *os = result_listener->stream()) {
        *os << "\nactual = ...\n";
        guanaqo_test::print(*os, arg);
        *os << "and expected = ...\n";
        guanaqo_test::print(*os, expect);
        *os << "with difference = ...\n";
        guanaqo_test::print(*os, arg - expect);
        *os << "which has relative infinity norm             " << diffnorm;
        *os << ",\nwhich is greater than the relative tolerance " << rtol;
    }
    return diffnorm <= rtol;
}
