#pragma once

#ifdef GUANAQO_WITH_QUAD_PRECISION

#include <guanaqo/export.h>
#include <iosfwd>

namespace guanaqo {
GUANAQO_EXPORT
std::ostream &operator<<(std::ostream &os, __float128 f);
} // namespace guanaqo
using guanaqo::operator<<;

#endif
