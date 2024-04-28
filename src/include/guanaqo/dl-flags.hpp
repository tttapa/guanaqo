#pragma once

#include <guanaqo/export.h>

namespace guanaqo {

/// Flags to be passed to `dlopen`.
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
