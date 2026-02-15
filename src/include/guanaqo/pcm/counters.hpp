#pragma once

/// @file
/// @ingroup trace_pcm
/// Performance counter snapshots and scoped collectors.

#include <guanaqo/export.h>
#include <cstdint>
#include <memory>
#include <utility>

namespace guanaqo::pcm {

// Accumulated performance counters
/// @ingroup trace_pcm
struct ThreadPerfCounters {
    // Basic counters
    uint64_t instructions = 0;
    uint64_t cycles       = 0;
    uint64_t ref_cycles   = 0;

    // Cache counters
    uint64_t l2_misses = 0;
    uint64_t l2_hits   = 0;
    uint64_t l3_misses = 0;

    // Branch prediction
    uint64_t branch_misses = 0;

    // Total slots
    uint64_t all_slots = 0;

    // Top-Down Microarchitecture Analysis (TMA) Level 1
    uint64_t frontend_bound_slots  = 0;
    uint64_t backend_bound_slots   = 0;
    uint64_t bad_speculation_slots = 0;
    uint64_t retiring_slots        = 0;

    // Top-Down Microarchitecture Analysis (TMA) Level 2
    uint64_t mem_bound_slots = 0; // Subset of backend_bound
    uint64_t fetch_lat_slots = 0; // Subset of frontend_bound
};

namespace detail {

GUANAQO_EXPORT struct ScopedCounters {
    virtual ~ScopedCounters()          = default;
    virtual ThreadPerfCounters &get()  = 0;
    virtual ThreadPerfCounters &stop() = 0;
};

} // namespace detail

/// @addtogroup trace_pcm
/// @{

/// May return null if PCM is not available.
GUANAQO_EXPORT std::unique_ptr<detail::ScopedCounters> start_counters();
/// Disables performance counters globally. Blocks until all active counters have stopped.
GUANAQO_EXPORT void disable_counters();
/// Enables performance counters globally.
GUANAQO_EXPORT void enable_counters();

/// Pins the thread to the current CPU and records a snapshot of performance events.
/// When stop is called or when the object is destroyed, another snapshot is taken and the
/// counters for the thread are updated. The updated counters are passed to the provided function,
/// which can be used for logging Perfetto counters or for other purposes.
template <class F>
GUANAQO_EXPORT struct ScopedCounters {
    F func;
    std::unique_ptr<detail::ScopedCounters> impl;

    ScopedCounters(F &&func)
        : func(std::forward<F>(func)), impl(start_counters()) {}
    ScopedCounters(const ScopedCounters &)            = delete;
    ScopedCounters &operator=(const ScopedCounters &) = delete;
    ~ScopedCounters() { stop(); }
    ThreadPerfCounters *get() { return impl ? &impl->get() : nullptr; }
    void stop() {
        if (impl) {
            func(impl->stop());
            impl.reset();
        }
    }
};

template <class F>
ScopedCounters(F &&) -> ScopedCounters<F>;
template <class F>
ScopedCounters(F &) -> ScopedCounters<F &>;

/// @}

} // namespace guanaqo::pcm
