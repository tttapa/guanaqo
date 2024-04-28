#ifdef GUANAQO_WITH_QUAD_PRECISION

#include <guanaqo/quadmath/quadmath-print.hpp>
#include <guanaqo/quadmath/quadmath.hpp>

#include <cassert>
#include <ostream>

namespace guanaqo {
std::ostream &operator<<(std::ostream &os, __float128 f) {
    char buf[128];
    auto precision = static_cast<int>(os.precision());
    [[maybe_unused]] int n =
        quadmath_snprintf(buf, sizeof(buf), "%#.*Qg", precision, f);
    assert((size_t)n < sizeof buf);
    return os << buf;
}
} // namespace guanaqo

#endif