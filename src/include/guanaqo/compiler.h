#pragma once

/// @file
/// @ingroup macros
/// Compiler feature detection helpers.

#ifdef __clang__
#define GUANAQO_CLANG_OLDER_THAN(maj) (__clang_maj__ < (maj))
#else
#define GUANAQO_CLANG_OLDER_THAN(maj) 0
#endif

#if defined(__GNUC__) && !defined(__clang__)
#define GUANAQO_GCC_OLDER_THAN(maj) (__GNUC__ < (maj))
#else
#define GUANAQO_GCC_OLDER_THAN(maj) 0
#endif

#ifdef _MSC_VER
#define GUANAQ_MSVC_OLDER_THAN(maj, min) (_MSC_VER < ((maj) * 100 + (min)))
#else
#define GUANAQ_MSVC_OLDER_THAN(maj, min) 0
#endif
