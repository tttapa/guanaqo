#include <guanaqo/demangled-typename.hpp>

#include <cstdlib>
#include <memory>
#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace guanaqo {

std::string demangled_typename(const std::type_info &t) {
#ifdef __GNUC__
    return std::unique_ptr<char, decltype(&std::free)>{
        abi::__cxa_demangle(t.name(), nullptr, nullptr, nullptr), std::free}
        .get();
#else
    return t.name();
#endif
}

} // namespace guanaqo
