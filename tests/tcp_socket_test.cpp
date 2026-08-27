#include <gtest/gtest.h>
#include <hj/testing/stacktrace.hpp>
#include <thread>
#include <vector>
#include <memory>
#include <future>
#include <boost/asio.hpp>

#include <hj/net/tcp/tcp_socket.hpp>
#include <hj/net/tcp/tcp_listener.hpp>

using boost::asio::ip::tcp;

TEST(tcp_socket, set_option)
{
    hj::tcp_socket::io_t        io;
    hj::tcp_socket::raw_sock_t *base2 = new hj::tcp_socket::raw_sock_t(io);
    base2->open(tcp::v4());

    auto sock2 = hj::tcp_socket::make_shared(
        io,
        std::unique_ptr<hj::tcp_socket::raw_sock_t>(base2));

    ASSERT_FALSE(
        sock2->set_option(hj::tcp_socket::opt_no_delay(true)).failed());
    ASSERT_FALSE(
        sock2->set_option(hj::tcp_socket::opt_send_buf_sz(1024)).failed());
    ASSERT_FALSE(
        sock2->set_option(hj::tcp_socket::opt_recv_buf_sz(1024)).failed());
    ASSERT_FALSE(
        sock2->set_option(hj::tcp_socket::opt_keep_alive(false)).failed());
}

TEST(tcp_socket, is_connected)
{
    std::promise<void> ready_promise;
    auto               ready_future = ready_promise.get_future();

    std::thread t([&ready_promise]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), 13000));
        ready_promise.set_value();
        for(int i = 0; i < 1; i++)
        {
            tcp::socket socket(io);
            acceptor.accept(socket);
            socket.close();
        }
    });

    ready_future.wait();
    hj::tcp_socket::io_t io;
    auto                 sock = hj::tcp_socket::make_shared(io);
    ASSERT_EQ(sock->status(), hj::tcp_socket::state::closed);
    ASSERT_FALSE(sock->connect("127.0.0.1", 13000).failed());
    ASSERT_EQ(sock->status(), hj::tcp_socket::state::connected);

    t.join();
}

TEST(tcp_socket, check_connected)
{
    std::promise<void> ready_promise;
    auto               ready_future = ready_promise.get_future();

    std::thread t([&ready_promise]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), 13001));
        ready_promise.set_value();
        for(int i = 0; i < 1; i++)
        {
            tcp::socket socket(io);
            acceptor.accept(socket);
            socket.close();
        }
    });

    ready_future.wait();
    hj::tcp_socket::io_t io;
    auto                 sock = hj::tcp_socket::make_shared(io);
    ASSERT_EQ(sock->status(), hj::tcp_socket::state::closed);
    ASSERT_FALSE(sock->connect("127.0.0.1", 13001).failed());
    ASSERT_EQ(sock->status(), hj::tcp_socket::state::connected);

    t.join();
}

TEST(tcp_socket, connect)
{
    int                accept_times = 0;
    std::promise<void> ready_promise;
    auto               ready_future = ready_promise.get_future();

    std::thread t([&accept_times, &ready_promise]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), 13002));
        ready_promise.set_value();
        for(int i = 0; i < 3; i++)
        {
            tcp::socket socket(io);
            acceptor.accept(socket);
            accept_times++;
            socket.close();
        }
    });

    ready_future.wait();
    hj::tcp_socket::io_t io;
    auto                 sock = hj::tcp_socket::make_shared(io);
    ASSERT_FALSE(sock->connect("127.0.0.1", 13002).failed());

    auto sock1 = hj::tcp_socket::make_shared(io);
    ASSERT_FALSE(
        sock1->connect("127.0.0.1", 13002, std::chrono::milliseconds(500))
            .failed());

    auto sock2 = hj::tcp_socket::make_shared(io);
    ASSERT_FALSE(
        sock2->connect("127.0.0.1", 13002, std::chrono::milliseconds(500), 5)
            .failed());

    t.join();
    ASSERT_EQ(accept_times, 3);
}

TEST(tcp_socket, close)
{
    std::promise<void> ready_promise;
    auto               ready_future = ready_promise.get_future();

    std::thread t([&ready_promise]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), 13004));
        ready_promise.set_value();
        for(int i = 0; i < 1; i++)
        {
            tcp::socket socket(io);
            acceptor.accept(socket);
        }
    });

    ready_future.wait();
    hj::tcp_socket::io_t io;
    auto                 sock = hj::tcp_socket::make_shared(io);

    ASSERT_EQ(sock->status(), hj::tcp_socket::state::closed);
    sock->close();
    ASSERT_EQ(sock->status(), hj::tcp_socket::state::closed);
    ASSERT_FALSE(sock->connect("127.0.0.1", 13004).failed());
    ASSERT_EQ(sock->status(), hj::tcp_socket::state::connected);
    sock->close();
    ASSERT_EQ(sock->status(), hj::tcp_socket::state::closed);
    sock->close();
    ASSERT_EQ(sock->status(), hj::tcp_socket::state::closed);

    t.join();
}

TEST(tcp_socket, write)
{
    std::promise<void> ready_promise;
    auto               ready_future = ready_promise.get_future();

    std::thread t([&ready_promise]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), 13005));
        ready_promise.set_value();
        for(int i = 0; i < 2; i++)
        {
            auto raw_sock = std::make_unique<tcp::socket>(io);
            acceptor.accept(*raw_sock);
            auto sock =
                hj::tcp_socket::make_shared(io,
                                            std::move(raw_sock),
                                            hj::tcp_socket::state::connected);

            unsigned char         buf[1024];
            hj::tcp_socket::err_t err;
            ASSERT_EQ(sock->read(buf, 1024, err), 6);
            ASSERT_FALSE(err.failed());
            std::string str(reinterpret_cast<char *>(buf), 5);

            if(i == 0)
                ASSERT_EQ(str, std::string("hello"));
            else if(i == 1)
                ASSERT_EQ(str, std::string("harry"));

            sock->close();
        }
    });

    ready_future.wait();
    hj::tcp_socket::io_t io;
    auto                 sock = hj::tcp_socket::make_shared(io);
    ASSERT_FALSE(sock->connect("127.0.0.1", 13005).failed());
    hj::tcp_socket::err_t err;
    ASSERT_EQ(
        sock->write(reinterpret_cast<const unsigned char *>("hello"), 6, err),
        6);
    ASSERT_FALSE(err.failed());

    auto sock1 = hj::tcp_socket::make_shared(io);
    ASSERT_FALSE(sock1->connect("127.0.0.1", 13005).failed());
    ASSERT_EQ(
        sock1->write(reinterpret_cast<const unsigned char *>("harry"), 6, err),
        6);
    ASSERT_FALSE(err.failed());

    t.join();
}

TEST(tcp_socket, read)
{
    std::promise<void> ready_promise;
    auto               ready_future = ready_promise.get_future();

    std::thread t([&ready_promise]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), 13007));
        ready_promise.set_value();
        for(int i = 0; i < 2; i++)
        {
            auto raw_sock = std::make_unique<tcp::socket>(io);
            acceptor.accept(*raw_sock);
            auto sock = hj::tcp_socket::make_shared(io, std::move(raw_sock));

            hj::tcp_socket::err_t err;
            ASSERT_EQ(
                sock->write(reinterpret_cast<const unsigned char *>("hello"),
                            6,
                            err),
                6);
            ASSERT_FALSE(err.failed());
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            ASSERT_EQ(
                sock->write(reinterpret_cast<const unsigned char *>("harry"),
                            6,
                            err),
                6);
            ASSERT_FALSE(err.failed());
            sock->close();
        }
    });

    ready_future.wait();
    hj::tcp_socket::io_t  io;
    auto                  sock = hj::tcp_socket::make_shared(io);
    unsigned char         buf[1024];
    hj::tcp_socket::err_t err;
    ASSERT_FALSE(sock->connect("127.0.0.1", 13007).failed());

    ASSERT_EQ(sock->read(buf, 6, err), 6);
    ASSERT_FALSE(err.failed());
    ASSERT_EQ(std::string(reinterpret_cast<char *>(buf), 5),
              std::string("hello"));
    ASSERT_EQ(sock->read(buf, 6, err), 6);
    ASSERT_FALSE(err.failed());
    ASSERT_EQ(std::string(reinterpret_cast<char *>(buf), 5),
              std::string("harry"));
    sock->close();

    auto sock1 = hj::tcp_socket::make_shared(io);
    ASSERT_FALSE(sock1->connect("127.0.0.1", 13007).failed());

    ASSERT_EQ(sock1->read(buf, 6, err), 6);
    ASSERT_FALSE(err.failed());
    ASSERT_EQ(std::string(reinterpret_cast<char *>(buf), 5),
              std::string("hello"));
    ASSERT_EQ(sock1->read(buf, 6, err), 6);
    ASSERT_FALSE(err.failed());
    ASSERT_EQ(std::string(reinterpret_cast<char *>(buf), 5),
              std::string("harry"));
    sock1->close();

    t.join();
}

TEST(tcp_socket, read_at_least)
{
    const uint16_t     port = 13008;
    std::promise<void> ready_promise;
    auto               ready_future = ready_promise.get_future();

    std::thread t([port, &ready_promise]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), port));
        ready_promise.set_value();

        hj::tcp_socket::err_t err;
        for(int i = 0; i < 1; i++)
        {
            auto raw_sock = std::make_unique<tcp::socket>(io);
            acceptor.accept(*raw_sock);
            auto sock = hj::tcp_socket::make_shared(io, std::move(raw_sock));

            ASSERT_EQ(
                sock->write(reinterpret_cast<const unsigned char *>("hello"),
                            6,
                            err),
                6);
            ASSERT_FALSE(err.failed());

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            ASSERT_EQ(
                sock->write(reinterpret_cast<const unsigned char *>("harry"),
                            6,
                            err),
                6);
            ASSERT_FALSE(err.failed());
            sock->close();
        }
        acceptor.close();
    });

    ready_future.wait();
    hj::tcp_socket::io_t        io;
    auto                        sock = hj::tcp_socket::make_shared(io);
    hj::tcp_socket::streambuf_t buf;
    hj::tcp_socket::err_t       err;
    ASSERT_FALSE(sock->connect("127.0.0.1", port).failed());

    ASSERT_EQ(buf.size(), 0);
    ASSERT_EQ(sock->read_at_least(buf, 6, err), 6);
    ASSERT_EQ(buf.size(), 6);
    ASSERT_FALSE(err.failed());
#if BOOST_VERSION < 108700
    auto data = boost::asio::buffer_cast<const char *>(buf.data());
#else
    auto data = static_cast<const char *>(buf.data().data());
#endif
    ASSERT_EQ(std::string(data, 5), std::string("hello"));
    buf.consume(6);

    ASSERT_EQ(buf.size(), 0);
    ASSERT_EQ(sock->read_at_least(buf, 6, err), 6);
    ASSERT_FALSE(err.failed());
    ASSERT_EQ(buf.size(), 6);
#if BOOST_VERSION < 108700
    auto data1 = boost::asio::buffer_cast<const char *>(buf.data());
#else
    auto data1 = static_cast<const char *>(buf.data().data());
#endif
    ASSERT_EQ(std::string(data1, 5), std::string("harry"));
    buf.consume(6);

    t.join();
    sock->close();
}

TEST(tcp_socket, robustness_connect_refused)
{
    hj::tcp_socket::io_t io;
    auto                 sock = hj::tcp_socket::make_shared(io);

    auto err =
        sock->connect("127.0.0.1", 59999, std::chrono::milliseconds(500), 1);

    ASSERT_TRUE(err.failed());
    ASSERT_EQ(sock->status(), hj::tcp_socket::state::closed);
}

TEST(tcp_socket, robustness_connect_timeout)
{
    hj::tcp_socket::io_t io;
    auto                 sock = hj::tcp_socket::make_shared(io);

    auto start = std::chrono::steady_clock::now();
    auto err =
        sock->connect("192.0.2.1", 80, std::chrono::milliseconds(200), 1);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    ASSERT_TRUE(err.failed());
    ASSERT_EQ(sock->status(), hj::tcp_socket::state::closed);
}

TEST(tcp_socket, robustness_peer_reset)
{
    const uint16_t     port = 13101;
    std::promise<void> ready_promise;
    auto               ready_future = ready_promise.get_future();

    std::thread t([port, &ready_promise]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), port));
        ready_promise.set_value();

        tcp::socket socket(io);
        acceptor.accept(socket);

        boost::asio::socket_base::linger option(true, 0);
        socket.set_option(option);

        socket.close();
    });

    ready_future.wait();
    hj::tcp_socket::io_t io;
    auto                 sock = hj::tcp_socket::make_shared(io);
    ASSERT_FALSE(sock->connect("127.0.0.1", port).failed());

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    unsigned char         buf[100];
    hj::tcp_socket::err_t err;
    sock->read(buf, sizeof(buf), err);

    ASSERT_TRUE(err.failed());

    bool is_reset_or_aborted =
        (err == boost::asio::error::connection_reset)
        || (err == boost::asio::error::connection_aborted)
        || (err == boost::system::errc::connection_reset)
        || (err == boost::system::errc::connection_aborted)
#ifdef _WIN32
        || (err.value() == WSAECONNRESET) || (err.value() == WSAECONNABORTED);
#else
        || (err.value() == ECONNRESET) || (err.value() == ECONNABORTED);
#endif

    ASSERT_TRUE(is_reset_or_aborted) << "Unexpected error code: " << err.value()
                                     << " (" << err.message() << ")";

    t.join();
}

TEST(tcp_socket, robustness_eof_handling)
{
    const uint16_t     port = 13102;
    std::promise<void> ready_promise;
    auto               ready_future = ready_promise.get_future();

    std::thread t([port, &ready_promise]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), port));
        ready_promise.set_value();

        tcp::socket socket(io);
        acceptor.accept(socket);
        socket.close();
    });

    ready_future.wait();
    hj::tcp_socket::io_t io;
    auto                 sock = hj::tcp_socket::make_shared(io);
    ASSERT_FALSE(sock->connect("127.0.0.1", port).failed());

    unsigned char         buf[100];
    hj::tcp_socket::err_t err;
    size_t                n = sock->read(buf, sizeof(buf), err);

    ASSERT_EQ(n, 0);
    ASSERT_EQ(err, boost::asio::error::eof);
    t.join();
}

TEST(tcp_socket, robustness_partial_read)
{
    const uint16_t     port = 13104;
    std::promise<void> ready_promise;
    auto               ready_future = ready_promise.get_future();

    std::thread t([port, &ready_promise]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), port));
        ready_promise.set_value();

        auto raw_sock = std::make_unique<tcp::socket>(io);
        acceptor.accept(*raw_sock);
        auto sock = hj::tcp_socket::make_shared(io, std::move(raw_sock));

        hj::tcp_socket::err_t err;
        sock->write(reinterpret_cast<const unsigned char *>("12345"), 5, err);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        sock->write(reinterpret_cast<const unsigned char *>("67890"), 5, err);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        sock->close();
    });

    ready_future.wait();
    hj::tcp_socket::io_t io;
    auto                 sock = hj::tcp_socket::make_shared(io);
    ASSERT_FALSE(sock->connect("127.0.0.1", port).failed());

    unsigned char         buf[10] = {0};
    hj::tcp_socket::err_t err;

    size_t n1 = sock->read(buf, 10, err);
    ASSERT_EQ(n1, 5);
    ASSERT_FALSE(err.failed());

    size_t n2 = sock->read(buf + 5, 5, err);
    ASSERT_EQ(n2, 5);
    ASSERT_FALSE(err.failed());
    ASSERT_EQ(std::string(reinterpret_cast<char *>(buf), 10), "1234567890");

    t.join();
}

TEST(tcp_socket, robustness_partial_write)
{
    const uint16_t     port = 13105;
    std::promise<void> ready_promise;
    auto               ready_future = ready_promise.get_future();

    std::thread t([port, &ready_promise]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), port));
        ready_promise.set_value();

        auto raw_sock = std::make_unique<tcp::socket>(io);
        acceptor.accept(*raw_sock);
        auto sock = hj::tcp_socket::make_shared(io, std::move(raw_sock));

        sock->set_option(hj::tcp_socket::opt_recv_buf_sz(4096));
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        sock->close();
    });

    ready_future.wait();
    hj::tcp_socket::io_t io;
    auto                 sock = hj::tcp_socket::make_shared(io);
    ASSERT_FALSE(sock->connect("127.0.0.1", port).failed());

    sock->set_option(hj::tcp_socket::opt_send_buf_sz(4096));
    ASSERT_FALSE(sock->non_blocking(true).failed());

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::vector<unsigned char> big_buf(64 * 1024 * 1024, 'a');

    hj::tcp_socket::err_t err;
    size_t                total_written = 0;
    bool                  had_partial   = false;

    for(int i = 0; i < 100; ++i)
    {
        size_t written = sock->write(big_buf.data(), big_buf.size(), err);
        total_written += written;

        if(written < big_buf.size())
        {
            had_partial = true;
        }

        if(err == boost::asio::error::would_block
           || err == boost::system::errc::operation_would_block)
        {
            err = {};
            break;
        }

        if(err.failed())
            break;
    }

    ASSERT_FALSE(err.failed());
    ASSERT_TRUE(had_partial) << "Total written: " << total_written;

    t.join();
}

TEST(tcp_socket, robustness_move_semantics_disabled)
{
    EXPECT_FALSE(std::is_move_constructible<hj::tcp_socket>::value);
    EXPECT_FALSE(std::is_move_assignable<hj::tcp_socket>::value);
}

TEST(tcp_socket, read_exactly)
{
    const uint16_t     port = 13009;
    std::promise<void> ready_promise;
    auto               ready_future = ready_promise.get_future();

    std::thread t([port, &ready_promise]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), port));
        ready_promise.set_value();

        {
            auto raw_sock = std::make_unique<tcp::socket>(io);
            acceptor.accept(*raw_sock);
            auto sock = hj::tcp_socket::make_shared(io, std::move(raw_sock));

            hj::tcp_socket::err_t err;
            sock->write(reinterpret_cast<const unsigned char *>("0123456789"),
                        10,
                        err);
            ASSERT_FALSE(err.failed());

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            sock->close();
        }

        {
            auto raw_sock = std::make_unique<tcp::socket>(io);
            acceptor.accept(*raw_sock);
            auto sock = hj::tcp_socket::make_shared(io, std::move(raw_sock));

            hj::tcp_socket::err_t err;
            sock->write(reinterpret_cast<const unsigned char *>("abcde"),
                        5,
                        err);
            ASSERT_FALSE(err.failed());
            sock->close();
        }

        acceptor.close();
    });

    ready_future.wait();
    hj::tcp_socket::io_t io;

    {
        auto                        sock = hj::tcp_socket::make_shared(io);
        hj::tcp_socket::streambuf_t buf;
        hj::tcp_socket::err_t       err;

        ASSERT_FALSE(sock->connect("127.0.0.1", port).failed());

        size_t n1 = sock->read_exactly(buf, 6, err);
        ASSERT_FALSE(err.failed());
        ASSERT_EQ(n1, 6);
        ASSERT_EQ(buf.size(), 6);

#if BOOST_VERSION < 108700
        auto data1 = boost::asio::buffer_cast<const char *>(buf.data());
#else
        auto data1 = static_cast<const char *>(buf.data().data());
#endif
        ASSERT_EQ(std::string(data1, 6), "012345");
        buf.consume(6);

        size_t n2 = sock->read_exactly(buf, 4, err);
        ASSERT_FALSE(err.failed());
        ASSERT_EQ(n2, 4);
        ASSERT_EQ(buf.size(), 4);

#if BOOST_VERSION < 108700
        auto data2 = boost::asio::buffer_cast<const char *>(buf.data());
#else
        auto data2 = static_cast<const char *>(buf.data().data());
#endif
        ASSERT_EQ(std::string(data2, 4), "6789");
        buf.consume(4);

        sock->close();
    }

    {
        auto                        sock = hj::tcp_socket::make_shared(io);
        hj::tcp_socket::streambuf_t buf;
        hj::tcp_socket::err_t       err;

        ASSERT_FALSE(sock->connect("127.0.0.1", port).failed());

        size_t n = sock->read_exactly(buf, 10, err);

        ASSERT_TRUE(err.failed());
        ASSERT_EQ(err, boost::asio::error::eof);

        ASSERT_EQ(n, 5);
        ASSERT_EQ(buf.size(), 5);

#if BOOST_VERSION < 108700
        auto data = boost::asio::buffer_cast<const char *>(buf.data());
#else
        auto data = static_cast<const char *>(buf.data().data());
#endif
        ASSERT_EQ(std::string(data, 5), "abcde");

        sock->close();
    }

    t.join();
}

TEST(tcp_socket, adopt_open_socket_is_not_connected)
{
    hj::tcp_socket::io_t io;

    auto sock = std::make_unique<hj::tcp_socket::raw_sock_t>(io);

    boost::system::error_code ec;
    sock->open(tcp::v4(), ec);
    ASSERT_FALSE(ec.failed());
    ASSERT_TRUE(sock->is_open());

    auto wrapper = hj::tcp_socket::make_shared(io,
                                               std::move(sock),
                                               hj::tcp_socket::state::closed);

    ASSERT_TRUE(wrapper->is_open());
    ASSERT_EQ(wrapper->status(), hj::tcp_socket::state::closed);
}

TEST(tcp_socket, async_connect_dangling_ptr_safety)
{
    hj::tcp_socket::io_t  io;
    bool                  handler_called = false;
    hj::tcp_socket::err_t captured_err;

    {
        auto sock = hj::tcp_socket::make_shared(io);

        sock->async_connect(
            "127.0.0.1",
            59998,
            [&handler_called, &captured_err](const hj::tcp_socket::err_t &err) {
                handler_called = true;
                captured_err   = err;
            });
    }

    io.run();

    ASSERT_TRUE(handler_called);
    ASSERT_TRUE(captured_err.failed());
}

TEST(tcp_socket, async_close_cancels_connect)
{
    hj::tcp_socket::io_t io;
    auto                 sock = hj::tcp_socket::make_shared(io);

    bool                  handler_called = false;
    hj::tcp_socket::err_t result;

    sock->async_connect(
        "192.0.2.1",
        80,
        [&handler_called, &result](const hj::tcp_socket::err_t &ec) {
            handler_called = true;
            result         = ec;
        });

    sock->close();

    io.run();

    ASSERT_TRUE(handler_called);
    ASSERT_EQ(result, boost::asio::error::operation_aborted);
    ASSERT_EQ(sock->status(), hj::tcp_socket::state::closed);
}

TEST(tcp_socket, async_connect_success)
{
    const uint16_t     port = 13201;
    std::promise<void> ready_promise;
    auto               ready_future = ready_promise.get_future();

    std::thread t([port, &ready_promise]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), port));
        ready_promise.set_value();
        tcp::socket socket(io);
        acceptor.accept(socket);
    });

    ready_future.wait();

    hj::tcp_socket::io_t io;
    auto                 sock = hj::tcp_socket::make_shared(io);

    bool                  handler_called = false;
    hj::tcp_socket::err_t result;

    sock->async_connect(
        "127.0.0.1",
        port,
        [&handler_called, &result](const hj::tcp_socket::err_t &ec) {
            handler_called = true;
            result         = ec;
        });

    io.run();

    ASSERT_TRUE(handler_called);
    ASSERT_FALSE(result.failed());
    ASSERT_EQ(sock->status(), hj::tcp_socket::state::connected);

    t.join();
}

TEST(tcp_socket, async_connect_refused)
{
    hj::tcp_socket::io_t io;
    auto                 sock = hj::tcp_socket::make_shared(io);

    bool                  handler_called = false;
    hj::tcp_socket::err_t result;

    sock->async_connect(
        "127.0.0.1",
        59999,
        [&handler_called, &result](const hj::tcp_socket::err_t &ec) {
            handler_called = true;
            result         = ec;
        });

    io.run();

    ASSERT_TRUE(handler_called);
    ASSERT_TRUE(result.failed());
    ASSERT_EQ(result, boost::system::errc::connection_refused);
    ASSERT_EQ(sock->status(), hj::tcp_socket::state::closed);
}

TEST(tcp_socket, async_duplicate_async_connect)
{
    hj::tcp_socket::io_t io;
    auto                 sock = hj::tcp_socket::make_shared(io);

    bool                  first_called  = false;
    bool                  second_called = false;
    hj::tcp_socket::err_t first_err;
    hj::tcp_socket::err_t second_err;

    sock->async_connect(
        "192.0.2.1",
        80,
        [&first_called, &first_err](const hj::tcp_socket::err_t &ec) {
            first_called = true;
            first_err    = ec;
        });

    ASSERT_EQ(sock->status(), hj::tcp_socket::state::connecting);

    sock->async_connect(
        "192.0.2.1",
        80,
        [&second_called, &second_err](const hj::tcp_socket::err_t &ec) {
            second_called = true;
            second_err    = ec;
        });

    ASSERT_FALSE(second_called);

    sock->close();

    io.run();

    ASSERT_TRUE(first_called);
    ASSERT_TRUE(second_called);

    ASSERT_EQ(first_err, boost::asio::error::operation_aborted);
    ASSERT_EQ(second_err, boost::system::errc::operation_in_progress);
}

TEST(tcp_socket, async_connect_followed_by_sync_connect)
{
    hj::tcp_socket::io_t io;
    auto                 sock = hj::tcp_socket::make_shared(io);

    bool                  async_called = false;
    hj::tcp_socket::err_t async_err;

    sock->async_connect(
        "192.0.2.1",
        80,
        [&async_called, &async_err](const hj::tcp_socket::err_t &ec) {
            async_called = true;
            async_err    = ec;
        });

    ASSERT_EQ(sock->status(), hj::tcp_socket::state::connecting);

    hj::tcp_socket::err_t sync_err =
        sock->connect("192.0.2.1", 80, std::chrono::milliseconds(100), 1);

    ASSERT_EQ(sync_err,
              boost::system::errc::make_error_code(
                  boost::system::errc::operation_in_progress));

    sock->close();
    io.run();

    ASSERT_TRUE(async_called);
    ASSERT_EQ(async_err, boost::asio::error::operation_aborted);
}

TEST(tcp_socket, async_read_write)
{
    std::promise<void> ready_promise;
    auto               ready_future = ready_promise.get_future();

    std::thread t([&ready_promise]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        auto buf_ptr = std::make_shared<std::array<unsigned char, 1024>>();
        std::size_t nrecved = 0;

        li.async_accept(
            13200,
            [buf_ptr, &nrecved](const hj::tcp_listener::err_t  &err,
                                std::shared_ptr<hj::tcp_socket> sock) {
                ASSERT_EQ(err.failed(), false);
                ASSERT_EQ(sock->status(), hj::tcp_socket::state::connected);

                sock->async_read(
                    buf_ptr->data(),
                    5,
                    [buf_ptr, &nrecved, sock](const hj::tcp_socket::err_t &err,
                                              std::size_t                  sz) {
                        ASSERT_EQ(err.failed(), false);
                        ASSERT_EQ(sz == 5, true);
                        nrecved += sz;

                        sock->async_read(buf_ptr->data() + 5,
                                         5,
                                         [buf_ptr, &nrecved, sock](
                                             const hj::tcp_socket::err_t &err,
                                             std::size_t                  sz) {
                                             ASSERT_EQ(err.failed(), false);
                                             ASSERT_EQ(sz == 5, true);
                                             nrecved += sz;
                                         });
                    });
            });

        ready_promise.set_value();
        io.run_for(std::chrono::milliseconds(500));
        ASSERT_EQ(nrecved, 10);
    });

    ready_future.wait();
    hj::tcp_socket::io_t io;
    auto                 sock = hj::tcp_socket::make_shared(io);
    auto send_buf1 = std::make_shared<std::array<unsigned char, 5>>();
    std::memcpy(send_buf1->data(), "hello", 5);
    auto send_buf2 = std::make_shared<std::array<unsigned char, 5>>();
    std::memcpy(send_buf2->data(), "harry", 5);

    sock->async_connect(
        "127.0.0.1",
        13200,
        [send_buf1, send_buf2, sock](const hj::tcp_socket::err_t &err) {
            ASSERT_EQ(err.failed(), false);

            sock->async_write(
                send_buf1->data(),
                5,
                [send_buf1, send_buf2, sock](const hj::tcp_socket::err_t &err,
                                             std::size_t                  sz) {
                    ASSERT_EQ(err.failed(), false);
                    ASSERT_EQ(sz == 5, true);

                    sock->async_write(
                        send_buf2->data(),
                        5,
                        [send_buf2](const hj::tcp_socket::err_t &err,
                                    std::size_t                  sz) {
                            ASSERT_EQ(err.failed(), false);
                            ASSERT_EQ(sz == 5, true);
                        });
                });
        });

    io.run_for(std::chrono::milliseconds(500));
    t.join();
}

TEST(tcp_socket, async_write_read_not_connected)
{
    hj::tcp_socket::io_t io;
    auto                 client = hj::tcp_socket::make_shared(io);

    bool write_called = false;
    bool read_called  = false;

    hj::tcp_socket::err_t write_err;
    hj::tcp_socket::err_t read_err;

    std::string data = "test";
    client->async_write(reinterpret_cast<const unsigned char *>(data.c_str()),
                        data.length(),
                        [&](const hj::tcp_socket::err_t &err, size_t bytes) {
                            write_called = true;
                            write_err    = err;
                            ASSERT_EQ(bytes, 0);
                        });

    unsigned char buf[64];
    client->async_read(buf,
                       sizeof(buf),
                       [&](const hj::tcp_socket::err_t &err, size_t bytes) {
                           read_called = true;
                           read_err    = err;
                           ASSERT_EQ(bytes, 0);
                       });

    io.run();

    ASSERT_TRUE(write_called);
    ASSERT_EQ(write_err, boost::system::errc::not_connected);

    ASSERT_TRUE(read_called);
    ASSERT_EQ(read_err, boost::system::errc::not_connected);
}

TEST(tcp_socket, async_read_eof_handling)
{
    const uint16_t     port = 13302;
    std::promise<void> ready_promise;
    std::promise<void> close_promise;
    auto               ready_future = ready_promise.get_future();
    auto               close_future = close_promise.get_future();

    std::thread server_thread([port, &ready_promise, &close_future]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), port));
        ready_promise.set_value();

        tcp::socket peer_sock(io);
        acceptor.accept(peer_sock);
        close_future.wait();
        peer_sock.close();
    });

    ready_future.wait();

    hj::tcp_socket::io_t io;
    auto                 client = hj::tcp_socket::make_shared(io);

    ASSERT_FALSE(client->connect("127.0.0.1", port).failed());

    bool                  read_called = false;
    hj::tcp_socket::err_t read_err;
    size_t                read_bytes = 0;
    unsigned char         read_buf[64];

    client->async_read(read_buf,
                       sizeof(read_buf),
                       [&](const hj::tcp_socket::err_t &err, size_t bytes) {
                           read_called = true;
                           read_err    = err;
                           read_bytes  = bytes;
                       });

    close_promise.set_value();

    io.restart();
    io.run();

    ASSERT_TRUE(read_called);
    ASSERT_EQ(read_bytes, 0);
    ASSERT_EQ(read_err, boost::asio::error::eof);
    ASSERT_EQ(client->status(), hj::tcp_socket::state::closed);

    server_thread.join();
}