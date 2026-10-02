#include <guanaqo/dl-flags.hpp>

#if defined(GUANAQO_HAVE_DLFCN)
#include <dlfcn.h>
#endif

namespace guanaqo {

DynamicLoadFlags::operator int() const {
#if defined(GUANAQO_HAVE_DLFCN)
    return (global ? RTLD_GLOBAL : RTLD_LOCAL) | //
           (lazy ? RTLD_LAZY : RTLD_NOW) |       //
           (nodelete ? RTLD_NODELETE : 0) |
#ifdef RTLD_DEEPBIND
           (deepbind ? RTLD_DEEPBIND : 0) |
#endif
           0;
#else
    return 0;
#endif
}

} // namespace guanaqo
