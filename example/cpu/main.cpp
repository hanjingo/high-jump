#include <iostream>

#include <libcpp/os/cpu.hpp>

int main(int argc, char* argv[])
{
    std::cout << "ARCH = " << ARCH << std::endl;

    std::cout << "cpu_cores() = " << libcpp::cpu_cores() << std::endl;

    std::cout << "bind_cpu_core(0) = " << libcpp::bind_cpu_core(0) << std::endl;

    std::cin.get();
    return 0;
}