#include <iostream>
#include <thread>

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    std::cout << "hello child" << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return 0;
}