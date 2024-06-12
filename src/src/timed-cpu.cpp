#include <guanaqo/timed-cpu.hpp>

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
