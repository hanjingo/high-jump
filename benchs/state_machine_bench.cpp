#include <benchmark/benchmark.h>
#include <hj/algo/state_machine.hpp>
#include <memory>

// Recreate the state machine used by tests (small, header-only boost::sml)
// Events
struct StartEvent
{
};
struct PauseEvent
{
};
struct ResumeEvent
{
};
struct StopEvent
{
};
struct ResetEvent
{
};

// States
struct IdleState
{
};
struct RunningState
{
};
struct PausedState
{
};
struct StoppedState
{
};

// Actions (no-op for bench)
auto on_start  = []() {};
auto on_pause  = []() {};
auto on_resume = []() {};
auto on_stop   = []() {};
auto on_reset  = []() {};

// define the state machine as a lambda variable (matches tests)
static auto state_machine = []() {
    using namespace hj::sml;
    return make_transition_table(
        *state<IdleState> + event<StartEvent> / on_start = state<RunningState>,
        state<RunningState> + event<PauseEvent> / on_pause = state<PausedState>,
        state<PausedState> + event<ResumeEvent> / on_resume =
            state<RunningState>,
        state<RunningState> + event<StopEvent> / on_stop = state<StoppedState>,
        state<PausedState> + event<StopEvent> / on_stop  = state<StoppedState>,
        state<StoppedState> + event<ResetEvent> / on_reset = state<IdleState>);
};

using sm_t = hj::sml::sm<decltype(state_machine)>;

// Construct state machine cost
static void bm_state_machine_construct(benchmark::State &state)
{
    for(auto _ : state)
    {
        sm_t sm(state_machine);
        benchmark::DoNotOptimize(sm);
    }
}
BENCHMARK(bm_state_machine_construct)->Iterations(10000);

// Process full workflow Start -> Pause -> Resume -> Stop -> Reset multiple times
static void bm_state_machine_full_work_flow(benchmark::State &state)
{
    const int repeats = static_cast<int>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        sm_t sm(state_machine);
        state.ResumeTiming();

        for(int i = 0; i < repeats; ++i)
        {
            sm.process_event(StartEvent{});
            sm.process_event(PauseEvent{});
            sm.process_event(ResumeEvent{});
            sm.process_event(StopEvent{});
            sm.process_event(ResetEvent{});
        }

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_state_machine_full_work_flow)->Arg(10)->Arg(100);

// Check state.is() hot path
static void bm_state_machine_is_check(benchmark::State &state)
{
    const int checks = static_cast<int>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        sm_t sm(state_machine);
        sm.process_event(StartEvent{}); // move to Running
        state.ResumeTiming();

        for(int i = 0; i < checks; ++i)
        {
            bool b = sm.is(hj::sml::state<RunningState>);
            benchmark::DoNotOptimize(b);
        }

        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_state_machine_is_check)->Arg(1000)->Arg(10000);
