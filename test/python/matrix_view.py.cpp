#include <guanaqo/mat-view.hpp>
#include <guanaqo/nanobind/matrix-view.hpp>
#include <cstddef>
#include <type_traits>

NB_MODULE(MODULE_NAME, m) {
    m.def("times_two",
          [](guanaqo::MatrixView<double, ptrdiff_t, ptrdiff_t> mat) {
              for (std::ptrdiff_t c = 0; c < mat.cols; ++c)
                  for (std::ptrdiff_t r = 0; r < mat.rows; ++r)
                      mat(r, c) *= 2.0;
          });
    m.def("times_two_F", [](guanaqo::MatrixView<double> mat) {
        for (std::ptrdiff_t c = 0; c < mat.cols; ++c)
            for (std::ptrdiff_t r = 0; r < mat.rows; ++r)
                mat(r, c) *= 2.0;
    });
    m.def("times_two_F4",
          [](guanaqo::MatrixView<double, ptrdiff_t,
                                 std::integral_constant<ptrdiff_t, 4>>
                 mat) {
              for (std::ptrdiff_t c = 0; c < mat.cols; ++c)
                  for (std::ptrdiff_t r = 0; r < mat.rows; ++r)
                      mat(r, c) *= 2.0;
          });
    m.def("times_two_C", [](guanaqo::MatrixViewRM<double> mat) {
        for (std::ptrdiff_t c = 0; c < mat.cols; ++c)
            for (std::ptrdiff_t r = 0; r < mat.rows; ++r)
                mat(r, c) *= 2.0;
    });
}
