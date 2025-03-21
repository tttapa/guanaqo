#pragma once

#include <guanaqo/stringify.h>
#include <stdexcept>

/// @def GUANAQO_ASSUME(x)
/// Invokes undefined behavior if the expression @p x does not evaluate to true.
/// @throws std::logic_error in debug mode (when `NDEBUG` is not defined).

#if defined(NDEBUG) && !GUANAQO_VERIFY_ASSUMPTIONS
#if __has_cpp_attribute(assume) >= 202207L
#define GUANAQO_ASSUME(x) [[assume(x)]]
#else
#include <utility>
#if __cpp_lib_unreachable >= 202202L
#define GUANAQO_ASSUME(x)                                                      \
    do {                                                                       \
        if (!(x))                                                              \
            std::unreachable();                                                \
    } while (false)
#elif defined(__GNUC__) // GCC, Clang
#define GUANAQO_ASSUME(x)                                                      \
    do {                                                                       \
        if (!(x))                                                              \
            __builtin_unreachable();                                           \
    } while (false)
#elif defined(_MSC_VER) // MSVC
#define GUANAQO_ASSUME(x) __assume(x)
#endif // __cpp_lib_unreachable >= 202202L
#endif // __has_cpp_attribute(assume)
#endif // defined(NDEBUG) && !GUANAQO_VERIFY_ASSUMPTIONS

#define GUANAQO_ASSERT(x)                                                      \
    do {                                                                       \
        if (!(x))                                                              \
            throw std::logic_error("Assertion " #x " failed (" __FILE__        \
                                   ":" GUANAQO_STRINGIFY(__LINE__) ")");       \
    } while (false)

#ifndef GUANAQO_ASSUME
#define GUANAQO_ASSUME(x) GUANAQO_ASSERT(x)
#endif
