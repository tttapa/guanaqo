#include <guanaqo/timed-cpu.hpp>
#include <iomanip>

#ifdef _WIN32
#include <Windows.h>
#else
#include <ctime>
#endif

namespace {

int64_t getProcessCpuTime() {
#ifdef _WIN32
    FILETIME creation_time, exit_time, kernel_time, user_time;
    GetProcessTimes(GetCurrentProcess(), &creation_time, &exit_time,
                    &kernel_time, &user_time);
    return static_cast<int64_t>(user_time.dwHighDateTime) << 32 |
           user_time.dwLowDateTime;
#else
    struct timespec cpu_time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu_time);
    return static_cast<int64_t>(cpu_time.tv_sec) * 1'000'000'000 +
           cpu_time.tv_nsec;
#endif
}

} // namespace

namespace guanaqo {

std::ostream &operator<<(std::ostream &os, TimingsCPU t) {
    using millis_f64 = std::chrono::duration<double, std::milli>;
    auto wall_ms     = millis_f64(t.wall_time).count();
    auto cpu_ms      = millis_f64(t.cpu_time).count();
    auto cpu_pct     = 100 * cpu_ms / wall_ms;
    auto prec        = os.precision(6);
    os << std::setw(8) << wall_ms << " ms (wall) ─ " //
       << std::setw(8) << cpu_ms << " ms (CPU) ─ "   //
       << std::setprecision(5) << std::setw(7) << cpu_pct << "%";
    os.precision(prec);
    return os;
}

Timed<TimingsCPU>::Timed(TimingsCPU &time) : time(time) {
    wall_start_time = clock::now();
    cpu_start_time  = getProcessCpuTime();
}

Timed<TimingsCPU>::~Timed() {
    auto wall_end_time = clock::now();
    auto cpu_end_time  = getProcessCpuTime();
    ++time.num_invocations;
    time.wall_time += wall_end_time - wall_start_time;
    time.cpu_time += std::chrono::nanoseconds{cpu_end_time - cpu_start_time};
}

} // namespace guanaqo
