#include <thread>
#include <atomic>
#include <gtest/gtest.h>
#include <hj/sync/object_pool.hpp>
#include <string>

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
    const char *name;
    int        *age;
    float      *salary;
    const char *email;
};

TEST(object_pool, create)
{
    hj::object_pool<worker> pool;
    pool.create("harry", 30, 10000.0, "hehehunanchina@live.com");
    auto obj1 = pool.acquire();
    ASSERT_EQ(obj1 != nullptr, true);
    ASSERT_EQ(obj1->name == std::string("harry"), true);
    ASSERT_EQ(obj1->age == 30, true);
    ASSERT_EQ(obj1->salary == 10000.0, true);
    ASSERT_EQ(obj1->email == std::string("hehehunanchina@live.com"), true);
    pool.release(obj1);

    hj::object_pool<c_worker> cpool;
    c_worker                 *cw = cpool.allocate();
    cw->name                     = "lucy";
    cw->age                      = new int(18);
    cw->salary                   = new float(5000.0f);
    cw->email                    = "lucy@abc.com";
    cpool.release(cw);
    auto cw2 = cpool.acquire();
    ASSERT_STREQ(cw2->name, "lucy");
    ASSERT_EQ(*cw2->age, 18);
    ASSERT_FLOAT_EQ(*cw2->salary, 5000.0f);
    ASSERT_STREQ(cw2->email, "lucy@abc.com");
    cpool.release(cw2);
}

TEST(object_pool, release)
{
    hj::object_pool<worker> pool;
    pool.create("tom", 25, 8000.0, "tom@abc.com");
    auto obj = pool.acquire();
    ASSERT_NE(obj, nullptr);
    obj->age = 26;
    pool.release(obj);
    auto obj2 = pool.acquire();
    ASSERT_EQ(obj2->age, 26);
    pool.release(obj2);

    hj::object_pool<c_worker> cpool;
    c_worker                 *cw = cpool.allocate();
    cw->name                     = "lucy";
    cw->age                      = new int(18);
    cw->salary                   = new float(5000.0f);
    cw->email                    = "lucy@abc.com";
    cpool.release(cw);
    auto cw2 = cpool.acquire();
    ASSERT_STREQ(cw2->name, "lucy");
    ASSERT_EQ(*cw2->age, 18);
    ASSERT_FLOAT_EQ(*cw2->salary, 5000.0f);
    ASSERT_STREQ(cw2->email, "lucy@abc.com");
    cpool.release(cw2);
}

TEST(object_pool, multithread_release)
{
    hj::object_pool<worker> pool;
    constexpr int           total   = 64;
    constexpr int           threads = 8;
    std::vector<worker *>   objs;
    for(int i = 0; i < total; ++i)
        objs.push_back(pool.create("release" + std::to_string(i),
                                   20 + i,
                                   200.0f + i,
                                   "release@abc.com"));

    std::vector<worker *> acquired;
    pool.acquire_bulk(acquired, total);
    ASSERT_EQ(acquired.size(), total);
    ASSERT_EQ(pool.size(), 0);
    std::atomic<int>         success{0};
    std::atomic<int>         idx{0};
    std::vector<std::thread> ths;
    for(int t = 0; t < threads; ++t)
    {
        ths.emplace_back([&pool, &acquired, &success, &idx, total, threads]() {
            while(true)
            {
                int i = idx++;
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
    ASSERT_EQ(pool.size(), total);
}

TEST(object_pool, acquire)
{
    hj::object_pool<worker> pool;
    pool.create("jack", 40, 12000.0, "jack@abc.com");
    auto obj = pool.acquire();
    ASSERT_NE(obj, nullptr);
    ASSERT_EQ(obj->name, "jack");
    pool.release(obj);

    hj::object_pool<c_worker> cpool;
    c_worker                 *cw = cpool.allocate();
    cw->name                     = "bob";
    cw->age                      = new int(22);
    cw->salary                   = new float(6000.0f);
    cw->email                    = "bob@abc.com";
    cpool.release(cw);
    auto cw2 = cpool.acquire();
    ASSERT_NE(cw2, nullptr);
    ASSERT_STREQ(cw2->name, "bob");
    cpool.release(cw2);
}

TEST(object_pool, multithread_acquire)
{
    hj::object_pool<worker> pool;
    constexpr int           total   = 64;
    constexpr int           threads = 8;
    std::atomic<int>        success{0};
    for(int i = 0; i < total; ++i)
        pool.create("acquire" + std::to_string(i),
                    10 + i,
                    100.0f + i,
                    "acquire@abc.com");
    ASSERT_EQ(pool.size(), total);
    std::vector<std::thread> ths;
    std::vector<worker *>    results(total, nullptr);
    std::atomic<int>         idx{0};
    for(int t = 0; t < threads; ++t)
    {
        ths.emplace_back([&pool, &results, &success, &idx, total, threads]() {
            while(true)
            {
                int i = idx++;
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
    ASSERT_EQ(pool.size(), 0);
    for(int i = 0; i < total; ++i)
    {
        ASSERT_NE(results[i], nullptr);
        ASSERT_EQ(results[i]->name, "acquire" + std::to_string(i));
    }

    for(auto *obj : results)
        pool.release(obj);
    ASSERT_EQ(pool.size(), total);
}

TEST(object_pool, size)
{
    hj::object_pool<worker> pool;
    ASSERT_EQ(pool.size(), 0);
    pool.create("alice", 35, 9000.0, "alice@abc.com");
    ASSERT_EQ(pool.size(), 1);
    auto obj = pool.acquire();
    ASSERT_EQ(pool.size(), 0);
    pool.release(obj);
    ASSERT_EQ(pool.size(), 1);

    hj::object_pool<c_worker> cpool;
    ASSERT_EQ(cpool.size(), 0);
    c_worker *cw = cpool.allocate();
    cw->name     = "eve";
    cw->age      = new int(28);
    cw->salary   = new float(7000.0f);
    cw->email    = "eve@abc.com";
    cpool.release(cw);
    ASSERT_EQ(cpool.size(), 1);
    auto cw2 = cpool.acquire();
    ASSERT_EQ(cpool.size(), 0);
    cpool.release(cw2);
    ASSERT_EQ(cpool.size(), 1);
}

TEST(object_pool, bulk)
{
    hj::object_pool<worker> pool;
    std::vector<worker *>   objs;

    for(int i = 0; i < 10; ++i)
        pool.create("user" + std::to_string(i),
                    20 + i,
                    1000.0f + i,
                    "user@abc.com");

    ASSERT_EQ(pool.size(), 10);

    pool.acquire_bulk(objs, 5);
    ASSERT_EQ(objs.size(), 5);
    for(int i = 0; i < 5; ++i)
    {
        ASSERT_NE(objs[i], nullptr);
        ASSERT_EQ(objs[i]->name, "user" + std::to_string(i));
    }
    ASSERT_EQ(pool.size(), 5);

    pool.release_bulk(objs);
    ASSERT_EQ(pool.size(), 10);

    hj::object_pool<c_worker> cpool;
    std::vector<c_worker *>   cvec;
    for(int i = 0; i < 8; ++i)
    {
        c_worker *cw = cpool.allocate();
        cw->name     = "cuser";
        cw->age      = new int(10 + i);
        cw->salary   = new float(2000.0f + i);
        cw->email    = "cuser@abc.com";
        cpool.release(cw);
    }
    ASSERT_EQ(cpool.size(), 8);
    cpool.acquire_bulk(cvec, 4);
    ASSERT_EQ(cvec.size(), 4);
    for(int i = 0; i < 4; ++i)
    {
        ASSERT_NE(cvec[i], nullptr);
        ASSERT_STREQ(cvec[i]->name, "cuser");
    }
    ASSERT_EQ(cpool.size(), 4);
    cpool.release_bulk(cvec);
    ASSERT_EQ(cpool.size(), 8);
}

TEST(object_pool, multithread_bulk)
{
    hj::object_pool<worker> pool;
    constexpr int           total   = 100;
    constexpr int           threads = 8;
    std::atomic<int>        success_acquire{0};
    std::atomic<int>        success_release{0};

    for(int i = 0; i < total; ++i)
        pool.create("mtuser" + std::to_string(i),
                    30 + i,
                    2000.0f + i,
                    "mt@abc.com");

    ASSERT_EQ(pool.size(), total);

    std::vector<std::thread>           ths;
    std::vector<std::vector<worker *>> results(threads);
    std::atomic<int>                   acquired_total{0};
    for(int t = 0; t < threads; ++t)
    {
        ths.emplace_back(
            [&pool, &results, t, &acquired_total, total, threads]() {
                while(true)
                {
                    std::vector<worker *> tmp;
                    pool.acquire_bulk(tmp, total / threads);
                    if(tmp.empty())
                        break;
                    acquired_total += tmp.size();
                    results[t].insert(results[t].end(), tmp.begin(), tmp.end());
                }
            });
    }
    for(auto &th : ths)
        th.join();
    int sum = 0;
    for(const auto &vec : results)
        sum += vec.size();
    ASSERT_EQ(sum, total);
    ASSERT_EQ(pool.size(), 0);

    ths.clear();
    for(int t = 0; t < threads; ++t)
    {
        ths.emplace_back(
            [&pool, &results, t, &success_release, total, threads]() {
                pool.release_bulk(results[t]);
                success_release += results[t].size();
            });
    }
    for(auto &th : ths)
        th.join();
    ASSERT_EQ(success_release, total);
    ASSERT_EQ(pool.size(), total);
}