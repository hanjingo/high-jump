#include <iostream>

#include <libcpp/net/udp.hpp>

int main(int argc, char* argv[])
{
    std::thread t1([]() {
        char buf[1024] = {0};
        std::cout << "start listening..." << std::endl;

        libcpp::udp_socket sock{};
        sock.bind(10086);
        for (;;)
        {
            libcpp::udp_socket::endpoint_t ep;
            sock.recv(buf, 1024, ep);
            std::cout << "Srv> recv " << std::string(buf) << std::endl;

            std::string msg{"world"};
            sock.send(msg.c_str(), 5, ep);
            std::cout << "Srv> send " << msg << std::endl;
        }
    });
    t1.detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::thread t2([]() {
        char buf[1024] = {0};
        std::cout << "start sync calling..." << std::endl;

        libcpp::udp_socket sock{0};
        
        libcpp::udp_socket::endpoint_t ep = libcpp::udp_socket::endpoint_v4(10086);
        std::string msg{"hello"};
        sock.send(msg.c_str(), 5, ep);
        std::cout << "Cli> send " << msg << std::endl;

        sock.recv(buf, 1024, ep);
        std::cout << "Cli> recv " << std::string(buf) << std::endl;
    });
    t2.detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::thread t3([]() {
        char buf[1024] = {0};
        std::cout << "start async calling..." << std::endl;

        libcpp::udp_socket sock{0};
        
        libcpp::udp_socket::endpoint_t ep = libcpp::udp_socket::endpoint_v4(10086);
        std::string msg{"hello"};
        sock.async_send(msg.c_str(), 5, ep, [&](const libcpp::udp_socket::err_t& err, std::size_t len){
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::cout << "Cli> asyc send " << msg << " with len=" << len << std::endl;
        });
        
        sock.async_recv(buf, 1024, ep, [&](const libcpp::udp_socket::err_t& err, std::size_t len){
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::cout << "Cli> async recv " << std::string(buf) << " with len=" << len << std::endl;
        });
        sock.poll();
    });
    t3.detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cin.get();
    return 0;
}