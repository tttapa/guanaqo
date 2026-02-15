#pragma once

/// @file
/// @ingroup macros
/// Assertion and assumption macros with debug/release semantics.

#include <guanaqo/stringify.h>
#include <stdexcept>

/// @def GUANAQO_ASSUME(x)
/// @ingroup macros
/// Invokes undefined behavior if the expression @p x does not evaluate to true.
/// @throws std::logic_error in debug mode if @p x is false.

/// @def GUANAQO_DEBUG_ASSERT(x)
/// @ingroup macros
/// Check the expression @p x (in debug mode only).
/// @throws std::logic_error in debug mode if @p x is false.

/// @def GUANAQO_ASSERT(x)
/// @ingroup macros
/// Check the expression @p x (regardless of debug or release mode).
/// @throws std::logic_error if @p x is false.

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
#define GUANAQO_DEBUG_ASSERT(x)                                                \
    do {                                                                       \
        (void)sizeof(x);                                                       \
    } while (false)
#endif // defined(NDEBUG) && !GUANAQO_VERIFY_ASSUMPTIONS

#ifndef GUANAQO_DEBUG_ASSERT
#define GUANAQO_DEBUG_ASSERT(x)                                                \
    do {                                                                       \
        if (!(x))                                                              \
            throw std::logic_error("Assertion " #x " failed (" __FILE__        \
                                   ":" GUANAQO_STRINGIFY(__LINE__) ")");       \
    } while (false)
#endif

#define GUANAQO_ASSERT(x)                                                      \
    do {                                                                       \
        if (!(x))                                                              \
            throw std::logic_error("Assertion " #x " failed (" __FILE__        \
                                   ":" GUANAQO_STRINGIFY(__LINE__) ")");       \
    } while (false)

#ifndef GUANAQO_ASSUME
#define GUANAQO_ASSUME(x) GUANAQO_ASSERT(x)
#endif
