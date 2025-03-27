#include <gtest/gtest.h>
#include <libcpp/net/tcp.hpp>

TEST(tcp_socket, tcp_socket)
{
    libcpp::tcp_socket::io_t io;
    std::thread([&]() {
        char buf1[1024] = {0};
        libcpp::tcp_listener listener{io};
        auto sock = listener.accept(10085);
        sock->recv(buf1, 1024);
        sock->send(std::string("world1").c_str(), 7);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        sock->close();
        delete sock;
    }).detach();

    char buf2[1024] = {0};
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    libcpp::tcp_socket sock1{io};
    ASSERT_EQ(sock1.connect("127.0.0.1", 10085), true);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(sock1.send(std::string("hello").c_str(), 6) == 6, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(sock1.recv(buf2, 1024) == 7, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_EQ(sock1.is_connected(), true);
    sock1.close();
    ASSERT_EQ(sock1.is_connected(), false);
}

TEST(tcp_socket, tcp_socket_async)
{
    libcpp::tcp_socket::io_t io;
    std::thread([&]() {
        char buf1[1024] = {0};
        libcpp::tcp_listener listener{io};
        auto sock = listener.accept(10086);
        sock->recv(buf1, 1024);
        sock->send(std::string("world2").c_str(), 7);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        sock->close();
        delete sock;
    }).detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    libcpp::tcp_socket sock2{io};
    sock2.async_connect(
        "127.0.0.1", 
        10086, 
        [&](const libcpp::tcp_socket::err_t& err, libcpp::tcp_socket* sock){
            ASSERT_EQ(!err.failed(), true);
            ASSERT_EQ(sock != nullptr, true);

            sock2.async_send(
                std::string("hello").c_str(), 
                6,
                [](const libcpp::tcp_socket::err_t& err, std::size_t sz) {
                    ASSERT_EQ(!err.failed(), true);
                    ASSERT_EQ(sz == 6, true);
                }
            );
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            char buf[1024] = {0};
            sock2.async_recv(
                buf, 
                1024,
                [](const libcpp::tcp_socket::err_t& err, std::size_t sz) {
                ASSERT_EQ(!err.failed(), true);
                ASSERT_EQ(sz == 7, true);
            });
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    sock2.close();
    ASSERT_EQ(sock2.is_connected(), false);
}