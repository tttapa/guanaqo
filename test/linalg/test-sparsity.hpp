#pragma once

#include <Eigen/Core>

#include <guanaqo/linalg/sparsity-conversions.hpp>
#include <guanaqo/linalg/sparsity.hpp>

namespace sp = guanaqo::linalg::sparsity;
using guanaqo::linalg::index_t;
using indexvec = Eigen::VectorX<index_t>;
using real_t   = double;
using vec      = Eigen::VectorXd;
using guanaqo::linalg::sparsity::cast_sz;
using sp::Sparsity;

#include <gtest/gtest.h>
#include <test-util/eigen-matchers.hpp>

#include <vector>

#include <Eigen/Sparse>
using mat    = Eigen::MatrixX<real_t>;
using intvec = Eigen::VectorX<int>;

template <class T>
auto spn(T &&t) {
    return std::span{t.data(), static_cast<size_t>(t.size())};
}

template <class T>
auto egn(std::span<T> t) {
    using V  = Eigen::VectorX<std::remove_const_t<T>>;
    using CV = std::conditional_t<std::is_const_v<T>, const V, V>;
    return Eigen::Map<CV>{t.data(), static_cast<Eigen::Index>(t.size())};
}

template <class T>
auto egn(std::vector<T> &t) {
    return egn(std::span{t});
}

template <class T>
auto egn(const std::vector<T> &t) {
    return egn(std::span{t});
}
