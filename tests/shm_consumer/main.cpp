#include <iostream>
#include <thread>
#include <exception>
#include <libcpp/sync/shared_memory.hpp>
#include <libcpp/os/application.hpp>

int main(int argc, char* argv[])
{
    libcpp::options opt;
    opt.add<std::string>("key", std::string("not found"));
    opt.add<std::string>("result", std::string("not found"));
    std::string key = opt.parse<std::string>(argc, argv, "key");
    std::string key_result = opt.parse<std::string>(argc, argv, "result");

    libcpp::shared_memory shm{key.c_str(), 256};
    if (shm.map() == nullptr)
    {
        throw std::runtime_error("shm addr is nullptr");
        return 0;
    }

    libcpp::shared_memory::remove(key_result.c_str());
    libcpp::shared_memory shm_result{key_result.c_str(), 256};
    if (shm_result.map() == nullptr)
    {
        throw std::runtime_error("shm_result addr is nullptr");
        return 0;
    }

    int count = 0;
    int* ptr = static_cast<int*>(shm.addr());
    int old = *ptr;
    while (old < 50)
    {
        if (*ptr == old)
            continue;

        old = *ptr;
        count += old;
    }

    int* result = static_cast<int*>(shm_result.addr());
    *result = count;
    return 0;
}