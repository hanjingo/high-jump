#include <iostream>
#include <thread>
#include <hj/sync/shared_memory.hpp>
#include <hj/os/options.hpp>

int main(int argc, char* argv[])
{
    hj::options opt;
    opt.add<std::string>("key", std::string("not found"));
    std::string key = opt.parse<std::string>(argc, argv, "key");

    hj::shared_memory shm{key.c_str(), 256};
    if (shm.map() == nullptr)
    {
        throw std::runtime_error("shm addr is nullptr");
        return 0;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    int* ptr = static_cast<int*>(shm.addr());
    for (int i = 1; i <= 10; i++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        *ptr = i;
    }

    return 0;
}