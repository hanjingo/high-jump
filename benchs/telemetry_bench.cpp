#include <benchmark/benchmark.h>
#include <hj/testing/telemetry.hpp>
#include <thread>
#include <chrono>
#include <cstdlib>

using namespace hj;

// Simple helper workload used by tests
static void _hello_work()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

// Tracer: ostream default exporter
static void bm_telemetry_tracer_ostream(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        auto tracer = telemetry::make_ostream_tracer("bench_tracer");
        state.ResumeTiming();

        auto span = tracer.start_span("bench_span");
        _hello_work();
        tracer.end_span(span);

        benchmark::DoNotOptimize(span);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_telemetry_tracer_ostream)->Iterations(100);

// Tracer: custom processor but using ostream exporter under the hood
static void bm_telemetry_tracer_custom(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        auto exporter = opentelemetry::exporter::trace::
            OStreamSpanExporterFactory::Create();
        auto processor =
            telemetry::make_simple_trace_span_processor(std::move(exporter));
        auto tracer = telemetry::make_custom_tracer("bench_custom_tracer",
                                                    std::move(processor));
        state.ResumeTiming();

        auto span = tracer.start_span("bench_custom_span");
        _hello_work();
        tracer.end_span(span);

        benchmark::DoNotOptimize(span);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_telemetry_tracer_custom)->Iterations(100);

// Meter: ostream counters
static void bm_telemetry_meter_ostream(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        telemetry::clean_up_metrics();
        auto meter =
            telemetry::make_ostream_meter("bench_meter", "0.1", "", 200, 50);
        auto cpu = meter.create_double_counter("cpu", "cpu usage", "percent");
        auto mem = meter.create_double_counter("mem", "mem usage", "MB");
        state.ResumeTiming();

        cpu->Add(1.23);
        mem->Add(4.56);

        benchmark::DoNotOptimize(cpu);
        benchmark::DoNotOptimize(mem);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_telemetry_meter_ostream)->Iterations(100);

// Guarded: OTLP / file exporters are heavier and may require network or file writes
static void bm_telemetry_otlp_guarded(benchmark::State &state)
{
    if(std::getenv("HJ_BENCH_ALLOW_TELEMETRY") == nullptr)
    {
        state.SkipWithError("telemetry heavy exporters disabled; set "
                            "HJ_BENCH_ALLOW_TELEMETRY=1 to enable");
        return;
    }

    for(auto _ : state)
    {
        state.PauseTiming();
        // OTLP file exporter
        auto tracer = telemetry::make_otlp_file_tracer(
            "bench_otlp_file",
            std::string("telemetry-bench.json"));
        state.ResumeTiming();

        auto span = tracer.start_span("bench_otlp_span");
        _hello_work();
        tracer.end_span(span);

        benchmark::DoNotOptimize(span);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_telemetry_otlp_guarded)->Iterations(10);
