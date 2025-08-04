#include <iostream>

#include <libcpp/os/application.hpp>
#include <libcpp/os/process.hpp>

int main(int argc, char* argv[])
{
    boost::process::pstream p;
    std::string hello("hello");
    p << hello;

    std::cout << "hello" << std::endl;
    return 0;
}