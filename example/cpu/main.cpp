#include <iostream>

#include <libcpp/os/cpu.hpp>

int main(int argc, char* argv[])
{
    std::cout << "ARCH = " << ARCH << std::endl;

    std::cout << "cpu::cores() = " << libcpp::cpu::cores() << std::endl;

    std::cout << "cpu::bind(0) = " << libcpp::cpu::bind(0) << std::endl;

    std::cin.get();
    return 0;
}