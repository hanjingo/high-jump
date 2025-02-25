#include <iostream>
#include <thread>
#include <libcpp/os/process.hpp>
#include <libcpp/os/application.hpp>

int main(int argc, char* argv[])
{
    // boost::process::pstream p;
    // std::string hello("hello");
    // p << hello;

    std::cout << "hello" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return 0;
}