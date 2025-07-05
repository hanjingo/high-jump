#include <iostream>
#include <thread>
#include <libcpp/os/process.hpp>

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    while (true)
    {
        std::cout << "daemon watching..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return 0;
}