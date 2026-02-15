#pragma once

/// @file
/// @ingroup type_erasure
/// Macros to populate type-erased vtables from member functions in @ref guanaqo::TypeErased

#include <guanaqo/type-erasure.hpp>
#include <functional> // std::invoke

/// @ingroup type_erasure
#define GUANAQO_TE_REQUIRED_METHOD(vtable, type, member)                       \
    do {                                                                       \
        static_assert(                                                         \
            requires { &type::member; },                                       \
            "Missing required method '" #type "::" #member "'");               \
        (vtable).member =                                                      \
            ::guanaqo::type_erased_wrapped<type, &type::member>();             \
    } while (0)
/// @ingroup type_erasure
#define GUANAQO_TE_OPTIONAL_METHOD(vtable, type, member, instance)             \
    do {                                                                       \
        if constexpr (requires { &type::member; }) {                           \
            using vtable_t     = ::std::remove_cvref_t<decltype(vtable)>;      \
            auto assign_vtable = [&] {                                         \
                (vtable).member =                                              \
                    ::guanaqo::type_erased_wrapped<type, &type::member,        \
                                                   const vtable_t &>();        \
            };                                                                 \
            if constexpr (requires { &type::provides_##member; }) {            \
                if (::std::invoke(&type::provides_##member, instance))         \
                    assign_vtable();                                           \
            } else {                                                           \
                assign_vtable();                                               \
            }                                                                  \
        }                                                                      \
    } while (0)
