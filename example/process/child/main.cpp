#include <iostream>
#include <libcpp/os/process.hpp>
#include <libcpp/os/application.hpp>

int main(int argc, char* argv[])
{
    boost::process::pstream p;
    std::string hello("hello");
    p << hello;

    std::cout << "hello" << std::endl;
    return 0;
}