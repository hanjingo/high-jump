#include <gtest/gtest.h>
#include <libcpp/io/dbuffer.hpp>
#include <thread>
#include <shared_mutex>
#include <chrono>

TEST(dbuffer, read)
{
    libcpp::dbuffer<std::vector<int> > dbuf{};
    std::vector<int> buf1{1, 2, 3};
    std::vector<int> buf2{0, 0, 0};
    ASSERT_EQ(dbuf.write(buf1), true);
    ASSERT_EQ(dbuf.read(buf2), true);
    for (int i = 0; i < buf1.size(); ++i)
    {
        ASSERT_EQ(buf1[i], buf2[i]);
        ASSERT_EQ(buf2[i], i+1);
    }

    std::vector<int> buf3{4, 5, 6};
    ASSERT_EQ(dbuf.write(buf3), true);
    ASSERT_EQ(dbuf.read(buf2), true);
    for (int i = 0; i < buf3.size(); ++i)
        ASSERT_EQ(buf3[i], buf2[i]);

    int times = 100000;
    auto dbuffer_total_start = std::chrono::system_clock::now();
    std::thread t1([&](){
        auto start = std::chrono::system_clock::now();
        for (int n = 0; n < times; ++n)
        {
            dbuf.write(buf1);
        }
        auto end = std::chrono::system_clock::now();
        std::cout << "dbuffer write time passed:" << (end - start).count() << std::endl;
    });
    std::thread t2([&](){
        auto start = std::chrono::system_clock::now();
        for (int n = 0; n < times; ++n)
        {
            dbuf.read(buf2);
        }
        auto end = std::chrono::system_clock::now();
        std::cout << "dbuffer read time passed:" << (end - start).count() << std::endl;
    });
    t1.join();
    t2.join();
    auto dbuffer_total_end = std::chrono::system_clock::now();
    std::cout << "dbuffer total read/write time passed:" << (dbuffer_total_end - dbuffer_total_start).count() << std::endl;

    std::shared_mutex mu;
    auto rwlock_total_start = std::chrono::system_clock::now();
    std::thread t3([&](){
        auto start = std::chrono::system_clock::now();
        for (int n = 0; n < times; ++n)
        {
            mu.lock();
            buf3 = buf1; // write
            mu.unlock();
        }
        auto end = std::chrono::system_clock::now();
        std::cout << "rwlock write time passed:" << (end - start).count() << std::endl;
    });
    std::thread t4([&](){
        auto start = std::chrono::system_clock::now();
        for (int n = 0; n < times; ++n)
        {
            mu.lock_shared();
            buf2 = buf3; // read
            mu.unlock_shared();
        }
        auto end = std::chrono::system_clock::now();
        std::cout << "rwlock read time passed:" << (end - start).count() << std::endl;
    });
    t3.join();
    t4.join();
    auto rwlock_total_end = std::chrono::system_clock::now();
    std::cout << "rwlock total read/write time passed:" << (rwlock_total_end - rwlock_total_start).count() << std::endl;
}

TEST(dbuffer, write)
{
}