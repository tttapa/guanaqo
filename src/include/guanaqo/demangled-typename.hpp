#pragma once

/// @file
/// @ingroup core
/// Pretty-print type names.

#include <guanaqo/export.h>

#include <string>
#include <typeinfo>

namespace guanaqo {

/// Get the pretty name of the given type as a string.
/// @ingroup core
GUANAQO_EXPORT std::string demangled_typename(const std::type_info &t);

} // namespace guanaqo
