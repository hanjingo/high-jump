#include <gtest/gtest.h>
#include <hj/testing/telemetry.hpp>
#include <thread>
#include <chrono>
#include <map>
#include <string>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <psapi.h>
#else
#include <unistd.h>
#endif

static void _hello()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

static void _world()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

static double _get_cpu_usage()
{
#if defined(_WIN32) || defined(_WIN64)
    FILETIME idleTime, kernelTime, userTime;
    if(GetSystemTimes(&idleTime, &kernelTime, &userTime))
    {
        ULARGE_INTEGER k, u;
        k.LowPart  = kernelTime.dwLowDateTime;
        k.HighPart = kernelTime.dwHighDateTime;
        u.LowPart  = userTime.dwLowDateTime;
        u.HighPart = userTime.dwHighDateTime;
        return (double) (k.QuadPart + u.QuadPart) / 10000000.0;
    }
    return 0.0;
#else
    FILE *fp = fopen("/proc/stat", "r");
    if(!fp)
        return 0.0;

    char buf[256];
    if(!fgets(buf, sizeof(buf), fp))
    {
        fclose(fp);
        return 0.0;
    }
    fclose(fp);
    unsigned long user, nice, system, idle;
    sscanf(buf, "cpu %lu %lu %lu %lu", &user, &nice, &system, &idle);
    return (double) (user + nice + system) / (user + nice + system + idle);
#endif
}

static double _get_mem_usage()
{
#if defined(_WIN32) || defined(_WIN64)
    PROCESS_MEMORY_COUNTERS pmc;
    if(GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
        return pmc.WorkingSetSize / 1024.0 / 1024.0;
    return 0.0;
#else
    FILE *fp = fopen("/proc/self/statm", "r");
    if(!fp)
        return 0.0;
    long rss = 0;
    if(fscanf(fp, "%*s %ld", &rss) != 1)
    {
        fclose(fp);
        return 0.0;
    }
    fclose(fp);
    long page_size = sysconf(_SC_PAGESIZE);
    return (double) rss * page_size / 1024.0 / 1024.0;
#endif
}

TEST(TelemetryTest, TraceOStreamExportDefault)
{
    auto tracer = hj::telemetry::make_ostream_tracer("ostream1");
    for(int i = 0; i < 2; ++i)
    {
        auto span_hello = tracer.start_span("call_hello");
        _hello();
        tracer.end_span(span_hello);

        auto span_world = tracer.start_span("call_world");
        _world();
        tracer.end_span(span_world);
    }
}

TEST(TelemetryTest, TraceCustomSpanExporter)
{
    using namespace hj::telemetry;
    class my_custom_trace_span_exporter
        : public hj::telemetry::custom_trace_span_exporter_t
    {
      public:
        my_custom_trace_span_exporter()
            : hj::telemetry::custom_trace_span_exporter_t()
        {
        }

        void on_export(
            const std::unique_ptr<trace_recordable_t> &recordable) noexcept
        {
            auto ptr = recordable.get();
            if(!ptr)
            {
                std::cout << "[Custom] recordable is null" << std::endl;
                return;
            }
            auto span = dynamic_cast<trace_span_data_t *>(ptr);
            if(!span)
            {
                std::cout << "[Custom] recordable is not SpanData" << std::endl;
                return;
            }
            std::cout << "[Custom] span info: {\n";
            std::cout << "  name: " << to_std_string(span->GetName()) << "\n";
            std::cout << "  trace_id: " << to_std_string(span->GetTraceId())
                      << "\n";
            std::cout << "  span_id: " << to_std_string(span->GetSpanId())
                      << "\n";
            std::cout << "  parent_span_id: "
                      << to_std_string(span->GetParentSpanId()) << "\n";
            std::cout << "  start: "
                      << span->GetStartTime().time_since_epoch().count()
                      << "\n";
            std::cout << "  duration: " << span->GetDuration().count() << "\n";
            std::cout << "  attributes: ";
            for(const auto &attr : span->GetAttributes())
            {
                std::cout << attr.first << "="
                          << detail::get_value_str(*span, attr.first) << ", ";
            }
            std::cout << "\n}" << std::endl;
        }
    };

    auto exporter = std::make_unique<my_custom_trace_span_exporter>();
    auto processor =
        hj::telemetry::make_simple_trace_span_processor(std::move(exporter));
    auto tracer = hj::telemetry::make_custom_tracer("my_custom_span_exporter1",
                                                    std::move(processor));
    for(int i = 0; i < 2; ++i)
    {
        auto span_hello = tracer.start_span("call_hello");
        _hello();
        tracer.end_span(span_hello);

        auto span_world = tracer.start_span("call_world");
        _world();
        tracer.end_span(span_world);
    }
}

TEST(TelemetryTest, TraceOtlpHttpExport)
{
    // prefer to use aliyun cloud as otlp server: https://help.aliyun.com/zh/opentelemetry/user-guide/before-you-begin-before-you-begin?spm=a2c4g.11186623.0.0.1ecc7bdb3PnAO4

    std::string endpoint = "http://xxx";
    if(endpoint == "http://xxx")
        GTEST_SKIP()
            << "Please configure a valid OTLP endpoint to run this trace test.";

    auto tracer = hj::telemetry::make_otlp_http_tracer(
        "otlp_http_test",
        endpoint,
        true,
        hj::telemetry::http_request_content_type::kBinary);
    for(int i = 0; i < 2; ++i)
    {
        auto span_hello = tracer.start_span("call_hello");
        _hello();
        tracer.end_span(span_hello);

        auto span_world = tracer.start_span("call_world");
        _world();
        tracer.end_span(span_world);
    }
}

TEST(TelemetryTest, TraceOtlpFileExport)
{
    std::string file_pattern = "trace-otlp-file.json";
    auto        tracer =
        hj::telemetry::make_otlp_file_tracer("otlp_file_test", file_pattern);
    for(int i = 0; i < 2; ++i)
    {
        auto span_hello = tracer.start_span("call_hello");
        _hello();
        tracer.end_span(span_hello);

        auto span_world = tracer.start_span("call_world");
        _world();
        tracer.end_span(span_world);
    }
}

TEST(TelemetryTest, MeterOStreamExportDefault)
{
    hj::telemetry::clean_up_metrics();

    auto meter =
        hj::telemetry::make_ostream_meter("meter1", "1.2.0", "", 500, 100);
    auto cpu_counter =
        meter.create_double_counter("cpu_usage", "count CPU usage", "percent");
    auto mem_counter = meter.create_double_counter("mem_usage",
                                                   "count Memory usage",
                                                   "percent");
    for(uint32_t i = 0; i < 5; ++i)
    {
        auto cpu = _get_cpu_usage();
        auto mem = _get_mem_usage();
        cpu_counter->Add(cpu);
        mem_counter->Add(mem);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void async_obs_gauge(opentelemetry::metrics::ObserverResult observer,
                     void                                  *state)
{
    auto observer_double =
        opentelemetry::nostd::get<opentelemetry::nostd::shared_ptr<
            opentelemetry::metrics::ObserverResultT<double> > >(observer);
    observer_double->Observe(10.2f);
}

TEST(TelemetryTest, MeterOtlpHttpExport)
{
    // prefer to use aliyun cloud as otlp server: https://help.aliyun.com/zh/opentelemetry/user-guide/before-you-begin-before-you-begin?spm=a2c4g.11186623.0.0.1ecc7bdb3PnAO4

    std::string endpoint = "http://xxx";
    if(endpoint == "http://xxx")
        GTEST_SKIP()
            << "Please configure a valid OTLP endpoint to run this meter test.";

    hj::telemetry::clean_up_metrics();

    auto meter = hj::telemetry::make_otlp_http_meter(
        "meter_otlp_http_test",
        "1.2.0",
        "",
        endpoint,
        hj::telemetry::http_request_content_type::kBinary,
        500,
        100,
        true);

    auto cpu_gauge = meter.create_double_obs_gauge("system.cpu.usage",
                                                   "CPU usage"); // for aliyun
    cpu_gauge->AddCallback(async_obs_gauge, nullptr);

    for(int i = 0; i < 10; ++i)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

TEST(TelemetryTest, MeterOtlpFileExport)
{
    hj::telemetry::clean_up_metrics();

    auto meter = hj::telemetry::make_otlp_file_meter("meter_otlp_file_test",
                                                     "1.2.0",
                                                     "",
                                                     "meter-otlp-file.json",
                                                     500,
                                                     100,
                                                     true);

    auto cpu_gauge =
        meter.create_double_obs_gauge("system.cpu.usage", "CPU usage");
    cpu_gauge->AddCallback(async_obs_gauge, nullptr);

    for(int i = 0; i < 5; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}