#include <gtest/gtest.h>
#include <hj/testing/stacktrace.hpp>
#include <thread>
#include <vector>
#include <memory>
#include <boost/asio.hpp>

#include <hj/net/tcp/tcp_socket.hpp>

using boost::asio::ip::tcp;

TEST(tcp_socket, set_option)
{
    hj::tcp_socket::io_t    io;
    hj::tcp_socket::sock_t *base2 = new hj::tcp_socket::sock_t(io);
    base2->open(tcp::v4());

    hj::tcp_socket sock2{io, std::unique_ptr<hj::tcp_socket::sock_t>(base2)};

    ASSERT_FALSE(sock2.set_option(hj::tcp_socket::opt_no_delay(true)).failed());
    ASSERT_FALSE(
        sock2.set_option(hj::tcp_socket::opt_send_buf_sz(1024)).failed());
    ASSERT_FALSE(
        sock2.set_option(hj::tcp_socket::opt_recv_buf_sz(1024)).failed());
    ASSERT_FALSE(
        sock2.set_option(hj::tcp_socket::opt_keep_alive(false)).failed());
}

TEST(tcp_socket, is_connected)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), 13000));
        for(int i = 0; i < 1; i++)
        {
            tcp::socket socket(io);
            acceptor.accept(socket);
            socket.close();
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};
    ASSERT_EQ(sock.status(), hj::tcp_socket::state::closed);
    ASSERT_FALSE(sock.connect("127.0.0.1", 13000).failed());
    ASSERT_EQ(sock.status(), hj::tcp_socket::state::connected);

    t.join();
}

TEST(tcp_socket, check_connected)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), 13001));
        for(int i = 0; i < 1; i++)
        {
            tcp::socket socket(io);
            acceptor.accept(socket);
            socket.close();
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};
    ASSERT_EQ(sock.status(), hj::tcp_socket::state::closed);
    ASSERT_FALSE(sock.connect("127.0.0.1", 13001).failed());
    ASSERT_EQ(sock.status(), hj::tcp_socket::state::connected);

    t.join();
}

TEST(tcp_socket, connect)
{
    int         accept_times = 0;
    std::thread t([&accept_times]() {
        hj::tcp_socket::io_t      io;
        tcp::acceptor             acceptor(io, tcp::endpoint(tcp::v4(), 13002));
        boost::asio::steady_timer tm{io};
        for(int i = 0; i < 3; i++)
        {
            tcp::socket socket(io);
            acceptor.accept(socket);
            accept_times++;
            socket.close();
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};
    ASSERT_FALSE(sock.connect("127.0.0.1", 13002).failed());

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_socket sock1{io};
    ASSERT_FALSE(
        sock1.connect("127.0.0.1", 13002, std::chrono::milliseconds(30))
            .failed());

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_socket sock2{io};
    ASSERT_FALSE(
        sock2.connect("127.0.0.1", 13002, std::chrono::milliseconds(20), 5)
            .failed());

    t.join();
    ASSERT_EQ(accept_times, 3);
}

TEST(tcp_socket, close)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), 13004));
        for(int i = 0; i < 1; i++)
        {
            tcp::socket socket(io);
            acceptor.accept(socket);
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};

    ASSERT_EQ(sock.status(), hj::tcp_socket::state::closed);
    sock.close();
    ASSERT_EQ(sock.status(), hj::tcp_socket::state::closed);
    ASSERT_FALSE(sock.connect("127.0.0.1", 13004).failed());
    ASSERT_EQ(sock.status(), hj::tcp_socket::state::connected);
    sock.close();
    ASSERT_EQ(sock.status(), hj::tcp_socket::state::closed);
    sock.close();
    ASSERT_EQ(sock.status(), hj::tcp_socket::state::closed);

    t.join();
}

TEST(tcp_socket, write)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), 13005));
        for(int i = 0; i < 2; i++)
        {
            auto raw_sock = std::make_unique<tcp::socket>(io);
            acceptor.accept(*raw_sock);
            hj::tcp_socket sock{io, std::move(raw_sock)};

            unsigned char         buf[1024];
            hj::tcp_socket::err_t err;
            ASSERT_EQ(sock.read(buf, 1024, err), 6);
            ASSERT_FALSE(err.failed());
            std::string str(reinterpret_cast<char *>(buf), 5);

            if(i == 0)
                ASSERT_EQ(str, std::string("hello"));
            else if(i == 1)
                ASSERT_EQ(str, std::string("harry"));

            sock.close();
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};
    ASSERT_FALSE(sock.connect("127.0.0.1", 13005).failed());
    hj::tcp_socket::err_t err;
    ASSERT_EQ(sock.write(reinterpret_cast<const unsigned char *>(
                             std::string("hello").c_str()),
                         6,
                         err),
              6);
    ASSERT_FALSE(err.failed());

    hj::tcp_socket sock1{io};
    ASSERT_FALSE(sock1.connect("127.0.0.1", 13005).failed());
    ASSERT_EQ(sock1.write(reinterpret_cast<const unsigned char *>(
                              std::string("harry").c_str()),
                          6,
                          err),
              6);
    ASSERT_FALSE(err.failed());

    t.join();
}

TEST(tcp_socket, read)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), 13007));
        for(int i = 0; i < 2; i++)
        {
            auto raw_sock = std::make_unique<tcp::socket>(io);
            acceptor.accept(*raw_sock);
            hj::tcp_socket sock{io, std::move(raw_sock)};

            hj::tcp_socket::err_t err;
            ASSERT_EQ(sock.write(reinterpret_cast<const unsigned char *>(
                                     std::string("hello").c_str()),
                                 6,
                                 err),
                      6);
            ASSERT_FALSE(err.failed());
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            ASSERT_EQ(sock.write(reinterpret_cast<const unsigned char *>(
                                     std::string("harry").c_str()),
                                 6,
                                 err),
                      6);
            ASSERT_FALSE(err.failed());
            sock.close();
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_socket::io_t  io;
    hj::tcp_socket        sock{io};
    unsigned char         buf[1024];
    hj::tcp_socket::err_t err;
    ASSERT_FALSE(sock.connect("127.0.0.1", 13007).failed());

    ASSERT_EQ(sock.read(buf, 6, err), 6);
    ASSERT_FALSE(err.failed());
    ASSERT_EQ(std::string(reinterpret_cast<char *>(buf), 5),
              std::string("hello"));
    ASSERT_EQ(sock.read(buf, 6, err), 6);
    ASSERT_FALSE(err.failed());
    ASSERT_EQ(std::string(reinterpret_cast<char *>(buf), 5),
              std::string("harry"));
    sock.close();

    hj::tcp_socket sock1{io};
    ASSERT_FALSE(sock1.connect("127.0.0.1", 13007).failed());

    ASSERT_EQ(sock1.read(buf, 6, err), 6);
    ASSERT_FALSE(err.failed());
    ASSERT_EQ(std::string(reinterpret_cast<char *>(buf), 5),
              std::string("hello"));
    ASSERT_EQ(sock1.read(buf, 6, err), 6);
    ASSERT_FALSE(err.failed());
    ASSERT_EQ(std::string(reinterpret_cast<char *>(buf), 5),
              std::string("harry"));
    sock1.close();

    t.join();
}

TEST(tcp_socket, read_at_least)
{
    std::thread t([]() {
        hj::tcp_socket::io_t  io;
        tcp::acceptor         acceptor(io, tcp::endpoint(tcp::v4(), 13008));
        hj::tcp_socket::err_t err;
        for(int i = 0; i < 1; i++)
        {
            auto raw_sock = std::make_unique<tcp::socket>(io);
            acceptor.accept(*raw_sock);
            hj::tcp_socket sock{io, std::move(raw_sock)};

            ASSERT_EQ(sock.write(reinterpret_cast<const unsigned char *>(
                                     std::string("hello").c_str()),
                                 6,
                                 err),
                      6);
            ASSERT_FALSE(err.failed());

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            ASSERT_EQ(sock.write(reinterpret_cast<const unsigned char *>(
                                     std::string("harry").c_str()),
                                 6,
                                 err),
                      6);
            ASSERT_FALSE(err.failed());
            sock.close();
        }
        acceptor.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_socket::io_t        io;
    hj::tcp_socket              sock{io};
    hj::tcp_socket::streambuf_t buf;
    hj::tcp_socket::err_t       err;
    ASSERT_FALSE(sock.connect("127.0.0.1", 13008).failed());

    ASSERT_EQ(buf.size(), 0);
    ASSERT_EQ(sock.read_at_least(buf, 6, err), 6);
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
    ASSERT_EQ(sock.read_at_least(buf, 6, err), 6);
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
    sock.close();
}

TEST(tcp_socket, status_chg)
{
    hj::tcp_socket::io_t io;
    auto                 base = new hj::tcp_socket::sock_t(io);

    hj::tcp_socket sock{io, std::unique_ptr<hj::tcp_socket::sock_t>(base)};

    ASSERT_TRUE(sock.status_chg(hj::tcp_socket::state::closed));
    ASSERT_EQ(sock.status(), hj::tcp_socket::state::closed);

    ASSERT_FALSE(sock.status_chg(hj::tcp_socket::state::connected));
    ASSERT_EQ(sock.status(), hj::tcp_socket::state::closed);

    ASSERT_TRUE(sock.status_chg(hj::tcp_socket::state::connecting));
    ASSERT_TRUE(sock.status_chg(hj::tcp_socket::state::connected));
    ASSERT_EQ(sock.status(), hj::tcp_socket::state::connected);
}

TEST(tcp_socket_robustness, connect_refused)
{
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};

    auto err =
        sock.connect("127.0.0.1", 59999, std::chrono::milliseconds(500), 1);

    ASSERT_TRUE(err.failed());
    ASSERT_EQ(sock.status(), hj::tcp_socket::state::closed);
}

TEST(tcp_socket_robustness, connect_timeout)
{
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};

    auto start = std::chrono::steady_clock::now();
    auto err = sock.connect("192.0.2.1", 80, std::chrono::milliseconds(200), 1);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    ASSERT_TRUE(err.failed());
    ASSERT_GE(duration.count(), 180);
    ASSERT_LE(duration.count(), 1500);
    ASSERT_EQ(sock.status(), hj::tcp_socket::state::closed);
}

TEST(tcp_socket_robustness, peer_reset)
{
    const uint16_t port = 13101;
    std::thread    t([port]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), port));
        tcp::socket          socket(io);
        acceptor.accept(socket);

        boost::asio::socket_base::linger option(true, 0);
        socket.set_option(option);

        socket.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};
    ASSERT_FALSE(sock.connect("127.0.0.1", port).failed());

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    unsigned char         buf[100];
    hj::tcp_socket::err_t err;
    sock.read(buf, sizeof(buf), err);

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

TEST(tcp_socket_robustness, eof_handling)
{
    const uint16_t port = 13102;
    std::thread    t([port]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), port));
        tcp::socket          socket(io);
        acceptor.accept(socket);
        socket.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};
    ASSERT_FALSE(sock.connect("127.0.0.1", port).failed());

    unsigned char         buf[100];
    hj::tcp_socket::err_t err;
    size_t                n = sock.read(buf, sizeof(buf), err);

    ASSERT_EQ(n, 0);
    ASSERT_EQ(err, boost::asio::error::eof);
    t.join();
}

TEST(tcp_socket_robustness, partial_read)
{
    const uint16_t port = 13104;
    std::thread    t([port]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), port));
        auto                 raw_sock = std::make_unique<tcp::socket>(io);
        acceptor.accept(*raw_sock);
        hj::tcp_socket sock{io, std::move(raw_sock)};

        hj::tcp_socket::err_t err;
        sock.write(reinterpret_cast<const unsigned char *>("12345"), 5, err);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        sock.write(reinterpret_cast<const unsigned char *>("67890"), 5, err);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        sock.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};
    ASSERT_FALSE(sock.connect("127.0.0.1", port).failed());

    unsigned char         buf[10] = {0};
    hj::tcp_socket::err_t err;

    size_t n1 = sock.read(buf, 10, err);
    ASSERT_EQ(n1, 5);
    ASSERT_FALSE(err.failed());

    size_t n2 = sock.read(buf + 5, 5, err);
    ASSERT_EQ(n2, 5);
    ASSERT_FALSE(err.failed());
    ASSERT_EQ(std::string(reinterpret_cast<char *>(buf), 10), "1234567890");

    t.join();
}

TEST(tcp_socket_robustness, partial_write)
{
    const uint16_t port = 13105;
    std::thread    t([port]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), port));
        auto                 raw_sock = std::make_unique<tcp::socket>(io);
        acceptor.accept(*raw_sock);
        hj::tcp_socket sock{io, std::move(raw_sock)};

        sock.set_option(hj::tcp_socket::opt_recv_buf_sz(4096));
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        sock.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};
    ASSERT_FALSE(sock.connect("127.0.0.1", port).failed());

    sock.set_option(hj::tcp_socket::opt_send_buf_sz(4096));
    ASSERT_FALSE(sock.non_blocking(true).failed());

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    std::vector<unsigned char> big_buf(64 * 1024 * 1024, 'a');

    hj::tcp_socket::err_t err;
    size_t                total_written = 0;
    bool                  had_partial   = false;

    for(int i = 0; i < 100; ++i)
    {
        size_t written = sock.write(big_buf.data(), big_buf.size(), err);
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

TEST(tcp_socket_robustness, move_semantics_disabled)
{
    EXPECT_FALSE(std::is_move_constructible<hj::tcp_socket>::value);
    EXPECT_FALSE(std::is_move_assignable<hj::tcp_socket>::value);
}

TEST(tcp_socket, read_exactly)
{
    const uint16_t port = 13009;

    std::thread t([port]() {
        hj::tcp_socket::io_t io;
        tcp::acceptor        acceptor(io, tcp::endpoint(tcp::v4(), port));

        {
            auto raw_sock = std::make_unique<tcp::socket>(io);
            acceptor.accept(*raw_sock);
            hj::tcp_socket sock{io, std::move(raw_sock)};

            hj::tcp_socket::err_t err;
            sock.write(reinterpret_cast<const unsigned char *>("0123456789"),
                       10,
                       err);
            ASSERT_FALSE(err.failed());

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            sock.close();
        }

        {
            auto raw_sock = std::make_unique<tcp::socket>(io);
            acceptor.accept(*raw_sock);
            hj::tcp_socket sock{io, std::move(raw_sock)};

            hj::tcp_socket::err_t err;
            sock.write(reinterpret_cast<const unsigned char *>("abcde"),
                       5,
                       err);
            ASSERT_FALSE(err.failed());
            sock.close();
        }

        acceptor.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    hj::tcp_socket::io_t io;

    {
        hj::tcp_socket              sock{io};
        hj::tcp_socket::streambuf_t buf;
        hj::tcp_socket::err_t       err;

        ASSERT_FALSE(sock.connect("127.0.0.1", port).failed());

        size_t n1 = sock.read_exactly(buf, 6, err);
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

        size_t n2 = sock.read_exactly(buf, 4, err);
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

        sock.close();
    }

    {
        hj::tcp_socket              sock{io};
        hj::tcp_socket::streambuf_t buf;
        hj::tcp_socket::err_t       err;

        ASSERT_FALSE(sock.connect("127.0.0.1", port).failed());

        size_t n = sock.read_exactly(buf, 10, err);

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

        sock.close();
    }

    t.join();
}

TEST(tcp_socket, adopt_open_socket_is_not_connected)
{
    hj::tcp_socket::io_t io;

    auto sock = std::make_unique<hj::tcp_socket::sock_t>(io);

    boost::system::error_code ec;
    sock->open(tcp::v4(), ec);
    ASSERT_FALSE(ec.failed());
    ASSERT_TRUE(sock->is_open());

    hj::tcp_socket wrapper(io, std::move(sock), hj::tcp_socket::state::closed);

    ASSERT_TRUE(wrapper.is_open());
    ASSERT_EQ(wrapper.status(), hj::tcp_socket::state::closed);
}