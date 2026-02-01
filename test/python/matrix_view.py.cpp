#include <guanaqo/mat-view.hpp>
#include <guanaqo/nanobind/matrix-view.hpp>
#include <cstddef>
#include <type_traits>

NB_MODULE(MODULE_NAME, m) {
    using namespace nanobind::literals;
    m.def(
        "times_two",
        [](guanaqo::MatrixView<double, ptrdiff_t, ptrdiff_t> mat) {
            for (std::ptrdiff_t c = 0; c < mat.cols; ++c)
                for (std::ptrdiff_t r = 0; r < mat.rows; ++r)
                    mat(r, c) *= 2.0;
        },
        "mat"_a.noconvert());
    m.def(
        "times_two_F",
        [](guanaqo::MatrixView<double> mat) {
            for (std::ptrdiff_t c = 0; c < mat.cols; ++c)
                for (std::ptrdiff_t r = 0; r < mat.rows; ++r)
                    mat(r, c) *= 2.0;
        },
        "mat"_a.noconvert());
    m.def(
        "times_two_F4",
        [](guanaqo::MatrixView<double, ptrdiff_t,
                               std::integral_constant<ptrdiff_t, 4>>
               mat) {
            for (std::ptrdiff_t c = 0; c < mat.cols; ++c)
                for (std::ptrdiff_t r = 0; r < mat.rows; ++r)
                    mat(r, c) *= 2.0;
        },
        "mat"_a.noconvert());
    m.def(
        "times_two_C",
        [](guanaqo::MatrixViewRM<double> mat) {
            for (std::ptrdiff_t c = 0; c < mat.cols; ++c)
                for (std::ptrdiff_t r = 0; r < mat.rows; ++r)
                    mat(r, c) *= 2.0;
        },
        "mat"_a.noconvert());

    m.def(
        "ptr_const",
        [](guanaqo::MatrixView<const double, ptrdiff_t, ptrdiff_t> mat) {
            return reinterpret_cast<std::uintptr_t>(mat.data);
        },
        "mat"_a);
    m.def(
        "ptr_const_F",
        [](guanaqo::MatrixView<const double> mat) {
            return reinterpret_cast<std::uintptr_t>(mat.data);
        },
        "mat"_a);
    m.def(
        "ptr_const_F4",
        [](guanaqo::MatrixView<const double, ptrdiff_t,
                               std::integral_constant<ptrdiff_t, 4>>
               mat) { return reinterpret_cast<std::uintptr_t>(mat.data); },
        "mat"_a);
    m.def(
        "ptr_const_C",
        [](guanaqo::MatrixViewRM<const double> mat) {
            return reinterpret_cast<std::uintptr_t>(mat.data);
        },
        "mat"_a);
}
