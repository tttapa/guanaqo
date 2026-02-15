#pragma once

/// @file
/// @ingroup trace_perfetto
/// Perfetto tracing macros and session helpers.

#include <guanaqo/export.h>
#include <guanaqo/preprocessor.h>

#if GUANAQO_WITH_PERFETTO

#include <guanaqo/pcm/counters.hpp>
#include <perfetto.h>

#include <filesystem>

PERFETTO_DEFINE_CATEGORIES_IN_NAMESPACE(
    guanaqo::trace,
    ::perfetto::Category("trace").SetDescription(
        "General trace events for regions of code"),
    ::perfetto::Category("gflops").SetDescription(
        "Counts the total number of floating point operations"),
    ::perfetto::Category("pcm").SetDescription(
        "Performance counters for CPU metrics"));

namespace guanaqo {

PERFETTO_USE_CATEGORIES_FROM_NAMESPACE(guanaqo::trace);

namespace trace {

namespace fs = std::filesystem;

/// @addtogroup trace_perfetto
/// @{

/// Initialize Perfetto for in-process tracing and register guanaqo's track event categories.
GUANAQO_EXPORT void initialize_tracing();
/// Start a new tracing session with the specified buffer size in KiB.
GUANAQO_EXPORT std::unique_ptr<::perfetto::TracingSession>
start_tracing(uint32_t memory_kb = 128 * 1024);
/// Stop the tracing session and discard the trace data.
GUANAQO_EXPORT void
abort_tracing(std::unique_ptr<::perfetto::TracingSession> tracing_session);
/// Stop the tracing session and write the trace data to the specified output file.
GUANAQO_EXPORT void
stop_tracing(std::unique_ptr<::perfetto::TracingSession> tracing_session,
             const fs::path &output_path);

/// @}

namespace detail {
inline ::perfetto::base::PlatformThreadId get_thread_id_fast() {
    static thread_local auto thread_id = ::perfetto::base::GetThreadId();
    return thread_id;
}
inline auto get_thread_track() {
    return ::perfetto::ThreadTrack::ForThread(get_thread_id_fast());
}
} // namespace detail

/// Get a reference to the thread-local GFLOP counter.
/// @ingroup trace_perfetto
GUANAQO_EXPORT uint64_t &get_thread_gflop_count();

#if GUANAQO_WITH_PCM_TRACING
/// @ingroup trace_perfetto
GUANAQO_EXPORT struct ScopedLinalgCounters {
    ::perfetto::ThreadTrack parent_track = detail::get_thread_track();
    std::unique_ptr<pcm::detail::ScopedCounters> impl = pcm::start_counters();

    ScopedLinalgCounters() { trace_gflops(impl ? &impl->get() : nullptr); }
    ScopedLinalgCounters(const ScopedLinalgCounters &)            = delete;
    ScopedLinalgCounters &operator=(const ScopedLinalgCounters &) = delete;
    ~ScopedLinalgCounters() { trace_gflops(impl ? &impl->stop() : nullptr); }

    void trace_gflops(pcm::ThreadPerfCounters *ctr) const;
};
#else
/// @ingroup trace_perfetto
GUANAQO_EXPORT struct ScopedLinalgCounters {
    ::perfetto::ThreadTrack parent_track = detail::get_thread_track();

    ScopedLinalgCounters() { trace_gflops(); }
    ScopedLinalgCounters(const ScopedLinalgCounters &)            = delete;
    ScopedLinalgCounters &operator=(const ScopedLinalgCounters &) = delete;
    ~ScopedLinalgCounters() { trace_gflops(); }

    void trace_gflops() const {
        TRACE_COUNTER("gflops",
                      ::perfetto::CounterTrack("gflops", parent_track),
                      get_thread_gflop_count());
    }
};
#endif

} // namespace trace
} // namespace guanaqo

#define GUANAQO_TRACE_LINALG_PRIVATE(name, gflops, uid)                        \
    ::guanaqo::trace::ScopedLinalgCounters GUANAQO_CONCATENATE_TOKENS(ctr_,    \
                                                                      uid){};  \
    ::guanaqo::trace::get_thread_gflop_count() += (gflops);                    \
    TRACE_EVENT("gflops", name,                                                \
                GUANAQO_CONCATENATE_TOKENS(ctr_, uid).parent_track)

#define GUANAQO_TRACE_LINALG_IMPL(name, gflops)                                \
    GUANAQO_TRACE_LINALG_PRIVATE(name, gflops, __COUNTER__)

#define GUANAQO_TRACE_INSTANT_IMPL(name, instance)                             \
    TRACE_EVENT_INSTANT("trace", name, "instance", instance)

#define GUANAQO_TRACE_REGION_IMPL(name, instance)                              \
    TRACE_EVENT("trace", name, "instance", instance)

/// @addtogroup trace_perfetto
/// @{

#define GUANAQO_TRACE_LINALG(name, gflops)                                     \
    PERFETTO_USE_CATEGORIES_FROM_NAMESPACE_SCOPED(guanaqo::trace);             \
    GUANAQO_TRACE_LINALG_IMPL(name, gflops)

#define GUANAQO_TRACE_INSTANT(name, instance)                                  \
    PERFETTO_USE_CATEGORIES_FROM_NAMESPACE_SCOPED(guanaqo::trace);             \
    GUANAQO_TRACE_INSTANT_IMPL(name, instance)

#define GUANAQO_TRACE_REGION(name, instance)                                   \
    PERFETTO_USE_CATEGORIES_FROM_NAMESPACE_SCOPED(guanaqo::trace);             \
    GUANAQO_TRACE_REGION_IMPL(name, instance)

// TODO: deprecate
#define GUANAQO_TRACE(name, instance, ...)                                     \
    PERFETTO_USE_CATEGORIES_FROM_NAMESPACE_SCOPED(guanaqo::trace);             \
    GUANAQO_TRACE_REGION(name, instance)

#define GUANAQO_TRACE_STATIC_STR(s)                                            \
    ::perfetto::StaticString { s }

/// @}

#endif
