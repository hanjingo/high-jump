#include <gtest/gtest.h>
#include <libcpp/net/tcp.hpp>
#include <csignal>

// TEST(tcp_listener, set_option)
// {
//     libcpp::tcp_listener::io_t io;
//     libcpp::tcp_listener li{io};
//     ASSERT_EQ(li.set_option(libcpp::tcp_socket::opt_reuse_addr(true)), false);

//     // li.open(boost::asio::ip::tcp::v4());
//     // ASSERT_EQ(li.set_option(libcpp::tcp_socket::opt_reuse_addr(true)), true);
// }

// TEST(tcp_listener, accept)
// {
//     std::thread t([]() {
//         libcpp::tcp_socket::io_t io;
//         libcpp::tcp_listener li{io};
//         for (int i = 0; i < 2; i++)
//         {
//             auto sock = li.accept(10091);
//             ASSERT_EQ(sock != nullptr, true);
//             sock->close();
//         }
//         li.close();
//     });

//     std::this_thread::sleep_for(std::chrono::milliseconds(10));
//     libcpp::tcp_socket::io_t io;
//     libcpp::tcp_socket sock{io};
//     ASSERT_EQ(sock.connect("127.0.0.1", 10091), true);

//     libcpp::tcp_socket sock1{io};
//     ASSERT_EQ(sock1.connect("127.0.0.1", 10091), true);
//     t.join();
// }

// TEST(tcp_listener, async_accept)
// {
//     static int async_accept_times = 0;
//     std::thread t([](){
//         libcpp::tcp_socket::io_t io;
//         libcpp::tcp_listener li{io};
//         for (int i = 0; i < 2; i++)
//         {
//             li.async_accept(10091, [](const libcpp::tcp_listener::err_t& err, libcpp::tcp_socket* sock) {
//                 ASSERT_EQ(err.failed(), false);
//                 ASSERT_EQ(sock->is_connected(), true);
//                 ASSERT_EQ(sock->check_connected(), true);
//                 async_accept_times++;
//             });
//         }

//         io.run_for(std::chrono::milliseconds(100));
//     });

//     std::this_thread::sleep_for(std::chrono::milliseconds(10));
//     libcpp::tcp_socket::io_t io;
//     libcpp::tcp_socket sock{io};
//     sock.async_connect("127.0.0.1", 10091, [&](const libcpp::tcp_socket::err_t& err, libcpp::tcp_socket* sock){
//         ASSERT_EQ(err.failed(), false);
//         ASSERT_EQ(sock != nullptr, true);
//         ASSERT_EQ(sock->is_connected(), true);
//     });

//     libcpp::tcp_socket sock1{io};
//     sock1.async_connect("127.0.0.1", 10091, [&](const libcpp::tcp_socket::err_t& err, libcpp::tcp_socket* sock){
//         ASSERT_EQ(err.failed(), false);
//         ASSERT_EQ(sock != nullptr, true);
//         ASSERT_EQ(sock->is_connected(), true);
//     });

//     io.run();
//     t.join();
//     ASSERT_EQ(async_accept_times, 2);
// }

// TEST(tcp_listener, close)
// {
//     libcpp::tcp_socket::io_t io;
//     libcpp::tcp_listener li{io};
//     ASSERT_EQ(li.is_closed(), false);
//     li.close();
//     ASSERT_EQ(li.is_closed(), true);
//     li.close();
//     ASSERT_EQ(li.is_closed(), true);
// }

// // TEST(tcp_listener, add_signal)
// // {
// //     libcpp::tcp_socket::io_t io;
// //     libcpp::tcp_listener li{io};
// //     static libcpp::tcp_listener::signal_t last_sig;
// //     li.add_signal(SIGINT, [](const libcpp::tcp_listener::err_t& err, libcpp::tcp_listener::signal_t sig){
// //         ASSERT_EQ(sig == SIGINT, true);
// //         last_sig = sig;
// //     });
// //     raise(SIGINT);
// //     io.run();

// //     li.add_signal(SIGILL, [](const libcpp::tcp_listener::err_t& err, libcpp::tcp_listener::signal_t sig){
// //         ASSERT_EQ(sig == SIGILL, true);
// //         last_sig = sig;
// //     });
// //     raise(SIGILL);
// //     io.run();

// //     ASSERT_EQ(last_sig == SIGILL, true);
// // }