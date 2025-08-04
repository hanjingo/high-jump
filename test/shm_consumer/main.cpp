#include <exception>
#include <iostream>
#include <thread>

#include <libcpp/os/application.hpp>
#include <libcpp/sync/shared_memory.hpp>

int main(int argc, char* argv[])
{
    libcpp::options opt;
    opt.add<std::string>("key", std::string("not found"));
    opt.add<std::string>("result", std::string("not found"));
    std::string key = opt.parse<std::string>(argc, argv, "key");
    std::string key_result = opt.parse<std::string>(argc, argv, "result");

    // read
    libcpp::shared_memory shm{ key.c_str(), 256 };
    if (shm.map() == nullptr)
    {
        throw std::runtime_error("shm addr is nullptr");
        return 0;
    }

    int count = 0;
    int* ptr = static_cast<int*>(shm.addr());
    int old = *ptr;
    while (old < 10)
    {
        if (*ptr == old)
            continue;

        old = *ptr;
        count += old;
    }

    // write
    libcpp::shared_memory shm_result{ key_result.c_str(), 256 };
    if (shm_result.map() == nullptr)
    {
        throw std::runtime_error("shm_result addr is nullptr");
        return 0;
    }
    int* result = static_cast<int*>(shm_result.addr());
    *result = count;
    std::cout << "consumer write *result=" << *result << std::endl;
    return 0;
}