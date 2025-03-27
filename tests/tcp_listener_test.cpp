#include <gtest/gtest.h>
#include <libcpp/net/tcp.hpp>

TEST(tcp_listener, tcp_listener)
{
    libcpp::tcp_socket::io_t io;
    std::thread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        libcpp::tcp_socket sock1{io};
        sock1.connect("127.0.0.1", 10090);
        sock1.close();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        libcpp::tcp_socket sock2{io};
        sock2.connect("127.0.0.1", 10090);
        sock2.close();
    }).detach();

    libcpp::tcp_listener listener{io};
    listener.async_accept(10090, [](const libcpp::tcp_listener::err_t& err, libcpp::tcp_socket* sock){
        ASSERT_EQ(sock != nullptr, true);
        sock->close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}