#pragma once

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

inline ::perfetto::base::PlatformThreadId GetThreadIdFast() {
    static thread_local auto thread_id = ::perfetto::base::GetThreadId();
    return thread_id;
}

inline auto getThreadTrack() {
    return ::perfetto::ThreadTrack::ForThread(GetThreadIdFast());
}

GUANAQO_EXPORT uint64_t &get_thread_gflop_count();

#if GUANAQO_WITH_PCM
template <class F>
GUANAQO_EXPORT struct ScopedLinalgCounters {
    F func;
    std::unique_ptr<pcm::detail::ScopedCounters> impl;

    ScopedLinalgCounters(F &&func)
        : func(std::forward<F>(func)), impl(pcm::start_counters()) {}
    ScopedLinalgCounters(const ScopedLinalgCounters &)            = delete;
    ScopedLinalgCounters &operator=(const ScopedLinalgCounters &) = delete;
    pcm::ThreadPerfCounters *get() { return impl ? &impl->get() : nullptr; }
    ~ScopedLinalgCounters() { func(impl ? &impl->stop() : nullptr); }
};
#else
template <class F>
GUANAQO_EXPORT struct ScopedLinalgCounters {
    F func;

    ScopedLinalgCounters(F &&func) : func(std::forward<F>(func)) {}
    ScopedLinalgCounters(const ScopedLinalgCounters &)            = delete;
    ScopedLinalgCounters &operator=(const ScopedLinalgCounters &) = delete;
    ~ScopedLinalgCounters() { func(nullptr); }
};
#endif

template <class F>
ScopedLinalgCounters(F &&) -> ScopedLinalgCounters<F>;
template <class F>
ScopedLinalgCounters(F &) -> ScopedLinalgCounters<F &>;

} // namespace trace
} // namespace guanaqo

#if GUANAQO_WITH_PCM

#define GUANAQO_COUNT_PCM_PRIVATE(ctr, name, uid)                              \
    TRACE_COUNTER("pcm", ::perfetto::CounterTrack(#name, track_##uid),         \
                  time_##uid, (ctr)->name)

#define GUANAQO_TRACE_LINALG_PRIVATE(name, gflops, uid)                        \
    auto &GUANAQO_CONCATENATE_TOKENS(gflops_, uid) =                           \
        ::guanaqo::trace::get_thread_gflop_count();                            \
    auto GUANAQO_CONCATENATE_TOKENS(track_, uid) =                             \
        ::guanaqo::trace::getThreadTrack();                                    \
    auto GUANAQO_CONCATENATE_TOKENS(                                           \
        log_ctr_, uid) = [&](const ::guanaqo::pcm::ThreadPerfCounters *ctr) {  \
        TRACE_COUNTER("gflops",                                                \
                      ::perfetto::CounterTrack(                                \
                          "gflops", GUANAQO_CONCATENATE_TOKENS(track_, uid)),  \
                      GUANAQO_CONCATENATE_TOKENS(gflops_, uid));               \
        if (ctr) {                                                             \
            auto GUANAQO_CONCATENATE_TOKENS(time_, uid) =                      \
                PERFETTO_TRACK_EVENT_NAMESPACE::TrackEvent::GetTraceTimeNs();  \
            GUANAQO_COUNT_PCM_PRIVATE(ctr, instructions, uid);                 \
            GUANAQO_COUNT_PCM_PRIVATE(ctr, cycles, uid);                       \
            GUANAQO_COUNT_PCM_PRIVATE(ctr, ref_cycles, uid);                   \
            GUANAQO_COUNT_PCM_PRIVATE(ctr, l2_misses, uid);                    \
            GUANAQO_COUNT_PCM_PRIVATE(ctr, l2_hits, uid);                      \
            GUANAQO_COUNT_PCM_PRIVATE(ctr, l3_misses, uid);                    \
            GUANAQO_COUNT_PCM_PRIVATE(ctr, branch_misses, uid);                \
            GUANAQO_COUNT_PCM_PRIVATE(ctr, all_slots, uid);                    \
            GUANAQO_COUNT_PCM_PRIVATE(ctr, frontend_bound_slots, uid);         \
            GUANAQO_COUNT_PCM_PRIVATE(ctr, backend_bound_slots, uid);          \
            GUANAQO_COUNT_PCM_PRIVATE(ctr, bad_speculation_slots, uid);        \
            GUANAQO_COUNT_PCM_PRIVATE(ctr, retiring_slots, uid);               \
            GUANAQO_COUNT_PCM_PRIVATE(ctr, mem_bound_slots, uid);              \
            GUANAQO_COUNT_PCM_PRIVATE(ctr, fetch_lat_slots, uid);              \
        }                                                                      \
    };                                                                         \
    auto GUANAQO_CONCATENATE_TOKENS(ctr_, uid) =                               \
        ::guanaqo::trace::ScopedLinalgCounters{                                \
            GUANAQO_CONCATENATE_TOKENS(log_ctr_, uid)};                        \
    GUANAQO_CONCATENATE_TOKENS(log_ctr_, uid)(                                 \
        GUANAQO_CONCATENATE_TOKENS(ctr_, uid).get());                          \
    TRACE_EVENT("gflops", name, GUANAQO_CONCATENATE_TOKENS(track_, uid));      \
    GUANAQO_CONCATENATE_TOKENS(gflops_, uid) += (gflops)

#else // GUANAQO_WITH_PCM

#define GUANAQO_TRACE_LINALG_PRIVATE(name, gflops, uid)                        \
    auto &GUANAQO_CONCATENATE_TOKENS(gflops_, uid) =                           \
        ::guanaqo::trace::get_thread_gflop_count();                            \
    auto GUANAQO_CONCATENATE_TOKENS(track_, uid) =                             \
        ::guanaqo::trace::getThreadTrack();                                    \
    auto GUANAQO_CONCATENATE_TOKENS(                                           \
        log_ctr_, uid) = [&](const ::guanaqo::pcm::ThreadPerfCounters *ctr) {  \
        TRACE_COUNTER("gflops",                                                \
                      ::perfetto::CounterTrack(                                \
                          "gflops", GUANAQO_CONCATENATE_TOKENS(track_, uid)),  \
                      GUANAQO_CONCATENATE_TOKENS(gflops_, uid));               \
    };                                                                         \
    auto GUANAQO_CONCATENATE_TOKENS(ctr_, uid) =                               \
        ::guanaqo::trace::ScopedLinalgCounters{                                \
            GUANAQO_CONCATENATE_TOKENS(log_ctr_, uid)};                        \
    GUANAQO_CONCATENATE_TOKENS(log_ctr_, uid)(                                 \
        GUANAQO_CONCATENATE_TOKENS(ctr_, uid).get());                          \
    TRACE_EVENT("gflops", name, GUANAQO_CONCATENATE_TOKENS(track_, uid));      \
    GUANAQO_CONCATENATE_TOKENS(gflops_, uid) += (gflops)

#endif // GUANAQO_WITH_PCM

#define GUANAQO_TRACE_LINALG_IMPL(name, gflops)                                \
    GUANAQO_TRACE_LINALG_PRIVATE(name, gflops, __COUNTER__)

#define GUANAQO_TRACE_INSTANT_IMPL(name, instance)                             \
    TRACE_EVENT_INSTANT("trace", name, "instance", instance)

#define GUANAQO_TRACE_REGION_IMPL(name, instance)                              \
    TRACE_EVENT("trace", name, "instance", instance)

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

#endif
