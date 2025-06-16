#include <gtest/gtest.h>
#include <libcpp/net/tcp.hpp>
#include <libcpp/testing/stacktrace.hpp>

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
    libcpp::tcp_socket::io_t io;
    libcpp::tcp_socket sock{io};
    ASSERT_EQ(sock.set_option(libcpp::tcp_socket::opt_no_delay(true)), false);
    ASSERT_EQ(sock.set_option(libcpp::tcp_socket::opt_send_buf_sz(1024)), false);
    ASSERT_EQ(sock.set_option(libcpp::tcp_socket::opt_recv_buf_sz(1024)), false);
    ASSERT_EQ(sock.set_option(libcpp::tcp_socket::opt_reuse_addr(true)), false);
    ASSERT_EQ(sock.set_option(libcpp::tcp_socket::opt_keep_alive(false)), false);
    // ASSERT_EQ(sock.set_option(libcpp::tcp_socket::opt_broadcast(false)), false);

    libcpp::tcp_socket::sock_t* base1 = new libcpp::tcp_socket::sock_t(io);
    libcpp::tcp_socket sock1{io, base1};
    ASSERT_EQ(sock1.set_option(libcpp::tcp_socket::opt_no_delay(true)), false);
    ASSERT_EQ(sock1.set_option(libcpp::tcp_socket::opt_send_buf_sz(1024)), false);
    ASSERT_EQ(sock1.set_option(libcpp::tcp_socket::opt_recv_buf_sz(1024)), false);
    ASSERT_EQ(sock1.set_option(libcpp::tcp_socket::opt_reuse_addr(true)), false);
    ASSERT_EQ(sock1.set_option(libcpp::tcp_socket::opt_keep_alive(false)), false);
    // ASSERT_EQ(sock1.set_option(libcpp::tcp_socket::opt_broadcast(false)), false);

    libcpp::tcp_socket::sock_t* base2 = new libcpp::tcp_socket::sock_t(io);
    base2->open(boost::asio::ip::tcp::v4());
    libcpp::tcp_socket sock2{io, base2};
    ASSERT_EQ(sock2.set_option(libcpp::tcp_socket::opt_no_delay(true)), true);
    ASSERT_EQ(sock2.set_option(libcpp::tcp_socket::opt_send_buf_sz(1024)), true);
    ASSERT_EQ(sock2.set_option(libcpp::tcp_socket::opt_recv_buf_sz(1024)), true);
    ASSERT_EQ(sock2.set_option(libcpp::tcp_socket::opt_reuse_addr(true)), true);
    ASSERT_EQ(sock2.set_option(libcpp::tcp_socket::opt_keep_alive(false)), true);
    // ASSERT_EQ(sock2.set_option(libcpp::tcp_socket::opt_broadcast(false)), true);
}

TEST(tcp_socket, is_connected)
{
    std::thread t([]() {
        libcpp::tcp_socket::io_t io;
        libcpp::tcp_listener li{io};
        for (int i = 0; i < 1; i++)
        {
            auto sock = li.accept(10091);
            ASSERT_EQ(sock != nullptr, true);
            sock->close();
            delete sock;
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    libcpp::tcp_socket::io_t io;
    libcpp::tcp_socket sock{io};
    ASSERT_EQ(sock.is_connected(), false);
    ASSERT_EQ(sock.connect("127.0.0.1", 10091), true);
    ASSERT_EQ(sock.is_connected(), true);

    t.join();
}

TEST(tcp_socket, check_connected)
{
    std::thread t([]() {
        libcpp::tcp_socket::io_t io;
        libcpp::tcp_listener li{io};
        for (int i = 0; i < 1; i++)
        {
            auto sock = li.accept(10091);
            ASSERT_EQ(sock != nullptr, true);
            sock->close();
            delete sock;
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    libcpp::tcp_socket::io_t io;
    libcpp::tcp_socket sock{io};
    ASSERT_EQ(sock.check_connected(), false);
    ASSERT_EQ(sock.connect("127.0.0.1", 10091), true);
    ASSERT_EQ(sock.check_connected(), true);

    t.join();
}

TEST(tcp_socket, connect)
{
    int accept_times = 0;
    std::thread t([&accept_times]() {
        libcpp::tcp_socket::io_t io;
        libcpp::tcp_listener li{io};
        libcpp::tcp_socket::steady_timer_t tm{io};
        for (int i = 0; i < 3; i++)
        {
            auto sock = li.accept(10091);
            ASSERT_EQ(sock != nullptr, true);
            accept_times++;
            sock->close();
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_socket::io_t io;
    libcpp::tcp_socket sock{io};
    ASSERT_EQ(sock.connect("127.0.0.1", 10091), true);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_socket sock1{io};
    ASSERT_EQ(sock1.connect("127.0.0.1", 10091, std::chrono::milliseconds(30)), true);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_socket sock2{io};
    ASSERT_EQ(sock2.connect("127.0.0.1", 10091, std::chrono::milliseconds(20), 5), true);

    t.join();
    ASSERT_EQ(accept_times == 3, true);
}

TEST(tcp_socket, async_connect)
{
    int accept_times = 0;
    libcpp::tcp_socket::io_t io;
    libcpp::tcp_listener li{io};
    li.async_accept(10091, [&li, &accept_times](const libcpp::tcp_listener::err_t& err, libcpp::tcp_socket* sock){
        ASSERT_EQ(err.failed(), false);
        ASSERT_EQ(sock != nullptr, true);
        accept_times++;
        sock->close();

        li.async_accept(10091, [&accept_times](const libcpp::tcp_listener::err_t& err, libcpp::tcp_socket* sock){
            ASSERT_EQ(err.failed(), false);
            ASSERT_EQ(sock != nullptr, true);
            accept_times++;
            sock->close();
        });
    });

    libcpp::tcp_socket sock{io};
    bool lambda1_entryed = false;
    sock.async_connect("127.0.0.1", 10091, 
        [&lambda1_entryed](const libcpp::tcp_socket::err_t& err) {
            ASSERT_EQ(err.failed(), false);
            lambda1_entryed = true;
    });

    bool lambda2_entryed = false;
    libcpp::tcp_socket sock1{io};
    sock1.async_connect("127.0.0.1", 10091, 
        [&lambda2_entryed](const libcpp::tcp_socket::err_t& err) {
            ASSERT_EQ(err.failed(), false);
            lambda2_entryed = true;
    });

    io.run_for(std::chrono::milliseconds(100));

    ASSERT_EQ(lambda1_entryed, true);
    ASSERT_EQ(lambda2_entryed, true);
    ASSERT_EQ(accept_times == 2, true);
}

TEST(tcp_socket, disconnect)
{
    std::thread t([]() {
        libcpp::tcp_socket::io_t io;
        libcpp::tcp_listener li{io};
        for (int i = 0; i < 1; i++)
        {
            auto sock = li.accept(10091);
            ASSERT_EQ(sock != nullptr, true);
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_socket::io_t io;
    libcpp::tcp_socket sock{io};
    
    ASSERT_EQ(sock.is_connected(), false);
    sock.disconnect();
    ASSERT_EQ(sock.is_connected(), false);
    ASSERT_EQ(sock.connect("127.0.0.1", 10091), true);
    ASSERT_EQ(sock.is_connected(), true);
    sock.disconnect();
    ASSERT_EQ(sock.is_connected(), false);
    sock.disconnect();
    ASSERT_EQ(sock.is_connected(), false);

    t.join();
}

TEST(tcp_socket, send)
{
    std::thread t([]() {
        libcpp::tcp_socket::io_t io;
        libcpp::tcp_listener li{io};
        for (int i = 0; i < 2; i++)
        {
            auto sock = li.accept(10091);
            ASSERT_EQ(sock != nullptr, true);

            unsigned char buf[1024];
            ASSERT_EQ(sock->recv(buf, 1024) == 6, true);
            std::string str(reinterpret_cast<char*>(buf), 5);

            if (i == 0)
                ASSERT_EQ(str == std::string("hello"), true);
            else if (i == 1)
                ASSERT_EQ(str == std::string("harry"), true);
            else
                ASSERT_EQ(true, false);

            sock->close();
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_socket::io_t io;
    libcpp::tcp_socket sock{io};
    ASSERT_EQ(sock.connect("127.0.0.1", 10091), true);
    ASSERT_EQ(sock.send(reinterpret_cast<const unsigned char*>(std::string("hello").c_str()), 6) == 6, true);

    libcpp::tcp_socket sock1{io};
    ASSERT_EQ(sock1.connect("127.0.0.1", 10091), true);
    ASSERT_EQ(sock1.send(reinterpret_cast<const unsigned char*>(std::string("harry").c_str()), 6) == 6, true);

    t.join();
}

TEST(tcp_socket, async_send)
{
    std::thread t([](){
        libcpp::tcp_socket::io_t io;
        libcpp::tcp_listener li{io};
        li.async_accept(10091, [](const libcpp::tcp_listener::err_t& err, libcpp::tcp_socket* sock) {
            ASSERT_EQ(err.failed(), false);
            ASSERT_EQ(sock->is_connected(), true);

            unsigned char buf[1024];
            sock->async_recv(buf, 1024, [sock, &buf](const libcpp::tcp_socket::err_t& err, std::size_t sz){
                ASSERT_EQ(err.failed(), false);
                ASSERT_EQ(sz == 5, true);
                ASSERT_EQ(std::string(reinterpret_cast<char*>(buf), 5) == "hello", true);
                delete sock;
            });
        });

        io.run_for(std::chrono::milliseconds(100));
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_socket::io_t io;
    libcpp::tcp_socket sock{io};
    sock.async_connect("127.0.0.1", 10091, [&sock](const libcpp::tcp_socket::err_t& err){
        ASSERT_EQ(err.failed(), false);
        ASSERT_EQ(sock.is_connected(), true);
        ASSERT_EQ(sock.check_connected(), true);

        sock.async_send(reinterpret_cast<const unsigned char*>(std::string("hello").c_str()), 5, 
            [](const libcpp::tcp_socket::err_t& err, std::size_t sz){
            ASSERT_EQ(err.failed(), false);
            ASSERT_EQ(sz == 5, true);
        });
    });

    io.run_for(std::chrono::milliseconds(100));
    t.join();
}

TEST(tcp_socket, recv)
{
    std::thread t([]() {
        libcpp::tcp_socket::io_t io;
        libcpp::tcp_listener li{io};
        for (int i = 0; i < 2; i++)
        {
            auto sock = li.accept(10091);
            ASSERT_EQ(sock != nullptr, true);
            ASSERT_EQ(sock->send(reinterpret_cast<const unsigned char*>(std::string("hello").c_str()), 6) == 6, true);
            ASSERT_EQ(sock->send(reinterpret_cast<const unsigned char*>(std::string("harry").c_str()), 6) == 6, true);
            sock->close();
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_socket::io_t io;
    libcpp::tcp_socket sock{io};
    unsigned char buf[1024];
    ASSERT_EQ(sock.connect("127.0.0.1", 10091), true);
    ASSERT_EQ(sock.recv(buf, 1024) == 6, true);
    ASSERT_EQ(std::string(reinterpret_cast<char*>(buf), 5) == std::string("hello"), true);
    ASSERT_EQ(sock.recv(buf, 1024) == 6, true);
    ASSERT_EQ(std::string(reinterpret_cast<char*>(buf), 5) == std::string("harry"), true);
    sock.close();

    libcpp::tcp_socket sock1{io};
    ASSERT_EQ(sock1.connect("127.0.0.1", 10091), true);
    ASSERT_EQ(sock1.recv(buf, 1024) == 6, true);
    ASSERT_EQ(std::string(reinterpret_cast<char*>(buf), 5) == std::string("hello"), true);
    ASSERT_EQ(sock1.recv(buf, 1024) == 6, true);
    ASSERT_EQ(std::string(reinterpret_cast<char*>(buf), 5) == std::string("harry"), true);
    sock1.close();

    t.join();
}

TEST(tcp_socket, recv_until)
{
    std::thread t([]() {
        libcpp::tcp_socket::io_t io;
        libcpp::tcp_listener li{io};
        for (int i = 0; i < 1; i++)
        {
            auto sock = li.accept(10091);
            ASSERT_EQ(sock != nullptr, true);
            ASSERT_EQ(sock->send(reinterpret_cast<const unsigned char*>(std::string("hello").c_str()), 6) == 6, true);
            ASSERT_EQ(sock->send(reinterpret_cast<const unsigned char*>(std::string("harry").c_str()), 6) == 6, true);
            sock->close();
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_socket::io_t io;
    libcpp::tcp_socket sock{io};
    libcpp::tcp_socket::streambuf_t buf;
    ASSERT_EQ(sock.connect("127.0.0.1", 10091), true);

    ASSERT_EQ(buf.size() == 0, true);
    ASSERT_EQ(sock.recv_until(buf, 6) == 6, true);
    ASSERT_EQ(buf.size() == 6, true);
    auto data = boost::asio::buffer_cast<const char*>(buf.data());
    ASSERT_EQ(std::string(data, 5) == std::string("hello"), true);
    buf.consume(6);

    ASSERT_EQ(buf.size() == 0, true);
    ASSERT_EQ(sock.recv_until(buf, 6) == 6, true);
    ASSERT_EQ(buf.size() == 6, true);
    auto data1 = boost::asio::buffer_cast<const char*>(buf.data());
    ASSERT_EQ(std::string(data1, 5) == std::string("harry"), true);
    buf.consume(6);

    t.join();
    sock.close();
}

TEST(tcp_socket, async_recv)
{
    std::thread t([](){
        libcpp::tcp_socket::io_t io;
        libcpp::tcp_listener li{io};
        unsigned char buf[1024];
        std::size_t nrecved = 0;
        li.async_accept(10091, [&buf, &nrecved](const libcpp::tcp_listener::err_t& err, libcpp::tcp_socket* sock) {
            ASSERT_EQ(err.failed(), false);
            ASSERT_EQ(sock->is_connected(), true);

            sock->async_recv(buf, 1024, [&buf, &nrecved](const libcpp::tcp_socket::err_t& err, std::size_t sz){
                ASSERT_EQ(err.failed(), false);
                ASSERT_EQ(sz >= 5, true);
                nrecved += sz;
            });

            sock->async_recv(buf, 1024, [&buf, &nrecved](const libcpp::tcp_socket::err_t& err, std::size_t sz){
                ASSERT_EQ(err.failed(), false);
                ASSERT_EQ(sz >= 5, true);
                nrecved += sz;
            });
        });

        io.run_for(std::chrono::milliseconds(100));
        ASSERT_EQ(nrecved, 10);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_socket::io_t io;
    libcpp::tcp_socket sock{io};
    sock.async_connect("127.0.0.1", 10091, [&sock](const libcpp::tcp_socket::err_t& err){
        ASSERT_EQ(err.failed(), false);

        sock.async_send(reinterpret_cast<const unsigned char*>(std::string("hello").c_str()), 5, 
        [](const libcpp::tcp_socket::err_t& err, std::size_t sz){
            ASSERT_EQ(err.failed(), false);
            ASSERT_EQ(sz == 5, true);
        });

        sock.async_send(reinterpret_cast<const unsigned char*>(std::string("harry").c_str()), 5, 
        [](const libcpp::tcp_socket::err_t& err, std::size_t sz){
            ASSERT_EQ(err.failed(), false);
            ASSERT_EQ(sz == 5, true);
        });
    });

    io.run_for(std::chrono::milliseconds(100));
    t.join();
}

TEST(tcp_socket, set_conn_status)
{
    libcpp::tcp_socket::io_t io;
    auto base = new libcpp::tcp_socket::sock_t(io);
    libcpp::tcp_socket sock{io, base};
    ASSERT_EQ(sock.set_conn_status(false), true);
    ASSERT_EQ(sock.is_connected(), false);

    ASSERT_EQ(sock.set_conn_status(true), true);
    ASSERT_EQ(sock.is_connected(), true);
}

TEST(tcp_socket, close)
{
    libcpp::tcp_socket::io_t io;
    libcpp::tcp_socket sock{io};
    sock.close();
    sock.close();
}