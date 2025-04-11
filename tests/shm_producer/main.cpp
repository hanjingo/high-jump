#include <iostream>
#include <thread>
#include <libcpp/sync/shared_memory.hpp>
#include <libcpp/os/application.hpp>

int main(int argc, char* argv[])
{
    libcpp::options opt;
    opt.add<std::string>("key", std::string("not found"));
    std::string key = opt.parse<std::string>(argc, argv, "key");

    libcpp::shared_memory::remove(key.c_str());
    libcpp::shared_memory shm{key.c_str(), 256};
    shm.truncate(256);
    if (shm.map() == nullptr)
    {
        throw std::runtime_error("shm addr is nullptr");
        return 0;
    }
    int* ptr = static_cast<int*>(shm.addr());
    for (int i = 1; i <= 50; i++)
    {
        *ptr = i;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}