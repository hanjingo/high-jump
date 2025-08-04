#include <iostream>
#include <thread>

#include <libcpp/os/application.hpp>
#include <libcpp/os/process.hpp>

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    boost::process::pstream p;
    std::string hello("hello");
    p << hello;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return 0;
}