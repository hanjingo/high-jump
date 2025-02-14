#include <iostream>
#include <chrono>

#include <libcpp/sync/process.hpp>

int main(int argc, char* argv[])
{
    // boost::process::system("echo hello");

    // boost::process::system("echo world", boost::process::std_out > "001.txt");

    // boost::process::child p1("echo child1>hello");
    
    // boost::process::child p2("ping 192.168.0.1");
    // if (!p2.wait_for(std::chrono::seconds(3)))
    // {
    //     std::cout << "wait p2 timeout, terminated!" << std::endl;
    //     p2.terminate();
    // }

    // std::cout << "this_process=" << boost::this_process::get_id() << std::endl;

    std::cin.get();
    return 0;
}