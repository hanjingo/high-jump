#include <iostream>

#include "hello.hpp"
#include "world.hpp"
#include <libcpp/util/init.hpp>

INIT(std::cout << "init 1" << std::endl;);
INIT(std::cout << "init 2" << std::endl;);

int main(int argc, char* argv[])
{
    std::cout << "main start" << std::endl;

    // init hello.hpp
    // init world.hpp
    // init 1
    // init 2
    // main start
    std::cin.get();
    return 0;
}