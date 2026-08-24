#include <atomic>
#include <gtest/gtest.h>
#include <hj/sync/object_pool.hpp>
#include <set>
#include <string>
#include <thread>
#include <memory>
#include <vector>
#include <cstdint>
#include <stdexcept>

class worker
{
  public:
    worker(std::string _name, int _age, float _salary, std::string _email)
        : name{_name}
        , age{_age}
        , salary{_salary}
        , email{_email}
    {
    }
    ~worker() {}

    std::string name;
    int         age;
    float       salary;
    std::string email;
};

struct c_worker
{
    const char *name{nullptr};
    int        *age{nullptr};
    float      *salary{nullptr};
    const char *email{nullptr};
};

struct tracked_resource
{
    inline static std::atomic<int> ctor_count{0};
    inline static std::atomic<int> dtor_count{0};

    std::string          name;
    std::unique_ptr<int> heap_data;

    tracked_resource(std::string n, int val)
        : name(std::move(n))
        , heap_data(std::make_unique<int>(val))
    {
        ++ctor_count;
    }

    ~tracked_resource() { ++dtor_count; }

    static void reset_counters()
    {
        ctor_count = 0;
        dtor_count = 0;
    }
};

TEST(object_pool, acquire)
{
    hj::object_pool<worker> pool;
    auto obj1 = pool.acquire_or_create("harry",
                                       30,
                                       10000.0f,
                                       "hehehunanchina@live.com");
    ASSERT_NE(obj1, nullptr);
    ASSERT_EQ(obj1->name, "harry");
    ASSERT_EQ(obj1->age, 30);
    ASSERT_FLOAT_EQ(obj1->salary, 10000.0f);
    ASSERT_EQ(obj1->email, "hehehunanchina@live.com");
    pool.release(obj1);

    hj::object_pool<c_worker> cpool;
    c_worker                 *cw = cpool.acquire_or_create();
    ASSERT_NE(cw, nullptr);
    cw->name   = "lucy";
    cw->age    = new int(18);
    cw->salary = new float(5000.0f);
    cw->email  = "lucy@abc.com";
    cpool.release(cw);

    auto cw2 = cpool.acquire();
    ASSERT_STREQ(cw2->name, "lucy");
    ASSERT_EQ(*cw2->age, 18);
    ASSERT_FLOAT_EQ(*cw2->salary, 5000.0f);
    ASSERT_STREQ(cw2->email, "lucy@abc.com");

    delete cw2->age;
    delete cw2->salary;
    cpool.release(cw2);
}

TEST(object_pool, acquire_smart_raii)
{
    hj::object_pool<worker> pool;
    ASSERT_EQ(pool.size_approx(), 0);

    {
        auto smart_obj = pool.acquire_or_create_smart("raii_user",
                                                      28,
                                                      9500.0f,
                                                      "raii@abc.com");
        ASSERT_NE(smart_obj, nullptr);
        ASSERT_EQ(smart_obj->name, "raii_user");
        ASSERT_EQ(pool.size_approx(), 0);
    }

    ASSERT_EQ(pool.size_approx(), 1);


    auto raw_obj = pool.acquire();
    ASSERT_EQ(raw_obj->name, "raii_user");
    pool.release(raw_obj);
}

TEST(object_pool, release)
{
    hj::object_pool<worker> pool;
    auto obj = pool.acquire_or_create("tom", 25, 8000.0f, "tom@abc.com");
    ASSERT_NE(obj, nullptr);
    obj->age = 26;
    pool.release(obj);

    auto obj2 = pool.acquire();
    ASSERT_EQ(obj2->age, 26);
    pool.release(obj2);
}

TEST(object_pool, multithread_release)
{
    hj::object_pool<worker> pool;
    constexpr int           total   = 64;
    constexpr int           threads = 8;
    std::vector<worker *>   objs;

    for(int i = 0; i < total; ++i)
    {
        objs.push_back(pool.acquire_or_create("release" + std::to_string(i),
                                              20 + i,
                                              200.0f + i,
                                              "release@abc.com"));
    }

    pool.release_bulk(objs);
    ASSERT_EQ(pool.size_approx(), total);


    std::vector<worker *> acquired;
    pool.acquire_bulk(acquired, total);
    ASSERT_EQ(acquired.size(), total);
    ASSERT_EQ(pool.size_approx(), 0);

    std::atomic<int>         success{0};
    std::atomic<int>         idx{0};
    std::vector<std::thread> ths;

    for(int t = 0; t < threads; ++t)
    {
        ths.emplace_back([&pool, &acquired, &success, &idx]() {
            while(true)
            {
                size_t i = idx++;
                if(i >= acquired.size())
                    break;
                pool.release(acquired[i]);
                ++success;
            }
        });
    }

    for(auto &th : ths)
        th.join();

    ASSERT_EQ(success, total);
    ASSERT_EQ(pool.size_approx(), total);
}

TEST(object_pool, multithread_acquire)
{
    hj::object_pool<worker> pool;
    constexpr int           total   = 64;
    constexpr int           threads = 8;
    std::atomic<int>        success{0};

    std::vector<worker *> init_objs;
    for(int i = 0; i < total; ++i)
    {
        init_objs.push_back(
            pool.acquire_or_create("acquire" + std::to_string(i),
                                   10 + i,
                                   100.0f + i,
                                   "acquire@abc.com"));
    }
    pool.release_bulk(init_objs);
    ASSERT_EQ(pool.size_approx(), total);


    std::vector<std::thread> ths;
    std::vector<worker *>    results(total, nullptr);
    std::atomic<int>         idx{0};

    for(int t = 0; t < threads; ++t)
    {
        ths.emplace_back([&pool, &results, &success, &idx]() {
            while(true)
            {
                size_t i = idx++;
                if(i >= results.size())
                    break;
                worker *obj = pool.acquire();
                if(obj)
                {
                    results[i] = obj;
                    ++success;
                }
            }
        });
    }

    for(auto &th : ths)
        th.join();

    ASSERT_EQ(success, total);
    ASSERT_EQ(pool.size_approx(), 0);


    std::set<std::string> expected_names;
    std::set<std::string> actual_names;

    for(int i = 0; i < total; ++i)
    {
        expected_names.insert("acquire" + std::to_string(i));
        ASSERT_NE(results[i], nullptr);
        actual_names.insert(results[i]->name);
    }

    ASSERT_EQ(expected_names, actual_names);

    pool.release_bulk(results);
    ASSERT_EQ(pool.size_approx(), total);
}

TEST(object_pool, size)
{
    hj::object_pool<worker> pool;
    ASSERT_EQ(pool.size_approx(), 0);


    auto obj = pool.acquire_or_create("alice", 35, 9000.0f, "alice@abc.com");
    ASSERT_EQ(pool.size_approx(), 0);


    pool.release(obj);
    ASSERT_EQ(pool.size_approx(), 1);


    auto obj2 = pool.acquire();
    ASSERT_EQ(pool.size_approx(), 0);
    pool.release(obj2);
    ASSERT_EQ(pool.size_approx(), 1);
}

TEST(object_pool, concurrent_size_approx_behavior)
{
    hj::object_pool<tracked_resource> pool;
    constexpr int                     kNumThreads = 4;
    constexpr int                     kIterations = 1000;

    std::atomic<bool>        start_flag{false};
    std::vector<std::thread> threads;

    for(int i = 0; i < kNumThreads; ++i)
    {
        threads.emplace_back([=, &pool, &start_flag]() {
            while(!start_flag.load())
            {
                std::this_thread::yield();
            }

            for(int j = 0; j < kIterations; ++j)
            {
                auto ptr    = pool.acquire_or_create("test", j);
                auto approx = pool.size_approx();
                EXPECT_GE(approx, 0);

                pool.release(ptr);
            }
        });
    }

    start_flag = true;
    for(auto &t : threads)
    {
        t.join();
    }
}

TEST(object_pool, bulk)
{
    hj::object_pool<worker> pool;
    std::vector<worker *>   created;

    for(int i = 0; i < 10; ++i)
    {
        created.push_back(pool.acquire_or_create("user" + std::to_string(i),
                                                 20 + i,
                                                 1000.0f + i,
                                                 "user@abc.com"));
    }
    pool.release_bulk(created);
    ASSERT_EQ(pool.size_approx(), 10);


    std::vector<worker *> objs;
    pool.acquire_bulk(objs, 5);
    ASSERT_EQ(objs.size(), 5);
    ASSERT_EQ(pool.size_approx(), 5);


    pool.release_bulk(objs);
    ASSERT_EQ(pool.size_approx(), 10);
}

TEST(object_pool, multithread_bulk)
{
    hj::object_pool<worker> pool;
    constexpr int           total   = 100;
    constexpr int           threads = 8;
    std::atomic<int>        success_release{0};

    std::vector<worker *> init_objs;
    for(int i = 0; i < total; ++i)
    {
        init_objs.push_back(pool.acquire_or_create("mtuser" + std::to_string(i),
                                                   30 + i,
                                                   2000.0f + i,
                                                   "mt@abc.com"));
    }
    pool.release_bulk(init_objs);
    ASSERT_EQ(pool.size_approx(), total);


    std::vector<std::thread>           ths;
    std::vector<std::vector<worker *>> results(threads);

    for(int t = 0; t < threads; ++t)
    {
        ths.emplace_back([&pool, &results, t, total, threads]() {
            while(true)
            {
                std::vector<worker *> tmp;
                pool.acquire_bulk(tmp, total / threads);
                if(tmp.empty())
                    break;
                results[t].insert(results[t].end(), tmp.begin(), tmp.end());
            }
        });
    }

    for(auto &th : ths)
        th.join();

    int sum = 0;
    for(const auto &vec : results)
        sum += static_cast<int>(vec.size());

    ASSERT_EQ(sum, total);
    ASSERT_EQ(pool.size_approx(), 0);


    ths.clear();
    for(int t = 0; t < threads; ++t)
    {
        ths.emplace_back([&pool, &results, t, &success_release]() {
            pool.release_bulk(results[t]);
            success_release += static_cast<int>(results[t].size());
        });
    }

    for(auto &th : ths)
        th.join();

    ASSERT_EQ(success_release, total);
    ASSERT_EQ(pool.size_approx(), total);
}

TEST(object_pool_nontrivial, constructor_destructor_balance)
{
    tracked_resource::reset_counters();

    {
        hj::object_pool<tracked_resource> pool;

        auto obj1 = pool.acquire_or_create("item1", 100);
        auto obj2 = pool.acquire_or_create("item2", 200);


        ASSERT_EQ(tracked_resource::ctor_count, 2);
        ASSERT_EQ(tracked_resource::dtor_count, 0);

        pool.release(obj1);
        pool.release(obj2);
        ASSERT_EQ(tracked_resource::dtor_count, 0);

        auto obj3 = pool.acquire();
        ASSERT_NE(obj3, nullptr);
        pool.release(obj3);
    }

    ASSERT_EQ(tracked_resource::ctor_count, tracked_resource::dtor_count);
}

TEST(object_pool_nontrivial, explicit_clear_destroys_all)
{
    tracked_resource::reset_counters();

    hj::object_pool<tracked_resource> pool;

    auto obj1 = pool.acquire_or_create("a", 1);
    auto obj2 = pool.acquire_or_create("b", 2);
    auto obj3 = pool.acquire_or_create("c", 3);


    pool.release(obj1);
    pool.release(obj2);
    pool.release(obj3);

    ASSERT_EQ(tracked_resource::ctor_count, 3);
    ASSERT_EQ(tracked_resource::dtor_count, 0);

    pool.clear();

    ASSERT_EQ(tracked_resource::dtor_count, 3);
    ASSERT_EQ(tracked_resource::ctor_count, tracked_resource::dtor_count);
}

TEST(object_pool_nontrivial, trim_releases_excess_objects)
{
    tracked_resource::reset_counters();

    hj::object_pool<tracked_resource> pool;
    std::vector<tracked_resource *>   items;

    for(int i = 0; i < 10; ++i)
    {
        items.push_back(pool.acquire_or_create("item" + std::to_string(i), i));
    }
    pool.release_bulk(items);

    ASSERT_EQ(tracked_resource::ctor_count, 10);
    ASSERT_EQ(tracked_resource::dtor_count, 0);
    ASSERT_EQ(pool.size_approx(), 10);


    pool.trim(3);

    ASSERT_EQ(pool.size_approx(), 3);
    ASSERT_EQ(tracked_resource::dtor_count, 7);

    pool.clear();
    ASSERT_EQ(tracked_resource::ctor_count, tracked_resource::dtor_count);
}

TEST(object_pool_industrial, double_release)
{
#if defined(NDEBUG)
    GTEST_SKIP() << "Double release check is active only in Debug mode with "
                    "assertions enabled.";
#else
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";

    hj::object_pool<worker> pool;
    auto                    obj =
        pool.acquire_or_create("doublerelease", 20, 100.0f, "test@abc.com");
    ASSERT_NE(obj, nullptr);

    pool.release(obj);

    EXPECT_DEATH({ pool.release(obj); }, "Assertion failed: exchanged");
#endif
}

TEST(object_pool_industrial, cross_pool_release)
{
#if defined(NDEBUG)
    GTEST_SKIP() << "Cross-pool release check is active only in Debug mode "
                    "with assertions enabled.";
#else
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";

    hj::object_pool<worker> pool1;
    hj::object_pool<worker> pool2;

    auto obj1 =
        pool1.acquire_or_create("crosspool", 25, 200.0f, "cross@abc.com");
    ASSERT_NE(obj1, nullptr);

    EXPECT_DEATH(
        { pool2.release(obj1); },
        "Assertion failed: wrapper->owner_pool == this");

    pool1.release(obj1);
#endif
}

// ==========================================
// 3. Constructor Exception & State Restoration
// ==========================================
struct throwing_worker
{
    inline static std::atomic<int> attempt_count{0};

    throwing_worker(int)
    {
        ++attempt_count;
        throw std::runtime_error("boom");
    }
};

TEST(object_pool_industrial, constructor_exception_safety)
{
    throwing_worker::attempt_count = 0;
    hj::object_pool<throwing_worker> pool;

    EXPECT_THROW({ pool.acquire_or_create(42); }, std::runtime_error);

    EXPECT_EQ(throwing_worker::attempt_count, 1);
    EXPECT_EQ(pool.size_approx(), 0);

    hj::object_pool<worker> safe_pool;
    auto                    ok_obj =
        safe_pool.acquire_or_create("recovered", 30, 300.0f, "ok@abc.com");
    ASSERT_NE(ok_obj, nullptr);
    safe_pool.release(ok_obj);
    EXPECT_EQ(safe_pool.size_approx(), 1);
}

struct alignas(64) aligned_object
{
    uint64_t data[8];
};

TEST(object_pool_industrial, over_aligned_object)
{
    static_assert(alignof(aligned_object) == 64,
                  "Aligned object must have 64-byte alignment.");

    hj::object_pool<aligned_object> pool;
    auto                            obj = pool.acquire_or_create();
    ASSERT_NE(obj, nullptr);

    std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(obj);
    EXPECT_EQ(addr % alignof(aligned_object), 0)
        << "Allocated object address " << addr << " is not aligned to "
        << alignof(aligned_object);

    pool.release(obj);
    EXPECT_EQ(pool.size_approx(), 1);
}

TEST(object_pool_industrial, stress_test_heavy_concurrency)
{
    hj::object_pool<tracked_resource> pool;
    tracked_resource::reset_counters();

    constexpr int kThreads             = 32;
    constexpr int kIterationsPerThread = 5000;

    std::atomic<bool>        start_flag{false};
    std::vector<std::thread> threads;
    threads.reserve(kThreads);

    for(int t = 0; t < kThreads; ++t)
    {
        threads.emplace_back([t, &pool, &start_flag, kIterationsPerThread]() {
            while(!start_flag.load(std::memory_order_acquire))
            {
                std::this_thread::yield();
            }

            std::vector<tracked_resource *> local_cache;
            local_cache.reserve(64);

            for(int i = 0; i < kIterationsPerThread; ++i)
            {
                if(i % 3 == 0)
                {
                    auto *res =
                        pool.acquire_or_create("stress_" + std::to_string(t),
                                               i);
                    if(res)
                    {
                        local_cache.push_back(res);
                    }
                } else if(i % 3 == 1 && !local_cache.empty())
                {
                    pool.release(local_cache.back());
                    local_cache.pop_back();
                } else
                {
                    std::vector<tracked_resource *> bulk_out;
                    pool.acquire_bulk(bulk_out, 4);
                    for(auto *ptr : bulk_out)
                    {
                        local_cache.push_back(ptr);
                    }
                }

                if(local_cache.size() >= 32)
                {
                    pool.release_bulk(local_cache);
                    local_cache.clear();
                }
            }

            if(!local_cache.empty())
            {
                pool.release_bulk(local_cache);
            }
        });
    }

    start_flag.store(true, std::memory_order_release);

    for(auto &th : threads)
    {
        th.join();
    }

    pool.clear();

    EXPECT_EQ(tracked_resource::ctor_count, tracked_resource::dtor_count);
}