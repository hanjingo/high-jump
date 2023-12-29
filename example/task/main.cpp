#include <iostream>

#include <libcpp/io/task.hpp>

int main(int argc, char* argv[])
{
    auto task1 = libcpp::task::create();
    task1->then([](libcpp::task * task, std::string name) {
        std::cout << "task 1_1 with name = " << name << ", size = " << task->size() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }, task1, std::string("he"))
    ->then([](libcpp::task * task) {
        std::cout << "task 1_2" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        task->skip_next();
    }, task1)
    ->then([]() {
        std::cout << "task 1_3" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    })
    ->then([&](libcpp::task * task) {
        std::cout << "task 1_4" << " with size = " << task->size() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        task1->skip_all();
    }, task1)
    ->then([]() {
        std::cout << "task 1_5" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    })
    ->then([]() {
        std::cout << "task 1_6" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    });

    auto task2 = libcpp::task::create([](int age) {
        std::cout << "task 2_1 with age = " << age << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }, 30);
    task2->then([&]() {
        std::cout << "task 2_2" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        task2->skip_next();
    })
    ->then([]() {
        std::cout << "task 2_3" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    })
    ->then([&]() {
        std::cout << "task 2_4" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        task2->skip_all();
    })
    ->then([]() {
        std::cout << "task 2_5" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    })
    ->then([&]() {
        std::cout << "task 2_6" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        task2->skip_all();
    });

    std::thread t1([&]() {
        task1->run();
    });
    t1.detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::thread t2([&]() {
        task2->run();
    });
    t2.detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    task2->pause();
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    task2->resume();

    std::cin.get();
    return 0;
}
