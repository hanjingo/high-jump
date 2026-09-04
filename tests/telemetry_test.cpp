#include <gtest/gtest.h>
#include <hj/testing/telemetry.hpp>
#include <thread>
#include <chrono>
#include <map>
#include <string>
#include <future>

#if defined(_WIN32) || defined(_WIN64)
#ifndef NOMINMAX
#define NOMINMAX
#endif
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

#if defined(_WIN32) || defined(_WIN64)
static ULARGE_INTEGER filetime_to_ull(const FILETIME &ft)
{
    ULARGE_INTEGER ull;
    ull.LowPart  = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    return ull;
}
#endif

static double _get_cpu_usage()
{
#if defined(_WIN32) || defined(_WIN64)
    static ULARGE_INTEGER last_idle = {0}, last_kernel = {0}, last_user = {0};

    FILETIME idleTime, kernelTime, userTime;
    if(!GetSystemTimes(&idleTime, &kernelTime, &userTime))
    {
        return 0.0;
    }

    ULARGE_INTEGER idle   = filetime_to_ull(idleTime);
    ULARGE_INTEGER kernel = filetime_to_ull(kernelTime);
    ULARGE_INTEGER user   = filetime_to_ull(userTime);

    ULONGLONG idle_diff   = idle.QuadPart - last_idle.QuadPart;
    ULONGLONG kernel_diff = kernel.QuadPart - last_kernel.QuadPart;
    ULONGLONG user_diff   = user.QuadPart - last_user.QuadPart;

    last_idle   = idle;
    last_kernel = kernel;
    last_user   = user;

    ULONGLONG sys_total = kernel_diff + user_diff;
    if(sys_total == 0)
    {
        return 0.0;
    }

    ULONGLONG kernel_non_idle = sys_total - idle_diff;
    double cpu_usage = (double) (kernel_non_idle * 100.0) / (double) sys_total;
    return cpu_usage < 0.0 ? 0.0 : (cpu_usage > 100.0 ? 100.0 : cpu_usage);
#else
    static unsigned long last_user = 0, last_nice = 0, last_system = 0,
                         last_idle = 0;

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

    unsigned long total_prev = last_user + last_nice + last_system + last_idle;
    unsigned long total_curr = user + nice + system + idle;

    unsigned long total_diff = total_curr - total_prev;
    unsigned long idle_diff  = idle - last_idle;

    last_user   = user;
    last_nice   = nice;
    last_system = system;
    last_idle   = idle;

    if(total_diff == 0)
        return 0.0;

    double cpu_usage =
        (double) (total_diff - idle_diff) * 100.0 / (double) total_diff;
    return cpu_usage < 0.0 ? 0.0 : (cpu_usage > 100.0 ? 100.0 : cpu_usage);
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

static std::string
get_span_attribute_value_str(const hj::telemetry::trace_span_data_t &span,
                             const std::string                      &target)
{
    for(const auto &[key, value] : span.GetAttributes())
    {
        if(key != target)
            continue;

        auto value_str = [](const auto &v) -> std::string {
            using T = std::decay_t<decltype(v)>;
            if constexpr(std::is_same_v<T, bool>)
                return v ? "true" : "false";
            else if constexpr(std::is_arithmetic_v<T>)
                return std::to_string(v);
            else if constexpr(std::is_same_v<T, std::string>)
                return v;
            else if constexpr(std::is_same_v<T,
                                             opentelemetry::nostd::string_view>)
                return std::string{v.data(), v.size()};
            else
                return "[unsupported type]";
        };
        return opentelemetry::nostd::visit(value_str, value);
    }
    return "";
}

TEST(telemetry, runtime_config_initialization)
{
    hj::telemetry::config cfg;
    cfg.service_name           = "my_service";
    cfg.service_version        = "1.0.0";
    cfg.deployment_environment = "production";
    cfg.type                   = hj::telemetry::exporter_type::ostream;

    hj::telemetry::runtime telemetry(cfg);

    auto tracer = telemetry.tracer_handle("network");
    auto span   = tracer.scoped_span("send_request");
    span.set_attribute("peer", "server1");
    span.add_event("request_sent");

    auto meter = telemetry.meter_handle("network");
    auto request_count =
        meter.counter<uint64_t>("requests_total", "Total requests", "1");
    request_count->Add(1);

    auto latency = meter.histogram<double>("request_duration", "Latency", "ms");
    latency->Record(14.5, {}, opentelemetry::context::Context{});

    EXPECT_TRUE(telemetry.force_flush());
    EXPECT_TRUE(telemetry.shutdown());
}

TEST(telemetry, trace_context_propagation_test)
{
    hj::telemetry::config cfg;
    cfg.service_name = "service_a";
    cfg.type         = hj::telemetry::exporter_type::ostream;

    hj::telemetry::runtime telemetry(cfg);

    auto tracer_a = telemetry.tracer_handle("service_a_tracer");
    auto span_a   = tracer_a.start_scoped_span("call_from_a");

    std::map<std::string, std::string>           http_headers;
    hj::telemetry::propagation::text_map_carrier carrier(http_headers);
    hj::telemetry::propagation::inject(carrier);

    EXPECT_TRUE(http_headers.find("traceparent") != http_headers.end());

    hj::telemetry::propagation::text_map_carrier remote_carrier(http_headers);
    auto                                         extracted_context =
        hj::telemetry::propagation::extract(remote_carrier);

    auto tracer_b = telemetry.tracer_handle("service_b_tracer");
    auto span_b =
        tracer_b.start_span_with_context("handle_in_b", extracted_context);

    EXPECT_NE(span_b, nullptr);
    span_b->End();

    telemetry.force_flush();
}

TEST(telemetry, trace_ostream_export_default)
{
    auto rt     = hj::telemetry::make_ostream_runtime("ostream1");
    auto tracer = rt.get_tracer("ostream1");
    for(int i = 0; i < 2; ++i)
    {
        auto span_hello = tracer.start_span("call_hello");
        _hello();
        tracer.end_span(span_hello);

        auto span_world = tracer.start_span("call_world");
        _world();
        tracer.end_span(span_world);
    }
    rt.shutdown();
}

TEST(telemetry, trace_scoped_raii_demo)
{
    auto rt     = hj::telemetry::make_ostream_runtime("raii_demo");
    auto tracer = rt.get_tracer("raii_demo");

    {
        auto parent_span =
            tracer.start_scoped_span("parent_job", {{"user_id", 10001}});

        {
            auto child_span =
                tracer.start_scoped_span("child_task", {{"db_retry", true}});
            child_span.add_event("querying_database");
        }
    }

    rt.force_flush();
    rt.shutdown();
}

TEST(telemetry, trace_custom_span_exporter)
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

        void on_export(const std::unique_ptr<trace_recordable_t>
                           &recordable) noexcept override
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
                          << get_span_attribute_value_str(*span, attr.first)
                          << ", ";
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

TEST(telemetry, trace_otlp_http_export)
{
    std::string endpoint = "http://xxx";
    if(endpoint == "http://xxx")
        GTEST_SKIP()
            << "Please configure a valid OTLP endpoint to run this trace test.";

    auto rt     = hj::telemetry::make_otlp_http_runtime(endpoint, true);
    auto tracer = rt.get_tracer("otlp_http_test");
    for(int i = 0; i < 2; ++i)
    {
        auto span_hello = tracer.start_span("call_hello");
        _hello();
        tracer.end_span(span_hello);

        auto span_world = tracer.start_span("call_world");
        _world();
        tracer.end_span(span_world);
    }
    rt.shutdown();
}

TEST(telemetry, trace_otlp_file_export)
{
    std::string file_pattern = "trace-otlp-file.json";
    auto        rt     = hj::telemetry::make_otlp_file_runtime(file_pattern);
    auto        tracer = rt.get_tracer("otlp_file_test");
    for(int i = 0; i < 2; ++i)
    {
        auto span_hello = tracer.start_span("call_hello");
        _hello();
        tracer.end_span(span_hello);

        auto span_world = tracer.start_span("call_world");
        _world();
        tracer.end_span(span_world);
    }
    rt.shutdown();
}

static void
sys_metrics_callback(opentelemetry::metrics::ObserverResult observer,
                     void                                  *state)
{
    auto observer_double =
        opentelemetry::nostd::get<opentelemetry::nostd::shared_ptr<
            opentelemetry::metrics::ObserverResultT<double>>>(observer);

    observer_double->Observe(_get_cpu_usage());
}

TEST(telemetry, meter_ostream_export_default)
{
    hj::telemetry::clean_up_metrics();

    std::promise<void> p;
    auto               f = p.get_future();

    std::thread worker([&p]() {
        auto rt    = hj::telemetry::make_ostream_runtime("meter1");
        auto meter = rt.get_meter("meter1", "1.2.0", "");

        auto request_counter = meter.create_u64_counter("http.requests.total",
                                                        "Total HTTP requests",
                                                        "1");

        auto cpu_gauge = meter.create_double_obs_gauge("system.cpu.usage",
                                                       "System CPU usage",
                                                       "%");
        cpu_gauge->AddCallback(sys_metrics_callback, nullptr);

        for(uint32_t i = 0; i < 3; ++i)
        {
            request_counter->Add(10);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        rt.shutdown();
        hj::telemetry::clean_up_metrics();
        p.set_value();
    });

    f.wait();
    if(worker.joinable())
    {
        worker.join();
    }
}

TEST(telemetry, meter_otlp_http_export)
{
    std::string endpoint = "http://xxx";
    if(endpoint == "http://xxx")
        GTEST_SKIP()
            << "Please configure a valid OTLP endpoint to run this meter test.";

    hj::telemetry::clean_up_metrics();

    auto rt    = hj::telemetry::make_otlp_http_runtime(endpoint, true);
    auto meter = rt.get_meter("meter_otlp_http_test", "1.2.0", "");

    auto cpu_gauge =
        meter.create_double_obs_gauge("system.cpu.usage", "CPU usage");
    cpu_gauge->AddCallback(sys_metrics_callback, nullptr);

    for(int i = 0; i < 3; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    rt.shutdown();
    hj::telemetry::clean_up_metrics();
}

TEST(telemetry, meter_otlp_file_export)
{
    hj::telemetry::clean_up_metrics();

    auto rt    = hj::telemetry::make_otlp_file_runtime("meter-otlp-file.json");
    auto meter = rt.get_meter("meter_otlp_file_test", "1.2.0", "");

    auto cpu_gauge =
        meter.create_double_obs_gauge("system.cpu.usage", "CPU usage");
    cpu_gauge->AddCallback(sys_metrics_callback, nullptr);

    for(int i = 0; i < 3; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    rt.shutdown();
    hj::telemetry::clean_up_metrics();
}

TEST(telemetry, multi_tracer_shared_runtime_demo)
{
    auto runtime = hj::telemetry::make_ostream_runtime("my_microservice");

    auto net_tracer = runtime.get_tracer("network");
    auto db_tracer  = runtime.get_tracer("database");

    {
        auto span_net = net_tracer.start_scoped_span("http_request");
        {
            auto span_db =
                db_tracer.start_scoped_span("sql_query",
                                            {{"db.system", "mysql"}});
            span_db.add_event("connected");
        }
    }

    runtime.force_flush();
    runtime.shutdown();
}

TEST(telemetry, runtime_graceful_shutdown)
{
    bool shutdown_called = false;

    class test_exporter : public hj::telemetry::custom_trace_span_exporter_t
    {
      public:
        explicit test_exporter(bool &flag)
            : shutdown_flag(flag)
        {
        }
        void on_export(const std::unique_ptr<hj::telemetry::trace_recordable_t>
                           &) noexcept override
        {
        }
        void on_shutdown() noexcept override { shutdown_flag = true; }

      private:
        bool &shutdown_flag;
    };

    {
        auto exporter  = std::make_unique<test_exporter>(shutdown_called);
        auto processor = hj::telemetry::make_simple_trace_span_processor(
            std::move(exporter));
        auto sdk_trace_provider =
            opentelemetry::sdk::trace::TracerProviderFactory::Create(
                std::move(processor));

        hj::telemetry::runtime rt{opentelemetry::nostd::shared_ptr<
                                      opentelemetry::trace::TracerProvider>(
                                      sdk_trace_provider.release()),
                                  opentelemetry::nostd::shared_ptr<
                                      opentelemetry::metrics::MeterProvider>{}};

        auto tracer = rt.get_tracer("shutdown_test");
        auto span   = tracer.start_span("test_span");
        tracer.end_span(span);

        rt.shutdown();
        EXPECT_TRUE(shutdown_called);
    }
}

TEST(telemetry, meter_all_instruments_demo)
{
    using namespace hj::telemetry;
    clean_up_metrics();

    std::promise<void> p;
    auto               f = p.get_future();

    std::thread worker([&p]() {
        auto rt    = make_ostream_runtime("demo_meter");
        auto meter = rt.get_meter("demo_meter");

        auto requests_counter = meter.counter<uint64_t>("http.server.requests",
                                                        "Total requests served",
                                                        "1");
        requests_counter->Add(
            1,
            {{"http.status_code", 200}, {"http.method", "GET"}});

        auto active_conns =
            meter.up_down_counter<int64_t>("db.client.active_connections",
                                           "Current active DB connections",
                                           "1");
        active_conns->Add(1);
        active_conns->Add(-1);

        auto rpc_latency = meter.histogram<double>("rpc.server.duration",
                                                   "RPC latency distribution",
                                                   "ms");
        rpc_latency->Record(12.3, {}, opentelemetry::context::Context{});

        auto sys_memory = meter.obs_gauge<double>("system.memory.usage",
                                                  "Memory usage percentage",
                                                  "%");
        sys_memory->AddCallback(sys_metrics_callback, nullptr);

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        rt.shutdown();
        clean_up_metrics();
        p.set_value();
    });

    f.wait();
    if(worker.joinable())
    {
        worker.join();
    }
}

TEST(telemetry_industrial, empty_tracer_edge_cases)
{
    auto rt     = hj::telemetry::make_ostream_runtime("empty_test");
    auto tracer = rt.get_tracer("empty_test");

    auto span = tracer.start_span("");
    EXPECT_NE(span, nullptr);
    tracer.end_span(span);

    {
        auto guard = tracer.start_scoped_span("");
        guard.set_attribute("key", "value");
        guard.add_event("empty_event");
    }
    rt.shutdown();
}

TEST(telemetry_industrial, invalid_provider_safety)
{
    hj::telemetry::runtime null_rt(
        opentelemetry::nostd::shared_ptr<
            opentelemetry::trace::TracerProvider>{},
        opentelemetry::nostd::shared_ptr<
            opentelemetry::metrics::MeterProvider>{});

    auto tracer = null_rt.get_tracer("null_tracer");
    auto meter  = null_rt.get_meter("null_meter");

    auto span = tracer.start_span("should_not_crash");
    EXPECT_EQ(span, nullptr);

    auto scoped = tracer.start_scoped_span("scoped_null");
    scoped.set_attribute("test", 123);
    scoped.add_event("event");

    auto counter = meter.counter<uint64_t>("null_counter");
    EXPECT_EQ(counter, nullptr);

    EXPECT_TRUE(null_rt.force_flush());
    EXPECT_TRUE(null_rt.shutdown());
}

TEST(telemetry_industrial, span_guard_move_semantics)
{
    auto rt     = hj::telemetry::make_ostream_runtime("move_test");
    auto tracer = rt.get_tracer("move_test");

    {
        auto a = tracer.start_scoped_span("span_a", {{"phase", "initial"}});
        auto b = std::move(a);

        b.set_attribute("phase", "moved");
        b.add_event("b_event");
    }

    rt.force_flush();
    rt.shutdown();
}

TEST(telemetry_industrial, exception_path_automatic_end)
{
    auto rt     = hj::telemetry::make_ostream_runtime("exception_test");
    auto tracer = rt.get_tracer("exception_test");

    auto function_that_throws = [&tracer]() {
        auto guard = tracer.start_scoped_span("faulty_operation");
        guard.set_attribute("status", "running");
        throw std::runtime_error("critical business failure");
    };

    EXPECT_THROW(
        {
            try
            {
                function_that_throws();
            }
            catch(const std::exception &e)
            {
                throw;
            }
        },
        std::runtime_error);

    rt.force_flush();
    rt.shutdown();
}

TEST(telemetry_industrial, multithread_high_frequency_stress)
{
    auto rt = hj::telemetry::make_otlp_file_runtime("stress-test-spans.json");
    auto tracer = rt.get_tracer("stress_service");

    const int thread_count     = 10;
    const int spans_per_thread = 1000;

    std::atomic<bool>        start_flag{false};
    std::vector<std::thread> threads;
    threads.reserve(thread_count);

    auto start_time = std::chrono::high_resolution_clock::now();

    for(int i = 0; i < thread_count; ++i)
    {
        threads.emplace_back([&tracer, &start_flag, spans_per_thread]() {
            while(!start_flag.load(std::memory_order_acquire))
            {
                std::this_thread::yield();
            }

            for(int j = 0; j < spans_per_thread; ++j)
            {
                auto span = tracer.start_span("stress_span", {{"index", j}});
                tracer.end_span(span);
            }
        });
    }

    start_flag.store(true, std::memory_order_release);

    for(auto &t : threads)
    {
        t.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        end_time - start_time)
                        .count();

    std::cout << "[Stress Profile] Thread count: " << thread_count
              << ", Total spans: " << (thread_count * spans_per_thread)
              << ", Cost: " << duration << " ms" << std::endl;

    rt.force_flush();
    rt.shutdown();
}

TEST(telemetry_industrial, shutdown_flush_export_lifecycle_validation)
{
    std::atomic<int>  export_counter{0};
    std::atomic<bool> shutdown_called{0};

    class lifecycle_test_exporter
        : public hj::telemetry::custom_trace_span_exporter_t
    {
      public:
        lifecycle_test_exporter(std::atomic<int> &cnt, std::atomic<bool> &shut)
            : export_cnt(cnt)
            , shutdown_flag(shut)
        {
        }

        void on_export(const std::unique_ptr<hj::telemetry::trace_recordable_t>
                           &recordable) noexcept override
        {
            if(recordable)
            {
                export_cnt.fetch_add(1, std::memory_order_relaxed);
            }
        }

        void on_shutdown() noexcept override
        {
            shutdown_flag.store(true, std::memory_order_release);
        }

      private:
        std::atomic<int>  &export_cnt;
        std::atomic<bool> &shutdown_flag;
    };

    {
        auto exporter =
            std::make_unique<lifecycle_test_exporter>(export_counter,
                                                      shutdown_called);
        auto processor = hj::telemetry::make_simple_trace_span_processor(
            std::move(exporter));

        auto sdk_trace_provider =
            opentelemetry::sdk::trace::TracerProviderFactory::Create(
                std::move(processor));

        hj::telemetry::runtime rt{opentelemetry::nostd::shared_ptr<
                                      opentelemetry::trace::TracerProvider>(
                                      sdk_trace_provider.release()),
                                  opentelemetry::nostd::shared_ptr<
                                      opentelemetry::metrics::MeterProvider>{}};

        auto tracer = rt.get_tracer("lifecycle_service");

        {
            auto span = tracer.start_span("lifecycle_span_1");
            tracer.end_span(span);
        }
        {
            auto span = tracer.start_span("lifecycle_span_2");
            tracer.end_span(span);
        }

        EXPECT_TRUE(rt.force_flush());
        EXPECT_TRUE(rt.shutdown());
        EXPECT_TRUE(shutdown_called.load(std::memory_order_acquire));
    }

    EXPECT_GE(export_counter.load(std::memory_order_relaxed), 2);
}