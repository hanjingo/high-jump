/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025-2026 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef TELEMETRY_HPP
#define TELEMETRY_HPP

#include <atomic>
#include <chrono>
#include <cstddef>
#include <initializer_list>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <opentelemetry/common/attribute_value.h>
#include <opentelemetry/common/key_value_iterable_view.h>
#include <opentelemetry/exporters/ostream/metric_exporter_factory.h>
#include <opentelemetry/exporters/ostream/span_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_file_client_options.h>
#include <opentelemetry/exporters/otlp/otlp_file_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_file_exporter_options.h>
#include <opentelemetry/exporters/otlp/otlp_file_metric_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter.h>
#include <opentelemetry/exporters/otlp/otlp_http_metric_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_http_metric_exporter_options.h>
#include <opentelemetry/metrics/meter_provider.h>
#include <opentelemetry/metrics/provider.h>
#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/nostd/string_view.h>
#include <opentelemetry/nostd/variant.h>
#include <opentelemetry/sdk/common/exporter_utils.h>
#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h>
#include <opentelemetry/sdk/metrics/meter_context_factory.h>
#include <opentelemetry/sdk/metrics/meter_provider_factory.h>
#include <opentelemetry/sdk/metrics/view/instrument_selector_factory.h>
#include <opentelemetry/sdk/metrics/view/meter_selector_factory.h>
#include <opentelemetry/sdk/metrics/view/view_factory.h>
#include <opentelemetry/sdk/resource/resource.h>
#include <opentelemetry/sdk/trace/batch_span_processor_factory.h>
#include <opentelemetry/sdk/trace/batch_span_processor_options.h>
#include <opentelemetry/sdk/trace/simple_processor_factory.h>
#include <opentelemetry/sdk/trace/span_data.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>
#include <opentelemetry/trace/propagation/http_trace_context.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/scope.h>
#include <opentelemetry/trace/tracer.h>
#include <opentelemetry/context/propagation/text_map_propagator.h>

namespace hj::telemetry
{

// ============================================================================
// Type Aliases & Forward Declarations
// ============================================================================
using trace_span_t =
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span>;
using trace_span_processor_t = opentelemetry::sdk::trace::SpanProcessor;
using trace_span_exporter_t  = opentelemetry::sdk::trace::SpanExporter;
using trace_recordable_t     = opentelemetry::sdk::trace::Recordable;
using trace_span_data_t      = opentelemetry::sdk::trace::SpanData;
using trace_id_t             = opentelemetry::trace::TraceId;
using trace_span_id_t        = opentelemetry::trace::SpanId;
using export_result_t        = opentelemetry::sdk::common::ExportResult;

using attribute_value_t = opentelemetry::common::AttributeValue;
using attribute_pair_t =
    std::pair<opentelemetry::nostd::string_view, attribute_value_t>;
using attribute_list_t = std::initializer_list<attribute_pair_t>;
using attribute_view_t =
    opentelemetry::common::KeyValueIterableView<attribute_list_t>;

using http_request_content_type =
    opentelemetry::exporter::otlp::HttpRequestContentType;

// ============================================================================
// Config & Resource Sub-modules
// ============================================================================
enum class exporter_type
{
    ostream,
    otlp_http,
    otlp_file
};

struct config
{
    std::string service_name{"default-service"};
    std::string service_version{"1.0.0"};
    std::string service_instance_id{""};
    std::string deployment_environment{"production"};

    exporter_type type{exporter_type::ostream};
    std::string   otlp_endpoint{"http://localhost:4318"};
    std::string   file_pattern{"telemetry-trace.json"};

    std::map<std::string, std::string> headers;
    std::chrono::milliseconds          timeout{5000};
    bool                               debug{false};
    http_request_content_type content_type{http_request_content_type::kBinary};

    std::map<std::string, std::string> custom_attributes;
};

using resource_options = config;

namespace detail
{
inline opentelemetry::nostd::string_view
to_otel_sv(std::string_view sv) noexcept
{
    return opentelemetry::nostd::string_view(sv.data(), sv.size());
}

inline std::string to_std_string(const trace_id_t &id)
{
    std::string s;
    if(id.IsValid() && id.kSize == 16)
    {
        s.resize(2 * id.kSize);
        id.ToLowerBase16(s);
    }
    return s;
}

inline std::string to_std_string(const trace_span_id_t &id)
{
    std::string s;
    if(id.IsValid() && id.kSize == 8)
    {
        s.resize(2 * id.kSize);
        id.ToLowerBase16(s);
    }
    return s;
}

inline std::string to_std_string(const opentelemetry::nostd::string_view &sv)
{
    return std::string{sv.data(), sv.size()};
}

inline opentelemetry::sdk::resource::Resource
create_otel_resource(const config &cfg)
{
    opentelemetry::sdk::resource::ResourceAttributes attrs;

    if(!cfg.service_name.empty())
        attrs["service.name"] = cfg.service_name;
    if(!cfg.service_version.empty())
        attrs["service.version"] = cfg.service_version;
    if(!cfg.service_instance_id.empty())
        attrs["service.instance.id"] = cfg.service_instance_id;
    if(!cfg.deployment_environment.empty())
        attrs["deployment.environment"] = cfg.deployment_environment;

    for(const auto &[k, v] : cfg.custom_attributes)
    {
        attrs[k] = v;
    }

    return opentelemetry::sdk::resource::Resource::Create(attrs);
}
} // namespace detail

template <typename T>
inline std::string to_std_string(const T &value)
{
    return detail::to_std_string(value);
}

inline auto make_attributes(attribute_list_t attrs) noexcept
{
    return attribute_view_t{attrs};
}

// ============================================================================
// Propagation Sub-module (Trace Context Propagation)
// ============================================================================
namespace propagation
{

class text_map_carrier
    : public opentelemetry::context::propagation::TextMapCarrier
{
  public:
    explicit text_map_carrier(std::map<std::string, std::string> &map)
        : map_(map)
    {
    }

    opentelemetry::nostd::string_view
    Get(opentelemetry::nostd::string_view key) const noexcept override
    {
        auto k  = detail::to_std_string(key);
        auto it = map_.find(k);
        if(it != map_.end())
        {
            return opentelemetry::nostd::string_view(it->second);
        }
        return "";
    }

    void Set(opentelemetry::nostd::string_view key,
             opentelemetry::nostd::string_view value) noexcept override
    {
        map_[detail::to_std_string(key)] = detail::to_std_string(value);
    }

  private:
    std::map<std::string, std::string> &map_;
};

template <typename Carrier>
inline void inject(Carrier &carrier)
{
    auto propagator = opentelemetry::trace::propagation::HttpTraceContext();
    auto current_context = opentelemetry::context::RuntimeContext::GetCurrent();
    propagator.Inject(carrier, current_context);
}

template <typename Carrier>
inline opentelemetry::context::Context
extract(const Carrier                   &carrier,
        opentelemetry::context::Context &context =
            opentelemetry::context::RuntimeContext::GetCurrent())
{
    auto propagator = opentelemetry::trace::propagation::HttpTraceContext();
    return propagator.Extract(carrier, context);
}

} // namespace propagation

// ============================================================================
// Span & Span Guard Sub-module
// ============================================================================
class [[nodiscard]] span_guard final
{
  public:
    explicit span_guard(trace_span_t span) noexcept
        : span_(std::move(span))
        , scope_(span_)
    {
    }

    ~span_guard() noexcept
    {
        try
        {
            if(span_)
            {
                span_->End();
            }
        }
        catch(...)
        {
        }
    }

    span_guard(const span_guard &)            = delete;
    span_guard &operator=(const span_guard &) = delete;
    span_guard &operator=(span_guard &&)      = delete;

    span_guard(span_guard &&other) noexcept
        : span_(std::move(other.span_))
        , scope_(std::move(other.scope_))
    {
    }

    opentelemetry::trace::Span *operator->() const noexcept
    {
        return span_.get();
    }
    opentelemetry::trace::Span &operator*() const noexcept { return *span_; }

    [[nodiscard]] trace_span_t get_span() const noexcept { return span_; }

    void add_event(std::string_view name, attribute_list_t attrs = {}) noexcept
    {
        try
        {
            if(!span_)
                return;
            if(attrs.size() == 0)
            {
                span_->AddEvent(detail::to_otel_sv(name));
                return;
            }
            span_->AddEvent(detail::to_otel_sv(name), attribute_view_t{attrs});
        }
        catch(...)
        {
        }
    }

    void set_attribute(std::string_view         key,
                       const attribute_value_t &value) noexcept
    {
        try
        {
            if(span_)
            {
                span_->SetAttribute(detail::to_otel_sv(key), value);
            }
        }
        catch(...)
        {
        }
    }

  private:
    trace_span_t                span_;
    opentelemetry::trace::Scope scope_;
};

// ============================================================================
// Tracer Sub-module
// ============================================================================
class tracer final
{
  public:
    tracer() = default;

    explicit tracer(
        opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> tracer)
        : tracer_(std::move(tracer))
    {
    }

    ~tracer() = default;

    [[nodiscard]] span_guard scoped_span(std::string_view name,
                                         attribute_list_t attrs = {}) noexcept
    {
        return start_scoped_span(name, attrs);
    }

    [[nodiscard]] span_guard
    start_scoped_span(std::string_view name,
                      attribute_list_t attrs = {}) noexcept
    {
        try
        {
            if(!tracer_)
            {
                return span_guard{trace_span_t{}};
            }
            opentelemetry::trace::StartSpanOptions options;
            auto                                   span =
                tracer_->StartSpan(detail::to_otel_sv(name), attrs, options);
            return span_guard{std::move(span)};
        }
        catch(...)
        {
            return span_guard{trace_span_t{}};
        }
    }

    [[nodiscard]] trace_span_t start_span(std::string_view name,
                                          attribute_list_t attrs = {}) noexcept
    {
        try
        {
            if(!tracer_)
            {
                return trace_span_t{};
            }
            opentelemetry::trace::StartSpanOptions options;
            return tracer_->StartSpan(detail::to_otel_sv(name), attrs, options);
        }
        catch(...)
        {
            return trace_span_t{};
        }
    }

    [[nodiscard]] trace_span_t
    start_span_with_context(std::string_view                       name,
                            const opentelemetry::context::Context &context,
                            attribute_list_t attrs = {}) noexcept
    {
        try
        {
            if(!tracer_)
            {
                return trace_span_t{};
            }
            opentelemetry::trace::StartSpanOptions options;
            options.parent =
                opentelemetry::trace::GetSpan(context)->GetContext();
            return tracer_->StartSpan(detail::to_otel_sv(name), attrs, options);
        }
        catch(...)
        {
            return trace_span_t{};
        }
    }

    void end_span(trace_span_t &span) noexcept
    {
        try
        {
            if(span)
            {
                span->End();
            }
        }
        catch(...)
        {
        }
    }

    std::string curr_trace_id() const noexcept
    {
        try
        {
            if(!tracer_)
                return "";
            auto span = tracer_->GetCurrentSpan();
            if(!span)
                return "";
            return detail::to_std_string(span->GetContext().trace_id());
        }
        catch(...)
        {
            return "";
        }
    }

    std::string curr_span_id() const noexcept
    {
        try
        {
            if(!tracer_)
                return "";
            auto span = tracer_->GetCurrentSpan();
            if(!span)
                return "";
            return detail::to_std_string(span->GetContext().span_id());
        }
        catch(...)
        {
            return "";
        }
    }

    bool force_flush(
        std::chrono::microseconds = std::chrono::microseconds(5000000)) noexcept
    {
        return true;
    }

  private:
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> tracer_;
};

// ============================================================================
// Meter Sub-module
// ============================================================================
class meter final
{
  public:
    meter() = default;

    explicit meter(
        opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Meter> meter)
        : meter_(std::move(meter))
    {
    }

    ~meter() = default;

    template <typename T = uint64_t>
    auto counter(std::string_view name,
                 std::string_view desc = "",
                 std::string_view unit = "") noexcept
    {
        try
        {
            if constexpr(std::is_same_v<T, double>)
            {
                using return_type =
                    decltype(std::declval<opentelemetry::nostd::shared_ptr<
                                 opentelemetry::metrics::Meter>>()
                                 ->CreateDoubleCounter(
                                     std::declval<
                                         opentelemetry::nostd::string_view>(),
                                     std::declval<
                                         opentelemetry::nostd::string_view>(),
                                     std::declval<
                                         opentelemetry::nostd::string_view>()));
                if(!meter_)
                    return return_type{};
                return meter_->CreateDoubleCounter(detail::to_otel_sv(name),
                                                   detail::to_otel_sv(desc),
                                                   detail::to_otel_sv(unit));
            } else
            {
                using return_type =
                    decltype(std::declval<opentelemetry::nostd::shared_ptr<
                                 opentelemetry::metrics::Meter>>()
                                 ->CreateUInt64Counter(
                                     std::declval<
                                         opentelemetry::nostd::string_view>(),
                                     std::declval<
                                         opentelemetry::nostd::string_view>(),
                                     std::declval<
                                         opentelemetry::nostd::string_view>()));
                if(!meter_)
                    return return_type{};
                return meter_->CreateUInt64Counter(detail::to_otel_sv(name),
                                                   detail::to_otel_sv(desc),
                                                   detail::to_otel_sv(unit));
            }
        }
        catch(...)
        {
            using return_type =
                std::decay_t<decltype(counter<T>(name, desc, unit))>;
            return return_type{};
        }
    }

    template <typename T = int64_t>
    auto up_down_counter(std::string_view name,
                         std::string_view desc = "",
                         std::string_view unit = "") noexcept
    {
        try
        {
            if constexpr(std::is_same_v<T, double>)
            {
                using return_type =
                    decltype(std::declval<opentelemetry::nostd::shared_ptr<
                                 opentelemetry::metrics::Meter>>()
                                 ->CreateDoubleUpDownCounter(
                                     std::declval<
                                         opentelemetry::nostd::string_view>(),
                                     std::declval<
                                         opentelemetry::nostd::string_view>(),
                                     std::declval<
                                         opentelemetry::nostd::string_view>()));
                if(!meter_)
                    return return_type{};
                return meter_->CreateDoubleUpDownCounter(
                    detail::to_otel_sv(name),
                    detail::to_otel_sv(desc),
                    detail::to_otel_sv(unit));
            } else
            {
                using return_type =
                    decltype(std::declval<opentelemetry::nostd::shared_ptr<
                                 opentelemetry::metrics::Meter>>()
                                 ->CreateInt64UpDownCounter(
                                     std::declval<
                                         opentelemetry::nostd::string_view>(),
                                     std::declval<
                                         opentelemetry::nostd::string_view>(),
                                     std::declval<
                                         opentelemetry::nostd::string_view>()));
                if(!meter_)
                    return return_type{};
                return meter_->CreateInt64UpDownCounter(
                    detail::to_otel_sv(name),
                    detail::to_otel_sv(desc),
                    detail::to_otel_sv(unit));
            }
        }
        catch(...)
        {
            using return_type =
                std::decay_t<decltype(up_down_counter<T>(name, desc, unit))>;
            return return_type{};
        }
    }

    template <typename T = double>
    auto obs_gauge(std::string_view name,
                   std::string_view desc = "",
                   std::string_view unit = "") noexcept
    {
        try
        {
            if constexpr(std::is_same_v<T, int64_t>)
            {
                using return_type =
                    decltype(std::declval<opentelemetry::nostd::shared_ptr<
                                 opentelemetry::metrics::Meter>>()
                                 ->CreateInt64ObservableGauge(
                                     std::declval<
                                         opentelemetry::nostd::string_view>(),
                                     std::declval<
                                         opentelemetry::nostd::string_view>(),
                                     std::declval<
                                         opentelemetry::nostd::string_view>()));
                if(!meter_)
                    return return_type{};
                return meter_->CreateInt64ObservableGauge(
                    detail::to_otel_sv(name),
                    detail::to_otel_sv(desc),
                    detail::to_otel_sv(unit));
            } else
            {
                using return_type =
                    decltype(std::declval<opentelemetry::nostd::shared_ptr<
                                 opentelemetry::metrics::Meter>>()
                                 ->CreateDoubleObservableGauge(
                                     std::declval<
                                         opentelemetry::nostd::string_view>(),
                                     std::declval<
                                         opentelemetry::nostd::string_view>(),
                                     std::declval<
                                         opentelemetry::nostd::string_view>()));
                if(!meter_)
                    return return_type{};
                return meter_->CreateDoubleObservableGauge(
                    detail::to_otel_sv(name),
                    detail::to_otel_sv(desc),
                    detail::to_otel_sv(unit));
            }
        }
        catch(...)
        {
            using return_type =
                std::decay_t<decltype(obs_gauge<T>(name, desc, unit))>;
            return return_type{};
        }
    }

    template <typename T = double>
    auto histogram(std::string_view name,
                   std::string_view desc = "",
                   std::string_view unit = "") noexcept
    {
        try
        {
            if constexpr(std::is_same_v<T, uint64_t>)
            {
                using return_type =
                    decltype(std::declval<opentelemetry::nostd::shared_ptr<
                                 opentelemetry::metrics::Meter>>()
                                 ->CreateUInt64Histogram(
                                     std::declval<
                                         opentelemetry::nostd::string_view>(),
                                     std::declval<
                                         opentelemetry::nostd::string_view>(),
                                     std::declval<
                                         opentelemetry::nostd::string_view>()));
                if(!meter_)
                    return return_type{};
                return meter_->CreateUInt64Histogram(detail::to_otel_sv(name),
                                                     detail::to_otel_sv(desc),
                                                     detail::to_otel_sv(unit));
            } else
            {
                using return_type =
                    decltype(std::declval<opentelemetry::nostd::shared_ptr<
                                 opentelemetry::metrics::Meter>>()
                                 ->CreateDoubleHistogram(
                                     std::declval<
                                         opentelemetry::nostd::string_view>(),
                                     std::declval<
                                         opentelemetry::nostd::string_view>(),
                                     std::declval<
                                         opentelemetry::nostd::string_view>()));
                if(!meter_)
                    return return_type{};
                return meter_->CreateDoubleHistogram(detail::to_otel_sv(name),
                                                     detail::to_otel_sv(desc),
                                                     detail::to_otel_sv(unit));
            }
        }
        catch(...)
        {
            using return_type =
                std::decay_t<decltype(histogram<T>(name, desc, unit))>;
            return return_type{};
        }
    }

    auto create_u64_counter(std::string_view name,
                            std::string_view desc = "",
                            std::string_view unit = "") noexcept
    {
        return counter<uint64_t>(name, desc, unit);
    }
    auto create_double_counter(std::string_view name,
                               std::string_view desc = "",
                               std::string_view unit = "") noexcept
    {
        return counter<double>(name, desc, unit);
    }
    auto create_double_obs_gauge(std::string_view name,
                                 std::string_view desc = "",
                                 std::string_view unit = "") noexcept
    {
        return obs_gauge<double>(name, desc, unit);
    }
    auto create_u64_histogram(std::string_view name,
                              std::string_view desc = "",
                              std::string_view unit = "") noexcept
    {
        return histogram<uint64_t>(name, desc, unit);
    }

  private:
    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Meter> meter_;
};

// ============================================================================
// Exporters Sub-module
// ============================================================================
namespace exporters
{
class custom_trace_span_exporter
    : public opentelemetry::sdk::trace::SpanExporter
{
  public:
    custom_trace_span_exporter()
        : is_shutdown_(false)
    {
    }
    ~custom_trace_span_exporter() override = default;

    std::unique_ptr<trace_recordable_t> MakeRecordable() noexcept override
    {
        return std::make_unique<trace_span_data_t>();
    }

    export_result_t
    Export(const opentelemetry::nostd::span<std::unique_ptr<trace_recordable_t>>
               &spans) noexcept override
    {
        if(is_shutdown_.load(std::memory_order_acquire))
        {
            return export_result_t::kFailure;
        }
        for(const auto &recordable : spans)
        {
            on_export(recordable);
        }
        return export_result_t::kSuccess;
    }

    bool Shutdown(std::chrono::microseconds =
                      std::chrono::microseconds(0)) noexcept override
    {
        bool expected = false;
        if(is_shutdown_.compare_exchange_strong(expected, true))
        {
            on_shutdown();
        }
        return true;
    }

    bool ForceFlush(std::chrono::microseconds =
                        std::chrono::microseconds(0)) noexcept override
    {
        return true;
    }

    virtual void on_export(
        const std::unique_ptr<trace_recordable_t> &recordable) noexcept = 0;
    virtual void on_shutdown() noexcept {}

  private:
    std::atomic<bool> is_shutdown_;
};
} // namespace exporters

using custom_trace_span_exporter   = exporters::custom_trace_span_exporter;
using custom_trace_span_exporter_t = custom_trace_span_exporter;

// ============================================================================
// Telemetry Runtime Sub-module
// ============================================================================
class runtime final
{
  public:
    runtime() = default;

    explicit runtime(const config &cfg)
    {
        auto resource = detail::create_otel_resource(cfg);
        std::unique_ptr<trace_span_processor_t> span_processor;
        if(cfg.type == exporter_type::ostream)
        {
            auto span_exporter = opentelemetry::exporter::trace::
                OStreamSpanExporterFactory::Create();
            span_processor =
                opentelemetry::sdk::trace::SimpleSpanProcessorFactory::Create(
                    std::move(span_exporter));
        } else if(cfg.type == exporter_type::otlp_http)
        {
            opentelemetry::exporter::otlp::OtlpHttpExporterOptions trace_opts;
            if(!cfg.otlp_endpoint.empty())
                trace_opts.url = cfg.otlp_endpoint;
            trace_opts.console_debug = cfg.debug;
            trace_opts.content_type  = cfg.content_type;
            trace_opts.timeout       = cfg.timeout;
            for(const auto &[k, v] : cfg.headers)
            {
                trace_opts.http_headers.insert({k, v});
            }
            auto span_exporter = std::make_unique<
                opentelemetry::exporter::otlp::OtlpHttpExporter>(trace_opts);

            opentelemetry::sdk::trace::BatchSpanProcessorOptions bsp_opts;
            span_processor =
                opentelemetry::sdk::trace::BatchSpanProcessorFactory::Create(
                    std::move(span_exporter),
                    bsp_opts);
        } else if(cfg.type == exporter_type::otlp_file)
        {
            opentelemetry::exporter::otlp::OtlpFileClientFileSystemOptions
                fs_backend;
            fs_backend.file_pattern = cfg.file_pattern;
            opentelemetry::exporter::otlp::OtlpFileExporterOptions trace_opts;
            trace_opts.backend_options = fs_backend;

            auto span_exporter =
                opentelemetry::exporter::otlp::OtlpFileExporterFactory::Create(
                    trace_opts);
            opentelemetry::sdk::trace::BatchSpanProcessorOptions bsp_opts;
            span_processor =
                opentelemetry::sdk::trace::BatchSpanProcessorFactory::Create(
                    std::move(span_exporter),
                    bsp_opts);
        }

        if(span_processor)
        {
            auto sdk_trace_provider =
                opentelemetry::sdk::trace::TracerProviderFactory::Create(
                    std::move(span_processor),
                    resource);
            trace_provider_ = opentelemetry::nostd::shared_ptr<
                opentelemetry::trace::TracerProvider>(
                sdk_trace_provider.release());
            opentelemetry::trace::Provider::SetTracerProvider(trace_provider_);
        }

        std::unique_ptr<opentelemetry::sdk::metrics::PushMetricExporter>
            metric_exporter;
        if(cfg.type == exporter_type::ostream)
        {
            metric_exporter = opentelemetry::exporter::metrics::
                OStreamMetricExporterFactory::Create();
        } else if(cfg.type == exporter_type::otlp_http)
        {
            opentelemetry::exporter::otlp::OtlpHttpMetricExporterOptions
                metric_opts;
            if(!cfg.otlp_endpoint.empty())
                metric_opts.url = cfg.otlp_endpoint;
            metric_opts.console_debug = cfg.debug;
            metric_opts.content_type  = cfg.content_type;
            metric_opts.timeout       = cfg.timeout;
            for(const auto &[k, v] : cfg.headers)
            {
                metric_opts.http_headers.insert({k, v});
            }
            metric_exporter = opentelemetry::exporter::otlp::
                OtlpHttpMetricExporterFactory::Create(metric_opts);
        } else if(cfg.type == exporter_type::otlp_file)
        {
            opentelemetry::exporter::otlp::OtlpFileMetricExporterOptions
                metric_opts;
            opentelemetry::exporter::otlp::OtlpFileClientFileSystemOptions
                fs_backend;
            fs_backend.file_pattern     = cfg.file_pattern;
            metric_opts.backend_options = fs_backend;
            metric_exporter             = opentelemetry::exporter::otlp::
                OtlpFileMetricExporterFactory::Create(metric_opts);
        }

        if(metric_exporter)
        {
            opentelemetry::sdk::metrics::PeriodicExportingMetricReaderOptions
                reader_options;
            reader_options.export_interval_millis =
                std::chrono::milliseconds(500);
            reader_options.export_timeout_millis =
                std::chrono::milliseconds(100);

            auto reader = opentelemetry::sdk::metrics::
                PeriodicExportingMetricReaderFactory::Create(
                    std::move(metric_exporter),
                    reader_options);

            auto views =
                std::make_unique<opentelemetry::sdk::metrics::ViewRegistry>();
            auto context =
                opentelemetry::sdk::metrics::MeterContextFactory::Create(
                    std::move(views),
                    resource);
            context->AddMetricReader(std::move(reader));

            auto sdk_meter_provider =
                opentelemetry::sdk::metrics::MeterProviderFactory::Create(
                    std::move(context));
            meter_provider_ = opentelemetry::nostd::shared_ptr<
                opentelemetry::metrics::MeterProvider>(
                sdk_meter_provider.release());
            opentelemetry::metrics::Provider::SetMeterProvider(meter_provider_);
        }
    }

    runtime(
        opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider>
            trace_provider,
        opentelemetry::nostd::shared_ptr<opentelemetry::metrics::MeterProvider>
            meter_provider = {})
        : trace_provider_(std::move(trace_provider))
        , meter_provider_(std::move(meter_provider))
    {
        if(trace_provider_)
        {
            opentelemetry::trace::Provider::SetTracerProvider(trace_provider_);
        }
        if(meter_provider_)
        {
            opentelemetry::metrics::Provider::SetMeterProvider(meter_provider_);
        }
    }

    ~runtime() noexcept { shutdown(); }

    runtime(const runtime &)            = delete;
    runtime &operator=(const runtime &) = delete;

    runtime(runtime &&other) noexcept
        : trace_provider_(std::move(other.trace_provider_))
        , meter_provider_(std::move(other.meter_provider_))
    {
    }

    runtime &operator=(runtime &&other) noexcept
    {
        if(this != &other)
        {
            shutdown();
            trace_provider_ = std::move(other.trace_provider_);
            meter_provider_ = std::move(other.meter_provider_);
        }
        return *this;
    }

    [[nodiscard]] tracer tracer_handle(std::string_view name,
                                       std::string_view version = "") noexcept
    {
        return get_tracer(name, version);
    }

    [[nodiscard]] tracer get_tracer(std::string_view name,
                                    std::string_view version = "") noexcept
    {
        try
        {
            if(!trace_provider_)
            {
                return tracer{};
            }
            return tracer{
                trace_provider_->GetTracer(detail::to_otel_sv(name),
                                           detail::to_otel_sv(version))};
        }
        catch(...)
        {
            return tracer{};
        }
    }

    [[nodiscard]] meter meter_handle(std::string_view name,
                                     std::string_view version = "",
                                     std::string_view schema  = "") noexcept
    {
        return get_meter(name, version, schema);
    }

    [[nodiscard]] meter get_meter(std::string_view name,
                                  std::string_view version = "",
                                  std::string_view schema  = "") noexcept
    {
        try
        {
            if(!meter_provider_)
            {
                return meter{};
            }
            return meter{meter_provider_->GetMeter(detail::to_otel_sv(name),
                                                   detail::to_otel_sv(version),
                                                   detail::to_otel_sv(schema))};
        }
        catch(...)
        {
            return meter{};
        }
    }

    bool force_flush(std::chrono::microseconds timeout =
                         std::chrono::microseconds(5000000)) noexcept
    {
        try
        {
            bool ok = true;
            if(trace_provider_)
            {
                if(auto sdk = dynamic_cast<
                       opentelemetry::sdk::trace::TracerProvider *>(
                       trace_provider_.get()))
                {
                    ok &= sdk->ForceFlush(timeout);
                }
            }
            if(meter_provider_)
            {
                if(auto sdk = dynamic_cast<
                       opentelemetry::sdk::metrics::MeterProvider *>(
                       meter_provider_.get()))
                {
                    ok &= sdk->ForceFlush(timeout);
                }
            }
            return ok;
        }
        catch(...)
        {
            return false;
        }
    }

    bool shutdown(std::chrono::microseconds timeout =
                      std::chrono::microseconds(5000000)) noexcept
    {
        try
        {
            bool ok = true;
            if(trace_provider_)
            {
                if(auto sdk = dynamic_cast<
                       opentelemetry::sdk::trace::TracerProvider *>(
                       trace_provider_.get()))
                {
                    ok &= sdk->Shutdown(timeout);
                }
                std::shared_ptr<opentelemetry::trace::TracerProvider> none;
                opentelemetry::trace::Provider::SetTracerProvider(none);
                trace_provider_ = nullptr;
            }

            if(meter_provider_)
            {
                if(auto sdk = dynamic_cast<
                       opentelemetry::sdk::metrics::MeterProvider *>(
                       meter_provider_.get()))
                {
                    ok &= sdk->Shutdown(timeout);
                }
                std::shared_ptr<opentelemetry::metrics::MeterProvider> none;
                opentelemetry::metrics::Provider::SetMeterProvider(none);
                meter_provider_ = nullptr;
            }
            return ok;
        }
        catch(...)
        {
            return false;
        }
    }

  private:
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider>
        trace_provider_;
    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::MeterProvider>
        meter_provider_;
};

// ============================================================================
// Backward-compatible Helper Factories
// ============================================================================
inline std::unique_ptr<trace_span_processor_t> make_simple_trace_span_processor(
    std::unique_ptr<trace_span_exporter_t> exporter)
{
    return opentelemetry::sdk::trace::SimpleSpanProcessorFactory::Create(
        std::move(exporter));
}

inline std::unique_ptr<trace_span_processor_t> make_batch_trace_span_processor(
    std::unique_ptr<trace_span_exporter_t> exporter,
    std::size_t                            max_queue_size = 2048,
    std::chrono::milliseconds schedule_delay = std::chrono::milliseconds(2000),
    std::size_t               max_batch_size = 512)
{
    opentelemetry::sdk::trace::BatchSpanProcessorOptions opts;
    opts.max_queue_size        = max_queue_size;
    opts.schedule_delay_millis = schedule_delay;
    opts.max_export_batch_size = max_batch_size;
    return opentelemetry::sdk::trace::BatchSpanProcessorFactory::Create(
        std::move(exporter),
        opts);
}

inline runtime make_ostream_runtime(std::string_view service_name = "default")
{
    config cfg;
    cfg.service_name = std::string(service_name);
    cfg.type         = exporter_type::ostream;
    return runtime(cfg);
}

inline runtime make_otlp_http_runtime(std::string_view endpoint,
                                      bool             debug = false)
{
    config cfg;
    cfg.type          = exporter_type::otlp_http;
    cfg.otlp_endpoint = std::string(endpoint);
    cfg.debug         = debug;
    return runtime(cfg);
}

inline runtime make_otlp_file_runtime(std::string_view file_pattern)
{
    config cfg;
    cfg.type         = exporter_type::otlp_file;
    cfg.file_pattern = std::string(file_pattern);
    return runtime(cfg);
}

inline tracer
make_custom_tracer(std::string_view                        name,
                   std::unique_ptr<trace_span_processor_t> processor)
{
    try
    {
        auto sdk_trace_provider =
            opentelemetry::sdk::trace::TracerProviderFactory::Create(
                std::move(processor));
        auto provider = opentelemetry::nostd::shared_ptr<
            opentelemetry::trace::TracerProvider>(sdk_trace_provider.release());
        if(!provider)
            return tracer{};
        return tracer{provider->GetTracer(detail::to_otel_sv(name))};
    }
    catch(...)
    {
        return tracer{};
    }
}

inline tracer make_ostream_tracer(std::string_view name)
{
    static auto rt = make_ostream_runtime();
    return rt.get_tracer(name);
}

inline tracer make_otlp_http_tracer(std::string_view name,
                                    std::string_view endpoint,
                                    bool             debug = false)
{
    static auto rt = make_otlp_http_runtime(endpoint, debug);
    return rt.get_tracer(name);
}

inline tracer make_otlp_file_tracer(std::string_view name,
                                    std::string_view file_pattern)
{
    static auto rt = make_otlp_file_runtime(file_pattern);
    return rt.get_tracer(name);
}

inline meter make_ostream_meter(std::string_view name,
                                std::string_view version = "",
                                std::string_view schema  = "")
{
    static auto rt = make_ostream_runtime();
    return rt.get_meter(name, version, schema);
}

inline meter make_otlp_http_meter(std::string_view name,
                                  std::string_view version,
                                  std::string_view schema,
                                  std::string_view endpoint,
                                  bool             debug = false)
{
    static auto rt = make_otlp_http_runtime(endpoint, debug);
    return rt.get_meter(name, version, schema);
}

inline meter make_otlp_file_meter(std::string_view name,
                                  std::string_view version,
                                  std::string_view schema,
                                  std::string_view file_pattern)
{
    static auto rt = make_otlp_file_runtime(file_pattern);
    return rt.get_meter(name, version, schema);
}

inline void clean_up_metrics()
{
    std::shared_ptr<opentelemetry::metrics::MeterProvider> none;
    opentelemetry::metrics::Provider::SetMeterProvider(none);
}

} // namespace hj::telemetry

#endif // TELEMETRY_HPP