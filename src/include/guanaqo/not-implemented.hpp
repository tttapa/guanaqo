#pragma once

/// @file
/// @ingroup core
/// Exception type for unimplemented functionality.

#include <guanaqo/export.h>
#include <stdexcept>

namespace guanaqo {

/// @ingroup core
struct GUANAQO_EXPORT not_implemented_error : std::logic_error {
    using std::logic_error::logic_error;
};

} // namespace guanaqo
