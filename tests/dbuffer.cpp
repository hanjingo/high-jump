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

    int times = 100000000;
    auto start = std::chrono::system_clock::now();
    for (int n = 0; n < times; ++n)
    {
        dbuf.write(n);
        dbuf.read(j);
    }
    auto end = std::chrono::system_clock::now();
    std::cout << "dbuffer time passed:" << (end - start).count() << std::endl;

    std::shared_mutex mu;
    start = std::chrono::system_clock::now();
    for (int n = 0; n < times; ++n)
    {
        mu.lock();
        i = n; // write
        mu.unlock();

        mu.lock_shared();
        j = i; // read
        mu.unlock_shared();
    }
    end = std::chrono::system_clock::now();
    std::cout << "rwmutex time passed:" << (end - start).count() << std::endl;
}

TEST(dbuffer, write)
{
}