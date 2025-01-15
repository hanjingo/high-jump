#include <iostream>

#include <functional>

#include <libcpp/time/timer.hpp>

static unsigned long ms = 1000;

void test_timer()
{
    libcpp::timer t6{1000 * ms, []()
    {
        std::cout << "tm6 expired" << std::endl;
    }};
    t6.wait();

    // this temporary timer will be invaliable
    libcpp::timer t7{1000 * ms, []()
    {
        std::cout << "tm7 expired" << std::endl;
    }};
}

void test_global_timer()
{
    // this global timer will continue run
    libcpp::timer::create_global(1000 * ms, []() {
        std::cout << "tm8 expired" << std::endl;
    });
}

int main(int argc, char* argv[])
{
    libcpp::timer t1{1000 * ms, []()
    {
        std::cout << "tm1 expired" << std::endl;
    }};

    libcpp::timer t2{2000 * ms, []()
    {
        std::cout << "tm2 expired" << std::endl;
    }};

    libcpp::timer t3{3000 * ms, []()
    {
        std::cout << "tm3 expired" << std::endl;
    }};
    t3.cancel();

    libcpp::timer t4{4000 * ms, []()
    {
        std::cout << "tm4 expired" << std::endl;
    }};
    t4.reset(1500 * ms);

    libcpp::timer t5{5000 * ms, []()
    {
        std::cout << "tm5 expired" << std::endl;
    }};
    t5.reset(1600 * ms, []() {
        std::cout << "tm5 expired by reset" << std::endl;
    });

    test_timer();
    test_global_timer();

    std::cin.get();
    return 0;
}