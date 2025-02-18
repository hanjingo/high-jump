#include <iostream>
#include <string>

#include <libcpp/os/signal.hpp>

void handler1(int sig)
{
    std::cout << "handler1 on sig:" << sig << std::endl;
}

void handler2(int sig)
{
    std::cout << "handler2 on sig:" << sig << std::endl;
}

int main(int argc, char* argv[])
{
    // signal(SIGINT, handler1);

    libcpp::sigcatch({SIGABRT}, [](int sig){ std::cout << "lambda on sig:" << sig << std::endl; }, true);

    libcpp::sigcatch({SIGILL}, std::bind(handler2, std::placeholders::_1), 1);

    // raise(SIGINT);

    libcpp::sig_raise(SIGABRT, SIGILL);

    std::cout << "last signal = " << libcpp::last_signal << std::endl;

    std::cin.get();
    return 0;
}