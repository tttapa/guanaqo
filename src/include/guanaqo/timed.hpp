#pragma once

/// @file
/// @ingroup timing
/// RAII timing helper.

#include <chrono>
#include <functional>

namespace guanaqo {

/// @addtogroup timing
/// @{

template <class T>
struct Timed;

template <class Rep, class Period>
struct Timed<std::chrono::duration<Rep, Period>> {
    Timed(std::chrono::duration<Rep, Period> &time) : time(time) {
        time -= std::chrono::steady_clock::now().time_since_epoch();
    }
    ~Timed() { time += std::chrono::steady_clock::now().time_since_epoch(); }
    Timed(const Timed &)            = delete;
    Timed(Timed &&)                 = delete;
    Timed &operator=(const Timed &) = delete;
    Timed &operator=(Timed &&)      = delete;
    std::chrono::duration<Rep, Period> &time;
};

#ifndef DOXYGEN
template <class T>
Timed(T &) -> Timed<T>;
#endif

template <class T, class F, class... Args>
decltype(auto) timed(T &time, F &&func, Args &&...args) {
    Timed timer{time};
    return std::invoke(std::forward<F>(func), std::forward<Args>(args)...);
}

/// @}

} // namespace guanaqo
