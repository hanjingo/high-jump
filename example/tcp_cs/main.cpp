#include <iostream>
#include <string>
#include <libcpp/net/tcp/tcp_client.hpp>
#include <libcpp/net/tcp/tcp_server.hpp>

class tcp_msg : public libcpp::message
{
    std::size_t size() {};
    std::size_t encode() {};
    bool decode(const unsigned char* buf, std::size_t len) {};
};

int main(int argc, char* argv[])
{
    std::thread t1([]() {
        char buf[1024] = {0};
        std::cout << "start server1..." << std::endl;
        libcpp::tcp_server srv{};
    });
    t1.detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::thread t2([]() {
        char buf[1024] = {0};
        std::cout << "start server2..." << std::endl;
        libcpp::tcp_server srv{};
    });
    t2.detach();

    std::cout << "\n" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::thread t3([]() {
        char buf[1024] = {0};
        std::cout << "start client1..." << std::endl;
        libcpp::tcp_client<int> cli{};
        cli.connect("192.168.0.1", 10086, 1);

        std::string topic("hello");
        // cli.sub(topic);
    });
    t3.detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::thread t4([]() {
        char buf[1024] = {0};
        std::cout << "start client1..." << std::endl;
        libcpp::tcp_client<std::string> cli{};
        cli.connect("192.168.0.1", 10086, "conn1");

        // tcp_msg msg; 
        // std::string topic("hello");
        // cli.pub(topic, msg);
    });
    t4.detach();

    std::cin.get();
    return 0;
}