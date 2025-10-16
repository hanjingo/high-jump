#include "dll_example.h"
#include <iostream>

int hello()
{
    std::cout << "dll hello" << std::endl;
    return 1;
}

int world()
{
    std::cout << "dll world" << std::endl;
    return 2;
}