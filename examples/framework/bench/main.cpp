#include <libcpp/testing/benchmark.hpp>

int main(int argc, char* argv[])
{
	::benchmark::Initialize(&argc, argv);
	::benchmark::RunSpecifiedBenchmarks();
	return 0;
}