#include <chrono>
#include <iostream>
#include <thread>

#include <libcpp/sync/thread_pool.hpp>

int main(int argc, char* argv[])
{
    libcpp::thread_pool pool(2);

    pool.enqueue([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::cout << "thread cleanned" << std::endl;
    });
    pool.clear();

    auto ret = pool.enqueue([]() { return 1; });
    std::cout << "thread1 ret = " << ret.get() << std::endl;

    pool.enqueue([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::cout << "thread2" << std::endl;
    });

    pool.enqueue([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cout << "thread3" << std::endl;
    });

    // this thread will be blocked until pool not full.
    pool.enqueue([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "thread4" << std::endl;
    });

    std::cin.get();
    return 0;
}