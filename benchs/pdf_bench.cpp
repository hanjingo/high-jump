#include <benchmark/benchmark.h>
#include <hj/misc/pdf.hpp>
#include <string>
#include <vector>

// Small, CI-friendly micro-benchmarks for hj::pdf
namespace
{

// Measure cost of adding a page to a freshly created document.
static void bm_pdf_add_page(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::pdf::document doc;
        state.ResumeTiming();

        auto &page = doc.add_page();
        benchmark::DoNotOptimize(&page);
        benchmark::ClobberMemory();
    }
}

// Measure setting page size
static void bm_pdf_set_size(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::pdf::document doc;
        auto             &page = doc.add_page();
        state.ResumeTiming();

        bool ok = page.set_size(hj::pdf::page::size::a4);
        benchmark::DoNotOptimize(ok);
    }
}

// Measure getting text width (font must be set first)
static void bm_pdf_get_text_width(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::pdf::document doc;
        auto             &page = doc.add_page();
        page.set_font(hj::pdf::font_name::helvetica, 12);
        const std::string text = "Hello, PDF Benchmark!";
        state.ResumeTiming();

        float w = page.get_text_width(text);
        benchmark::DoNotOptimize(w);
    }
}

// Measure showing text (begin/end text excluded from setup if desired)
static void bm_pdf_show_text(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::pdf::document doc;
        auto             &page = doc.add_page();
        page.set_font(hj::pdf::font_name::helvetica, 12);
        state.ResumeTiming();

        page.begin_text();
        bool ok = page.show_text("Benchmarking text output");
        page.end_text();
        benchmark::DoNotOptimize(ok);
    }
}

// Measure saving document to memory (reasonable CI-friendly workload)
static void bm_pdf_save_to_memory(benchmark::State &state)
{
    for(auto _ : state)
    {
        state.PauseTiming();
        hj::pdf::document doc;
        doc.set_title("Benchmark doc");
        auto &page = doc.add_page();
        page.set_font(hj::pdf::font_name::helvetica, 12);
        page.begin_text();
        page.show_text_at("Line 1", hj::pdf::point(50, 700));
        page.end_text();
        std::vector<unsigned char> buffer;
        state.ResumeTiming();

        bool ok = doc.save(buffer);
        benchmark::DoNotOptimize(buffer.size());
        benchmark::DoNotOptimize(ok);
    }
}

} // namespace

BENCHMARK(bm_pdf_add_page)->Iterations(2000);
BENCHMARK(bm_pdf_set_size)->Iterations(2000);
BENCHMARK(bm_pdf_get_text_width)->Iterations(2000);
BENCHMARK(bm_pdf_show_text)->Iterations(1000);
BENCHMARK(bm_pdf_save_to_memory)->Iterations(200);
