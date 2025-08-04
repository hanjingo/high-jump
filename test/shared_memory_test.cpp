#include <thread>

#include <gtest/gtest.h>
#include <libcpp/os/process.hpp>
#include <libcpp/sync/shared_memory.hpp>

TEST (shared_memory, size)
{
    libcpp::shared_memory::remove ("mem");
    libcpp::shared_memory shm{"mem", 256};
    ASSERT_EQ (shm.size (), 256);
}

TEST (shared_memory, addr)
{
    libcpp::shared_memory::remove ("mem");
    libcpp::shared_memory shm{"mem", 256};
    ASSERT_EQ (shm.addr () == nullptr, true);
}

TEST (shared_memory, remove)
{
    libcpp::shared_memory::remove ("mem");
    libcpp::shared_memory shm{"mem", 256};
    libcpp::shared_memory::remove ("mem");
}

TEST (shared_memory, map)
{
    libcpp::shared_memory::remove ("mem");
    libcpp::shared_memory shm{"mem", 256};
    ASSERT_EQ (shm.addr () == nullptr, true);

    auto ptr = shm.map ();
    ASSERT_EQ (ptr != nullptr, true);
    ASSERT_EQ (shm.addr () != nullptr, true);
    ASSERT_EQ (shm.addr () == ptr, true);
}

// TEST(shared_memory, producer_1_consume_n)
// {
//     int count = 0;
//     for (int i = 1; i <= 10; i++)
//     {
//         count += i;
//     }

//     libcpp::shared_memory::remove("prod");
//     libcpp::shared_memory::remove("result1");
//     libcpp::shared_memory::remove("result2");
//     libcpp::shared_memory::remove("result3");

//     // prevent OS auto recycle
//     libcpp::shared_memory prod{"prod", 256};
//     ASSERT_EQ(prod.map() != nullptr, true);
//     libcpp::shared_memory result1{"result1", 256};
//     ASSERT_EQ(result1.map() != nullptr, true);
//     libcpp::shared_memory result2{"result2", 256};
//     ASSERT_EQ(result2.map() != nullptr, true);
//     libcpp::shared_memory result3{"result3", 256};
//     ASSERT_EQ(result3.map() != nullptr, true);

//     // start producer
//     libcpp::process::spawn("./shm_producer --key=prod");

//     // start consumers
//     libcpp::process::spawn("./shm_consumer --key=prod --result=result1");
//     libcpp::process::spawn("./shm_consumer --key=prod --result=result2");
//     libcpp::process::spawn("./shm_consumer --key=prod --result=result3");

//     // wait producer write
//     std::this_thread::sleep_for(std::chrono::milliseconds(300));

//     int* ptr1 = static_cast<int*>(result1.addr());
//     int* ptr2 = static_cast<int*>(result2.addr());
//     int* ptr3 = static_cast<int*>(result3.addr());

//     ASSERT_EQ(*ptr1, count);
//     ASSERT_EQ(*ptr2, count);
//     ASSERT_EQ(*ptr3, count);
// }