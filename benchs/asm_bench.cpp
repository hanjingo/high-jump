#include <benchmark/benchmark.h>
#include <hj/os/asm.h>

// Provide small wrappers that use the ASM macros depending on compiler/arch.
static inline int asm_add(int a, int b)
{
#if defined(__GNUC__) || defined(__clang__)
#if defined(__x86_64__) || defined(__i386__)
    asm volatile("addl %1, %0" : "+r"(a) : "r"(b));
    return a;
#elif defined(__aarch64__)
    int res;
    asm volatile("add %w0, %w1, %w2" : "=r"(res) : "r"(a), "r"(b));
    return res;
#else
    return a + b;
#endif
#elif defined(_MSC_VER) && defined(_M_IX86)
    int c = 0;
    ASM_BEGIN
    mov eax, a add eax, b mov c, eax ASM_END return c;
#else
    return a + b;
#endif
}

static void bm_asm_noop(benchmark::State &state)
{
    for(auto _ : state)
    {
        // empty asm - macro may expand to nothing on some platforms
        ASM("");
        ASM_V("");
        benchmark::DoNotOptimize(0);
    }
}
BENCHMARK(bm_asm_noop);

static void bm_asm_add(benchmark::State &state)
{
    int a = 1, b = 2;
    for(auto _ : state)
    {
        int r = asm_add(a, b);
        benchmark::DoNotOptimize(r);
    }
}
BENCHMARK(bm_asm_add);

static void bm_asm_looped_add(benchmark::State &state)
{
    int a = 0;
    for(auto _ : state)
    {
        for(int i = 0; i < 100; ++i)
            a = asm_add(a, i);
        benchmark::DoNotOptimize(a);
    }
}
BENCHMARK(bm_asm_looped_add);
