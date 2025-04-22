#include <gtest/gtest.h>
#include <libcpp/os/process.hpp>
#include <libcpp/sync/shared_memory.hpp>

TEST(shared_memory, size)
{
    libcpp::shared_memory::remove("mem");
    libcpp::shared_memory shm{"mem", 256};
    ASSERT_EQ(shm.size(), 256);

    shm.truncate(512);
    ASSERT_EQ(shm.size(), 512);
}

TEST(shared_memory, addr)
{
    libcpp::shared_memory::remove("mem");
    libcpp::shared_memory shm{"mem", 256};
    ASSERT_EQ(shm.addr() == nullptr, true);
}

TEST(shared_memory, remove)
{
    libcpp::shared_memory::remove("mem");
    libcpp::shared_memory shm{"mem", 256};
    libcpp::shared_memory::remove("mem");
}

TEST(shared_memory, truncate)
{
    libcpp::shared_memory::remove("mem");
    libcpp::shared_memory shm{"mem", 256};
    ASSERT_EQ(shm.size(), 256);

    shm.truncate(512);
    ASSERT_EQ(shm.size(), 512);

    shm.truncate(0);
    ASSERT_EQ(shm.size(), 0);
}

TEST(shared_memory, map)
{
    libcpp::shared_memory::remove("mem");
    libcpp::shared_memory shm{"mem", 256};
    ASSERT_EQ(shm.addr() == nullptr, true);

    auto ptr = shm.map();
    ASSERT_EQ(ptr != nullptr, true);
    ASSERT_EQ(shm.addr() != nullptr, true);
    ASSERT_EQ(shm.addr() == ptr, true);
}

TEST(shared_memory, producer_1_consume_n)
{
    int count = 0;
    for (int i = 1; i <= 50; i++)
    {
        count += i;
    }

    libcpp::process::spawn("./shm_producer --key=prod1");
    // std::this_thread::sleep_for(std::chrono::milliseconds(5));
    libcpp::process::spawn("./shm_consumer --key=prod1 --result=result1");
    libcpp::process::spawn("./shm_consumer --key=prod1 --result=result2");

    // std::this_thread::sleep_for(std::chrono::milliseconds(600));
    libcpp::shared_memory shm1{"result1", 256};
    libcpp::shared_memory shm2{"result2", 256};
    shm1.map();
    shm2.map();
    int* ptr1 = static_cast<int*>(shm1.addr());
    int* ptr2 = static_cast<int*>(shm2.addr());

    ASSERT_EQ(*ptr1 > 0, true);
    ASSERT_EQ(*ptr2 > 0, true);

    libcpp::shared_memory::remove("result1");
    libcpp::shared_memory::remove("result2");
}