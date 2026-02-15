#pragma once

/// @file
/// @ingroup control
/// Thread-safe stop signal.

#include <atomic>

namespace guanaqo {

/// Cancel a running solver or other job in a thread-safe and signal-safe way.
/// @ingroup control
class AtomicStopSignal {
  public:
    AtomicStopSignal() = default;
    AtomicStopSignal(const AtomicStopSignal &) : AtomicStopSignal() {}
    AtomicStopSignal &operator=(const AtomicStopSignal &) = delete;
    AtomicStopSignal(AtomicStopSignal &&) noexcept : AtomicStopSignal() {}
    AtomicStopSignal &operator=(AtomicStopSignal &&) noexcept { return *this; }

    void stop() { stop_flag.store(true, std::memory_order_seq_cst); }
    [[nodiscard]] bool stop_requested() const {
        return stop_flag.load(std::memory_order_relaxed);
    }

  private:
    std::atomic<bool> stop_flag{false};
};

} // namespace guanaqo
