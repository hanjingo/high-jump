#include <iostream>

#include <libcpp/os/process.hpp>

int main(int argc, char* argv[])
{
    while (true)
    {
        std::cout << "daemon watching..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return 0;
}