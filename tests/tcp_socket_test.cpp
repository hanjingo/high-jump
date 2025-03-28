#include <gtest/gtest.h>
#include <libcpp/net/tcp.hpp>

// TEST(tcp_socket, sig_catch)
// {
//     static int sigill_lambda_entryed_times = 0;
//     static int sigint_lambda_entryed_times = 0;
//     static int last_sig = 0;

//     libcpp::tcp_socket::io_t io;
//     libcpp::tcp_socket sock{io};

//     sock.signal_catch({SIGILL}, [](const libcpp::tcp_socket::err_t& err, libcpp::tcp_socket::signal_t sig)->bool{
//         std::cout << "sigill_lambda_entryed_times++" << std::endl;
//         sigill_lambda_entryed_times++;
//         last_sig = sig;
//         return true;
//     }, true);

//     raise(SIGILL);
//     io.run();
//     ASSERT_EQ(sigill_lambda_entryed_times == 1, true);
//     ASSERT_EQ(sigint_lambda_entryed_times == 0, true);
//     ASSERT_EQ(last_sig == SIGILL, true);

//     raise(SIGILL);
//     io.run();
//     ASSERT_EQ(sigill_lambda_entryed_times == 1, true);
//     ASSERT_EQ(sigint_lambda_entryed_times == 0, true);
//     ASSERT_EQ(last_sig == SIGILL, true);

//     sock.signal_catch({SIGINT, SIGILL}, [](const libcpp::tcp_socket::err_t& err, libcpp::tcp_socket::signal_t sig)->bool{
//         std::cout << "sigint_lambda_entryed_times++" << std::endl;
//         sigint_lambda_entryed_times++;
//         last_sig = sig;
//         return true;
//     }, true);
//     raise(SIGINT);
//     io.run();
//     ASSERT_EQ(sigill_lambda_entryed_times == 1, true);
//     ASSERT_EQ(sigint_lambda_entryed_times == 1, true);
//     ASSERT_EQ(last_sig == SIGINT, true);

//     raise(SIGILL);
//     io.run();
//     ASSERT_EQ(sigill_lambda_entryed_times == 1, true);
//     ASSERT_EQ(sigint_lambda_entryed_times == 1, true);
//     ASSERT_EQ(last_sig == SIGINT, true);

//     sock.signal_catch({SIGINT, SIGILL}, [](const libcpp::tcp_socket::err_t& err, libcpp::tcp_socket::signal_t sig)->bool{
//         std::cout << "sigint_lambda_entryed_times++" << std::endl;
//         sigint_lambda_entryed_times++;
//         last_sig = sig;
//         return true;
//     }, false);
//     raise(SIGINT);
//     io.run();
//     ASSERT_EQ(sigill_lambda_entryed_times == 1, true);
//     ASSERT_EQ(sigint_lambda_entryed_times == 2, true);
//     ASSERT_EQ(last_sig == SIGINT, true);

//     raise(SIGILL);
//     io.run();
//     ASSERT_EQ(sigill_lambda_entryed_times == 1, true);
//     ASSERT_EQ(sigint_lambda_entryed_times == 3, true);
//     ASSERT_EQ(last_sig == SIGILL, true);

//     raise(SIGTERM);
//     io.run();
//     ASSERT_EQ(sigill_lambda_entryed_times == 1, true);
//     ASSERT_EQ(sigint_lambda_entryed_times == 3, true);
//     ASSERT_EQ(last_sig == SIGILL, true);
// }

TEST(tcp_socket, set_option)
{
    // libcpp::tcp_socket::io_t io;
    // libcpp::tcp_socket sock{io};

    // sock.set_option(libcpp::tcp_socket::opt_no_delay(true));
    // sock.set_option(libcpp::tcp_socket::opt_send_buf_sz(1024));
    // sock.set_option(libcpp::tcp_socket::opt_recv_buf_sz(1024));
    // sock.set_option(libcpp::tcp_socket::opt_reuse_addr(true));
    // sock.set_option(libcpp::tcp_socket::opt_keep_alive(false));
}

TEST(tcp_socket, connect)
{
    std::thread t([]() {
        libcpp::tcp_socket::io_t io;
        libcpp::tcp_listener li{io};
        for (int i = 0; i < 2; i++)
        {
            if (i == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            auto sock = li.accept(10091);
            ASSERT_EQ(sock != nullptr, true);
            sock->close();
            delete sock;
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    libcpp::tcp_socket::io_t io;
    libcpp::tcp_socket sock{io};
    ASSERT_EQ(sock.connect("127.0.0.1", 10091), false);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ASSERT_EQ(sock.connect("127.0.0.1", 10091), true);
    t.join();
}

TEST(tcp_socket, async_connect)
{
    std::thread t([]() {
        libcpp::tcp_socket::io_t io;
        libcpp::tcp_listener li{io};
        for (int i = 0; i < 2; i++)
        {
            if (i == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            auto sock = li.accept(10091);
            ASSERT_EQ(sock != nullptr, true);
            sock->close();
            delete sock;
        }
    });

    libcpp::tcp_socket::io_t io;
    libcpp::tcp_socket sock{io};
    static bool lambda1_entryed = false;
    sock.async_connect("127.0.0.1", 10091, 
        [](const libcpp::tcp_socket::err_t& err, libcpp::tcp_socket* sock) {
            ASSERT_EQ(err.failed(), true);
            ASSERT_EQ(sock != nullptr, true);
            lambda1_entryed = true;
    });
    io.run();

    static bool lambda2_entryed = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    sock.async_connect("127.0.0.1", 10091, 
        [](const libcpp::tcp_socket::err_t& err, libcpp::tcp_socket* sock) {
            ASSERT_EQ(!err.failed(), true);
            ASSERT_EQ(sock != nullptr, true);
            lambda2_entryed = true;
    });
    io.run();

    t.join();
    io.run();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ASSERT_EQ(lambda1_entryed, true);
    ASSERT_EQ(lambda2_entryed, true);
}