#ifndef HELLO_HPP
#define HELLO_HPP

#include <iostream>
#include <libcpp/util/init.hpp>

INIT(
    std::cout << "init hello.hpp" << std::endl;
);

#endif