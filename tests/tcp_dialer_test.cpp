#include <gtest/gtest.h>
#include <hj/net/tcp.hpp>

TEST(tcp_dialer, accept)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        for(int i = 0; i < 2; i++)
        {
            auto sock = li.accept(11000);
            EXPECT_TRUE(sock);
            sock->close();
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_dialer::io_t io;
    hj::tcp_dialer       dialer{io};
    EXPECT_TRUE(dialer.dial("127.0.0.1", 11000));
    EXPECT_TRUE(dialer.dial("127.0.0.1", 11000));
    t.join();
}

TEST(tcp_dialer, async_dial)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        for(int i = 0; i < 2; i++)
        {
            auto sock = li.accept(11001);
            EXPECT_TRUE(sock);
            sock->close();
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_dialer::io_t io;
    hj::tcp_dialer       dialer{io};
    dialer.async_dial(
        "127.0.0.1",
        11001,
        [](const hj::tcp_dialer::err_t &err, hj::tcp_dialer::sock_ptr_t sock) {
            EXPECT_EQ(err.failed(), false);
            EXPECT_TRUE(sock);
        });
    dialer.async_dial(
        "127.0.0.1",
        11001,
        [](const hj::tcp_dialer::err_t &err, hj::tcp_dialer::sock_ptr_t sock) {
            EXPECT_EQ(err.failed(), false);
            EXPECT_TRUE(sock);
        });
    io.run();
    t.join();
}

TEST(tcp_dialer, size)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        for(int i = 0; i < 2; i++)
        {
            auto sock = li.accept(11002);
            EXPECT_TRUE(sock);
            sock->close();
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_dialer::io_t io;
    hj::tcp_dialer       dialer{io};
    auto                 sock1 = dialer.dial("127.0.0.1", 11002);
    EXPECT_TRUE(sock1);
    auto sock2 = dialer.dial("127.0.0.1", 11002);
    EXPECT_TRUE(sock2);
    EXPECT_EQ(dialer.size(), 2);
    t.join();
}

TEST(tcp_dialer, range)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        for(int i = 0; i < 2; i++)
        {
            auto sock = li.accept(11003);
            EXPECT_TRUE(sock);
            sock->close();
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_dialer::io_t io;
    hj::tcp_dialer       dialer{io};
    EXPECT_TRUE(dialer.dial("127.0.0.1", 11003));
    EXPECT_TRUE(dialer.dial("127.0.0.1", 11003));
    int count = 0;
    dialer.range([&](hj::tcp_dialer::sock_ptr_t sock) -> bool {
        (void) sock;
        count++;
        return true;
    });
    EXPECT_EQ(count, 2);
    t.join();
}

TEST(tcp_dialer, remove)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        for(int i = 0; i < 2; i++)
        {
            auto sock = li.accept(11004);
            EXPECT_TRUE(sock);
            sock->close();
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_dialer::io_t io;
    hj::tcp_dialer       dialer{io};
    auto                 sock1 = dialer.dial("127.0.0.1", 11004);
    auto                 sock2 = dialer.dial("127.0.0.1", 11004);
    EXPECT_EQ(dialer.size(), 2);
    dialer.remove(sock1);
    EXPECT_EQ(dialer.size(), 1);
    dialer.remove(sock2);
    EXPECT_EQ(dialer.size(), 0);
    t.join();
}

TEST(tcp_dialer, close)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        for(int i = 0; i < 2; i++)
        {
            auto sock = li.accept(11005);
            EXPECT_TRUE(sock);
            sock->close();
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_dialer::io_t io;
    hj::tcp_dialer       dialer{io};
    auto                 sock1 = dialer.dial("127.0.0.1", 11005);
    EXPECT_TRUE(sock1);
    auto sock2 = dialer.dial("127.0.0.1", 11005);
    EXPECT_TRUE(sock2);
    dialer.close();
    EXPECT_EQ(dialer.size(), 0);
    t.join();
}