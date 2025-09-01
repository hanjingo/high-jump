#ifndef TELEMETRY_HPP
#define TELEMETRY_HPP

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <functional>
#include <variant>

#include <opentelemetry/sdk/trace/span_data.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/sdk/trace/simple_processor.h>
#include <opentelemetry/sdk/trace/exporter.h>
#include <opentelemetry/sdk/common/exporter_utils.h>
#include <opentelemetry/exporters/ostream/span_exporter_factory.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/tracer.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/span_context.h>
#include <opentelemetry/trace/scope.h>
#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/nostd/string_view.h>
#include <opentelemetry/nostd/variant.h>

namespace libcpp 
{

namespace telemetry
{
using span_t            = opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span>;
using span_exporter_t   = std::unique_ptr<opentelemetry::sdk::trace::SpanExporter>;
using span_processor_t  = opentelemetry::sdk::trace::SpanProcessor;
using span_data_t       = opentelemetry::sdk::trace::SpanData;
using tracer_provider_t = opentelemetry::trace::TracerProvider;
using tracer_base_t     = opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer>;
using recordable_t      = std::unique_ptr<opentelemetry::sdk::trace::Recordable>;

struct ostream_exporter{};
struct custom_exporter{};

namespace detail
{
static std::string to_std_string(const opentelemetry::trace::TraceId& id)
{
    std::string s;
    if (id.IsValid() && id.kSize == 16)
    {
        s.resize(2 * id.kSize);
        id.ToLowerBase16(s);
    }
    return s;
}

static std::string to_std_string(const opentelemetry::trace::SpanId& id)
{
    std::string s;
    if (id.IsValid() && id.kSize == 8)
    {
        s.resize(2 * id.kSize);
        id.ToLowerBase16(s);
    }
    return s;
}

static std::string to_std_string(const opentelemetry::nostd::string_view& sv)
{
    return std::string{sv.data(), sv.size()};
}

static std::string get_value_str(const span_data_t& span, const std::string& target)
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
            else if constexpr (std::is_same_v<T, opentelemetry::nostd::string_view>)
                return std::string{v.data(), v.size()};
            else
                return "[unsupported type]";
        };
        return opentelemetry::nostd::visit(value_str, value);
    }
    return "";
}

// self defined span_exporter
class custom_span_exporter : public opentelemetry::sdk::trace::SpanExporter 
{
public:
    custom_span_exporter()
    {
    }

    std::unique_ptr<opentelemetry::sdk::trace::Recordable> MakeRecordable() noexcept override
    {
        return std::unique_ptr<opentelemetry::sdk::trace::Recordable>(new opentelemetry::sdk::trace::SpanData());
    }

    opentelemetry::sdk::common::ExportResult Export(
        const opentelemetry::nostd::span<std::unique_ptr<opentelemetry::sdk::trace::Recordable>>& spans) noexcept override
    {
        for (const auto& recordable : spans) 
        {
            on_export(recordable);
        }
        return opentelemetry::sdk::common::ExportResult::kSuccess;
    }

    bool Shutdown(std::chrono::microseconds timeout = std::chrono::microseconds(0)) noexcept override { return true; }

    bool ForceFlush(std::chrono::microseconds timeout = std::chrono::microseconds(0)) noexcept override { return true; }

    virtual void on_export(const recordable_t& recordable) noexcept
    {
        // Custom export logic here
    }
};
}

using custom_span_exporter_t = detail::custom_span_exporter;

template <typename T>
static std::string to_std_string(const T& value)
{
    return detail::to_std_string(value);
}

template<typename T>
class tracer 
{
public:
    explicit tracer(const std::string& name, span_exporter_t&& exporter)
        : _exporter{std::move(exporter)}
    {
        auto processor = std::make_unique<opentelemetry::sdk::trace::SimpleSpanProcessor>(std::move(_exporter));
        auto provider = opentelemetry::nostd::shared_ptr<tracer_provider_t>(new opentelemetry::sdk::trace::TracerProvider(std::move(processor)));
        opentelemetry::trace::Provider::SetTracerProvider(provider);
        _tracer = provider->GetTracer(name);
    }
    ~tracer()
    {
    }

    span_t start_span(const std::string& name, 
                      const std::map<std::string, std::string>& attrs = {}) 
    {
        auto span = _tracer->StartSpan(name);
        for (const auto& kv : attrs)
            span->SetAttribute(kv.first, kv.second);

        return span;
    }

    void end_span(span_t& span) 
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

    void add_event(span_t& span, 
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
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> _tracer;
    span_exporter_t _exporter;
    bool _is_custom;
};

static tracer<ostream_exporter> make_ostream_tracer(const std::string& name)
{
    auto exporter = opentelemetry::exporter::trace::OStreamSpanExporterFactory::Create();
    return tracer<ostream_exporter>(name, std::move(exporter));
}

static tracer<custom_exporter> make_custom_tracer(const std::string& name, span_exporter_t&& exporter)
{
    return tracer<custom_exporter>(name, std::move(exporter));
}


} // namespace telemetry
} // namespace libcpp

#endif // TELEMETRY_HPP