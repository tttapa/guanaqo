#pragma once

/// @file
/// @ingroup macros
/// Stringify and token concatenation helpers.

/// Concatenate the given tokens.
/// @ingroup macros
#define GUANAQO_CAT(a, b) GUANAQO_CAT_IMPL(a, b)
#define GUANAQO_CAT_IMPL(a, b) a##b
/// Convert the given token to a string literal.
/// @ingroup macros
#define GUANAQO_STRINGIFY(s) GUANAQO_STRINGIFY_IMPL(s)
#define GUANAQO_STRINGIFY_IMPL(s) #s
