#include <gtest/gtest.h>
#include <hj/sync/thread_pool.hpp>

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <vector>

#if defined(__linux__)
#include <sched.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

using namespace std::chrono_literals;

static unsigned int get_current_cpu_id()
{
#if defined(__linux__)
    int cpu = sched_getcpu();
    return cpu >= 0 ? static_cast<unsigned int>(cpu) : 0;
#elif defined(_WIN32)
    PROCESSOR_NUMBER proc_num;
    GetCurrentProcessorNumberEx(&proc_num);

    WORD         group_count = GetActiveProcessorGroupCount();
    unsigned int global_id   = 0;
    for(WORD g = 0; g < proc_num.Group; ++g)
    {
        global_id += GetActiveProcessorCount(g);
    }
    return global_id + proc_num.Number;
#else
    return 0;
#endif
}

TEST(thread_pool, size)
{
    hj::thread_pool tp1{0};
    ASSERT_EQ(tp1.size(), 1);

    hj::thread_pool tp2{1};
    ASSERT_EQ(tp2.size(), 1);

    hj::thread_pool tp3{};
    ASSERT_TRUE(tp3.size() > 0);
}

TEST(thread_pool, enqueue)
{
    hj::thread_pool tp{1};
    auto            ret = tp.enqueue([]() -> int { return 5; });
    ASSERT_EQ(ret.get(), 5);
}

TEST(thread_pool, shutdown)
{
    hj::thread_pool tp{1};
    int             n = 0;
    for(int i = 0; i < 100; ++i)
    {
        tp.enqueue([&]() { n++; });
    }

    tp.shutdown();
    ASSERT_EQ(n, 100);
}

TEST(thread_pool, multi_thread_enqueue)
{
    hj::thread_pool                tp{4};
    std::atomic<int>               sum{0};
    std::vector<std::future<void>> futs;
    for(int i = 0; i < 100; ++i)
        futs.push_back(tp.enqueue([&sum] { sum++; }));
    for(auto &f : futs)
        f.get();
    ASSERT_EQ(sum, 100);
}

TEST(thread_pool, enqueue_exception)
{
    hj::thread_pool tp{1};
    auto fut = tp.enqueue([]() -> int { throw std::runtime_error("fail"); });
    ASSERT_THROW(fut.get(), std::runtime_error);
}

TEST(thread_pool, shutdown_then_enqueue)
{
    hj::thread_pool tp{2};
    tp.shutdown();

    auto fut = tp.enqueue([]() { return 1; });
    ASSERT_FALSE(fut.valid());
}

TEST(thread_pool, affinity_ctor)
{
    try
    {
        std::unordered_set<unsigned int> cores{0};
        hj::thread_pool                  tp{cores};
        ASSERT_EQ(tp.size(), 1);
    }
    catch(const std::exception &e)
    {
        GTEST_SKIP() << "Affinity not supported or permission denied: "
                     << e.what();
    }
}

TEST(thread_pool, stress)
{
    hj::thread_pool                tp{8};
    std::atomic<int>               sum{0};
    std::vector<std::future<void>> futs;
    for(int i = 0; i < 1000; ++i)
        futs.push_back(tp.enqueue([&sum] { sum++; }));
    for(auto &f : futs)
        f.get();
    ASSERT_EQ(sum, 1000);
}

TEST(thread_pool, strict_affinity_verification)
{
    const unsigned int target_core = 0;

    try
    {
        std::unordered_set<unsigned int> cores{target_core};
        hj::thread_pool                  tp{cores};

        auto fut =
            tp.enqueue([]() -> unsigned int { return get_current_cpu_id(); });

        unsigned int actual_core = fut.get();

        ASSERT_EQ(actual_core, target_core);
    }
    catch(const std::exception &e)
    {
        GTEST_SKIP() << "Affinity test skipped due to lack of system "
                        "privileges or topology support: "
                     << e.what();
    }
}

TEST(thread_pool, worker_startup_failure)
{
    std::unordered_set<unsigned int> invalid_cores{999999};
    EXPECT_THROW({ hj::thread_pool tp(invalid_cores); }, std::runtime_error);
}

// 修复：不直接在 worker 线程内部触发 EXPECT 宏，采用子线程 capture 异常方式
TEST(thread_pool_p0, shutdown_from_worker_throws)
{
    hj::thread_pool   tp{2};
    std::atomic<bool> caught_logic_error{false};

    auto fut = tp.enqueue([&tp, &caught_logic_error]() {
        try
        {
            tp.shutdown();
        }
        catch(const std::logic_error &)
        {
            caught_logic_error = true;
        }
    });

    fut.get();
    EXPECT_TRUE(caught_logic_error.load());
    tp.shutdown();
}

// 修复：去 Sleep 化，使用 std::promise 驱动无 Flaky 依赖的精准同步
TEST(thread_pool, exception_handler)
{
    std::promise<void> handler_done_p;
    auto               handler_done_f = handler_done_p.get_future();

    hj::thread_pool tp{1, [&](const std::exception_ptr &e) {
                           try
                           {
                               if(e)
                                   std::rethrow_exception(e);
                           }
                           catch(const std::runtime_error &err)
                           {
                               if(std::string(err.what()) == "task_error")
                               {
                                   handler_done_p.set_value();
                               }
                           }
                       }};

    tp.enqueue([]() { throw std::runtime_error("task_error"); });

    ASSERT_EQ(handler_done_f.wait_for(2s), std::future_status::ready);
}

TEST(thread_pool, concurrent_enqueue)
{
    hj::thread_pool  tp{4};
    std::atomic<int> counter{0};
    constexpr int    threads_count    = 8;
    constexpr int    tasks_per_thread = 500;

    std::vector<std::thread> producers;
    for(int i = 0; i < threads_count; ++i)
    {
        producers.emplace_back([&tp, &counter, tasks_per_thread]() {
            for(int j = 0; j < tasks_per_thread; ++j)
            {
                tp.enqueue([&counter]() {
                    counter.fetch_add(1, std::memory_order_relaxed);
                });
            }
        });
    }

    for(auto &t : producers)
        t.join();
    tp.shutdown();
    EXPECT_EQ(counter.load(), threads_count * tasks_per_thread);
}

TEST(thread_pool, concurrent_enqueue_and_shutdown)
{
    hj::thread_pool   tp{4};
    std::atomic<bool> stop_producer{false};

    std::thread producer([&tp, &stop_producer]() {
        while(!stop_producer)
        {
            auto fut = tp.enqueue([] { return 1; });
            std::this_thread::yield();
        }
    });

    std::this_thread::sleep_for(20ms);
    tp.shutdown();
    stop_producer = true;
    producer.join();

    auto fut = tp.enqueue([] { return 1; });
    EXPECT_FALSE(fut.valid());
}

TEST(thread_pool, clear_pending_and_broken_promise)
{
    hj::thread_pool    tp{1};
    std::promise<void> worker_started_p;
    auto               worker_started_f = worker_started_p.get_future();

    std::promise<void> block_p;
    auto               block_f = block_p.get_future();

    tp.enqueue([&block_f, &worker_started_p]() {
        worker_started_p.set_value();
        block_f.wait();
    });

    worker_started_f.wait();

    auto pending_fut = tp.enqueue([]() -> int { return 42; });

    std::size_t canceled = tp.cancel_pending();
    EXPECT_EQ(canceled, 1);

    block_p.set_value();

    EXPECT_THROW(pending_fut.get(), std::future_error);
}

TEST(thread_pool, empty_affinity_set)
{
    std::unordered_set<unsigned int> empty_cores;
    EXPECT_THROW({ hj::thread_pool tp(empty_cores); }, std::invalid_argument);
}

TEST(thread_pool, move_only_and_references)
{
    hj::thread_pool tp{2};

    auto ptr  = std::make_unique<int>(100);
    auto fut1 = tp.enqueue([](std::unique_ptr<int> p) { return *p + 50; },
                           std::move(ptr));
    EXPECT_EQ(fut1.get(), 150);

    int  val  = 10;
    auto fut2 = tp.enqueue([](int &n) { n += 5; }, std::ref(val));
    fut2.get();
    EXPECT_EQ(val, 15);

    bool executed = false;
    auto fut3     = tp.enqueue([&executed]() { executed = true; });
    fut3.get();
    EXPECT_TRUE(executed);
}

TEST(thread_pool_p2, worker_scales)
{
    for(size_t threads : {1, 2, 64})
    {
        hj::thread_pool tp(threads);
        EXPECT_EQ(tp.worker_count(), threads);
        EXPECT_EQ(tp.active_thread_count(), threads);

        auto fut = tp.enqueue([] { return 1; });
        EXPECT_EQ(fut.get(), 1);
    }
}

TEST(thread_pool_p2, shutdown_during_long_running_task)
{
    std::atomic<bool>  task_completed{false};
    std::promise<void> task_started_p;
    auto               task_started_f = task_started_p.get_future();

    {
        hj::thread_pool tp{2};
        tp.enqueue([&task_completed, &task_started_p]() {
            task_started_p.set_value();
            std::this_thread::sleep_for(10ms);
            task_completed = true;
        });
        task_started_f.wait();
    }
    EXPECT_TRUE(task_completed.load());
}

TEST(thread_pool_p2, multiple_shutdown_calls)
{
    hj::thread_pool tp{2};
    tp.shutdown();
    tp.shutdown();
    EXPECT_EQ(tp.active_thread_count(), 0);
}

TEST(thread_pool, multi_producer_enqueue)
{
    constexpr int producer_count       = 8;
    constexpr int tasks_per_producer   = 1000;
    constexpr int total_expected_tasks = producer_count * tasks_per_producer;

    hj::thread_pool          tp{4};
    std::atomic<int>         completed_tasks{0};
    std::atomic<bool>        producer_failed{false};
    std::vector<std::thread> producers;
    producers.reserve(producer_count);

    for(int i = 0; i < producer_count; ++i)
    {
        producers.emplace_back(
            [&tp, &completed_tasks, &producer_failed, tasks_per_producer]() {
                for(int j = 0; j < tasks_per_producer; ++j)
                {
                    auto fut = tp.enqueue([&completed_tasks]() {
                        completed_tasks.fetch_add(1, std::memory_order_relaxed);
                    });
                    if(!fut.valid())
                    {
                        producer_failed.store(true, std::memory_order_relaxed);
                    }
                }
            });
    }

    for(auto &p : producers)
    {
        p.join();
    }

    tp.shutdown();

    ASSERT_FALSE(producer_failed.load());
    EXPECT_EQ(completed_tasks.load(), total_expected_tasks);
}

TEST(thread_pool, concurrent_enqueue_and_shutdown_race)
{
    constexpr int iterations = 50;

    for(int i = 0; i < iterations; ++i)
    {
        auto              tp = std::make_unique<hj::thread_pool>(4);
        std::atomic<bool> start_flag{false};
        std::atomic<int>  accepted_tasks{0};
        std::atomic<int>  executed_tasks{0};
        std::vector<std::future<void>> accepted_futs;
        std::mutex                     futs_mu;

        std::thread producer([&]() {
            while(!start_flag.load(std::memory_order_acquire))
            {
                std::this_thread::yield();
            }

            for(int j = 0; j < 300; ++j)
            {
                auto fut = tp->enqueue([&executed_tasks]() {
                    executed_tasks.fetch_add(1, std::memory_order_relaxed);
                });

                if(fut.valid())
                {
                    accepted_tasks.fetch_add(1, std::memory_order_relaxed);
                    std::lock_guard<std::mutex> lock(futs_mu);
                    accepted_futs.push_back(std::move(fut));
                }
            }
        });

        std::thread stopper([&]() {
            while(!start_flag.load(std::memory_order_acquire))
            {
                std::this_thread::yield();
            }
            std::this_thread::yield();
            tp->shutdown();
        });

        start_flag.store(true, std::memory_order_release);

        producer.join();
        stopper.join();

        for(auto &f : accepted_futs)
        {
            f.get();
        }

        EXPECT_EQ(accepted_tasks.load(), executed_tasks.load());

        auto post_shutdown_fut = tp->enqueue([] { return 1; });
        EXPECT_FALSE(post_shutdown_fut.valid());

        tp.reset();
    }
}

TEST(thread_pool, exception_handler_throws_triggers_terminate)
{
    EXPECT_DEATH(
        ([]() {
            hj::thread_pool tp{1, [](const std::exception_ptr &) {
                                   throw std::runtime_error(
                                       "handler exception!");
                               }};

            tp.enqueue([]() { throw std::runtime_error("task exception!"); });

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }()),
        ".*");
}

TEST(thread_pool, thread_count_metrics)
{
    constexpr std::size_t num_threads = 4;
    hj::thread_pool       tp{num_threads};

    EXPECT_EQ(tp.worker_count(), num_threads);
    EXPECT_EQ(tp.size(), num_threads);
    EXPECT_EQ(tp.thread_count(), num_threads);
    EXPECT_EQ(tp.active_thread_count(), num_threads);

    std::promise<void> worker_release_p;
    auto               worker_release_f = worker_release_p.get_future().share();
    std::atomic<int>   started_count{0};

    std::vector<std::future<void>> futs;
    for(std::size_t i = 0; i < num_threads; ++i)
    {
        futs.push_back(tp.enqueue([&started_count, worker_release_f]() {
            started_count.fetch_add(1, std::memory_order_relaxed);
            worker_release_f.wait();
        }));
    }

    while(started_count.load() < static_cast<int>(num_threads))
    {
        std::this_thread::yield();
    }

    EXPECT_EQ(tp.active_thread_count(), num_threads);

    worker_release_p.set_value();
    for(auto &f : futs)
    {
        f.get();
    }
}

TEST(thread_pool, active_thread_count_lifecycle_on_shutdown)
{
    auto tp = std::make_unique<hj::thread_pool>(4);
    EXPECT_EQ(tp->active_thread_count(), 4);

    std::promise<void> task_start_p;
    auto               task_start_f = task_start_p.get_future();

    tp->enqueue([&task_start_p]() { task_start_p.set_value(); });

    task_start_f.wait();
    tp->shutdown();

    EXPECT_EQ(tp->active_thread_count(), 0);
    EXPECT_EQ(tp->worker_count(), 4);
}

TEST(thread_pool, is_shutdown_status)
{
    hj::thread_pool tp{2};

    EXPECT_FALSE(tp.is_shutdown());

    auto fut = tp.enqueue([]() { return 42; });
    EXPECT_EQ(fut.get(), 42);
    EXPECT_FALSE(tp.is_shutdown());

    tp.shutdown();
    EXPECT_TRUE(tp.is_shutdown());

    tp.shutdown();
    EXPECT_TRUE(tp.is_shutdown());
}