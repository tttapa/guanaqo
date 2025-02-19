#pragma once

#include <any>
#include <memory>
#include <type_traits>
#include <typeinfo>

namespace guanaqo {

/// Like std::any, but storing just the pointer, without any dynamic allocation.
class any_ptr {
  public:
    any_ptr() = default;
    template <class T>
    any_ptr(T *ptr) noexcept
        : ptr{const_cast<void *>(reinterpret_cast<const void *>(ptr))},
          ptr_type{&typeid(T)}, is_const{std::is_const_v<T>} {}

    template <class T>
    T *cast() const {
        if (!ptr)
            return nullptr;
        if (typeid(T) != type())
            throw std::bad_any_cast();
        if (std::is_const_v<T> != is_const)
            throw std::bad_any_cast();
        return reinterpret_cast<T *>(ptr);
    }

    [[nodiscard]] bool has_value() const noexcept { return ptr != nullptr; }
    [[nodiscard]] bool is_immutable() const noexcept { return is_const; }
    [[nodiscard]] const std::type_info &type() const noexcept {
        return *ptr_type;
    }

  private:
    void *ptr                      = nullptr;
    const std::type_info *ptr_type = &typeid(void);
    bool is_const                  = true;
};

/// Like std::any, but storing a shared_ptr, to allow for move-only types.
class shared_any_ptr {
  public:
    shared_any_ptr() = default;
    template <class T>
    shared_any_ptr(std::shared_ptr<T> ptr) noexcept
        : ptr{std::move(ptr),
              const_cast<void *>(reinterpret_cast<const void *>(ptr.get()))},
          ptr_type{&typeid(T)}, is_const{std::is_const_v<T>} {}
    shared_any_ptr(const shared_any_ptr &)                = default;
    shared_any_ptr(shared_any_ptr &&) noexcept            = default;
    shared_any_ptr &operator=(const shared_any_ptr &)     = default;
    shared_any_ptr &operator=(shared_any_ptr &&) noexcept = default;

    template <class T>
    T *cast() const {
        if (!ptr)
            return nullptr;
        if (typeid(T) != type())
            throw std::bad_any_cast();
        if (std::is_const_v<T> != is_const)
            throw std::bad_any_cast();
        return reinterpret_cast<T *>(ptr.get());
    }

    [[nodiscard]] bool has_value() const noexcept { return bool{ptr}; }
    [[nodiscard]] bool is_immutable() const noexcept { return is_const; }
    [[nodiscard]] const std::type_info &type() const noexcept {
        return *ptr_type;
    }

  private:
    std::shared_ptr<void> ptr;
    const std::type_info *ptr_type = &typeid(void);
    bool is_const                  = true;
};

} // namespace guanaqo
