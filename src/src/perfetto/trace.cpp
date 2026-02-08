#include <guanaqo/perfetto/trace.hpp>

#if GUANAQO_WITH_PERFETTO

#include <fstream>
#include <future>
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
    guanaqo::trace::TrackEvent::Register();
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
    std::ofstream output{output_path, std::ios::out | std::ios::binary};
    if (!output)
        throw std::runtime_error(std::format("Failed to open output file: {}",
                                             output_path.string()));
    std::promise<void> done;
    tracing_session->FlushBlocking();
    tracing_session->ReadTrace(
        [&](::perfetto::TracingSession::ReadTraceCallbackArgs args) {
            output.write(args.data, std::streamsize(args.size));
            if (!args.has_more)
                done.set_value();
        });
    std::this_thread::sleep_for(
        std::chrono::seconds(10)); // TODO: this is a hack
    tracing_session->FlushBlocking();
    tracing_session->Stop();
    done.get_future().wait();
    tracing_session.reset();
}

#if GUANAQO_WITH_PCM
#define GUANAQO_COUNT_PCM(ctr, name, track, time)                              \
    TRACE_COUNTER("pcm", ::perfetto::CounterTrack(#name, track), time,         \
                  (ctr)->name)

// Having this in the header results in absolutely insane compile times ...
void ScopedLinalgCounters::trace_gflops(pcm::ThreadPerfCounters *ctr) const {
    auto time = TrackEvent::GetTraceTimeNs();
    TRACE_COUNTER("gflops", ::perfetto::CounterTrack("gflops", parent_track),
                  time, get_thread_gflop_count());
    if (ctr) {
        GUANAQO_COUNT_PCM(ctr, instructions, parent_track, time);
        GUANAQO_COUNT_PCM(ctr, cycles, parent_track, time);
        GUANAQO_COUNT_PCM(ctr, ref_cycles, parent_track, time);
        GUANAQO_COUNT_PCM(ctr, l2_misses, parent_track, time);
        GUANAQO_COUNT_PCM(ctr, l2_hits, parent_track, time);
        GUANAQO_COUNT_PCM(ctr, l3_misses, parent_track, time);
        GUANAQO_COUNT_PCM(ctr, branch_misses, parent_track, time);
        GUANAQO_COUNT_PCM(ctr, all_slots, parent_track, time);
        GUANAQO_COUNT_PCM(ctr, frontend_bound_slots, parent_track, time);
        GUANAQO_COUNT_PCM(ctr, backend_bound_slots, parent_track, time);
        GUANAQO_COUNT_PCM(ctr, bad_speculation_slots, parent_track, time);
        GUANAQO_COUNT_PCM(ctr, retiring_slots, parent_track, time);
        GUANAQO_COUNT_PCM(ctr, mem_bound_slots, parent_track, time);
        GUANAQO_COUNT_PCM(ctr, fetch_lat_slots, parent_track, time);
    }
}
#endif

} // namespace guanaqo::trace

PERFETTO_TRACK_EVENT_STATIC_STORAGE_IN_NAMESPACE_WITH_ATTRS(guanaqo::trace,
                                                            GUANAQO_EXPORT);

#endif
