#pragma once

/// @file
/// @ingroup trace_core
/// Tracing logger and macros (ITT, Perfetto, or fallback).

#include <guanaqo/export.h>
#include <guanaqo/perfetto/trace.hpp>
#include <guanaqo/preprocessor.h>
#include <guanaqo/stringify.h>

#include <concepts>
#include <functional>
#include <memory>
#include <span>
#include <utility>

#if GUANAQO_WITH_ITT
#include <ittnotify.h>
#include <algorithm>
#include <iosfwd>
#else
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <ostream>
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

    void trace_instant(__itt_string_handle *name, int64_t instance) const {
        if (!domain)
            return;
        instance = std::clamp<int64_t>(instance, 0, max_instance_num);
        auto id =
            __itt_id_make(nullptr, static_cast<unsigned long long>(instance));
        __itt_marker(domain, id, name, __itt_scope_task);
    }

    [[nodiscard]] std::span<const Log> get_logs() const { return {}; }

    void reset() {}
};

GUANAQO_EXPORT TraceLogger &get_trace_logger();

#if !GUANAQO_WITH_PERFETTO
#define GUANAQO_TRACE_IMPL(var_name, name, instance)                           \
    static auto GUANAQO_CAT(var_name, _name) =                                 \
        __itt_string_handle_create(name);                                      \
    const auto var_name = ::guanaqo::get_trace_logger().trace(                 \
        GUANAQO_CAT(var_name, _name), instance)
#define GUANAQO_TRACE_INSTANT_IMPL(var_name, name, instance)                   \
    do {                                                                       \
        static auto GUANAQO_CAT(var_name, _name) =                             \
            __itt_string_handle_create(name);                                  \
        ::guanaqo::get_trace_logger().trace_instant(                           \
            GUANAQO_CAT(var_name, _name), instance);                           \
    } while (0)
#define GUANAQO_TRACE(name, instance, ...)                                     \
    GUANAQO_TRACE_IMPL(GUANAQO_CAT(trace_log_, __COUNTER__), name, instance)
#define GUANAQO_TRACE_INSTANT(name, instance)                                  \
    GUANAQO_TRACE_INSTANT_IMPL(GUANAQO_CAT(trace_instant_, __COUNTER__), name, \
                               instance)
#define GUANAQO_TRACE_LINALG(name, gflops)                                     \
    GUANAQO_TRACE(name, 0) /* TODO: record gflops? */
#define GUANAQO_TRACE_REGION(name, instance) GUANAQO_TRACE(name, instance)
#define GUANAQO_TRACE_STATIC_STR(s) s
#endif

#else

GUANAQO_EXPORT std::size_t get_thread_id();

/// Class for recording trace logs, used when ITT or Perfetto tracing is not enabled.
/// @ingroup trace_core
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

    static clock::time_point t0;
    std::vector<Log> logs;

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

    TraceLogger(size_t capacity) { logs.reserve(capacity); }

    [[nodiscard]] ScopedLog trace(const char *name, int64_t instance,
                                  int64_t flop_count = -1) {
        if (logs.size() == logs.capacity())
            return ScopedLog{nullptr, {}};
        auto &log      = logs.emplace_back();
        auto t1        = clock::now();
        log.name       = name;
        log.instance   = instance;
        log.flop_count = flop_count;
        log.start_time = t1 - t0;
        log.thread_id  = get_thread_id();
        return ScopedLog{&log, t1}; // Note: assumes stable reference to log
    }

    void trace_instant(const char *name, int64_t instance,
                       int64_t flop_count = -1) {
        auto instant_log = trace(name, instance, flop_count);
        instant_log.log  = nullptr;
    }

    [[nodiscard]] std::span<const Log> get_logs() const {
        return std::span{logs};
    }

    /// Set the maximum number of logs that can be recorded. Additional logs are discarded.
    void reserve(size_t capacity) { logs.reserve(capacity); }
    /// Clear all recorded logs, but keep the reserved capacity.
    void reset() { logs.clear(); } // does not change capacity
    /// Clear all recorded logs and set capacity to 0 (essentially disabling the logger).
    void clear() { logs = {}; } // set capacity to 0
};

#if GUANAQO_WITH_TRACING
/// Get a reference to the global (but thread-local) trace logger instance.
/// @ingroup trace_core
GUANAQO_EXPORT TraceLogger &get_trace_logger();
/// Set the default capacity for trace loggers created by get_trace_logger().
GUANAQO_EXPORT size_t trace_logger_set_default_size(size_t size);
/// Call @p callback for each trace logger instance.
/// Note that the TraceLogger API is not thread-safe, so this function should not be called while
/// other threads are still logging to their trace loggers. Use a barrier or a critical section to
/// ensure this is the case.
GUANAQO_EXPORT void
for_each_trace_logger(const std::function<void(TraceLogger &)> &callback);
/// Like @ref for_each_trace_logger, but also forgets about all loggers. Threads that are still
/// running will still be able to log to their loggers, but they are no longer accessible through
/// @ref foreach_trace_logger. Loggers for threads that no longer exist will be cleaned up,
/// unless the callback takes ownership of them.
GUANAQO_EXPORT void drop_trace_loggers(
    const std::function<void(std::shared_ptr<TraceLogger>)> &callback =
        nullptr);
#endif

#endif

#if !GUANAQO_WITH_PERFETTO && !GUANAQO_WITH_ITT
#if GUANAQO_WITH_TRACING
#define GUANAQO_TRACE(name, ...)                                               \
    const auto GUANAQO_CAT(trace_log_, __COUNTER__) =                          \
        ::guanaqo::get_trace_logger().trace(name, __VA_ARGS__)
#define GUANAQO_TRACE_INSTANT(name, instance)                                  \
    do {                                                                       \
        ::guanaqo::get_trace_logger().trace_instant(name, instance);           \
    } while (0)
#define GUANAQO_TRACE_LINALG(name, gflops) GUANAQO_TRACE(name, 0, gflops)
#define GUANAQO_TRACE_REGION(name, instance) GUANAQO_TRACE(name, instance)
#define GUANAQO_TRACE_STATIC_STR(s) s
#else
#define GUANAQO_TRACE(name, ...) GUANAQO_NOOP()
#define GUANAQO_TRACE_INSTANT(name, instance) GUANAQO_NOOP()
#define GUANAQO_TRACE_LINALG(name, gflops) GUANAQO_NOOP()
#define GUANAQO_TRACE_REGION(name, instance) GUANAQO_NOOP()
#define GUANAQO_TRACE_STATIC_STR(s) s
#endif
#endif

#if GUANAQO_WITH_ITT
#define GUANAQO_IF_ITT(...) __VA_ARGS__
#else
#define GUANAQO_IF_ITT(...)
#endif

} // namespace guanaqo
