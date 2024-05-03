#include <guanaqo/dl.hpp>

#if _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <cassert>

namespace guanaqo {

#if _WIN32
std::shared_ptr<char> get_last_error_msg() {
    char *err = nullptr;
    auto opt  = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER |
               FORMAT_MESSAGE_IGNORE_INSERTS;
    auto lang = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
    if (FormatMessage(opt, NULL, GetLastError(), lang,
                      reinterpret_cast<char *>(&err), 0, NULL) != 0) {
        return std::shared_ptr<char>{err, [](char *e) { LocalFree(e); }};
    } else {
        static char msg[] = "(failed to get error message)";
        return std::shared_ptr<char>{msg, [](char *) {}};
    }
}

std::shared_ptr<void> load_lib(const std::filesystem::path &so_filename,
                               [[maybe_unused]] DynamicLoadFlags flags) {
    assert(!so_filename.empty());
    void *h = LoadLibraryW(so_filename.c_str());
    if (!h)
        throw dynamic_load_error("Unable to load \"" + so_filename.string() +
                                 "\": " + get_last_error_msg().get());
    return std::shared_ptr<void>{
        h, +[](void *h) { FreeLibrary(static_cast<HMODULE>(h)); }};
}

void *load_func(void *handle, const std::string &name) {
    assert(handle);
    auto *h = GetProcAddress(static_cast<HMODULE>(handle), name.c_str());
    if (!h)
        throw dynamic_load_error("Unable to load function '" + name +
                                 "': " + get_last_error_msg().get());
    return reinterpret_cast<void *>(h);
}
#else
std::shared_ptr<void> load_lib(const std::filesystem::path &so_filename,
                               DynamicLoadFlags dl_flags) {
    assert(!so_filename.empty());
    ::dlerror();
    void *h = ::dlopen(so_filename.c_str(), dl_flags);
    if (auto *err = ::dlerror())
        throw dynamic_load_error(err);
    return std::shared_ptr<void>{h, &::dlclose};
}

void *load_func(void *handle, const std::string &name) {
    assert(handle);
    ::dlerror();
    auto *h = ::dlsym(handle, name.c_str());
    if (auto *err = ::dlerror())
        throw dynamic_load_error("Unable to load function '" + name +
                                 "': " + err);
    return h;
}
#endif

} // namespace guanaqo
