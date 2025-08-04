#include <iostream>

#include <libcpp/util/defer.hpp>

using namespace libcpp;

void doDefer()
{
    std::cout << "defer before" << std::endl;
    DEFER(std::cout << "step1. do defer1" << std::endl;);
    DEFER(std::cout << "step2. do defer2" << std::endl;);
    std::cout << "defer after" << std::endl;
}

int main(int argc, char* argv[])
{
    /*
    defer before
    after before
    step2. do defer2
    step1. do defer1
    */
    doDefer();

    std::cin.get();
    return 0;
}