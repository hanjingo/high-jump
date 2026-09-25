#include <atomic>
#include <string>
#include <vector>
#include <future>

#include <gtest/gtest.h>

#include <hj/os/process.hpp>
#include <hj/sync/shared_memory.hpp>

#include <thread>

#if defined(_WIN32) || defined(_WIN64)
#include <process.h>
#else
#include <unistd.h>
#endif

namespace
{
std::atomic<unsigned long long> shared_memory_test_counter{0};

std::string unique_shared_memory_name(const char *prefix)
{
#if defined(_WIN32) || defined(_WIN64)
    const auto pid = static_cast<unsigned long long>(::_getpid());
#else
    const auto pid = static_cast<unsigned long long>(::getpid());
#endif

    const auto counter =
        shared_memory_test_counter.fetch_add(1, std::memory_order_relaxed);

    return "/" + std::string(prefix) + "_" + std::to_string(pid) + "_"
           + std::to_string(counter);
}

} // namespace

TEST(shared_memory, size)
{
    const auto name = unique_shared_memory_name("mem");

    hj::shared_memory::remove(name.c_str());

    hj::shared_memory shm{name.c_str(), 256};

    ASSERT_EQ(shm.size(), 256);

    hj::shared_memory::remove(name.c_str());
}

TEST(shared_memory, addr)
{
    const auto name = unique_shared_memory_name("mem");

    hj::shared_memory::remove(name.c_str());

    hj::shared_memory shm{name.c_str(), 256};

    ASSERT_EQ(shm.addr() == nullptr, true);

    hj::shared_memory::remove(name.c_str());
}

TEST(shared_memory, remove)
{
    const auto name = unique_shared_memory_name("mem");

    hj::shared_memory::remove(name.c_str());

    hj::shared_memory shm{name.c_str(), 256};

    hj::shared_memory::remove(name.c_str());
}

TEST(shared_memory, map)
{
    const auto name = unique_shared_memory_name("mem");

    hj::shared_memory::remove(name.c_str());

    hj::shared_memory shm{name.c_str(), 256};

    ASSERT_EQ(shm.addr() == nullptr, true);

    auto ptr = shm.map();

    ASSERT_NE(ptr, nullptr);
    ASSERT_NE(shm.addr(), nullptr);
    ASSERT_EQ(shm.addr(), ptr);

    shm.unmap();

    hj::shared_memory::remove(name.c_str());
}

TEST(shared_memory, create_and_open)
{
    const auto name = unique_shared_memory_name("memtest");

    hj::shared_memory::remove(name.c_str());

    hj::shared_memory shm_create{name.c_str(), 128};

    ASSERT_EQ(shm_create.size(), 128);

    auto ptr = shm_create.map();

    ASSERT_NE(ptr, nullptr);

    int *iptr = static_cast<int *>(ptr);
    iptr[0]   = 42;

    shm_create.unmap();

    hj::shared_memory shm_open{name.c_str(), 128};

    auto ptr2 = shm_open.map();

    ASSERT_NE(ptr2, nullptr);

    int *iptr2 = static_cast<int *>(ptr2);

    ASSERT_EQ(iptr2[0], 42);

    shm_open.unmap();

    hj::shared_memory::remove(name.c_str());
}

TEST(shared_memory, concurrent_map_unmap)
{
    const auto name = unique_shared_memory_name("concurrent");

    hj::shared_memory::remove(name.c_str());

    hj::shared_memory shm{name.c_str(), 128};

    std::vector<std::future<bool>> futs;

    for(int i = 0; i < 8; ++i)
    {
        futs.push_back(std::async(std::launch::async, [&shm]() {
            auto ptr = shm.map();

            if(ptr == nullptr)
                return false;

            if(shm.addr() == nullptr)
                return false;

            shm.unmap();

            return true;
        }));
    }

    for(auto &f : futs)
    {
        ASSERT_TRUE(f.get());
    }

    hj::shared_memory::remove(name.c_str());
}

TEST(shared_memory, boundary)
{
    const auto name = unique_shared_memory_name("boundary");

    hj::shared_memory::remove(name.c_str());

    hj::shared_memory shm{name.c_str(), 1};

    auto ptr = shm.map();

    ASSERT_NE(ptr, nullptr);

    char *cptr = static_cast<char *>(ptr);

    cptr[0] = 'A';

    shm.unmap();

    hj::shared_memory::remove(name.c_str());
}

// TEST(shared_memory, producer_1_consume_n)
// {
//     int count = 0;
//     for(int i = 1; i <= 10; i++)
//     {
//         count += i;
//     }

//     hj::shared_memory::remove("prod");
//     hj::shared_memory::remove("result1");
//     hj::shared_memory::remove("result2");
//     hj::shared_memory::remove("result3");

//     // prevent OS auto recycle
//     hj::shared_memory prod{"prod", 256};
//     ASSERT_EQ(prod.map() != nullptr, true);

//     hj::shared_memory result1{"result1", 256};
//     ASSERT_EQ(result1.map() != nullptr, true);

//     hj::shared_memory result2{"result2", 256};
//     ASSERT_EQ(result2.map() != nullptr, true);

//     hj::shared_memory result3{"result3", 256};
//     ASSERT_EQ(result3.map() != nullptr, true);

//     // start producer
//     hj::process::spawn("./shm_producer --key=prod");

//     // start consumers
//     hj::process::spawn("./shm_consumer --key=prod --result=result1");
//     hj::process::spawn("./shm_consumer --key=prod --result=result2");
//     hj::process::spawn("./shm_consumer --key=prod --result=result3");

//     // wait producer write
//     std::this_thread::sleep_for(std::chrono::milliseconds(300));

//     int *ptr1 = static_cast<int *>(result1.addr());
//     int *ptr2 = static_cast<int *>(result2.addr());
//     int *ptr3 = static_cast<int *>(result3.addr());

//     ASSERT_EQ(*ptr1, count);
//     ASSERT_EQ(*ptr2, count);
//     ASSERT_EQ(*ptr3, count);
// }