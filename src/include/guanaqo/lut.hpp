#pragma once

#include <array>
#include <type_traits>
#include <utility>

namespace guanaqo {

/// Returns a 2D array of the form:
///
/// ~~~
/// {{   f(0, 0),     f(0, 1),    ...,   f(0, C - 1)   },
///  {   f(1, 0),     f(1, 1),    ...,   f(1, C - 1)   },
///  {     ...,         ...,      ...,       ...       },
///  { f(R - 1, 0), f(R - 1, 1)}, ..., f(R - 1, C - 1) }}
/// ~~~
///
/// The argument @p f should be a function (or callable) that accepts two
/// arguments of type @ref std::integral_constant.
///
/// Example 1: compile-time arguments
/// ~~~
/// auto lut = guanaqo::make_2d_lut<int, 3, 4>(
///     []<int R, int C>(std::integral_constant<int, R>,
///                      std::integral_constant<int, C>) {
///         return std::pair{R, C};
///     });
/// for (const auto &row : lut) {
///     for (const auto &el : row)
///         std::cout << "(" << el.first << ", " << el.second << ")\t";
///     std::cout << "\n";
/// }
/// ~~~
/// ~~~
/// (0, 0)  (0, 1)  (0, 2)  (0, 3)
/// (1, 0)  (1, 1)  (1, 2)  (1, 3)
/// (2, 0)  (2, 1)  (2, 2)  (2, 3)
/// ~~~
///
/// Example 2: run-time arguments
/// ~~~
/// auto lut = guanaqo::make_2d_lut<int, 3, 4>([](int r, int c) {
///         return std::pair{r, c};
///     });
/// for (const auto &row : lut) {
///     for (const auto &el : row)
///         std::cout << "(" << el.first << ", " << el.second << ")\t";
///     std::cout << "\n";
/// }
/// ~~~
template <class I, I R, I C, class F>
consteval auto make_2d_lut(F f) {
    return [&]<I... Rs, I... Cs>(std::integer_sequence<I, Rs...>,
                                 std::integer_sequence<I, Cs...>) {
        auto make_row = [&]<I Rr>(std::integral_constant<I, Rr>) {
            using elem_t = std::common_type_t<decltype(f(
                std::integral_constant<I, Rr>(),
                std::integral_constant<I, Cs>()))...>;
            return std::array<elem_t, C>{
                f(std::integral_constant<I, Rr>(),
                  std::integral_constant<I, Cs>())...,
            };
        };
        using row_t = std::common_type_t<decltype(make_row(
            std::integral_constant<I, Rs>()))...>;
        return std::array<row_t, R>{
            make_row(std::integral_constant<I, Rs>())...,
        };
    }(std::make_integer_sequence<I, R>(), std::make_integer_sequence<I, C>());
}

/// Returns an array of the form:
///
/// ~~~
/// {   f(0),     f(1),    ...,   f(N - 1)   }
/// ~~~
///
/// The argument @p f should be a function (or callable) that accepts an
/// argument of type @ref std::integral_constant.
template <class I, I N, class F>
consteval auto make_1d_lut(F f) {
    return [&]<I... Ns>(std::integer_sequence<I, Ns...>) {
        using elem_t =
            std::common_type_t<decltype(f(std::integral_constant<I, Ns>()))...>;
        return std::array<elem_t, N>{f(std::integral_constant<I, Ns>())...};
    }(std::make_integer_sequence<I, N>());
}

} // namespace guanaqo
