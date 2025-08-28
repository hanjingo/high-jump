#include <benchmark/benchmark.h>
#include <libcpp/util/defer.hpp>

static void BM_Defer_Empty(benchmark::State& state) {
	for (auto _ : state) {
		libcpp::defer d([]{});
	}
}
BENCHMARK(BM_Defer_Empty);

static void BM_Defer_Lambda(benchmark::State& state) {
	int x = 0;
	for (auto _ : state) {
		libcpp::defer d([&]{ x++; });
	}
	benchmark::DoNotOptimize(x);
}
BENCHMARK(BM_Defer_Lambda);

static void BM_Defer_Macro(benchmark::State& state) {
	int y = 0;
	for (auto _ : state) {
		DEFER(y++);
	}
	benchmark::DoNotOptimize(y);
}
BENCHMARK(BM_Defer_Macro);

