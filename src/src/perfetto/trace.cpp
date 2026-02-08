#include <guanaqo/perfetto/trace.hpp>

#if GUANAQO_WITH_PERFETTO

#include <fstream>
#include <stdexcept>

namespace guanaqo::trace {

uint64_t &get_thread_gflop_count() {
    static thread_local uint64_t gflop_count = 0;
    return gflop_count;
}

void initialize_tracing() {
    ::perfetto::TracingInitArgs args;
    args.backends = ::perfetto::kInProcessBackend;

    ::perfetto::Tracing::Initialize(args);
    PERFETTO_TRACK_EVENT_NAMESPACE::TrackEvent::Register();
}

std::unique_ptr<::perfetto::TracingSession> start_tracing(uint32_t memory_kb) {
    ::perfetto::TraceConfig cfg;
    cfg.add_buffers()->set_size_kb(memory_kb);

    auto *ds_cfg = cfg.add_data_sources()->mutable_config();
    ds_cfg->set_name("track_event");

    std::unique_ptr<::perfetto::TracingSession> tracing_session =
        ::perfetto::Tracing::NewTrace();
    tracing_session->Setup(cfg);
    tracing_session->StartBlocking();
    return tracing_session;
}

void abort_tracing(
    std::unique_ptr<::perfetto::TracingSession> tracing_session) {
    tracing_session->StopBlocking();
}

void stop_tracing(std::unique_ptr<::perfetto::TracingSession> tracing_session,
                  const fs::path &output_path) {
    tracing_session->StopBlocking();
    std::ofstream output{output_path, std::ios::out | std::ios::binary};
    if (!output)
        throw std::runtime_error(std::format("Failed to open output file: {}",
                                             output_path.string()));
    tracing_session->ReadTrace(
        [&](::perfetto::TracingSession::ReadTraceCallbackArgs args) {
            output.write(args.data, std::streamsize(args.size));
        });
    // Ensure session is destroyed before output file is closed
    tracing_session.reset();
}

} // namespace guanaqo::trace

PERFETTO_TRACK_EVENT_STATIC_STORAGE_IN_NAMESPACE_WITH_ATTRS(guanaqo::trace,
                                                            GUANAQO_EXPORT);

#endif
