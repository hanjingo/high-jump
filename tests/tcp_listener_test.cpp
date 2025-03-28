#include <gtest/gtest.h>
#include <libcpp/net/tcp.hpp>
#include <csignal>

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

// TEST(tcp_listener, add_signal)
// {
//     libcpp::tcp_socket::io_t io;
//     libcpp::tcp_listener li{io};
//     static libcpp::tcp_listener::signal_t last_sig;
//     li.add_signal(SIGINT, [](const libcpp::tcp_listener::err_t& err, libcpp::tcp_listener::signal_t sig){
//         ASSERT_EQ(sig == SIGINT, true);
//         last_sig = sig;
//     });
//     raise(SIGINT);
//     io.run();

//     li.add_signal(SIGILL, [](const libcpp::tcp_listener::err_t& err, libcpp::tcp_listener::signal_t sig){
//         ASSERT_EQ(sig == SIGILL, true);
//         last_sig = sig;
//     });
//     raise(SIGILL);
//     io.run();

//     ASSERT_EQ(last_sig == SIGILL, true);
// }