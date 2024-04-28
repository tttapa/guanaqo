#pragma once

#include <guanaqo/export.h>
#include <stdexcept>

namespace guanaqo {

struct GUANAQO_EXPORT not_implemented_error : std::logic_error {
    using std::logic_error::logic_error;
};

} // namespace guanaqo
