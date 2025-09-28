#include <gtest/gtest.h>
#include <hj/net/tcp.hpp>
#include <csignal>

TEST(tcp_listener, is_closed)
{
    hj::tcp_listener::io_t io;
    hj::tcp_listener       li{io};
    ASSERT_EQ(li.is_closed(), false);
    li.close();
    ASSERT_EQ(li.is_closed(), true);
}

TEST(tcp_listener, set_option)
{
    std::thread t([]() {
        hj::tcp_listener::io_t io;
        hj::tcp_listener       li{io};

        ASSERT_EQ(li.set_option(hj::tcp_socket::opt_reuse_addr(true)), false);
        li.async_accept(
            12000,
            [](const hj::tcp_listener::err_t &err, hj::tcp_socket *sock) {
                ASSERT_EQ(err.failed(), false);
                ASSERT_EQ(sock != nullptr, true);
                sock->close();

                delete sock;
            });
        ASSERT_EQ(li.set_option(hj::tcp_socket::opt_reuse_addr(true)), true);

        io.run();
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};
    ASSERT_EQ(sock.connect("127.0.0.1", 12000), true);

    t.join();
}

TEST(tcp_listener, accept)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        for(int i = 0; i < 2; i++)
        {
            auto sock = li.accept(12001);
            ASSERT_EQ(sock != nullptr, true);
            sock->close();
            delete sock;
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};
    ASSERT_EQ(sock.connect("127.0.0.1", 12001), true);

    hj::tcp_socket sock1{io};
    ASSERT_EQ(sock1.connect("127.0.0.1", 12001), true);
    t.join();
}

TEST(tcp_listener, async_accept)
{
    static int  async_accept_times1 = 0;
    static int  async_accept_times2 = 0;
    std::thread t1([]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{
            io,
            [](const hj::tcp_listener::err_t &err, hj::tcp_socket *sock) {
                ASSERT_EQ(err.failed(), false);
                ASSERT_EQ(sock->is_connected(), true);
                ASSERT_EQ(sock->check_connected(), true);
                async_accept_times1++;

                delete sock;
            }};
        for(int i = 0; i < 2; i++)
            li.async_accept(12002);

        io.run();
    });

    std::thread t2([]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        for(int i = 0; i < 2; i++)
        {
            li.async_accept(
                12003,
                [](const hj::tcp_listener::err_t &err, hj::tcp_socket *sock) {
                    ASSERT_EQ(err.failed(), false);
                    ASSERT_EQ(sock->is_connected(), true);
                    ASSERT_EQ(sock->check_connected(), true);
                    async_accept_times2++;

                    delete sock;
                });
        }

        io.run();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};
    sock.async_connect("127.0.0.1",
                       12002,
                       [&sock](const hj::tcp_socket::err_t &err) {
                           ASSERT_EQ(err.failed(), false);
                           ASSERT_EQ(sock.is_connected(), true);
                       });

    hj::tcp_socket sock1{io};
    sock1.async_connect("127.0.0.1",
                        12002,
                        [&sock1](const hj::tcp_socket::err_t &err) {
                            ASSERT_EQ(err.failed(), false);
                            ASSERT_EQ(sock1.is_connected(), true);
                        });

    hj::tcp_socket sock2{io};
    sock2.async_connect("127.0.0.1",
                        12003,
                        [&sock2](const hj::tcp_socket::err_t &err) {
                            ASSERT_EQ(err.failed(), false);
                            ASSERT_EQ(sock2.is_connected(), true);
                        });

    hj::tcp_socket sock3{io};
    sock3.async_connect("127.0.0.1",
                        12003,
                        [&sock3](const hj::tcp_socket::err_t &err) {
                            ASSERT_EQ(err.failed(), false);
                            ASSERT_EQ(sock3.is_connected(), true);
                        });

    io.run();
    t1.join();
    t2.join();
    ASSERT_EQ(async_accept_times1, 2);
    ASSERT_EQ(async_accept_times2, 2);
}

TEST(tcp_listener, close)
{
    hj::tcp_socket::io_t io;
    hj::tcp_listener     li{io};
    ASSERT_EQ(li.is_closed(), false);
    li.close();
    ASSERT_EQ(li.is_closed(), true);
    li.close();
    ASSERT_EQ(li.is_closed(), true);
}