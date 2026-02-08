#pragma once

#include <guanaqo/export.h>
#include <guanaqo/perfetto/trace.hpp>
#include <guanaqo/preprocessor.h>
#include <guanaqo/stringify.h>

#include <algorithm>
#include <span>
#include <utility>

#if GUANAQO_WITH_ITT
#include <ittnotify.h>
#include <iosfwd>
#else
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <ostream>
#include <thread>
#endif

namespace guanaqo {

#if GUANAQO_WITH_ITT
struct TraceLogger {
    struct Log {
        friend std::ostream &operator<<(std::ostream &os, Log) { return os; }
    };

    __itt_domain *domain                      = __itt_domain_create("guanaqo");
    static constexpr int64_t max_instance_num = 255;
    TraceLogger(size_t = 0) {
        for (int64_t i = 0; i <= max_instance_num; ++i)
            __itt_id_create(
                domain,
                __itt_id_make(nullptr, static_cast<unsigned long long>(i)));
    }

    TraceLogger(const TraceLogger &)            = delete;
    TraceLogger &operator=(const TraceLogger &) = delete;
    TraceLogger(TraceLogger &&other) noexcept
        : domain{std::exchange(other.domain, nullptr)} {}
    TraceLogger &operator=(TraceLogger &&) = delete;
    ~TraceLogger() {
        if (!domain)
            return;
        for (int64_t i = 0; i <= max_instance_num; ++i)
            __itt_id_destroy(
                domain,
                __itt_id_make(nullptr, static_cast<unsigned long long>(i)));
    }

    struct ScopedLog {
        __itt_domain *domain;
        ScopedLog(__itt_domain *domain, __itt_string_handle *name,
                  int64_t instance)
            : domain{domain} {
            if (!domain)
                return;
            instance = std::clamp<int64_t>(instance, 0, max_instance_num);
            auto id  = __itt_id_make(nullptr,
                                     static_cast<unsigned long long>(instance));
            __itt_task_begin(domain, id, __itt_null, name);
        }
        ScopedLog(const ScopedLog &)            = delete;
        ScopedLog &operator=(const ScopedLog &) = delete;
        ScopedLog(ScopedLog &&other) noexcept
            : domain{std::exchange(other.domain, nullptr)} {}
        ScopedLog &operator=(ScopedLog &&) = delete;
        ~ScopedLog() {
            if (!domain)
                return;
            __itt_task_end(domain);
        }
    };

    [[nodiscard]] ScopedLog trace(__itt_string_handle *name,
                                  int64_t instance) const {
        return ScopedLog{domain, name, instance};
    }

    [[nodiscard]] std::span<const Log> get_logs() const { return {}; }

    void reset() {}
};

GUANAQO_EXPORT TraceLogger &get_trace_logger();
#define GUANAQO_TRACE_IMPL(var_name, name, instance)                           \
    static auto GUANAQO_CAT(var_name, _name) =                                 \
        __itt_string_handle_create(name);                                      \
    const auto var_name = ::guanaqo::get_trace_logger().trace(                 \
        GUANAQO_CAT(var_name, _name), instance)
#define GUANAQO_TRACE(name, instance)                                          \
    GUANAQO_TRACE_IMPL(GUANAQO_CAT(trace_log_, __COUNTER__), name, instance)

#else

struct TraceLogger {
    struct Log {
        const char *name = "";
        int64_t instance{0};
        std::chrono::nanoseconds start_time{0};
        std::chrono::nanoseconds duration{0};
        std::size_t thread_id{0};
        int64_t flop_count{0};

        friend std::ostream &operator<<(std::ostream &os, const Log &log) {
            return os << std::quoted(log.name) << ',' << log.instance << ','
                      << log.start_time.count() << ',' << log.duration.count()
                      << ',' << log.thread_id << ',' << log.flop_count;
        }
    };

    static std::ostream &write_column_headings(std::ostream &os) {
        return os << "name" << ',' << "instance" << ',' << "start_time" << ','
                  << "duration" << ',' << "thread_id" << ',' << "flop_count";
    }

    using clock = std::chrono::steady_clock;

    clock::time_point t0 = clock::now();
    std::vector<Log> logs;
    std::atomic_size_t count{0};

    struct ScopedLog {
        Log *log = nullptr;
        clock::time_point start_time_point;

        ScopedLog() = default;
        ScopedLog(Log *log, clock::time_point start_time_point)
            : log{log}, start_time_point{start_time_point} {}
        ScopedLog(const ScopedLog &)            = delete;
        ScopedLog &operator=(const ScopedLog &) = delete;
        ScopedLog(ScopedLog &&other) noexcept
            : log{std::exchange(other.log, nullptr)},
              start_time_point{other.start_time_point} {}
        ScopedLog &operator=(ScopedLog &&) = delete;
        ~ScopedLog() {
            if (log)
                log->duration = clock::now() - start_time_point;
        }
    };

    TraceLogger(size_t capacity) { logs.resize(capacity); }

    [[nodiscard]] ScopedLog trace(const char *name, int64_t instance,
                                  int64_t flop_count = -1) {
        size_t index = count.fetch_add(1, std::memory_order_relaxed);
        if (index >= logs.size())
            return ScopedLog{nullptr, {}};
        static constexpr std::hash<std::thread::id> hasher;
        auto &log      = logs[index];
        auto t1        = clock::now();
        log.name       = name;
        log.instance   = instance;
        log.flop_count = flop_count;
        log.start_time = t1 - t0;
        log.thread_id  = hasher(std::this_thread::get_id());
        return ScopedLog{&log, t1};
    }

    [[nodiscard]] std::span<const Log> get_logs() const {
        auto n = std::min(logs.size(), count.load(std::memory_order_relaxed));
        return std::span{logs}.first(n);
    }

    void reset() { count.store(0, std::memory_order_relaxed); }
};

#if GUANAQO_WITH_TRACING
GUANAQO_EXPORT TraceLogger &get_trace_logger();
#endif

#endif

#if GUANAQO_WITH_TRACING && !GUANAQO_WITH_PERFETTO
#define GUANAQO_TRACE(name, ...)                                               \
    const auto GUANAQO_CAT(trace_log_, __COUNTER__) =                          \
        ::guanaqo::get_trace_logger().trace(name, __VA_ARGS__)
#define GUANAQO_TRACE_INSTANT(category, name, instance)                        \
    do {                                                                       \
        ::guanaqo::get_trace_logger().trace(name, __VA_ARGS__)->log = nullptr; \
    } while (0)
#define GUANAQO_TRACE_LINALG(name, gflops) GUANAQO_TRACE(name, 0, gflops)
#define GUANAQO_TRACE_REGION(name, instance) GUANAQO_TRACE(name, instance)
#define GUANAQO_TRACE_STATIC_STR(s) s
#endif

#if !GUANAQO_WITH_TRACING && !GUANAQO_WITH_ITT && !GUANAQO_WITH_PERFETTO
#define GUANAQO_TRACE(...) GUANAQO_NOOP()
#define GUANAQO_TRACE_INSTANT(...) GUANAQO_NOOP()
#define GUANAQO_TRACE_LINALG(...) GUANAQO_NOOP()
#define GUANAQO_TRACE_REGION(...) GUANAQO_NOOP()
#define GUANAQO_TRACE_STATIC_STR(s) s
#endif

#if GUANAQO_WITH_ITT
#define GUANAQO_IF_ITT(...) __VA_ARGS__
#else
#define GUANAQO_IF_ITT(...)
#endif

} // namespace guanaqo
