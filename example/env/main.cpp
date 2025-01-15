#include <iostream>
#include <libcpp/os/env.hpp>

int main(int argc, char* argv[])
{
    std::cout << "YEAR = " << YEAR << std::endl;
    std::cout << "MONTH = " << MONTH << std::endl;
    std::cout << "DAY = " << DAY << std::endl;
    std::cout << "HOUR = " << HOUR << std::endl;
    std::cout << "MINUTE = " << MINUTE << std::endl;
    std::cout << "SECONDS = " << SECONDS << std::endl;
    std::cout << "DATE_TIME = " << std::string(DATE_TIME) << std::endl;

    std::cout << "getenv(\"PATH\") = " << libcpp::getenv("PATH") << std::endl;
    
    std::cin.get();
    return 0;
}