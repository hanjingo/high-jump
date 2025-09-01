#include <gtest/gtest.h>
#include <libcpp/testing/telemetry.hpp>
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

static double _get_cpu_usage() 
{
#if defined(_WIN32) || defined(_WIN64)
	FILETIME idleTime, kernelTime, userTime;
	if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) 
    {
		ULARGE_INTEGER k, u;
		k.LowPart = kernelTime.dwLowDateTime;
		k.HighPart = kernelTime.dwHighDateTime;
		u.LowPart = userTime.dwLowDateTime;
		u.HighPart = userTime.dwHighDateTime;
		return (double)(k.QuadPart + u.QuadPart) / 10000000.0;
	}
	return 0.0;
#else
	FILE* fp = fopen("/proc/stat", "r");
	if (!fp) 
        return 0.0;

	char buf[256];
	if (!fgets(buf, sizeof(buf), fp)) { fclose(fp); return 0.0; }
	fclose(fp);
	unsigned long user, nice, system, idle;
	sscanf(buf, "cpu %lu %lu %lu %lu", &user, &nice, &system, &idle);
	return (double)(user + nice + system) / (user + nice + system + idle);
#endif
}

static double _get_mem_usage() 
{
#if defined(_WIN32) || defined(_WIN64)
	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
		return pmc.WorkingSetSize / 1024.0 / 1024.0;
	return 0.0;
#else
	FILE* fp = fopen("/proc/self/statm", "r");
	if (!fp) 
        return 0.0;
	long rss = 0;
	if (fscanf(fp, "%*s %ld", &rss) != 1) { fclose(fp); return 0.0; }
	fclose(fp);
	long page_size = sysconf(_SC_PAGESIZE);
	return (double)rss * page_size / 1024.0 / 1024.0;
#endif
}

TEST(TelemetryTest, OStreamExportDefault)
{
	auto tracer = libcpp::telemetry::make_ostream_tracer("ostream1");
	for (int i = 0; i < 2; ++i) 
	{
		double cpu = _get_cpu_usage();
		double mem = _get_mem_usage();
		std::map<std::string, std::string> attrs = {
			{"cpu_usage", std::to_string(cpu)},
			{"mem_usage", std::to_string(mem)}
		};
		tracer.trace("resource_monitor", attrs);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		ASSERT_GE(cpu, 0.0);
		ASSERT_GE(mem, 0.0);
	}
}


TEST(TelemetryTest, CustomSpanExporter)
{
	using namespace libcpp::telemetry;
	class my_custom_span_exporter : public detail::custom_span_exporter
	{
	public:
		my_custom_span_exporter() : detail::custom_span_exporter()
		{
		}

		void on_export(const recordable_t& recordable) noexcept override
		{
            auto ptr = recordable.get();
            if (!ptr) {
                std::cout << "[Custom] recordable is null" << std::endl;
                return;
            }
			auto span = dynamic_cast<span_data_t*>(ptr);
            if (!span) {
                std::cout << "[Custom] recordable is not SpanData" << std::endl;
                return;
            }
            std::cout << "[Custom] span info: {\n";
            std::cout << "  name: " << to_std_string(span->GetName()) << "\n";
            std::cout << "  trace_id: " << to_std_string(span->GetTraceId()) << "\n";
            std::cout << "  span_id: " << to_std_string(span->GetSpanId()) << "\n";
            std::cout << "  parent_span_id: " << to_std_string(span->GetParentSpanId()) << "\n";
            std::cout << "  start: " << span->GetStartTime().time_since_epoch().count() << "\n";
            std::cout << "  duration: " << span->GetDuration().count() << "\n";
            std::cout << "  attributes: ";
            for (const auto& attr : span->GetAttributes()) {
               std::cout << attr.first << "=" << detail::get_value_str(*span, attr.first) << ", ";
            }
            std::cout << "\n}" << std::endl;
		}
	};

	auto exporter = std::make_unique<my_custom_span_exporter>();
	auto tracer = libcpp::telemetry::make_custom_tracer("my_custom_span_exporter1", std::move(exporter));
	for (int i = 0; i < 2; ++i) {
		double cpu = _get_cpu_usage();
		double mem = _get_mem_usage();
		std::map<std::string, std::string> attrs = {
			{"cpu_usage", std::to_string(cpu)},
			{"mem_usage", std::to_string(mem)}
		};
		tracer.trace("resource_monitor_custom", attrs);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		ASSERT_GE(cpu, 0.0);
		ASSERT_GE(mem, 0.0);
	}
}