#include <gtest/gtest.h>
#include <thread>
#include <future>
#include <vector>
#include <atomic>
#include <hj/net/tcp/tcp_dialer.hpp>
#include <hj/net/tcp/tcp_listener.hpp>

TEST(tcp_dialer, dial_connection_refused)
{
    hj::tcp_dialer::io_t  io;
    hj::tcp_dialer        dialer{io};
    hj::tcp_dialer::err_t err;

    auto sock =
        dialer.dial("127.0.0.1", 59999, err, std::chrono::milliseconds(500));
    EXPECT_FALSE(sock);
    EXPECT_TRUE(err.failed());
    EXPECT_EQ(dialer.size(), 0);
}

TEST(tcp_dialer, dial_timeout)
{
    hj::tcp_dialer::io_t  io;
    hj::tcp_dialer        dialer{io};
    hj::tcp_dialer::err_t err;

    auto start = std::chrono::steady_clock::now();
    auto sock =
        dialer.dial("10.255.255.1", 80, err, std::chrono::milliseconds(200));
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    EXPECT_FALSE(sock);
    EXPECT_TRUE(err.failed());
    EXPECT_GE(duration.count(), 180);
}

TEST(tcp_dialer, dial_invalid_address)
{
    hj::tcp_dialer::io_t  io;
    hj::tcp_dialer        dialer{io};
    hj::tcp_dialer::err_t err;

    auto sock = dialer.dial("invalid_ip_format", 80, err);
    EXPECT_FALSE(sock);
    EXPECT_TRUE(err.failed());

    sock = dialer.dial(nullptr, 80, err);
    EXPECT_FALSE(sock);
    EXPECT_TRUE(err.failed());
}

TEST(tcp_dialer, dial_max_size)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        auto                 li = hj::tcp_listener::create(io);
        ASSERT_FALSE(li->listen(11010));
        hj::tcp_dialer::err_t err;
        auto                  sock = li->accept(err);
        if(sock)
            sock->close();
        li->close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_dialer::io_t  io;
    hj::tcp_dialer        dialer{io, 1};
    hj::tcp_dialer::err_t err;

    auto sock1 = dialer.dial("127.0.0.1", 11010, err);
    EXPECT_TRUE(sock1);
    EXPECT_EQ(dialer.size(), 1);

    auto sock2 = dialer.dial("127.0.0.1", 11010, err);
    EXPECT_FALSE(sock2);

    EXPECT_EQ(err, std::errc::no_buffer_space);
    EXPECT_EQ(dialer.size(), 1);

    t.join();
}

TEST(tcp_dialer, dial_after_close)
{
    hj::tcp_dialer::io_t io;
    hj::tcp_dialer       dialer{io};
    dialer.close();

    hj::tcp_dialer::err_t err;
    auto                  sock = dialer.dial("127.0.0.1", 80, err);
    EXPECT_FALSE(sock);
    EXPECT_TRUE(err.failed());
}

TEST(tcp_dialer, async_dial_failure)
{
    hj::tcp_dialer::io_t io;
    hj::tcp_dialer       dialer{io};
    bool                 callback_called = false;

    dialer.async_dial(
        "127.0.0.1",
        59998,
        [&](const hj::tcp_dialer::err_t &err, hj::tcp_dialer::sock_ptr_t sock) {
            callback_called = true;
            EXPECT_TRUE(err.failed());
            EXPECT_FALSE(sock);
        });

    io.run();
    EXPECT_TRUE(callback_called);
}

TEST(tcp_dialer, async_dial_max_size)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        auto                 li = hj::tcp_listener::create(io);
        ASSERT_FALSE(li->listen(11014));
        hj::tcp_dialer::err_t err;

        auto sock = li->accept(err);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        if(sock)
            sock->close();
        li->close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_dialer::io_t  io;
    hj::tcp_dialer        dialer{io, 1};
    hj::tcp_dialer::err_t err;

    auto sock1 = dialer.dial("127.0.0.1", 11014, err);
    ASSERT_TRUE(sock1);
    EXPECT_EQ(dialer.size(), 1);

    bool callback_called = false;

    dialer.async_dial(
        "127.0.0.1",
        11014,
        [&](const hj::tcp_dialer::err_t &err, hj::tcp_dialer::sock_ptr_t sock) {
            callback_called = true;
            EXPECT_EQ(err, std::errc::no_buffer_space);
            EXPECT_FALSE(sock);
        });

    io.restart();
    io.run();

    EXPECT_TRUE(callback_called);
    t.join();
}

TEST(tcp_dialer, async_dial_close_race)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        auto                 li = hj::tcp_listener::create(io);
        ASSERT_FALSE(li->listen(11011));
        hj::tcp_dialer::err_t err;
        auto                  sock = li->accept(err);
        if(sock)
            sock->close();
        li->close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_dialer::io_t io;
    auto                 dialer = std::make_unique<hj::tcp_dialer>(io);
    bool                 cb_run = false;

    dialer->async_dial(
        "127.0.0.1",
        11011,
        [&](const hj::tcp_dialer::err_t &err, hj::tcp_dialer::sock_ptr_t sock) {
            cb_run = true;
            if(sock)
            {
                EXPECT_FALSE(dialer->is_exist(sock));
            }
        });

    dialer->close();

    io.run();
    EXPECT_TRUE(cb_run);
    EXPECT_EQ(dialer->size(), 0);
    t.join();
}

TEST(tcp_dialer, async_dial_lifetime)
{
    hj::tcp_dialer::io_t io;
    bool                 cb_run = false;

    {
        hj::tcp_dialer dialer{io};
        dialer.async_dial("127.0.0.1",
                          59997,
                          [&](const hj::tcp_dialer::err_t &err,
                              hj::tcp_dialer::sock_ptr_t   sock) {
                              cb_run = true;
                              EXPECT_TRUE(err.failed());
                              EXPECT_FALSE(sock);
                          });
    }

    io.run();
    EXPECT_TRUE(cb_run);
}

TEST(tcp_dialer, remove_unknown_and_null)
{
    hj::tcp_dialer::io_t io;
    hj::tcp_dialer       dialer{io};

    EXPECT_FALSE(dialer.remove(nullptr));

    auto dummy_sock = std::make_shared<hj::tcp_socket>(io);
    EXPECT_FALSE(dialer.remove(dummy_sock));
}

TEST(tcp_dialer, range_stop_and_exception)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        auto                 li = hj::tcp_listener::create(io);
        ASSERT_FALSE(li->listen(11012));
        for(int i = 0; i < 3; i++)
        {
            hj::tcp_dialer::err_t err;
            auto                  sock = li->accept(err);
            if(sock)
                sock->close();
        }
        li->close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_dialer::io_t  io;
    hj::tcp_dialer        dialer{io};
    hj::tcp_dialer::err_t err;

    dialer.dial("127.0.0.1", 11012, err);
    dialer.dial("127.0.0.1", 11012, err);
    dialer.dial("127.0.0.1", 11012, err);
    EXPECT_EQ(dialer.size(), 3);

    int visited = 0;
    err         = dialer.range([&](hj::tcp_dialer::sock_ptr_t) -> bool {
        visited++;
        return false;
    });
    EXPECT_EQ(visited, 1);
    EXPECT_EQ(err, boost::system::errc::operation_canceled);

    EXPECT_NO_THROW({
        dialer.range([&](hj::tcp_dialer::sock_ptr_t) -> bool {
            throw std::runtime_error("User callback error");
        });
    });
    EXPECT_EQ(dialer.size(), 3);

    t.join();
}

TEST(tcp_dialer, close_idempotent)
{
    hj::tcp_dialer::io_t io;
    hj::tcp_dialer       dialer{io};

    dialer.close();
    dialer.close();
    dialer.close();
    EXPECT_FALSE(dialer.is_open());
    EXPECT_EQ(dialer.size(), 0);
}

TEST(tcp_dialer, concurrent_close_and_dial)
{
    std::atomic<bool> stop_signal{false};

    std::thread t_server([&]() {
        hj::tcp_socket::io_t io;
        auto                 li = hj::tcp_listener::create(io);
        if(li->listen(11013))
            return;
        while(!stop_signal)
        {
            hj::tcp_dialer::err_t err;
            auto                  sock = li->accept(err);
            if(sock)
                sock->close();
        }
        li->close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    for(int i = 0; i < 50; ++i)
    {
        hj::tcp_dialer::io_t io;
        auto                 dialer = std::make_shared<hj::tcp_dialer>(io, 100);

        std::thread t_dial([dialer]() {
            hj::tcp_dialer::err_t err;
            for(int k = 0; k < 20; ++k)
            {
                dialer->dial("127.0.0.1",
                             11013,
                             err,
                             std::chrono::milliseconds(10));
            }
        });

        std::thread t_async([dialer]() {
            for(int k = 0; k < 20; ++k)
            {
                dialer->async_dial("127.0.0.1",
                                   11013,
                                   [](const hj::tcp_dialer::err_t &,
                                      hj::tcp_dialer::sock_ptr_t) {});
            }
        });

        std::thread t_close([dialer]() {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            dialer->close();
        });

        t_dial.join();
        t_async.join();
        t_close.join();

        io.run();
        EXPECT_EQ(dialer->size(), 0);
    }

    stop_signal = true;
    {
        hj::tcp_dialer::io_t  io;
        hj::tcp_dialer        d{io};
        hj::tcp_dialer::err_t err;
        d.dial("127.0.0.1", 11013, err);
    }
    t_server.join();
}