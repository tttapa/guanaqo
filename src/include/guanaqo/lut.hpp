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
        return std::array<row_t, R>{{
            make_row(std::integral_constant<I, Rs>())...,
        }};
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
        return std::array<elem_t, N>{{f(std::integral_constant<I, Ns>())...}};
    }(std::make_integer_sequence<I, N>());
}

namespace detail {

template <auto... Args>
struct lut;

// Recursive case: integer range
template <std::integral I, I N, auto... Ns>
struct lut<N, Ns...> {
    template <class F, class... Args>
    static consteval auto make(F f, Args... args) {
        return [&]<I... Is>(std::integer_sequence<I, Is...>) consteval {
            using elem_t = std::common_type_t<decltype(lut<Ns...>::make(
                f, args..., std::integral_constant<I, Is>{}))...>;
            return std::array<elem_t, N>{{lut<Ns...>::make(
                f, args..., std::integral_constant<I, Is>{})...}};
        }(std::make_integer_sequence<I, N>());
    }
};

// Recursive case: array
template <class Elem, std::size_t N, std::array<Elem, N> Arr, auto... Ns>
struct lut<Arr, Ns...> {
    template <class F, class... Args>
    static consteval auto make(F f, Args... args) {
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) consteval {
            using elem_t = std::common_type_t<decltype(lut<Ns...>::make(
                f, args..., std::integral_constant<Elem, Arr[Is]>{}))...>;
            return std::array<elem_t, N>{{lut<Ns...>::make(
                f, args..., std::integral_constant<Elem, Arr[Is]>{})...}};
        }(std::make_index_sequence<N>());
    }
};

// Base case: no ranges left → call f with all collected arguments
template <>
struct lut<> {
    template <class F, class... Args>
    static consteval auto make(F f, Args... args) {
        return f(args...);
    }
};

} // namespace detail

/// Generalization of @ref make_1d_lut and @ref make_2d_lut. Generates an
/// n-dimensional array of the given function @p f applied point-wise to the
/// cartesian product of the ranges given as template parameters.
///
/// @tparam Ranges
///         Integers or arrays of [structural types](https://en.cppreference.com/w/cpp/language/template_parameters.html#Constant_template_parameter):
///         an integer N results in the half-open range [0, N), and arrays
///         result in all array elements being passed to the given function.
///
/// Example:
/// ~~~
/// auto lut = guanaqo::make_lut<3, 4, std::array{10, 20, 30, 40, 50}>([](int i, int j, int k) {
///     return std::tuple{i, j, k};
/// });
/// for (const auto &slice : lut) {
///     for (const auto &row : slice) {
///         for (const auto [i, j, k] : row)
///             std::cout << "(" << i << ", " << j << ", " << k << ")\t";
///         std::cout << "\n";
///     }
///     std::cout << "\n";
/// }
/// ~~~
/// ~~~
/// (0, 0, 10)  (0, 0, 20)  (0, 0, 30)  (0, 0, 40)  (0, 0, 50)
/// (0, 1, 10)  (0, 1, 20)  (0, 1, 30)  (0, 1, 40)  (0, 1, 50)
/// (0, 2, 10)  (0, 2, 20)  (0, 2, 30)  (0, 2, 40)  (0, 2, 50)
/// (0, 3, 10)  (0, 3, 20)  (0, 3, 30)  (0, 3, 40)  (0, 3, 50)
///
/// (1, 0, 10)  (1, 0, 20)  (1, 0, 30)  (1, 0, 40)  (1, 0, 50)
/// (1, 1, 10)  (1, 1, 20)  (1, 1, 30)  (1, 1, 40)  (1, 1, 50)
/// (1, 2, 10)  (1, 2, 20)  (1, 2, 30)  (1, 2, 40)  (1, 2, 50)
/// (1, 3, 10)  (1, 3, 20)  (1, 3, 30)  (1, 3, 40)  (1, 3, 50)
///
/// (2, 0, 10)  (2, 0, 20)  (2, 0, 30)  (2, 0, 40)  (2, 0, 50)
/// (2, 1, 10)  (2, 1, 20)  (2, 1, 30)  (2, 1, 40)  (2, 1, 50)
/// (2, 2, 10)  (2, 2, 20)  (2, 2, 30)  (2, 2, 40)  (2, 2, 50)
/// (2, 3, 10)  (2, 3, 20)  (2, 3, 30)  (2, 3, 40)  (2, 3, 50)
/// ~~~
template <auto... Ranges, class F>
consteval auto make_lut(F f) {
    return detail::lut<Ranges...>::make(f);
}

} // namespace guanaqo
