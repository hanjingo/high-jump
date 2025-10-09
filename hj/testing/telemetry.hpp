/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
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

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <variant>

#include <opentelemetry/common/attribute_value.h>

#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/nostd/string_view.h>
#include <opentelemetry/nostd/variant.h>
#include <opentelemetry/sdk/common/exporter_utils.h>
#include <opentelemetry/sdk/resource/resource.h>

#include <opentelemetry/sdk/trace/span_data.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>
#include <opentelemetry/sdk/trace/simple_processor.h>
#include <opentelemetry/sdk/trace/simple_processor_factory.h>
#include <opentelemetry/sdk/trace/exporter.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/tracer.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/span_context.h>
#include <opentelemetry/trace/scope.h>

#include <opentelemetry/metrics/provider.h>
#include <opentelemetry/metrics/meter_provider.h>
#include <opentelemetry/sdk/metrics/aggregation/aggregation_config.h>
#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h>
#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_options.h>
#include <opentelemetry/sdk/metrics/instruments.h>
#include <opentelemetry/sdk/metrics/meter_context.h>
#include <opentelemetry/sdk/metrics/meter_context_factory.h>
#include <opentelemetry/sdk/metrics/meter_provider.h>
#include <opentelemetry/sdk/metrics/meter_provider_factory.h>
#include <opentelemetry/sdk/metrics/metric_reader.h>
#include <opentelemetry/sdk/metrics/provider.h>
#include <opentelemetry/sdk/metrics/push_metric_exporter.h>
#include <opentelemetry/sdk/metrics/view/instrument_selector.h>
#include <opentelemetry/sdk/metrics/view/instrument_selector_factory.h>
#include <opentelemetry/sdk/metrics/view/meter_selector.h>
#include <opentelemetry/sdk/metrics/view/meter_selector_factory.h>
#include <opentelemetry/sdk/metrics/view/view.h>
#include <opentelemetry/sdk/metrics/view/view_factory.h>

#include <opentelemetry/exporters/ostream/span_exporter_factory.h>
#include <opentelemetry/exporters/ostream/metric_exporter_factory.h>

#include <opentelemetry/exporters/otlp/otlp_http_exporter.h>
#include <opentelemetry/exporters/otlp/otlp_http_metric_exporter_options.h>
#include <opentelemetry/exporters/otlp/otlp_http_metric_exporter_factory.h>

#include <opentelemetry/exporters/otlp/otlp_file_exporter.h>
#include <opentelemetry/exporters/otlp/otlp_file_client_options.h>
#include <opentelemetry/exporters/otlp/otlp_file_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_file_exporter_options.h>
#include <opentelemetry/exporters/otlp/otlp_file_metric_exporter_factory.h>

namespace hj
{

namespace telemetry
{
using tracer_provider_t = opentelemetry::trace::TracerProvider;
using tracer_base_t =
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer>;
using trace_id_t = opentelemetry::trace::TraceId;
using trace_span_t =
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span>;
using trace_span_processor_t = opentelemetry::sdk::trace::SpanProcessor;
using trace_span_data_t      = opentelemetry::sdk::trace::SpanData;
using trace_span_id_t        = opentelemetry::trace::SpanId;
using trace_span_exporter_t  = opentelemetry::sdk::trace::SpanExporter;
using trace_recordable_t     = opentelemetry::sdk::trace::Recordable;

using meter_provider_t = opentelemetry::metrics::MeterProvider;
using meter_base_t =
    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Meter>;
using meter_context_t        = opentelemetry::v1::sdk::metrics::MeterContext;
using metric_view_t          = opentelemetry::sdk::metrics::View;
using metric_view_registry_t = opentelemetry::sdk::metrics::ViewRegistry;
using metric_instrument_selector_t =
    opentelemetry::sdk::metrics::InstrumentSelector;
using metric_meter_selector_t = opentelemetry::sdk::metrics::MeterSelector;
using metric_reader_t         = opentelemetry::sdk::metrics::MetricReader;
using metric_u64_counter_t =
    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Counter<uint64_t>>;
using metric_double_counter_t =
    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Counter<double>>;
using metric_obs_instrument_t = opentelemetry::nostd::shared_ptr<
    opentelemetry::metrics::ObservableInstrument>;
using metric_u64_histogram_t = opentelemetry::nostd::unique_ptr<
    opentelemetry::metrics::Histogram<uint64_t>>;
using metric_double_histogram_t =
    opentelemetry::nostd::unique_ptr<opentelemetry::metrics::Histogram<double>>;

using metric_periodic_reader_options_t =
    opentelemetry::sdk::metrics::PeriodicExportingMetricReaderOptions;
using metric_aggregation_config_t =
    opentelemetry::sdk::metrics::AggregationConfig;
using metric_histogram_aggregation_config_t =
    opentelemetry::sdk::metrics::HistogramAggregationConfig;

using export_result_t = opentelemetry::sdk::common::ExportResult;
using string_view_t   = opentelemetry::nostd::string_view;
using resource_t      = opentelemetry::sdk::resource::Resource;
using sec_t           = std::chrono::microseconds;
using ms_t            = std::chrono::milliseconds;

using otlp_http_options_t =
    opentelemetry::exporter::otlp::OtlpHttpExporterOptions;
using otlp_http_metric_exporter_options_t =
    opentelemetry::exporter::otlp::OtlpHttpMetricExporterOptions;
using otlp_file_options_t =
    opentelemetry::exporter::otlp::OtlpFileExporterOptions;
using otlp_file_metric_exporter_options_t =
    opentelemetry::exporter::otlp::OtlpFileMetricExporterOptions;
using otlp_file_cli_fs_options_t =
    opentelemetry::exporter::otlp::OtlpFileClientFileSystemOptions;

using instrument_type = opentelemetry::sdk::metrics::InstrumentType;
using http_request_content_type =
    opentelemetry::exporter::otlp::HttpRequestContentType;
using aggregation_type = opentelemetry::sdk::metrics::AggregationType;

struct custom_exporter
{
};
struct ostream_exporter
{
};
struct otlp_http_exporter
{
};
struct otlp_file_exporter
{
};

namespace detail
{
static std::string to_std_string(const trace_id_t &id)
{
    std::string s;
    if(id.IsValid() && id.kSize == 16)
    {
        s.resize(2 * id.kSize);
        id.ToLowerBase16(s);
    }
    return s;
}

static std::string to_std_string(const trace_span_id_t &id)
{
    std::string s;
    if(id.IsValid() && id.kSize == 8)
    {
        s.resize(2 * id.kSize);
        id.ToLowerBase16(s);
    }
    return s;
}

static std::string to_std_string(const string_view_t &sv)
{
    return std::string{sv.data(), sv.size()};
}

static std::string get_value_str(const trace_span_data_t &span,
                                 const std::string       &target)
{
    for(const auto &[key, value] : span.GetAttributes())
    {
        if(key != target)
            continue;

        auto value_str = [](const auto &v) -> std::string {
            using T = std::decay_t<decltype(v)>;
            if constexpr(std::is_same_v<T, std::string>)
                return v;
            else if constexpr(std::is_arithmetic_v<T>)
                return std::to_string(v);
            else if constexpr(std::is_same_v<T, bool>)
                return v ? "true" : "false";
            else if constexpr(std::is_same_v<T, string_view_t>)
                return std::string{v.data(), v.size()};
            else
                return "[unsupported type]";
        };
        return opentelemetry::nostd::visit(value_str, value);
    }
    return "";
}

static std::unique_ptr<metric_instrument_selector_t>
make_instrument_selector(instrument_type    type,
                         const std::string &name,
                         const std::string &unit) noexcept
{
    return opentelemetry::sdk::metrics::InstrumentSelectorFactory::Create(type,
                                                                          name,
                                                                          unit);
}

static std::unique_ptr<metric_meter_selector_t>
make_meter_selector(const std::string &name,
                    const std::string &version,
                    const std::string &scheme) noexcept
{
    return opentelemetry::sdk::metrics::MeterSelectorFactory::Create(name,
                                                                     version,
                                                                     scheme);
}

static std::unique_ptr<opentelemetry::sdk::metrics::View>
make_view(const std::string &name,
          const std::string &desc,
          const std::string &unit,
          aggregation_type   type) noexcept
{
    return opentelemetry::sdk::metrics::ViewFactory::Create(name,
                                                            desc,
                                                            unit,
                                                            type);
}

static std::unique_ptr<opentelemetry::sdk::metrics::View> make_view(
    const std::string                           &name,
    const std::string                           &desc,
    const std::string                           &unit,
    aggregation_type                             type,
    std::shared_ptr<metric_aggregation_config_t> aggregation_config) noexcept
{
    return opentelemetry::sdk::metrics::ViewFactory::Create(name,
                                                            desc,
                                                            unit,
                                                            type,
                                                            aggregation_config);
}

static std::unique_ptr<metric_histogram_aggregation_config_t>
make_histogram_aggregation_config(const std::vector<double> &boundaries,
                                  bool record_min_max = true) noexcept
{
    auto config = std::unique_ptr<metric_histogram_aggregation_config_t>(
        new metric_histogram_aggregation_config_t);
    config->boundaries_     = boundaries;
    config->record_min_max_ = record_min_max;
    return config;
}

static std::unique_ptr<metric_view_registry_t>
make_default_metric_view_registry(const std::string &name,
                                  const std::string &version,
                                  const std::string &scheme) noexcept
{
    auto views = std::make_unique<metric_view_registry_t>();

    // counter view
    auto instrument_selector_ct =
        make_instrument_selector(instrument_type::kCounter, "", "");
    auto meter_selector = make_meter_selector(name, version, scheme);
    auto sum_view       = make_view(name, "", "", aggregation_type::kSum);
    views->AddView(std::move(instrument_selector_ct),
                   std::move(meter_selector),
                   std::move(sum_view));

    // observable counter view
    auto obs_instrument_selector =
        make_instrument_selector(instrument_type::kObservableCounter, "", "");
    auto obs_meter_selector = make_meter_selector(name, version, scheme);
    auto obs_sum_view       = make_view(name, "", "", aggregation_type::kSum);
    views->AddView(std::move(obs_instrument_selector),
                   std::move(obs_meter_selector),
                   std::move(obs_sum_view));

    // histogram view
    auto histogram_instrument_selector =
        make_instrument_selector(instrument_type::kHistogram, "", "");
    auto histogram_meter_selector = make_meter_selector(name, version, scheme);
    auto histogram_aggregation_config =
        make_histogram_aggregation_config(std::vector<double>{0.0,
                                                              50.0,
                                                              100.0,
                                                              250.0,
                                                              500.0,
                                                              750.0,
                                                              1000.0,
                                                              2500.0,
                                                              5000.0,
                                                              10000.0,
                                                              20000.0});
    std::shared_ptr<metric_aggregation_config_t> aggregation_config(
        std::move(histogram_aggregation_config));
    auto histogram_view = make_view(name,
                                    "",
                                    "",
                                    aggregation_type::kHistogram,
                                    aggregation_config);
    views->AddView(std::move(histogram_instrument_selector),
                   std::move(histogram_meter_selector),
                   std::move(histogram_view));

    return views;
}

// self defined trace_span_exporter
class custom_trace_span_exporter
    : public opentelemetry::sdk::trace::SpanExporter
{
  public:
    custom_trace_span_exporter() {}

    std::unique_ptr<trace_recordable_t> MakeRecordable() noexcept override
    {
        return std::unique_ptr<opentelemetry::sdk::trace::Recordable>(
            new trace_span_data_t());
    }

    export_result_t
    Export(const opentelemetry::nostd::span<std::unique_ptr<trace_recordable_t>>
               &spans) noexcept override
    {
        for(const auto &recordable : spans)
        {
            on_export(recordable);
        }
        return export_result_t::kSuccess;
    }

    bool Shutdown(sec_t timeout = sec_t(0)) noexcept override { return true; }

    bool ForceFlush(sec_t timeout = sec_t(0)) noexcept override { return true; }

    virtual void
    on_export(const std::unique_ptr<trace_recordable_t> &recordable) noexcept
    {
        // Custom export logic here
        // Implement it!
    }
};
}

using custom_trace_span_exporter_t = detail::custom_trace_span_exporter;

template <typename T>
static std::string to_std_string(const T &value)
{
    return detail::to_std_string(value);
}

// ----------------------------------- tracer ------------------------------
template <typename T>
class tracer
{
  public:
    tracer()                        = delete;
    tracer(const std::string &name) = delete;
    explicit tracer(const std::string                        &name,
                    std::unique_ptr<trace_span_processor_t> &&processor)
    {
        auto provider = opentelemetry::nostd::shared_ptr<tracer_provider_t>(
            new opentelemetry::sdk::trace::TracerProvider(
                std::move(processor)));
        opentelemetry::trace::Provider::SetTracerProvider(provider);
        _tracer = provider->GetTracer(name);
    }
    ~tracer() {}

    trace_span_t
    start_span(const std::string                        &name,
               const std::map<std::string, std::string> &attrs = {})
    {
        auto span = _tracer->StartSpan(name);
        for(const auto &kv : attrs)
            span->SetAttribute(kv.first, kv.second);

        return span;
    }

    void end_span(trace_span_t &span)
    {
        if(span)
            span->End();
    }

    void trace(const std::string                        &name,
               const std::map<std::string, std::string> &attrs = {})
    {
        auto span = start_span(name, attrs);
        end_span(span);
    }

    void add_event(trace_span_t                             &span,
                   const std::string                        &event,
                   const std::map<std::string, std::string> &attrs = {})
    {
        if(!span)
            return;

        span->AddEvent(event);
        for(const auto &kv : attrs)
            span->SetAttribute(kv.first, kv.second);
    }

    std::string curr_trace_id()
    {
        auto        span = _tracer->GetCurrentSpan();
        std::string hex;
        span->GetContext().trace_id().ToLowerBase16(hex);
        return hex;
    }

    std::string curr_span_id()
    {
        auto        span = _tracer->GetCurrentSpan();
        std::string hex;
        span->GetContext().span_id().ToLowerBase16(hex);
        return hex;
    }

  private:
    tracer_base_t _tracer;
    bool          _is_custom;
};

// ----------------------------------- meter ----------------------------
template <typename T>
class meter
{
  public:
    meter() = delete;
    meter(const std::string                      &name,
          const std::string                      &version,
          const std::string                      &scheme,
          std::unique_ptr<metric_reader_t>        reader,
          std::unique_ptr<metric_view_registry_t> views)
    {
        auto context = opentelemetry::sdk::metrics::MeterContextFactory::Create(
            std::move(views));
        context->AddMetricReader(std::move(reader));

        auto provider =
            opentelemetry::sdk::metrics::MeterProviderFactory::Create(
                std::move(context));
        std::shared_ptr<hj::telemetry::meter_provider_t> api_provider(
            std::move(provider));
        opentelemetry::sdk::metrics::Provider::SetMeterProvider(api_provider);

        _meter = opentelemetry::metrics::Provider::GetMeterProvider()->GetMeter(
            name,
            version,
            scheme);
    }
    ~meter() {}

    metric_u64_counter_t
    create_u64_counter(const std::string &name,
                       const std::string &desc = "") noexcept
    {
        return _meter->CreateUInt64Counter(name, desc);
    }

    metric_double_counter_t
    create_double_counter(const std::string &name,
                          const std::string &desc = "",
                          const std::string &unit = "") noexcept
    {
        return _meter->CreateDoubleCounter(name, desc, unit);
    }

    metric_obs_instrument_t
    create_i64_obs_counter(const std::string &name,
                           const std::string &desc = "",
                           const std::string &unit = "") noexcept
    {
        return _meter->CreateInt64ObservableCounter(name, desc, unit);
    }

    metric_obs_instrument_t
    create_double_obs_counter(const std::string &name,
                              const std::string &desc = "",
                              const std::string &unit = "") noexcept
    {
        return _meter->CreateDoubleObservableCounter(name, desc, unit);
    }

    metric_obs_instrument_t
    create_i64_obs_gauge(const std::string &name,
                         const std::string &desc = "",
                         const std::string &unit = "") noexcept
    {
        return _meter->CreateInt64ObservableGauge(name, desc, unit);
    }

    metric_obs_instrument_t
    create_double_obs_gauge(const std::string &name,
                            const std::string &desc = "") noexcept
    {
        return _meter->CreateDoubleObservableGauge(name, desc);
    }

    metric_u64_histogram_t
    create_u64_histogram(const std::string &name,
                         const std::string &desc = "",
                         const std::string &unit = "") noexcept
    {
        return _meter->CreateUInt64Histogram(name, desc, unit);
    }


    metric_double_histogram_t
    create_double_histogram(const std::string &name,
                            const std::string &desc = "",
                            const std::string &unit = "") noexcept
    {
        return _meter->CreateDoubleHistogram(name, desc, unit);
    }

  private:
    meter_base_t _meter;
};

// ------------------------------ global api -------------------------------
// tracer API
static std::unique_ptr<trace_span_processor_t> make_simple_trace_span_processor(
    std::unique_ptr<trace_span_exporter_t> &&exporter)
{
    return std::make_unique<opentelemetry::sdk::trace::SimpleSpanProcessor>(
        std::move(exporter));
}

static void cleanup_tracer()
{
    std::shared_ptr<opentelemetry::trace::TracerProvider> none;
    opentelemetry::trace::Provider::SetTracerProvider(none);
}

static tracer<custom_exporter>
make_custom_tracer(const std::string                        &name,
                   std::unique_ptr<trace_span_processor_t> &&processor)
{
    return tracer<custom_exporter>(name, std::move(processor));
}

static tracer<ostream_exporter>
make_ostream_tracer(const std::string                        &name,
                    std::unique_ptr<trace_span_processor_t> &&processor)
{
    return tracer<ostream_exporter>(name, std::move(processor));
}

static tracer<ostream_exporter> make_ostream_tracer(const std::string &name)
{
    auto exporter =
        opentelemetry::exporter::trace::OStreamSpanExporterFactory::Create();
    auto processor = make_simple_trace_span_processor(std::move(exporter));
    return make_ostream_tracer(name, std::move(processor));
}

static tracer<otlp_http_exporter>
make_otlp_http_tracer(const std::string                        &name,
                      std::unique_ptr<trace_span_processor_t> &&processor)
{
    return tracer<otlp_http_exporter>(name, std::move(processor));
}

static tracer<otlp_http_exporter>
make_otlp_http_tracer(const std::string         &name,
                      const otlp_http_options_t &options)
{
    auto exporter =
        std::make_unique<opentelemetry::exporter::otlp::OtlpHttpExporter>(
            options);
    auto processor = make_simple_trace_span_processor(std::move(exporter));
    return make_otlp_http_tracer(name, std::move(processor));
}

static tracer<otlp_http_exporter>
make_otlp_http_tracer(const std::string              &name,
                      const std::string              &endpoint,
                      const bool                      debug,
                      const http_request_content_type content_type)
{
    opentelemetry::exporter::otlp::OtlpHttpExporterOptions options;
    if(!endpoint.empty())
        options.url = endpoint;

    options.console_debug = debug;
    options.content_type  = content_type;
    return make_otlp_http_tracer(name, options);
}

static tracer<otlp_file_exporter>
make_otlp_file_tracer(const std::string &name, const otlp_file_options_t &opts)
{
    auto exporter =
        opentelemetry::exporter::otlp::OtlpFileExporterFactory::Create(opts);
    auto processor =
        opentelemetry::sdk::trace::SimpleSpanProcessorFactory::Create(
            std::move(exporter));
    return tracer<otlp_file_exporter>(name, std::move(processor));
}

static tracer<otlp_file_exporter>
make_otlp_file_tracer(const std::string &name, const std::string &file_pattern)
{
    opentelemetry::exporter::otlp::OtlpFileClientFileSystemOptions fs_backend;
    fs_backend.file_pattern = file_pattern;

    opentelemetry::exporter::otlp::OtlpFileExporterOptions opts;
    opts.backend_options = fs_backend;

    return make_otlp_file_tracer(name, opts);
}

// meter API
static void clean_up_metrics()
{
    std::shared_ptr<opentelemetry::metrics::MeterProvider> none;
    opentelemetry::sdk::metrics::Provider::SetMeterProvider(none);
}

static std::unique_ptr<metric_view_registry_t>
make_default_metric_view_registry(const std::string &name,
                                  const std::string &version,
                                  const std::string &scheme)
{
    return detail::make_default_metric_view_registry(name, version, scheme);
}

static meter<ostream_exporter> make_ostream_meter(
    const std::string                                       &name,
    const std::string                                       &version,
    const std::string                                       &scheme,
    std::unique_ptr<hj::telemetry::metric_reader_t>        &&reader,
    std::unique_ptr<hj::telemetry::metric_view_registry_t> &&views)
{
    return meter<ostream_exporter>(name,
                                   version,
                                   scheme,
                                   std::move(reader),
                                   std::move(views));
}

static meter<ostream_exporter> make_ostream_meter(const std::string &name,
                                                  const std::string &version,
                                                  const std::string &scheme,
                                                  const std::size_t interval_ms,
                                                  const std::size_t timeout_ms)
{
    auto exporter = opentelemetry::exporter::metrics::
        OStreamMetricExporterFactory::Create();

    metric_periodic_reader_options_t options;
    options.export_interval_millis = ms_t(interval_ms);
    options.export_timeout_millis  = ms_t(timeout_ms);

    auto reader =
        opentelemetry::sdk::metrics::PeriodicExportingMetricReaderFactory::
            Create(std::move(exporter), options);

    auto views = make_default_metric_view_registry(name, version, scheme);

    return make_ostream_meter(name,
                              version,
                              scheme,
                              std::move(reader),
                              std::move(views));
}

static meter<otlp_http_exporter>
make_otlp_http_meter(const std::string              &name,
                     const std::string              &version,
                     const std::string              &scheme,
                     const std::string              &endpoint,
                     const http_request_content_type content_type,
                     const std::size_t               interval_ms,
                     const std::size_t               timeout_ms,
                     const bool                      debug)
{
    // Create the OTLP HTTP metric exporter
    otlp_http_metric_exporter_options_t exporter_options;
    exporter_options.url           = endpoint;
    exporter_options.console_debug = debug;
    exporter_options.content_type  = content_type;
    auto exporter =
        opentelemetry::exporter::otlp::OtlpHttpMetricExporterFactory::Create(
            exporter_options);

    // Create the periodic reader
    metric_periodic_reader_options_t reader_options;
    reader_options.export_interval_millis =
        std::chrono::milliseconds(interval_ms);
    reader_options.export_timeout_millis =
        std::chrono::milliseconds(timeout_ms);
    auto reader =
        opentelemetry::sdk::metrics::PeriodicExportingMetricReaderFactory::
            Create(std::move(exporter), reader_options);

    auto views = make_default_metric_view_registry(name, version, scheme);

    return meter<otlp_http_exporter>(name,
                                     version,
                                     scheme,
                                     std::move(reader),
                                     std::move(views));
}

static meter<otlp_file_exporter>
make_otlp_file_meter(const std::string &name,
                     const std::string &version,
                     const std::string &scheme,
                     const std::string &file_pattern,
                     const std::size_t  interval_ms,
                     const std::size_t  timeout_ms,
                     const bool         debug)
{
    otlp_file_cli_fs_options_t fs_backend;
    fs_backend.file_pattern = file_pattern;

    otlp_file_metric_exporter_options_t exporter_options;
    exporter_options.console_debug   = debug;
    exporter_options.backend_options = fs_backend;
    auto exporter =
        opentelemetry::exporter::otlp::OtlpFileMetricExporterFactory::Create(
            exporter_options);

    metric_periodic_reader_options_t reader_options;
    reader_options.export_interval_millis =
        std::chrono::milliseconds(interval_ms);
    reader_options.export_timeout_millis =
        std::chrono::milliseconds(timeout_ms);

    auto reader =
        opentelemetry::sdk::metrics::PeriodicExportingMetricReaderFactory::
            Create(std::move(exporter), reader_options);

    auto views = make_default_metric_view_registry(name, version, scheme);
    return meter<otlp_file_exporter>(name,
                                     version,
                                     scheme,
                                     std::move(reader),
                                     std::move(views));
}

} // namespace telemetry
} // namespace hj

#endif // TELEMETRY_HPP