#ifndef TELEMETRY_HPP
#define TELEMETRY_HPP

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <variant>

#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/nostd/string_view.h>
#include <opentelemetry/nostd/variant.h>
#include <opentelemetry/sdk/common/exporter_utils.h>
#include <opentelemetry/sdk/resource/resource.h>

#include <opentelemetry/sdk/trace/span_data.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/sdk/trace/simple_processor.h>
#include <opentelemetry/sdk/trace/exporter.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/tracer.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/span_context.h>
#include <opentelemetry/trace/scope.h>

#include <opentelemetry/sdk/metrics/meter_provider.h>
#include <opentelemetry/sdk/metrics/push_metric_exporter.h>
#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader.h>
#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h>
#include <opentelemetry/sdk/metrics/view/view_registry.h>
#include <opentelemetry/metrics/meter.h>
#include <opentelemetry/metrics/noop.h>

#include <opentelemetry/exporters/ostream/span_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter.h>
#include <opentelemetry/exporters/ostream/metric_exporter_factory.h>

namespace libcpp 
{

namespace telemetry
{
using tracer_provider_t        = opentelemetry::trace::TracerProvider;
using tracer_base_t            = opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer>;
using trace_id_t               = opentelemetry::trace::TraceId;
using trace_span_t             = opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span>;
using trace_span_processor_t   = opentelemetry::sdk::trace::SpanProcessor;
using trace_span_data_t        = opentelemetry::sdk::trace::SpanData;
using trace_span_id_t          = opentelemetry::trace::SpanId;
using trace_span_exporter_t    = opentelemetry::sdk::trace::SpanExporter;
using trace_recordable_t       = opentelemetry::sdk::trace::Recordable;

using meter_provider_t             = opentelemetry::sdk::metrics::MeterProvider;
using meter_base_t                 = opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Meter>;
using metric_view_t                = opentelemetry::sdk::metrics::View;
using metric_view_registry_t       = opentelemetry::sdk::metrics::ViewRegistry;
using metric_instrument_selector_t = opentelemetry::sdk::metrics::InstrumentSelector;
using metric_meter_selector_t      = opentelemetry::sdk::metrics::MeterSelector;
using metric_reader_t              = opentelemetry::sdk::metrics::MetricReader;
using metric_u64_counter_t         = opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Counter<uint64_t>>;
using metric_double_counter_t      = opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Counter<double>>;
using metric_obs_instrument_t      = opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObservableInstrument>;

using export_result_t          = opentelemetry::sdk::common::ExportResult;
using sec_t                    = std::chrono::microseconds;
using ms_t                     = std::chrono::milliseconds;
using string_view_t            = opentelemetry::nostd::string_view;
using resource_t               = opentelemetry::sdk::resource::Resource;

using otlp_http_opt_t          = opentelemetry::exporter::otlp::OtlpHttpExporterOptions;

enum class http_request_content_type
{
    kJson = opentelemetry::exporter::otlp::HttpRequestContentType::kJson,
    kBinary = opentelemetry::exporter::otlp::HttpRequestContentType::kBinary,
};

struct custom_exporter{};
struct ostream_exporter{};
struct otlp_http_exporter{};

namespace detail
{
static std::string to_std_string(const trace_id_t& id)
{
    std::string s;
    if (id.IsValid() && id.kSize == 16)
    {
        s.resize(2 * id.kSize);
        id.ToLowerBase16(s);
    }
    return s;
}

static std::string to_std_string(const trace_span_id_t& id)
{
    std::string s;
    if (id.IsValid() && id.kSize == 8)
    {
        s.resize(2 * id.kSize);
        id.ToLowerBase16(s);
    }
    return s;
}

static std::string to_std_string(const string_view_t& sv)
{
    return std::string{sv.data(), sv.size()};
}

static std::string get_value_str(const trace_span_data_t& span, const std::string& target)
{
    for (const auto& [key, value] : span.GetAttributes())
    {
        if (key != target)
            continue;

        auto value_str = [](const auto& v) -> std::string {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::string>)
                return v;
            else if constexpr (std::is_arithmetic_v<T>)
                return std::to_string(v);
            else if constexpr (std::is_same_v<T, bool>)
                return v ? "true" : "false";
            else if constexpr (std::is_same_v<T, string_view_t>)
                return std::string{v.data(), v.size()};
            else
                return "[unsupported type]";
        };
        return opentelemetry::nostd::visit(value_str, value);
    }
    return "";
}

static std::unique_ptr<metric_view_registry_t> make_default_metric_view_registry()
{
    auto views = std::make_unique<metric_view_registry_t>();

    // 匹配所有 Counter（无论名字、unit）
    views->AddView(
        std::make_unique<metric_instrument_selector_t>(
            opentelemetry::sdk::metrics::InstrumentType::kCounter, "*", "*"),
        std::make_unique<metric_meter_selector_t>("*", "", ""),
        std::make_unique<metric_view_t>("", "", "", opentelemetry::sdk::metrics::AggregationType::kSum, nullptr, nullptr)
    );

    // 匹配所有 DoubleCounter（unit 不限）
    views->AddView(
        std::make_unique<metric_instrument_selector_t>(
            opentelemetry::sdk::metrics::InstrumentType::kCounter, "*", "*"),
        std::make_unique<metric_meter_selector_t>("*", "", ""),
        std::make_unique<metric_view_t>("", "", "", opentelemetry::sdk::metrics::AggregationType::kSum, nullptr, nullptr)
    );

    // Gauge
    views->AddView(
        std::make_unique<metric_instrument_selector_t>(
            opentelemetry::sdk::metrics::InstrumentType::kGauge, "*", "*"),
        std::make_unique<metric_meter_selector_t>("*", "", ""),
        std::make_unique<metric_view_t>("", "", "", opentelemetry::sdk::metrics::AggregationType::kLastValue, nullptr, nullptr)
    );

    // ObservableGauge
    views->AddView(
        std::make_unique<metric_instrument_selector_t>(
            opentelemetry::sdk::metrics::InstrumentType::kObservableGauge, "*", "*"),
        std::make_unique<metric_meter_selector_t>("*", "", ""),
        std::make_unique<metric_view_t>("", "", "", opentelemetry::sdk::metrics::AggregationType::kLastValue, nullptr, nullptr)
    );

    return views;
}

// self defined trace_span_exporter
class custom_trace_span_exporter : public opentelemetry::sdk::trace::SpanExporter 
{
public:
    custom_trace_span_exporter()
    {
    }

    std::unique_ptr<trace_recordable_t> MakeRecordable() noexcept override
    {
        return std::unique_ptr<opentelemetry::sdk::trace::Recordable>(new trace_span_data_t());
    }

    export_result_t Export(const opentelemetry::nostd::span<std::unique_ptr<trace_recordable_t>>& spans) noexcept override
    {
        for (const auto& recordable : spans) 
        {
            on_export(recordable);
        }
        return export_result_t::kSuccess;
    }

    bool Shutdown(sec_t timeout = sec_t(0)) noexcept override { return true; }

    bool ForceFlush(sec_t timeout = sec_t(0)) noexcept override { return true; }

    virtual void on_export(const std::unique_ptr<trace_recordable_t>& recordable) noexcept
    {
        // Custom export logic here
        // Implement it!
    }
};
}

using custom_trace_span_exporter_t = detail::custom_trace_span_exporter;

template <typename T>
static std::string to_std_string(const T& value)
{
    return detail::to_std_string(value);
}

// ------------------------------------------------- tracer ------------------------------------------
template<typename T>
class tracer 
{
public:
    tracer() = delete;
    tracer(const std::string& name) = delete;
    explicit tracer(const std::string& name, 
                    std::unique_ptr<trace_span_processor_t>&& processor)
    {
        auto provider = opentelemetry::nostd::shared_ptr<tracer_provider_t>(
            new opentelemetry::sdk::trace::TracerProvider(std::move(processor)));
        opentelemetry::trace::Provider::SetTracerProvider(provider);
        _tracer = provider->GetTracer(name);
    }
    ~tracer()
    {
    }

    trace_span_t start_span(const std::string& name, 
                      const std::map<std::string, std::string>& attrs = {}) 
    {
        auto span = _tracer->StartSpan(name);
        for (const auto& kv : attrs)
            span->SetAttribute(kv.first, kv.second);

        return span;
    }

    void end_span(trace_span_t& span) 
    {
        if (span) 
            span->End();
    }

    void trace(const std::string& name, 
               const std::map<std::string, std::string>& attrs = {}) 
    {
        auto span = start_span(name, attrs);
        end_span(span);
    }

    void add_event(trace_span_t& span, 
                   const std::string& event, 
                   const std::map<std::string, std::string>& attrs = {}) 
    {
        if (!span) 
            return;

        span->AddEvent(event);
        for (const auto& kv : attrs)
            span->SetAttribute(kv.first, kv.second);
    }

    std::string curr_trace_id() 
    {
        auto span = _tracer->GetCurrentSpan();
        std::string hex;
        span->GetContext().trace_id().ToLowerBase16(hex);
        return hex;
    }

    std::string curr_span_id()
    {
        auto span = _tracer->GetCurrentSpan();
        std::string hex;
        span->GetContext().span_id().ToLowerBase16(hex);
        return hex;
    }

private:
    tracer_base_t _tracer;
    bool _is_custom;
};

// ------------------------------------------------- meter ------------------------------------------
template<typename T>
class meter 
{
public:
    meter() = delete;
    meter(const std::string& name) = delete;
    explicit meter(const std::string& name, 
                   std::shared_ptr<metric_reader_t> reader,
                   std::unique_ptr<metric_view_registry_t>&& views,
                   const resource_t &resource)
    {
        auto provider = std::make_shared<meter_provider_t>(std::move(views), resource);
        provider->AddMetricReader(reader);
        _meter = provider->GetMeter(name);
    }

    metric_u64_counter_t create_u64_counter(const std::string& name, 
                                            const std::string& desc = "")
    {
        return _meter->CreateUInt64Counter(name, desc);
    }

    metric_double_counter_t create_double_counter(const std::string& name,
                                                  const std::string& desc = "",
                                                  const std::string& unit = "") 
    {
        return _meter->CreateDoubleCounter(name, desc, unit);
    }

    metric_obs_instrument_t create_i64_obs_counter(const std::string& name,
                                                   const std::string& desc = "",
                                                   const std::string& unit = "") 
    {
        return _meter->CreateInt64ObservableCounter(name, desc, unit);
    }

    metric_obs_instrument_t create_double_obs_counter(const std::string& name,
                                                      const std::string& desc = "",
                                                      const std::string& unit = "") 
    {
        return _meter->CreateDoubleObservableCounter(name, desc, unit);
    }

    metric_obs_instrument_t create_i64_obs_gauge(const std::string& name,
                                                 const std::string& desc = "",
                                                 const std::string& unit = "") 
    {
        return _meter->CreateInt64ObservableGauge(name, desc, unit);
    }

    metric_obs_instrument_t create_double_obs_gauge(const std::string& name, 
                                                    const std::string& desc = "") 
    {
        return _meter->CreateDoubleObservableGauge(name, desc);
    }

    void add_counter(const std::string& name, 
                     uint64_t value, 
                     const std::map<std::string, std::string>& attrs = {}) 
    {
        auto counter = create_counter(name);
        counter->Add(value, attrs);
    }

    void observe_gauge(const std::string& name, 
                       double value, 
                       const std::map<std::string, std::string>& attrs = {}) 
    {
        auto gauge = create_gauge(name);
        gauge->Observe(value, attrs);
    }

private:
    meter_base_t _meter;
};

// ------------------------------------ global api --------------------------------------
// tracer API
static std::unique_ptr<trace_span_processor_t> make_simple_trace_span_processor(
    std::unique_ptr<trace_span_exporter_t>&& exporter)
{
    return std::make_unique<opentelemetry::sdk::trace::SimpleSpanProcessor>(std::move(exporter));
}

static void cleanup_tracer()
{
    std::shared_ptr<opentelemetry::trace::TracerProvider> none;
    opentelemetry::trace::Provider::SetTracerProvider(none);
}

static tracer<custom_exporter> make_custom_tracer(const std::string& name, 
                                                  std::unique_ptr<trace_span_processor_t>&& processor)
{
    return tracer<custom_exporter>(name, std::move(processor));
}

static tracer<ostream_exporter> make_ostream_tracer(const std::string& name, 
                                                    std::unique_ptr<trace_span_processor_t>&& processor)
{
    return tracer<ostream_exporter>(name, std::move(processor));
}

static tracer<ostream_exporter> make_ostream_tracer(const std::string& name)
{
    auto exporter = opentelemetry::exporter::trace::OStreamSpanExporterFactory::Create();
    auto processor = make_simple_trace_span_processor(std::move(exporter));
    return make_ostream_tracer(name, std::move(processor));
}

static tracer<otlp_http_exporter> make_otlp_http_tracer(const std::string& name, 
                                                        std::unique_ptr<trace_span_processor_t>&& processor)
{
    return tracer<otlp_http_exporter>(name, std::move(processor));
}

static tracer<otlp_http_exporter> make_otlp_http_tracer(const std::string& name, 
                                                        const otlp_http_opt_t& options)
{
    auto exporter = std::make_unique<opentelemetry::exporter::otlp::OtlpHttpExporter>(options);
    auto processor = make_simple_trace_span_processor(std::move(exporter));
    return make_otlp_http_tracer(name, std::move(processor));
}

static tracer<otlp_http_exporter> make_otlp_http_tracer(const std::string& name, 
                                                        const std::string& endpoint,
                                                        const bool debug,
                                                        const http_request_content_type content_type)
{
    opentelemetry::exporter::otlp::OtlpHttpExporterOptions options;
    if (!endpoint.empty())
        options.url = endpoint;

    options.console_debug = debug;
    options.content_type = static_cast<opentelemetry::exporter::otlp::HttpRequestContentType>(content_type);
    return make_otlp_http_tracer(name, options);
}

// meter API
static std::unique_ptr<metric_view_registry_t> make_default_metric_view_registry()
{
    return detail::make_default_metric_view_registry();
}

// static meter<ostream_exporter> make_ostream_meter(const std::string& name, 
//                                                   std::unique_ptr<metric_reader_t>&& reader)
// {
//     auto views = std::make_unique<metric_view_registry_t>();
//     auto resource = opentelemetry::sdk::resource::Resource::Create({{"service.name", ""}});
//     return meter<ostream_exporter>(name, std::move(reader), std::move(views), resource);
// }

static meter<ostream_exporter> make_ostream_meter(const std::string& name)
{
    auto exporter = opentelemetry::exporter::metrics::OStreamMetricExporterFactory::Create();
    opentelemetry::sdk::metrics::PeriodicExportingMetricReaderOptions options;

    auto reader = std::make_shared<opentelemetry::sdk::metrics::PeriodicExportingMetricReader>(
        std::move(exporter), 
        options);

    auto views = make_default_metric_view_registry();

    auto resource = opentelemetry::sdk::resource::Resource::Create({{"service.name", name}});
    return meter<ostream_exporter>(name, std::move(reader), std::move(views), resource);
}

} // namespace telemetry
} // namespace libcpp

#endif // TELEMETRY_HPP