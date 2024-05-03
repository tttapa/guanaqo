#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <guanaqo/print.hpp>

#include <Eigen/Core>

/// @file
/// @see https://google.github.io/googletest/reference/matchers.html#defining-matchers

namespace guanaqo_test {
template <class T>
void print(std::ostream &os, const T &arg) {
    using Scalar = typename T::Scalar;
    Eigen::MatrixX<Scalar> M{arg};
    guanaqo::detail::print_python_impl(os, guanaqo::MatrixView<const Scalar>{
                                               .data   = M.data(),
                                               .rows   = M.rows(),
                                               .cols   = M.cols(),
                                               .stride = M.outerStride(),
                                           });
}
} // namespace guanaqo_test

MATCHER_P(EigenEqual, expect, "") {
    auto diffnorm = (arg - expect).template lpNorm<Eigen::Infinity>();
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
    auto diffnorm =
        (arg - expect).cwiseQuotient(expect).template lpNorm<Eigen::Infinity>();
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
