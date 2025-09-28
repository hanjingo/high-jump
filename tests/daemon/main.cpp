#include <iostream>
#include <fstream>
#include <thread>
#include <hj/os/process.hpp>

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    std::ofstream fout("daemon_test.txt");
    fout << "daemon running" << std::endl;
    fout.close();

    while(true)
    {
        std::cout << "daemon watching..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return 0;
}