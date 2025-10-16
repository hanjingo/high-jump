#include <benchmark/benchmark.h>
#include <hj/hardware/rom.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <chrono>

static std::string make_temp_file(const std::string &content)
{
    auto tmp = std::filesystem::temp_directory_path();
    auto ts =
        std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::string name = "hj_rom_bench_" + std::to_string(ts) + ".bin";
    auto        path = tmp / name;

    std::ofstream ofs(path, std::ios::binary);
    ofs.write(content.data(), content.size());
    ofs.close();
    return path.string();
}

static void remove_file(const std::string &path)
{
    try
    {
        std::filesystem::remove(path);
    }
    catch(...)
    {
    }
}

// Benchmarks for hj::rom C API
namespace
{

static void bm_rom_init_free(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        rom_t r;
        state.ResumeTiming();

        rom_init(&r);
        benchmark::DoNotOptimize(r.data);
        benchmark::ClobberMemory();

        state.PauseTiming();
        rom_free(&r);
        state.ResumeTiming();
    }
}

static void bm_rom_load_from_file(benchmark::State &state)
{
    // small content to keep CI friendly
    const std::string content = "ROMDATA0123456789";
    for(auto _ : state)
    {
        state.PauseTiming();
        std::string path = make_temp_file(content);
        rom_t       r;
        rom_init(&r);
        state.ResumeTiming();

        bool ok = rom_load(&r, path.c_str());
        benchmark::DoNotOptimize(ok ? r.size : 0);

        state.PauseTiming();
        rom_free(&r);
        remove_file(path);
        state.ResumeTiming();
    }
}

static void bm_rom_read_small(benchmark::State &state)
{
    const std::string content(1024, 'A'); // 1 KB
    state.PauseTiming();
    std::string path = make_temp_file(content);
    rom_t       r;
    rom_init(&r);
    bool ok = rom_load(&r, path.c_str());
    state.ResumeTiming();

    if(!ok)
    {
        state.SkipWithError("rom_load failed");
        state.PauseTiming();
        rom_free(&r);
        remove_file(path);
        state.ResumeTiming();
        return;
    }

    std::vector<char> buf(128);
    for(auto _ : state)
    {
        size_t n = rom_read(&r, 10, buf.data(), buf.size());
        benchmark::DoNotOptimize(n);
    }

    state.PauseTiming();
    rom_free(&r);
    remove_file(path);
    state.ResumeTiming();
}

static void bm_rom_read_out_of_range(benchmark::State &state)
{
    const std::string content = "SHORT";
    state.PauseTiming();
    std::string path = make_temp_file(content);
    rom_t       r;
    rom_init(&r);
    bool ok = rom_load(&r, path.c_str());
    state.ResumeTiming();

    if(!ok)
    {
        state.SkipWithError("rom_load failed");
        state.PauseTiming();
        rom_free(&r);
        remove_file(path);
        state.ResumeTiming();
        return;
    }

    std::vector<char> buf(8);
    for(auto _ : state)
    {
        // offset well beyond size
        size_t n = rom_read(&r, 1024, buf.data(), buf.size());
        benchmark::DoNotOptimize(n);
    }

    state.PauseTiming();
    rom_free(&r);
    remove_file(path);
    state.ResumeTiming();
}

} // namespace

BENCHMARK(bm_rom_init_free)->Iterations(5000);
BENCHMARK(bm_rom_load_from_file)->Iterations(2000);
BENCHMARK(bm_rom_read_small)->Iterations(2000);
BENCHMARK(bm_rom_read_out_of_range)->Iterations(2000);
