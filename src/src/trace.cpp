#include <guanaqo/trace.hpp>
#include <list>
#include <mutex>
#include <thread>

#ifdef _POSIX_THREADS
#include <pthread.h>
#endif

#if GUANAQO_WITH_ITT
namespace guanaqo {
GUANAQO_EXPORT TraceLogger &get_trace_logger() {
    static TraceLogger instance;
    return instance;
}
} // namespace guanaqo

#else
namespace guanaqo {

#ifdef _POSIX_THREADS
template <class T>
concept pthread_self_converts_to = requires { T{::pthread_self()}; };
#else
template <class>
concept pthread_self_converts_to = false;
#endif

GUANAQO_EXPORT std::size_t get_thread_id() {
    return []<class S = std::size_t>() {
        // we need template instantiation to discard the non-selected branch
        if constexpr (pthread_self_converts_to<S>) {
            static thread_local S self{::pthread_self()};
            return self;
        } else {
            static constexpr std::hash<std::thread::id> hasher;
            return hasher(std::this_thread::get_id());
        }
    }();
}

TraceLogger::clock::time_point TraceLogger::t0 = clock::now();

static std::mutex trace_loggers_mutex;
static std::list<std::shared_ptr<TraceLogger>> trace_loggers;
static std::atomic<size_t> default_trace_logger_size = 16'384;

static auto new_trace_logger(size_t size) {
    auto logger = std::make_shared<TraceLogger>(size);
    std::lock_guard lock(trace_loggers_mutex);
    trace_loggers.push_back(logger);
    return logger;
}

GUANAQO_EXPORT size_t trace_logger_set_default_size(size_t size) {
    return default_trace_logger_size.exchange(size, std::memory_order_relaxed);
}

GUANAQO_EXPORT TraceLogger &get_trace_logger() {
    static thread_local auto instance = new_trace_logger(
        default_trace_logger_size.load(std::memory_order_relaxed));
    return *instance;
}

GUANAQO_EXPORT void drop_trace_loggers(
    const std::function<void(std::shared_ptr<TraceLogger>)> &callback) {
    if (callback) {
        auto old_loggers = [&] {
            std::lock_guard lock(trace_loggers_mutex);
            return std::exchange(trace_loggers, {});
        }();
        for (auto &logger : old_loggers)
            callback(std::move(logger));
    } else {
        std::lock_guard lock(trace_loggers_mutex);
        trace_loggers.clear();
    }
}

GUANAQO_EXPORT void
for_each_trace_logger(const std::function<void(TraceLogger &)> &callback) {
    std::lock_guard lock(trace_loggers_mutex);
    for (const auto &logger : trace_loggers)
        callback(*logger);
}

} // namespace guanaqo
#endif
