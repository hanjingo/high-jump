#include <benchmark/benchmark.h>
#include <hj/sync/channel.hpp>

#include <thread>
#include <vector>
#include <atomic>

using hj::channel;

// Enqueue-only benchmark: single thread pushes N items into the channel
static void bm_enqueue_only(benchmark::State &st)
{
    const int    N = static_cast<int>(st.range(0));
    channel<int> ch(1024);
    for(auto _ : st)
    {
        for(int i = 0; i < N; ++i)
            ch.enqueue(i);
        // drain
        int v;
        for(int i = 0; i < N; ++i)
            ch.try_dequeue(v);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_enqueue_only)->Arg(1000)->Arg(10000)->Arg(100000);

// try_dequeue busy-wait benchmark: producer pushes, consumer busy-waits on try_dequeue
static void bm_try_dequeue_busywait(benchmark::State &st)
{
    const int N = static_cast<int>(st.range(0));
    for(auto _ : st)
    {
        channel<int>     ch(1024);
        std::atomic<int> produced{0};

        std::thread producer([&]() {
            for(int i = 0; i < N; ++i)
            {
                ch.enqueue(i);
                ++produced;
            }
        });

        std::thread consumer([&]() {
            int v;
            int got = 0;
            while(got < N)
            {
                if(ch.try_dequeue(v))
                    ++got;
            }
        });

        producer.join();
        consumer.join();
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_try_dequeue_busywait)->Arg(1000)->Arg(10000)->Arg(50000);

// One producer and one consumer using wait_dequeue (blocking) to measure throughput
static void bm_one_producer_one_consumer_wait(benchmark::State &st)
{
    const int N = static_cast<int>(st.range(0));
    for(auto _ : st)
    {
        channel<int>     ch(1024);
        std::atomic<int> produced{0};

        std::thread producer([&]() {
            for(int i = 0; i < N; ++i)
            {
                ch.enqueue(i);
                ++produced;
            }
        });

        std::thread consumer([&]() {
            int v;
            for(int i = 0; i < N; ++i)
            {
                ch.wait_dequeue(v);
                benchmark::DoNotOptimize(v);
            }
        });

        producer.join();
        consumer.join();
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_one_producer_one_consumer_wait)
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000);

// Multi-producer, multi-consumer benchmark
static void bm_multi_producer_multi_consumer(benchmark::State &st)
{
    const int per_thread = static_cast<int>(st.range(0));
    const int producers  = 4;
    const int consumers  = 4;
    for(auto _ : st)
    {
        channel<int>     ch(1024);
        std::atomic<int> produced{0};
        std::atomic<int> consumed{0};

        std::vector<std::thread> pts, cts;
        for(int p = 0; p < producers; ++p)
        {
            pts.emplace_back([&ch, per_thread, &produced]() {
                for(int i = 0; i < per_thread; ++i)
                {
                    ch.enqueue(i);
                    ++produced;
                }
            });
        }

        for(int c = 0; c < consumers; ++c)
        {
            cts.emplace_back(
                [&ch, per_thread, producers, consumers, &consumed]() {
                    int v;
                    // each consumer will read producers * per_thread / consumers items
                    int target = (producers * per_thread) / consumers;
                    for(int i = 0; i < target; ++i)
                    {
                        ch.wait_dequeue(v);
                        ++consumed;
                    }
                });
        }

        for(auto &t : pts)
            t.join();
        for(auto &t : cts)
            t.join();

        benchmark::DoNotOptimize(produced.load());
        benchmark::DoNotOptimize(consumed.load());
        benchmark::ClobberMemory();
    }
}
BENCHMARK(bm_multi_producer_multi_consumer)->Arg(1000)->Arg(10000);
