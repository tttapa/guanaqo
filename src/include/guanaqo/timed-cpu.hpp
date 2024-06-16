#pragma once

#include <guanaqo/export.h>
#include <guanaqo/timed.hpp>
#include <iosfwd>

namespace guanaqo {

/// Measures the number of invocations of a specific piece of code and its
/// run time.
struct GUANAQO_EXPORT TimingsCPU {
    int64_t num_invocations{};
    std::chrono::nanoseconds wall_time{};
    std::chrono::nanoseconds cpu_time{};
};

GUANAQO_EXPORT std::ostream &operator<<(std::ostream &, TimingsCPU);

/// RAII class for measuring wall and CPU time.
template <>
struct GUANAQO_EXPORT Timed<TimingsCPU> {
    Timed(TimingsCPU &time);
    ~Timed();
    Timed(const Timed &)            = delete;
    Timed &operator=(const Timed &) = delete;

  private:
    using clock = std::chrono::steady_clock;
    TimingsCPU &time;
    clock::time_point wall_start_time;
    int64_t cpu_start_time;
};

} // namespace guanaqo
