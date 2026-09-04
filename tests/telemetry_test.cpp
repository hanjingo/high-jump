#include <gtest/gtest.h>
#include <hj/testing/telemetry.hpp>
#include <thread>
#include <chrono>
#include <map>
#include <string>

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

// 辅助函数：将 FILETIME 转换为 ULARGE_INTEGER
#if defined(_WIN32) || defined(_WIN64)
static ULARGE_INTEGER filetime_to_ull(const FILETIME &ft)
{
    ULARGE_INTEGER ull;
    ull.LowPart  = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    return ull;
}
#endif

// 修复后的跨平台 CPU 利用率计算（基于差分采样，输出真正的百分比 0.0 ~ 100.0）
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

    // 计算差值
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
    // CPU 占用率百分比
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

TEST(telemetry, trace_ostream_export_default)
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

TEST(telemetry, trace_scoped_raii_demo)
{
    auto tracer = hj::telemetry::make_ostream_tracer("raii_demo");

    {
        auto parent_span =
            tracer.start_scoped_span("parent_job", {{"user_id", 10001}});

        {
            auto child_span =
                tracer.start_scoped_span("child_task", {{"db_retry", true}});
            child_span.add_event("querying_database");
        }
    }

    tracer.force_flush();
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

TEST(telemetry, trace_otlp_file_export)
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

    auto meter =
        hj::telemetry::make_ostream_meter("meter1", "1.2.0", "", 500, 100);

    auto request_counter = meter.create_u64_counter("http.requests.total",
                                                    "Total HTTP requests",
                                                    "1");

    auto cpu_gauge = meter.create_double_obs_gauge("system.cpu.usage",
                                                   "System CPU usage",
                                                   "%");
    cpu_gauge->AddCallback(sys_metrics_callback, nullptr);

    for(uint32_t i = 0; i < 5; ++i)
    {
        request_counter->Add(10);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

TEST(telemetry, meter_otlp_http_export)
{
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

    auto cpu_gauge =
        meter.create_double_obs_gauge("system.cpu.usage", "CPU usage");
    cpu_gauge->AddCallback(sys_metrics_callback, nullptr);

    for(int i = 0; i < 5; ++i)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

TEST(telemetry, meter_otlp_file_export)
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
    cpu_gauge->AddCallback(sys_metrics_callback, nullptr);

    for(int i = 0; i < 5; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
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
                                  nullptr};

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
    auto meter = make_ostream_meter("demo_meter");

    // 1. Counter (单调递增，如 HTTP 请求总数)
    auto requests_counter = meter.counter<uint64_t>("http.server.requests",
                                                    "Total requests served",
                                                    "1");
    requests_counter->Add(1,
                          {{"http.status_code", 200}, {"http.method", "GET"}});

    // 2. UpDownCounter (双向增减，如当前数据库连接池大小)
    auto active_conns =
        meter.up_down_counter<int64_t>("db.client.active_connections",
                                       "Current active DB connections",
                                       "1");
    active_conns->Add(1);
    active_conns->Add(-1);

    // 3. Histogram (延迟分布统计，如 RPC 耗时)
    auto rpc_latency = meter.histogram<double>("rpc.server.duration",
                                               "RPC latency distribution",
                                               "ms");

    // 修正：直接记录数据，或使用单参数版本，或者只传基础数值
    rpc_latency->Record(12.3, {}, opentelemetry::context::Context{});

    // 4. Observable Gauge (异步回调驱动瞬时状态)
    auto sys_memory = meter.obs_gauge<double>("system.memory.usage",
                                              "Memory usage percentage",
                                              "%");
    sys_memory->AddCallback(sys_metrics_callback, nullptr);
}

TEST(telemetry, advanced_custom_view_registry_injection)
{
    using namespace hj::telemetry;
    clean_up_metrics();

    // 高级用户自主构建 ViewRegistry
    auto custom_views =
        std::make_unique<opentelemetry::sdk::metrics::ViewRegistry>();

    auto inst_selector =
        opentelemetry::sdk::metrics::InstrumentSelectorFactory::Create(
            opentelemetry::sdk::metrics::InstrumentType::kHistogram,
            "rpc.server.duration",
            "ms");
    auto meter_selector =
        opentelemetry::sdk::metrics::MeterSelectorFactory::Create(
            "custom_service",
            "1.0.0",
            "");

    // 使用 SDK 默认支持的 Histogram 聚合类型
    auto hist_view = opentelemetry::sdk::metrics::ViewFactory::Create(
        "rpc.server.duration.custom",
        "Custom histogram view description",
        opentelemetry::sdk::metrics::AggregationType::kHistogram);

    // 将 Selector 与 View 进行绑定添加
    custom_views->AddView(std::move(inst_selector),
                          std::move(meter_selector),
                          std::move(hist_view));

    // 通过参数注入给 runtime
    auto rt = make_ostream_runtime("custom_service", std::move(custom_views));
    auto custom_meter = rt.get_meter("custom_service", "1.0.0");

    auto hist = custom_meter.histogram<double>("rpc.server.duration",
                                               "RPC duration",
                                               "ms");
    // 补齐 3 个参数：value, attributes, context
    hist->Record(45.2, {}, opentelemetry::context::Context{});

    rt.force_flush();
}

TEST(telemetry_industrial, empty_tracer_edge_cases)
{
    auto tracer = hj::telemetry::make_ostream_tracer("empty_test");

    // 传入空字符串作为 Span 名称
    auto span = tracer.start_span("");
    EXPECT_NE(span, nullptr);
    tracer.end_span(span);

    // 作用域空名称 Span Guard
    {
        auto guard = tracer.start_scoped_span("");
        guard.set_attribute("key", "value");
        guard.add_event("empty_event");
    } // 确保自动析构结束
}

// ============================================================================
// 2. Invalid Provider 测试：验证传入 nullptr Provider 的防御性与健壮性
// ============================================================================
TEST(telemetry_industrial, invalid_provider_safety)
{
    // 显式传入 nullptr 构造 runtime
    hj::telemetry::runtime null_rt(nullptr, nullptr);

    auto tracer = null_rt.get_tracer("null_tracer");
    auto meter  = null_rt.get_meter("null_meter");

    // 操作应当全部安全降级（No-op），绝不发生段错误 (Segmentation Fault)
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

// ============================================================================
// 3. Move 语义测试：验证 span_guard 的合法移动构造
// ============================================================================
TEST(telemetry_industrial, span_guard_move_semantics)
{
    auto tracer = hj::telemetry::make_ostream_tracer("move_test");

    {
        auto a = tracer.start_scoped_span("span_a", {{"phase", "initial"}});
        // 验证移动构造：将 a 移动给 b，a 自身变空但不应触发崩溃
        auto b = std::move(a);

        b.set_attribute("phase", "moved");
        b.add_event("b_event");
    } // b 在此作用域结束时自动析构并成功调用 End()

    tracer.force_flush();
}

// ============================================================================
// 4. Exception Path 测试：验证抛出异常时 Span 能够自动正确结束
// ============================================================================
TEST(telemetry_industrial, exception_path_automatic_end)
{
    auto tracer = hj::telemetry::make_ostream_tracer("exception_test");

    auto function_that_throws = [&tracer]() {
        auto guard = tracer.start_scoped_span("faulty_operation");
        guard.set_attribute("status", "running");
        // 模拟业务逻辑抛出异常
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
                // 捕获异常，同时验证离开作用域时 span_guard 正常析构（不会内存泄漏或死锁）
                throw;
            }
        },
        std::runtime_error);

    tracer.force_flush();
}

// ============================================================================
// 5 & 6. Multithread & High Frequency 综合评测：100线程 / 100k Spans / 高频性能
// ============================================================================
TEST(telemetry_industrial, multithread_high_frequency_stress)
{
    // 使用文件或内存排查以减少标准输出控制台锁带来的性能失真
    auto rt = hj::telemetry::make_otlp_file_runtime("stress-test-spans.json");
    auto tracer = rt.get_tracer("stress_service");

    const int thread_count     = 10;   // 工业压测可调整为 100
    const int spans_per_thread = 1000; // 工业压测可调整为 100000 (总计 10M)

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
                // 模拟极简计算开销
                tracer.end_span(span);
            }
        });
    }

    // 同时并发冲刺
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
}

// ============================================================================
// 7. Shutdown -> Flush -> Export 完整链路工业级验证
// ============================================================================
TEST(telemetry_industrial, shutdown_flush_export_lifecycle_validation)
{
    std::atomic<int>  export_counter{0};
    std::atomic<bool> shutdown_called{0};

    // 自定义一个全链路生命周期追踪 Exporter
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
                                  nullptr};

        auto tracer = rt.get_tracer("lifecycle_service");

        // 1. 产生 Span
        {
            auto span = tracer.start_span("lifecycle_span_1");
            tracer.end_span(span);
        }
        {
            auto span = tracer.start_span("lifecycle_span_2");
            tracer.end_span(span);
        }

        // 2. 显式测试 Flush
        EXPECT_TRUE(rt.force_flush());

        // 3. 显式测试 Shutdown
        EXPECT_TRUE(rt.shutdown());
        EXPECT_TRUE(shutdown_called.load(std::memory_order_acquire));
    }

    // 验证在 shutdown 闭环完成后，Span 数据是否已被成功捕获并导出
    EXPECT_GE(export_counter.load(std::memory_order_relaxed), 2);
}