#include <iostream>

#include <libcpp/hardware/cpu.h>

int main(int argc, char* argv[])
{
    std::cout << "ARCH = " << ARCH << std::endl;

    std::cout << "cpu_cores() = " << cpu_cores() << std::endl;

    std::cout << "cpu_bind(0) = " << cpu_bind(0) << std::endl;

    std::cin.get();
    return 0;
}