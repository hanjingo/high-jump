#include <iostream>
#include <string>

#include <libcpp/sync/coroutine.hpp>

void f1(libcpp::coroutine<std::string>::push_type& out)
{
    out("hello");
    out("world");
    out("c++");
}

int main(int argc, char* argv[])
{
    std::cout << "coroutine send msg to main example >>" << std::endl;
    libcpp::coroutine<std::string>::pull_type co_1(f1);

    std::cout << "co_1 get arg = " << co_1.get() << std::endl;
    co_1();

    std::cout << "co_1 get arg = " << co_1.get() << std::endl;
    co_1();

    std::cout << "co_1 get arg = " << co_1.get() << std::endl;
    co_1();


    std::cout << "\ncoroutine send msg to main by lambda example >>"
              << std::endl;
    libcpp::coroutine<int>::pull_type co_2(
        [](libcpp::coroutine<int>::push_type& out) {
            out(1);
            out(2);
            out(3);
        });
    while (co_2)
    {
        std::cout << "co_2 get arg = " << co_2.get() << std::endl;
        co_2();
    }

    std::cout << "\nmain send msg to coroutine by lambda example >>"
              << std::endl;
    libcpp::coroutine<int>::push_type co_3(
        [](libcpp::coroutine<int>::pull_type& in) {
            std::cout << "co_3 get arg = " << in.get() << std::endl;
            in();
            std::cout << "co_3 get arg = " << in.get() << std::endl;
            in();
        });
    co_3(4);
    co_3(5);

    std::cout << "\ncoroutine iterator example1 >>" << std::endl;
    libcpp::coroutine<int>::push_type co_4(
        [](libcpp::coroutine<int>::pull_type& in) {
            for (auto itr = begin(in); itr != end(in); ++itr)
            {
                std::cout << "co_4 get arg = " << *itr << std::endl;
            }
        });
    co_4(0);
    co_4(1);
    co_4(2);

    std::cout << "\ncoroutine iterator example2 >>" << std::endl;
    libcpp::coroutine<int>::push_type co_5(
        [](libcpp::coroutine<int>::pull_type& in) {
            for (auto e : in)
            {
                std::cout << "co_5 get arg = " << e << std::endl;
            }
        });
    co_5(0);
    co_5(1);
    co_5(2);

    std::cin.get();
    return 0;
}