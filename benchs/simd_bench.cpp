#include <benchmark/benchmark.h>
#include <hj/hardware/simd.h>
#include <vector>
#include <random>

// Benchmarks for SIMD helpers in hj/hardware/simd.h

static void fill_random(float *buf, size_t n, uint32_t seed = 1)
{
    std::mt19937                          gen(seed);
    std::uniform_real_distribution<float> dist(-1000.0f, 1000.0f);
    for(size_t i = 0; i < n; ++i)
        buf[i] = dist(gen);
}

static void bm_simd_add_f32(benchmark::State &state)
{
    const size_t N = static_cast<size_t>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        float *a   = new float[N];
        float *b   = new float[N];
        float *out = new float[N];
        fill_random(a, N, 11);
        fill_random(b, N, 22);
        state.ResumeTiming();

        simd_add_f32(a, b, out, N);

        benchmark::DoNotOptimize(out[0]);
        benchmark::ClobberMemory();

        state.PauseTiming();
        delete[] a;
        delete[] b;
        delete[] out;
        state.ResumeTiming();
    }
}
BENCHMARK(bm_simd_add_f32)->Arg(8)->Arg(64)->Arg(1024);

static void bm_simd_mul_f32(benchmark::State &state)
{
    const size_t N = static_cast<size_t>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        float *a   = new float[N];
        float *b   = new float[N];
        float *out = new float[N];
        fill_random(a, N, 33);
        fill_random(b, N, 44);
        state.ResumeTiming();

        simd_mul_f32(a, b, out, N);

        benchmark::DoNotOptimize(out[0]);
        benchmark::ClobberMemory();

        state.PauseTiming();
        delete[] a;
        delete[] b;
        delete[] out;
        state.ResumeTiming();
    }
}
BENCHMARK(bm_simd_mul_f32)->Arg(8)->Arg(64)->Arg(1024);

static void bm_simd_dot_f32(benchmark::State &state)
{
    const size_t N = static_cast<size_t>(state.range(0));
    for(auto _ : state)
    {
        state.PauseTiming();
        float *a = new float[N];
        float *b = new float[N];
        fill_random(a, N, 55);
        fill_random(b, N, 66);
        state.ResumeTiming();

        float s = simd_dot_f32(a, b, N);

        benchmark::DoNotOptimize(s);
        benchmark::ClobberMemory();

        state.PauseTiming();
        delete[] a;
        delete[] b;
        state.ResumeTiming();
    }
}
BENCHMARK(bm_simd_dot_f32)->Arg(4)->Arg(16)->Arg(1000);
