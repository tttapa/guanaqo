#include <guanaqo/pcm/counters.hpp>
#include <atomic>

#if GUANAQO_WITH_PCM

#include <bit>
#include <cmath>
#include <format>
#include <memory>
#include <mutex>
#include <vector>

#include <msr.h>
#include <sched.h>
#include <types.h>
#include <unistd.h>

namespace guanaqo::pcm {

using namespace ::pcm;

GUANAQO_EXPORT ThreadPerfCounters &get_thread_perf_counters() {
    static thread_local ThreadPerfCounters counters;
    return counters;
}

namespace {

/// Get the current CPU ID.
int get_cpu(int cpu = -1) {
    if (cpu < 0)
        cpu = sched_getcpu();
    return cpu;
}

/// Implementation follows cpucounters.cpp:4288-4303 (setEvent lambda)
/// Creates an IA32_PERFEVTSELx register value from event number and umask
constexpr uint64 makeEventSelect(uint8_t event, uint8_t umask) {
    const decltype(EventSelectRegister::fields) fields{
        .event_select = event, // Bits 7:0 - Event select
        .umask        = umask, // Bits 15:8 - Unit mask
        .usr          = 1,     // Bit 16 - Count in user mode (CPL > 0)
        .os           = 1,     // Bit 17 - Count in kernel mode (CPL = 0)
        .edge         = 0,     // Bit 18 - Edge detect (0=count cycles)
        .pin_control  = 0,     // Bit 19 - Pin control
        .apic_int     = 0,     // Bit 20 - APIC interrupt enable
        .any_thread   = 0,     // Bit 21 - Any thread (0=this thread only)
        .enable       = 1,     // Bit 22 - Enable counter
        .invert       = 0,     // Bit 23 - Invert counter mask
        .cmask        = 0,     // Bits 31:24 - Counter mask
        .in_tx        = 0,     // Bit 32 - In TSX transaction
        .in_txcp      = 0,     // Bit 33 - In TSX abort handler
        .reservedX    = 0,     // Bits 63:34 - Reserved must be zero
    };
    return std::bit_cast<uint64>(fields);
}

/// Helper to extract a range of bits from a 64-bit value.
constexpr uint64 extract_bits(uint64 value, int start, int end) {
    uint64 mask = (uint64{1} << (end - start + 1)) - 1;
    return (value >> start) & mask;
}

class LightweightPerfCounters {
  private:
    std::unique_ptr<MsrHandle> msr;

    /// Event select configuration for different counters
    struct CounterConfig {
        uint64 evtsel_addr;
        uint64 pmc_addr;
        uint64 config;
    };

    /// Configure 4 programmable counters with Skylake+ events
    /// Event configuration follows cpucounters.cpp:3735-3738 and 4318-4319
    /// Uses `SKL_MEM_LOAD_RETIRED_*` constants from types.h:127-134
    static constexpr std::array<CounterConfig, 4> counters{{
        // Counter 0: L2 cache misses
        // Event: MEM_LOAD_RETIRED.L2_MISS (0xD1:0x10)
        {.evtsel_addr = IA32_PERFEVTSEL0_ADDR,
         .pmc_addr    = IA32_PMC0,
         .config      = makeEventSelect(SKL_MEM_LOAD_RETIRED_L2_MISS_EVTNR,
                                        SKL_MEM_LOAD_RETIRED_L2_MISS_UMASK)},
        // Counter 1: L2 cache hits
        // Event: MEM_LOAD_RETIRED.L2_HIT (0xD1:0x02)
        {.evtsel_addr = IA32_PERFEVTSEL1_ADDR,
         .pmc_addr    = IA32_PMC1,
         .config      = makeEventSelect(SKL_MEM_LOAD_RETIRED_L2_HIT_EVTNR,
                                        SKL_MEM_LOAD_RETIRED_L2_HIT_UMASK)},
        // Counter 2: L3 cache misses
        // Event: MEM_LOAD_RETIRED.L3_MISS (0xD1:0x20)
        {.evtsel_addr = IA32_PERFEVTSEL2_ADDR,
         .pmc_addr    = IA32_PMC2,
         .config      = makeEventSelect(SKL_MEM_LOAD_RETIRED_L3_MISS_EVTNR,
                                        SKL_MEM_LOAD_RETIRED_L3_MISS_UMASK)},
        // Counter 3: Branch mispredictions
        // Event: BR_MISP_RETIRED.ALL_BRANCHES (0xC5:0x00)
        {.evtsel_addr = IA32_PERFEVTSEL3_ADDR,
         .pmc_addr    = IA32_PMC3,
         .config      = makeEventSelect(0xC5, 0x00)},
    }};

  public:
    LightweightPerfCounters(uint32_t cpu)
        : msr(std::make_unique<MsrHandle>(cpu)) {}
    LightweightPerfCounters(const LightweightPerfCounters &) = delete;
    LightweightPerfCounters &
    operator=(const LightweightPerfCounters &)                   = delete;
    LightweightPerfCounters(LightweightPerfCounters &&) noexcept = default;
    LightweightPerfCounters &
    operator=(LightweightPerfCounters &&) noexcept = default;
    ~LightweightPerfCounters() { stop(); }

    // Start counting
    // Implementation follows cpucounters.cpp:4252-4422
    void start() {
        // Step 1: Disable counters during programming (cpucounters.cpp:4252)
        // This prevents spurious counts while we're configuring
        msr->write(pcm::IA32_CR_PERF_GLOBAL_CTRL, 0);

        // Step 2: Configure fixed counter control register (cpucounters.cpp:4138-4162)
        // IA32_CR_FIXED_CTR_CTRL (MSR 0x38D) controls 3 fixed counters:
        //   Counter 0: Instructions Retired (INST_RETIRED.ANY)
        //   Counter 1: Core Clocks Unhalted (CPU_CLK_UNHALTED.THREAD)
        //   Counter 2: Reference Clocks Unhalted (CPU_CLK_UNHALTED.REF_TSC)
        // Each counter uses 4 bits: [1:0]=enable user+kernel, [2]=any_thread, [3]=pmi
        // We set os=1 (kernel) and usr=1 (user) for each counter
        FixedEventControlRegister fixed_ctrl_reg;
        fixed_ctrl_reg.value       = 0;
        fixed_ctrl_reg.fields.os0  = 1; // Counter 0: count in kernel mode
        fixed_ctrl_reg.fields.usr0 = 1; // Counter 0: count in user mode
        fixed_ctrl_reg.fields.os1  = 1; // Counter 1: count in kernel mode
        fixed_ctrl_reg.fields.usr1 = 1; // Counter 1: count in user mode
        fixed_ctrl_reg.fields.os2  = 1; // Counter 2: count in kernel mode
        fixed_ctrl_reg.fields.usr2 = 1; // Counter 2: count in user mode

        // Check if TOPDOWN.SLOTS is supported (Ice Lake+) and enable fixed counter 3
        // NOTE: On hybrid CPUs (12th gen+), this only works on P-cores, not E-cores
        // Use taskset to pin to a P-core: taskset -c 0 ./lightweight_perf_example
        uint64 test_slots = 0;
        if (msr->read(TOPDOWN_SLOTS_ADDR, &test_slots) == sizeof(uint64)) {
            // Counter 3: count in kernel mode (TOPDOWN.SLOTS)
            fixed_ctrl_reg.fields.os3  = 1;
            fixed_ctrl_reg.fields.usr3 = 1; // Counter 3: count in user mode
        }

        // Step 3: Reset all fixed counters BEFORE writing control register (cpucounters.cpp:4257-4260)
        msr->write(INST_RETIRED_ADDR, 0);            // MSR 0x309
        msr->write(CPU_CLK_UNHALTED_THREAD_ADDR, 0); // MSR 0x30A
        msr->write(CPU_CLK_UNHALTED_REF_ADDR, 0);    // MSR 0x30B
        msr->write(IA32_CR_FIXED_CTR_CTRL, fixed_ctrl_reg.value);

        // Step 4: Reset programmable counters BEFORE writing event selectors (cpucounters.cpp:4381)
        for (const auto &counter : counters)
            msr->write(counter.pmc_addr, 0);

        // Step 5: Program programmable counter event selectors (cpucounters.cpp:4382)
        for (const auto &counter : counters)
            msr->write(counter.evtsel_addr, counter.config);

        // Step 6: Reset Top-Down metrics MSRs (cpucounters.cpp:4393-4400)
        // TOPDOWN_SLOTS (MSR 0x30C): Counts total pipeline slots
        // PERF_METRICS (MSR 0x329): Accumulated TMA metrics (Ice Lake+)
        msr->write(TOPDOWN_SLOTS_ADDR, 0); // MSR 0x30C (types.h:53)
        msr->write(PERF_METRICS_ADDR, 0);  // MSR 0x329 (types.h:54)

        // Step 7: Enable all counters globally (cpucounters.cpp:4386-4422)
        // IA32_CR_PERF_GLOBAL_CTRL (MSR 0x38F):
        //   Bits 0-3: Enable PMC0-3
        //   Bits 32-34: Enable fixed counters 0-2
        // Start with basic counters only (cpucounters.cpp:4388)
        uint64 global_ctrl = (uint64{0xF} << 0) | // PMC0-3
                             (uint64{0x7} << 32); // Fixed 0-2

        // Try to enable TOPDOWN.SLOTS (Ice Lake+) - if CPU doesn't support it, reading will fail
        // On hybrid CPUs, this check will fail on E-cores but succeed on P-cores
        uint64 test_val = 0;
        if (msr->read(TOPDOWN_SLOTS_ADDR, &test_val) == sizeof(uint64))
            // Enable fixed counter 3 (TOPDOWN.SLOTS)
            global_ctrl |= uint64{1} << 35;

        // Try to enable PERF_METRICS (Ice Lake+)
        if (msr->read(PERF_METRICS_ADDR, &test_val) == sizeof(uint64))
            // Enable PERF_METRICS
            global_ctrl |= uint64{1} << 48;

        // Clear any overflow status bits (cpucounters.cpp:4420)
        msr->write(IA32_PERF_GLOBAL_OVF_CTRL, global_ctrl);
        // Enable all configured counters (cpucounters.cpp:4421)
        msr->write(IA32_CR_PERF_GLOBAL_CTRL, global_ctrl);
    }

    // Stop counting
    void stop() {
        // (cpucounters.cpp:5264-5297)
        msr->write(IA32_CR_PERF_GLOBAL_CTRL, 0);
        // Disable event selectors
        for (const auto &counter : counters)
            msr->write(counter.evtsel_addr, 0);
        msr->write(IA32_CR_FIXED_CTR_CTRL, 0);
    }

    struct Snapshot {
        uint64 abs_instructions;  // Fixed counter 0
        uint64 abs_cycles;        // Fixed counter 1
        uint64 abs_ref_cycles;    // Fixed counter 2
        uint64 abs_l2_misses;     // PMC0
        uint64 abs_l2_hits;       // PMC1
        uint64 abs_l3_misses;     // PMC2
        uint64 abs_branch_misses; // PMC3

        // Top-Down Microarchitecture Analysis (Ice Lake+)
        uint64 delta_slots;
        uint64 delta_frontend_bound_slots;
        uint64 delta_backend_bound_slots;
        uint64 delta_bad_speculation_slots;
        uint64 delta_retiring_slots;
        uint64 delta_mem_bound_slots; // Level 2 metric
        uint64 delta_fetch_lat_slots; // Level 2 metric
    };

    static void accumulate(ThreadPerfCounters &ctr, const Snapshot &before,
                           const Snapshot &after) {
        ctr.instructions += after.abs_instructions - before.abs_instructions;
        ctr.cycles += after.abs_cycles - before.abs_cycles;
        ctr.ref_cycles += after.abs_ref_cycles - before.abs_ref_cycles;
        ctr.l2_misses += after.abs_l2_misses - before.abs_l2_misses;
        ctr.l2_hits += after.abs_l2_hits - before.abs_l2_hits;
        ctr.l3_misses += after.abs_l3_misses - before.abs_l3_misses;
        ctr.branch_misses += after.abs_branch_misses - before.abs_branch_misses;

        ctr.all_slots += after.delta_slots;
        ctr.frontend_bound_slots += after.delta_frontend_bound_slots;
        ctr.backend_bound_slots += after.delta_backend_bound_slots;
        ctr.bad_speculation_slots += after.delta_bad_speculation_slots;
        ctr.retiring_slots += after.delta_retiring_slots;
        ctr.mem_bound_slots += after.delta_mem_bound_slots;
        ctr.fetch_lat_slots += after.delta_fetch_lat_slots;
    }

    Snapshot read() {
        // Check for CPU migration
        auto current_cpu = static_cast<int32_t>(sched_getcpu());
        if (current_cpu != msr->getCoreId())
            throw std::runtime_error(std::format(
                "CPU migration detected: was on CPU {} , now on CPU {}",
                msr->getCoreId(), current_cpu));

        Snapshot s{};
        msr->read(INST_RETIRED_ADDR, &s.abs_instructions);
        msr->read(CPU_CLK_UNHALTED_THREAD_ADDR, &s.abs_cycles);
        msr->read(CPU_CLK_UNHALTED_REF_ADDR, &s.abs_ref_cycles);
        msr->read(counters[0].pmc_addr, &s.abs_l2_misses);
        msr->read(counters[1].pmc_addr, &s.abs_l2_hits);
        msr->read(counters[2].pmc_addr, &s.abs_l3_misses);
        msr->read(counters[3].pmc_addr, &s.abs_branch_misses);

        // Read Top-Down metrics (Ice Lake+)
        // (cpucounters.cpp:5702-5741)
        // IMPORTANT: These MSRs accumulate since the last reset - they are STATEFUL
        // PERF_METRICS: Contains accumulated ratios (8-bit per metric)
        // TOPDOWN_SLOTS: Contains accumulated slot count
        // Both must be reset after reading to get deltas on the next read
        uint64 perf_metrics  = 0;
        uint64 topdown_slots = 0;

        if (msr->read(PERF_METRICS_ADDR, &perf_metrics) == sizeof(uint64) &&
            msr->read(TOPDOWN_SLOTS_ADDR, &topdown_slots) == sizeof(uint64)) {

            // Reset counters for next measurement period (read+reset as delta)
            msr->write(PERF_METRICS_ADDR, 0);
            msr->write(TOPDOWN_SLOTS_ADDR, 0);

            // Extract Level 1 metric ratios (8-bit values, 0-255)
            uint64 retiring_ratio = extract_bits(perf_metrics, 0, 7);
            uint64 bad_spec_ratio = extract_bits(perf_metrics, 8, 15);
            uint64 frontend_ratio = extract_bits(perf_metrics, 16, 23);
            uint64 backend_ratio  = extract_bits(perf_metrics, 24, 31);

            // Extract Level 2 metric ratios (Sapphire Rapids+)
            uint64 mem_bound_ratio = extract_bits(perf_metrics, 56, 63);
            uint64 fetch_lat_ratio = extract_bits(perf_metrics, 48, 55);

            // Scale ratios to actual slot counts (DELTA values since last read)
            // The ratio represents accumulated behavior since last reset
            const auto total_ratio = retiring_ratio + bad_spec_ratio +
                                     frontend_ratio + backend_ratio;
            const auto inv_total_ratio =
                total_ratio > 0 ? 1.0 / static_cast<double>(total_ratio) : 0.0;
            auto delta_slots = [&](uint64 ratio) {
                return static_cast<uint64_t>(
                    std::round((static_cast<double>(ratio) * inv_total_ratio) *
                               static_cast<double>(topdown_slots)));
            };
            s.delta_slots                 = topdown_slots;
            s.delta_frontend_bound_slots  = delta_slots(frontend_ratio);
            s.delta_backend_bound_slots   = delta_slots(backend_ratio);
            s.delta_bad_speculation_slots = delta_slots(bad_spec_ratio);
            s.delta_retiring_slots        = delta_slots(retiring_ratio);
            s.delta_mem_bound_slots       = delta_slots(mem_bound_ratio);
            s.delta_fetch_lat_slots       = delta_slots(fetch_lat_ratio);
        }

        return s;
    }
};

struct alignas(128) CountersEntry {
    // Should be uncontended since each CPU is accessed by only one thread at a time
    std::mutex mtx;
    std::optional<LightweightPerfCounters> counters;
};

GUANAQO_EXPORT std::vector<CountersEntry> &get_all_perf_counters() {
    static std::vector<CountersEntry> instances = [] {
        long max_cpus = sysconf(_SC_NPROCESSORS_CONF);
        return std::vector<CountersEntry>(static_cast<size_t>(max_cpus));
    }();
    return instances;
}

std::pair<std::unique_lock<std::mutex>, LightweightPerfCounters &>
get_perf_counters(int cpu) {
    auto &inst = get_all_perf_counters()[static_cast<size_t>(cpu)];
    std::unique_lock lck{inst.mtx};
    if (!inst.counters)
        inst.counters.emplace(static_cast<uint32_t>(cpu)).start();
    return {std::move(lck), *inst.counters};
}

void stop_all_perf_counters() {
    for (auto &entry : get_all_perf_counters()) {
        std::lock_guard lck{entry.mtx};
        entry.counters.reset();
    }
}

} // namespace

namespace detail {
namespace {

struct ScopedThreadAffinity {
    cpu_set_t original_set;
    ScopedThreadAffinity(int cpu) {
        // Save original affinity
        sched_getaffinity(0, sizeof(cpu_set_t), &original_set);
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(cpu, &set); // Set affinity to target CPU
        sched_setaffinity(0, sizeof(cpu_set_t), &set);
    }
    ScopedThreadAffinity(const ScopedThreadAffinity &)            = delete;
    ScopedThreadAffinity &operator=(const ScopedThreadAffinity &) = delete;
    ~ScopedThreadAffinity() {
        // Restore original affinity
        sched_setaffinity(0, sizeof(cpu_set_t), &original_set);
    }
};

struct PCMScopedCounters : ScopedCounters {
    ScopedThreadAffinity affinity;
    std::pair<std::unique_lock<std::mutex>, LightweightPerfCounters &>
        cpu_counters;
    LightweightPerfCounters::Snapshot start;

    PCMScopedCounters(int cpu)
        : affinity(cpu), cpu_counters(get_perf_counters(cpu)) {
        start = cpu_counters.second.read();
    }
    ThreadPerfCounters &get() override { return get_thread_perf_counters(); }
    ThreadPerfCounters &stop() override {
        auto after            = cpu_counters.second.read();
        auto &thread_counters = get_thread_perf_counters();
        LightweightPerfCounters::accumulate(thread_counters, start, after);
        return thread_counters;
    }
};

} // namespace
} // namespace detail

GUANAQO_EXPORT std::atomic_bool &get_counters_enabled_flag() {
    static std::atomic_bool enabled{false};
    return enabled;
}

void disable_counters() {
    if (get_counters_enabled_flag().exchange(false, std::memory_order_relaxed))
        stop_all_perf_counters();
}

void enable_counters() {
    get_counters_enabled_flag().store(true, std::memory_order_relaxed);
}

std::unique_ptr<detail::ScopedCounters> start_counters() {
    if (get_counters_enabled_flag().load(std::memory_order_relaxed))
        return std::make_unique<detail::PCMScopedCounters>(get_cpu());
    return {};
}

} // namespace guanaqo::pcm

#else // Fallback implementation when PCM is not available

namespace guanaqo::pcm {

std::unique_ptr<detail::ScopedCounters> start_counters() { return {}; }
void disable_counters();
void enable_counters();

} // namespace guanaqo::pcm

#endif
