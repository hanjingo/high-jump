#include <gtest/gtest.h>
#include <libcpp/net/tcp.hpp>

TEST(tcp_dialer, accept)
{
    std::thread t([]() {
        libcpp::tcp_socket::io_t io;
        libcpp::tcp_listener li{io};
        for (int i = 0; i < 2; i++)
        {
            auto sock = li.accept(10091);
            ASSERT_EQ(sock != nullptr, true);
            sock->close();
            delete sock;
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_dialer::io_t io;
    libcpp::tcp_dialer dialer{io};
    ASSERT_EQ(dialer.dial("127.0.0.1", 10091) != nullptr, true);
    ASSERT_EQ(dialer.dial("127.0.0.1", 10091) != nullptr, true);
    t.join();
}

TEST(tcp_dialer, async_dial)
{
    std::thread t([]() {
        libcpp::tcp_socket::io_t io;
        libcpp::tcp_listener li{io};
        for (int i = 0; i < 2; i++)
        {
            auto sock = li.accept(10091);
            ASSERT_EQ(sock != nullptr, true);
            sock->close();
            delete sock;
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_dialer::io_t io;
    libcpp::tcp_dialer dialer{io};
    dialer.async_dial("127.0.0.1", 10091, 
        [](const libcpp::tcp_dialer::err_t& err, libcpp::tcp_dialer::sock_ptr_t sock) {
        ASSERT_EQ(err.failed(), false);
        ASSERT_EQ(sock != nullptr, true);
    });
    dialer.async_dial("127.0.0.1", 10091, 
        [](const libcpp::tcp_dialer::err_t& err, libcpp::tcp_dialer::sock_ptr_t sock) {
        ASSERT_EQ(err.failed(), false);
        ASSERT_EQ(sock != nullptr, true);
    });
    io.run();
    t.join();
}

TEST(tcp_dialer, size)
{
    std::thread t([]() {
        libcpp::tcp_socket::io_t io;
        libcpp::tcp_listener li{io};
        for (int i = 0; i < 2; i++)
        {
            auto sock = li.accept(10091);
            ASSERT_EQ(sock != nullptr, true);
            sock->close();
            delete sock;
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_dialer::io_t io;
    libcpp::tcp_dialer dialer{io};
    auto sock1 = dialer.dial("127.0.0.1", 10091);
    ASSERT_EQ(sock1 != nullptr, true);
    auto sock2 = dialer.dial("127.0.0.1", 10091);
    ASSERT_EQ(sock2 != nullptr, true);
    ASSERT_EQ(dialer.size(), 2);
    t.join();
}

TEST(tcp_dialer, range)
{
    std::thread t([]() {
        libcpp::tcp_socket::io_t io;
        libcpp::tcp_listener li{io};
        for (int i = 0; i < 2; i++)
        {
            auto sock = li.accept(10091);
            ASSERT_EQ(sock != nullptr, true);
            sock->close();
            delete sock;
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_dialer::io_t io;
    libcpp::tcp_dialer dialer{io};
    ASSERT_EQ(dialer.dial("127.0.0.1", 10091) != nullptr, true);
    ASSERT_EQ(dialer.dial("127.0.0.1", 10091) != nullptr, true);
    int count = 0;
    dialer.range([&](libcpp::tcp_dialer::sock_ptr_t sock) -> bool{
        (void)sock;
        count++;
        return true;
    });
    ASSERT_EQ(count, 2);
    t.join();
}

TEST(tcp_dialer, remove)
{
    std::thread t([]() {
        libcpp::tcp_socket::io_t io;
        libcpp::tcp_listener li{io};
        for (int i = 0; i < 2; i++)
        {
            auto sock = li.accept(10091);
            ASSERT_EQ(sock != nullptr, true);
            sock->close();
            delete sock;
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_dialer::io_t io;
    libcpp::tcp_dialer dialer{io};
    auto sock1 = dialer.dial("127.0.0.1", 10091);
    auto sock2 = dialer.dial("127.0.0.1", 10091);
    ASSERT_EQ(dialer.size(), 2);
    dialer.remove(sock1);
    ASSERT_EQ(dialer.size(), 1);
    dialer.remove(sock2);
    ASSERT_EQ(dialer.size(), 0);
    t.join();
}

TEST(tcp_dialer, close)
{
    std::thread t([]() {
        libcpp::tcp_socket::io_t io;
        libcpp::tcp_listener li{io};
        for (int i = 0; i < 2; i++)
        {
            auto sock = li.accept(10091);
            ASSERT_EQ(sock != nullptr, true);
            sock->close();
            delete sock;
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::tcp_dialer::io_t io;
    libcpp::tcp_dialer dialer{io};
    auto sock1 = dialer.dial("127.0.0.1", 10091);
    ASSERT_EQ(sock1 != nullptr, true);
    auto sock2 = dialer.dial("127.0.0.1", 10091);
     ASSERT_EQ(sock2 != nullptr, true);
    dialer.close();
    ASSERT_EQ(dialer.size(), 0);
    t.join();
}