#pragma once

/// @file
/// @ingroup dylib
/// Flags wrapper for dynamic library loading.

#include <guanaqo/export.h>

namespace guanaqo {

/// Flags to be passed to `dlopen`.
/// @ingroup dylib
struct GUANAQO_EXPORT DynamicLoadFlags {
    /// `RTLD_GLOBAL` (true) or `RTLD_LOCAL` (false).
    bool global = false;
    /// `RTLD_LAZY` (true) or `RTLD_NOW` (false).
    bool lazy = false;
    /// `RTLD_NODELETE`
    bool nodelete = true;
    /// `RTLD_DEEPBIND`
    bool deepbind = true;

    operator int() const;
};

} // namespace guanaqo
