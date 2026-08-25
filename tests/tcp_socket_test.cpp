#include <gtest/gtest.h>
#include <hj/net/tcp.hpp>
#include <hj/testing/stacktrace.hpp>

TEST(tcp_socket, set_option)
{
    hj::tcp_socket::io_t    io;
    hj::tcp_socket::sock_t *base2 = new hj::tcp_socket::sock_t(io);
    base2->open(boost::asio::ip::tcp::v4());
    hj::tcp_socket sock2{io, base2};
    ASSERT_FALSE(sock2.set_option(hj::tcp_socket::opt_no_delay(true)).failed());
    ASSERT_FALSE(
        sock2.set_option(hj::tcp_socket::opt_send_buf_sz(1024)).failed());
    ASSERT_FALSE(
        sock2.set_option(hj::tcp_socket::opt_recv_buf_sz(1024)).failed());
    ASSERT_FALSE(
        sock2.set_option(hj::tcp_socket::opt_keep_alive(false)).failed());
    // ASSERT_FALSE(sock2.set_option(hj::tcp_socket::opt_broadcast(false)).failed());
}

TEST(tcp_socket, is_connected)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        for(int i = 0; i < 1; i++)
        {
            auto sock = li.accept(13000);
            ASSERT_TRUE(sock);
            sock->close();
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};
    ASSERT_EQ(sock.stat(), hj::tcp_socket::state::closed);
    ASSERT_FALSE(sock.connect("127.0.0.1", 13000).failed());
    ASSERT_EQ(sock.stat(), hj::tcp_socket::state::connected);

    t.join();
}

TEST(tcp_socket, check_connected)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        for(int i = 0; i < 1; i++)
        {
            auto sock = li.accept(13001);
            ASSERT_TRUE(sock);
            sock->close();
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};
    ASSERT_EQ(sock.stat(), hj::tcp_socket::state::closed);
    ASSERT_FALSE(sock.connect("127.0.0.1", 13001).failed());
    ASSERT_EQ(sock.stat(), hj::tcp_socket::state::connected);

    t.join();
}

TEST(tcp_socket, connect)
{
    int         accept_times = 0;
    std::thread t([&accept_times]() {
        hj::tcp_socket::io_t           io;
        hj::tcp_listener               li{io};
        hj::tcp_socket::steady_timer_t tm{io};
        for(int i = 0; i < 3; i++)
        {
            auto sock = li.accept(13002);
            ASSERT_TRUE(sock);
            accept_times++;
            sock->close();
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

TEST(tcp_socket, async_connect)
{
    int                  accept_times = 0;
    hj::tcp_socket::io_t io;
    hj::tcp_listener     li{io};
    li.async_accept(
        13003,
        [&li, &accept_times](const hj::tcp_listener::err_t  &err,
                             std::shared_ptr<hj::tcp_socket> sock) {
            ASSERT_FALSE(err.failed());
            ASSERT_TRUE(sock);
            accept_times++;
            sock->close();

            li.async_accept(
                13003,
                [&accept_times](const hj::tcp_listener::err_t  &err,
                                std::shared_ptr<hj::tcp_socket> sock) {
                    ASSERT_FALSE(err.failed());
                    ASSERT_TRUE(sock);
                    accept_times++;
                    sock->close();
                });
        });

    hj::tcp_socket sock{io};
    bool           lambda1_entryed = false;
    sock.async_connect("127.0.0.1",
                       13003,
                       [&lambda1_entryed](const hj::tcp_socket::err_t &err) {
                           ASSERT_FALSE(err.failed());
                           lambda1_entryed = true;
                       });

    bool           lambda2_entryed = false;
    hj::tcp_socket sock1{io};
    sock1.async_connect("127.0.0.1",
                        13003,
                        [&lambda2_entryed](const hj::tcp_socket::err_t &err) {
                            ASSERT_FALSE(err.failed());
                            lambda2_entryed = true;
                        });

    io.run_for(std::chrono::milliseconds(100));

    ASSERT_TRUE(lambda1_entryed);
    ASSERT_TRUE(lambda2_entryed);
    ASSERT_EQ(accept_times, 2);
}

TEST(tcp_socket, close)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        for(int i = 0; i < 1; i++)
        {
            auto sock = li.accept(13004);
            ASSERT_TRUE(sock);
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};

    ASSERT_EQ(sock.stat(), hj::tcp_socket::state::closed);
    sock.close();
    ASSERT_EQ(sock.stat(), hj::tcp_socket::state::closed);
    ASSERT_FALSE(sock.connect("127.0.0.1", 13004).failed());
    ASSERT_EQ(sock.stat(), hj::tcp_socket::state::connected);
    sock.close();
    ASSERT_EQ(sock.stat(), hj::tcp_socket::state::closed);
    sock.close();
    ASSERT_EQ(sock.stat(), hj::tcp_socket::state::closed);

    t.join();
}

TEST(tcp_socket, write)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        for(int i = 0; i < 2; i++)
        {
            auto sock = li.accept(13005);
            ASSERT_TRUE(sock);

            unsigned char         buf[1024];
            hj::tcp_socket::err_t err;
            ASSERT_EQ(sock->read(buf, 1024, err), 6);
            ASSERT_FALSE(err.failed());
            std::string str(reinterpret_cast<char *>(buf), 5);

            if(i == 0)
                ASSERT_EQ(str, std::string("hello"));
            else if(i == 1)
                ASSERT_EQ(str, std::string("harry"));
            else
                ASSERT_EQ(true, false);

            sock->close();
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

TEST(tcp_socket, async_write)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        auto buf_ptr = std::make_shared<std::array<unsigned char, 1024>>();
        li.async_accept(
            13006,
            [buf_ptr](const hj::tcp_listener::err_t  &err,
                      std::shared_ptr<hj::tcp_socket> sock) {
                ASSERT_FALSE(err.failed());
                ASSERT_EQ(sock->stat(), hj::tcp_socket::state::connected);

                sock->async_read(
                    buf_ptr->data(),
                    5,
                    [sock, buf_ptr](const hj::tcp_socket::err_t &err,
                                    std::size_t                  sz) {
                        ASSERT_FALSE(err.failed());
                        ASSERT_EQ(sz, 5);
                        ASSERT_EQ(std::string(
                                      reinterpret_cast<char *>(buf_ptr->data()),
                                      5)
                                      == "hello",
                                  true);
                    });
            });

        io.run_for(std::chrono::milliseconds(100));
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};
    auto write_buf = std::make_shared<std::array<unsigned char, 5>>();
    std::memcpy(write_buf->data(), "hello", 5);
    sock.async_connect(
        "127.0.0.1",
        13006,
        [write_buf, &sock](const hj::tcp_socket::err_t &err) {
            ASSERT_FALSE(err.failed());
            ASSERT_EQ(sock.stat(), hj::tcp_socket::state::connected);

            sock.async_write(
                write_buf->data(),
                5,
                [write_buf](const hj::tcp_socket::err_t &err, std::size_t sz) {
                    ASSERT_FALSE(err.failed());
                    ASSERT_EQ(sz, 5);
                });
        });

    io.run_for(std::chrono::milliseconds(100));
    t.join();
}

TEST(tcp_socket, read)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        for(int i = 0; i < 2; i++)
        {
            auto sock = li.accept(13007);
            ASSERT_TRUE(sock);
            hj::tcp_socket::err_t err;
            ASSERT_EQ(sock->write(reinterpret_cast<const unsigned char *>(
                                      std::string("hello").c_str()),
                                  6,
                                  err),
                      6);
            ASSERT_FALSE(err.failed());
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            ASSERT_EQ(sock->write(reinterpret_cast<const unsigned char *>(
                                      std::string("harry").c_str()),
                                  6,
                                  err),
                      6);
            ASSERT_FALSE(err.failed());
            sock->close();
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_socket::io_t  io;
    hj::tcp_socket        sock{io};
    unsigned char         buf[1024];
    hj::tcp_socket::err_t err;
    ASSERT_FALSE(sock.connect("127.0.0.1", 13007).failed());
    ASSERT_EQ(sock.read(buf, 1024, err), 6);
    ASSERT_FALSE(err.failed());
    ASSERT_EQ(std::string(reinterpret_cast<char *>(buf), 5),
              std::string("hello"));
    ASSERT_EQ(sock.read(buf, 1024, err), 6);
    ASSERT_FALSE(err.failed());
    ASSERT_EQ(std::string(reinterpret_cast<char *>(buf), 5),
              std::string("harry"));
    sock.close();

    hj::tcp_socket sock1{io};
    ASSERT_FALSE(sock1.connect("127.0.0.1", 13007).failed());
    ASSERT_EQ(sock1.read(buf, 1024, err), 6);
    ASSERT_FALSE(err.failed());
    ASSERT_EQ(std::string(reinterpret_cast<char *>(buf), 5),
              std::string("hello"));
    ASSERT_EQ(sock1.read(buf, 1024, err), 6);
    ASSERT_FALSE(err.failed());
    ASSERT_EQ(std::string(reinterpret_cast<char *>(buf), 5),
              std::string("harry"));
    sock1.close();

    t.join();
}

TEST(tcp_socket, read_until)
{
    std::thread t([]() {
        hj::tcp_socket::io_t  io;
        hj::tcp_listener      li{io};
        hj::tcp_socket::err_t err;
        for(int i = 0; i < 1; i++)
        {
            auto sock = li.accept(13008);
            ASSERT_TRUE(sock);
            ASSERT_EQ(sock->write(reinterpret_cast<const unsigned char *>(
                                      std::string("hello").c_str()),
                                  6,
                                  err),
                      6);
            ASSERT_FALSE(err.failed());

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            ASSERT_EQ(sock->write(reinterpret_cast<const unsigned char *>(
                                      std::string("harry").c_str()),
                                  6,
                                  err),
                      6);
            ASSERT_FALSE(err.failed());
            sock->close();
        }
        li.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_socket::io_t        io;
    hj::tcp_socket              sock{io};
    hj::tcp_socket::streambuf_t buf;
    hj::tcp_socket::err_t       err;
    ASSERT_FALSE(sock.connect("127.0.0.1", 13008).failed());

    ASSERT_EQ(buf.size(), 0);
    ASSERT_EQ(sock.read_until(buf, 6, err), 6);
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
    ASSERT_EQ(sock.read_until(buf, 6, err), 6);
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

TEST(tcp_socket, async_read)
{
    std::thread t([]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        auto buf_ptr = std::make_shared<std::array<unsigned char, 1024>>();
        std::size_t                     nreaded = 0;
        std::shared_ptr<hj::tcp_socket> sock_ptr;
        li.async_accept(
            13009,
            [buf_ptr, &nreaded, &sock_ptr](
                const hj::tcp_listener::err_t  &err,
                std::shared_ptr<hj::tcp_socket> sock) {
                ASSERT_FALSE(err.failed());
                ASSERT_EQ(sock->stat(), hj::tcp_socket::state::connected);
                sock_ptr = sock;

                sock->async_read(
                    buf_ptr->data(),
                    5,
                    [buf_ptr, &nreaded](const hj::tcp_socket::err_t &err,
                                        std::size_t                  sz) {
                        ASSERT_FALSE(err.failed());
                        ASSERT_EQ(sz, 5);
                        nreaded += sz;
                    });

                sock->async_read(
                    buf_ptr->data() + 5,
                    5,
                    [buf_ptr, &nreaded](const hj::tcp_socket::err_t &err,
                                        std::size_t                  sz) {
                        ASSERT_FALSE(err.failed());
                        ASSERT_EQ(sz, 5);
                        nreaded += sz;
                    });
            });

        io.run_for(std::chrono::milliseconds(100));
        ASSERT_EQ(nreaded, 10);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};
    auto write_buf1 = std::make_shared<std::array<unsigned char, 5>>();
    std::memcpy(write_buf1->data(), "hello", 5);
    auto write_buf2 = std::make_shared<std::array<unsigned char, 5>>();
    std::memcpy(write_buf2->data(), "harry", 5);
    sock.async_connect(
        "127.0.0.1",
        13009,
        [write_buf1, write_buf2, &sock](const hj::tcp_socket::err_t &err) {
            ASSERT_FALSE(err.failed());

            sock.async_write(
                write_buf1->data(),
                5,
                [write_buf1](const hj::tcp_socket::err_t &err, std::size_t sz) {
                    ASSERT_FALSE(err.failed());
                    ASSERT_EQ(sz, 5);
                });

            sock.async_write(
                write_buf2->data(),
                5,
                [write_buf2](const hj::tcp_socket::err_t &err, std::size_t sz) {
                    ASSERT_FALSE(err.failed());
                    ASSERT_EQ(sz, 5);
                });
        });

    io.run_for(std::chrono::milliseconds(100));
    t.join();
}

TEST(tcp_socket, set_conn_status)
{
    hj::tcp_socket::io_t io;
    auto                 base = new hj::tcp_socket::sock_t(io);
    hj::tcp_socket       sock{io, base};
    ASSERT_TRUE(sock.set_conn_status(hj::tcp_socket::state::closed));
    ASSERT_EQ(sock.stat(), hj::tcp_socket::state::closed);

    ASSERT_TRUE(sock.set_conn_status(hj::tcp_socket::state::connected));
    ASSERT_EQ(sock.stat(), hj::tcp_socket::state::connected);
}

TEST(tcp_socket_robustness, connect_refused)
{
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};

    auto err =
        sock.connect("127.0.0.1", 59999, std::chrono::milliseconds(500), 1);

    ASSERT_TRUE(err.failed());
    ASSERT_EQ(sock.stat(), hj::tcp_socket::state::closed);
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
    ASSERT_LE(duration.count(), 600);
    ASSERT_EQ(sock.stat(), hj::tcp_socket::state::closed);
}

TEST(tcp_socket_robustness, peer_reset)
{
    const uint16_t port = 13101;
    std::thread    t([port]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        auto                 sock = li.accept(port);
        ASSERT_TRUE(sock);

        boost::asio::socket_base::linger option(true, 0);
        sock->set_option(option);

        sock->close();
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
        || (err.value() == WSAECONNRESET) || (err.value() == WSAECONNABORTED);

    ASSERT_TRUE(is_reset_or_aborted) << "Unexpected error code: " << err.value()
                                     << " (" << err.message() << ")";

    t.join();
}

TEST(tcp_socket_robustness, eof_handling)
{
    const uint16_t port = 13102;
    std::thread    t([port]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        auto                 sock = li.accept(port);
        ASSERT_TRUE(sock);
        sock->close();
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

TEST(tcp_socket_robustness, async_read_timeout_cancel)
{
    const uint16_t port = 13103;
    std::thread    t([port]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        auto                 sock = li.accept(port);
        ASSERT_TRUE(sock);

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};
    ASSERT_FALSE(sock.connect("127.0.0.1", port).failed());

    unsigned char buf[10];
    bool          called = false;

    sock.async_read(
        buf,
        sizeof(buf),
        [&called](const hj::tcp_socket::err_t &err, std::size_t bytes) {
            called = true;
            ASSERT_TRUE(err.failed());
            ASSERT_EQ(err, boost::system::errc::timed_out);
            ASSERT_EQ(bytes, 0);
        },
        std::chrono::milliseconds(100));

    io.restart();
    io.run_for(std::chrono::milliseconds(200));

    ASSERT_TRUE(called);
    t.join();
}

TEST(tcp_socket_robustness, partial_read)
{
    const uint16_t port = 13104;
    std::thread    t([port]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        auto                 sock = li.accept(port);
        ASSERT_TRUE(sock);

        hj::tcp_socket::err_t err;
        sock->write(reinterpret_cast<const unsigned char *>("12345"), 5, err);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        sock->write(reinterpret_cast<const unsigned char *>("67890"), 5, err);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        sock->close();
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
        hj::tcp_listener     li{io};
        auto                 sock = li.accept(port);
        ASSERT_TRUE(sock);

        sock->set_option(hj::tcp_socket::opt_recv_buf_sz(4096));
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        sock->close();
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

TEST(tcp_socket_robustness, destroy_during_async)
{
    const uint16_t port = 13106;
    std::thread    t([port]() {
        hj::tcp_socket::io_t io;
        hj::tcp_listener     li{io};
        auto                 sock = li.accept(port);
        ASSERT_TRUE(sock);
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    hj::tcp_socket::io_t io;

    bool cb_executed = false;
    {
        auto sock = std::make_shared<hj::tcp_socket>(io);
        ASSERT_FALSE(sock->connect("127.0.0.1", port).failed());

        auto buf = std::make_shared<std::array<unsigned char, 100>>();
        sock->async_read(
            buf->data(),
            buf->size(),
            [buf, &cb_executed](const hj::tcp_socket::err_t &err, std::size_t) {
                cb_executed = true;
                ASSERT_TRUE(err.failed());
            },
            std::chrono::milliseconds(100));
    }

    io.restart();
    io.run_for(std::chrono::milliseconds(200));

    ASSERT_TRUE(cb_executed);
    t.join();
}