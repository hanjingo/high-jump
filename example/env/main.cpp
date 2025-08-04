#include <iostream>

#include <libcpp/os/env.h>

int main(int argc, char* argv[])
{
    std::cout << "YEAR = " << COMPILE_YEAR << std::endl;
    std::cout << "MONTH = " << COMPILE_MONTH << std::endl;
    std::cout << "DAY = " << COMPILE_DAY << std::endl;
    std::cout << "HOUR = " << COMPILE_HOUR << std::endl;
    std::cout << "MINUTE = " << COMPILE_MINUTE << std::endl;
    std::cout << "SECONDS = " << COMPILE_SECONDS << std::endl;
    std::cout << "COMPILE_TIME = " << std::string(COMPILE_TIME) << std::endl;

    std::cin.get();
    return 0;
}