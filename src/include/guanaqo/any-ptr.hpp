#pragma once

#include <any>
#include <type_traits>
#include <typeinfo>

namespace guanaqo {

/// Like std::any, but storing just the pointer, without any dynamic allocation.
class any_ptr {
  public:
    any_ptr() = default;
    template <class T>
    any_ptr(T *ptr)
        : ptr{const_cast<void *>(reinterpret_cast<const void *>(ptr))},
          ptr_type{&typeid(T)}, is_const{std::is_const_v<T>} {}

    template <class T>
    T *cast() const {
        if (!ptr_type)
            return nullptr;
        if (typeid(T) != *ptr_type)
            throw std::bad_any_cast();
        if (std::is_const_v<T> != is_const)
            throw std::bad_any_cast();
        return reinterpret_cast<T *>(ptr);
    }

  private:
    void *ptr                      = nullptr;
    const std::type_info *ptr_type = nullptr;
    bool is_const                  = true;
};

} // namespace guanaqo
