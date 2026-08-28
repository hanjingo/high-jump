#include <gtest/gtest.h>
#include <hj/net/tcp/tcp_conn.hpp>
#include <hj/net/tcp/tcp_listener.hpp>

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace
{

class test_tcp_server
{
  public:
    using listener_ptr_t = std::shared_ptr<hj::tcp_listener>;
    using socket_ptr_t   = std::shared_ptr<hj::tcp_socket>;

    test_tcp_server(hj::tcp_conn::io_t &io, std::uint16_t port)
        : _io(io)
        , _listener(hj::tcp_listener::make_shared(_io))
        , _port(port)
    {
    }

    void start_accept_once(std::function<void(socket_ptr_t)> on_accept)
    {
        _listener->listen(_port);
        std::thread([this, on_accept = std::move(on_accept)]() {
            hj::tcp_listener::err_t err;
            auto                    sock = _listener->accept(err);
            if(!err.failed() && sock)
            {
                on_accept(sock);
            }
        }).detach();
    }

    void stop()
    {
        if(_listener)
        {
            _listener->close();
        }
    }

  private:
    hj::tcp_conn::io_t &_io;
    listener_ptr_t      _listener;
    std::uint16_t       _port;
};

} // namespace

TEST(tcp_conn, initial_state_and_factory)
{
    hj::tcp_conn::io_t io;

    auto conn = hj::tcp_conn::make_shared(io);
    ASSERT_NE(conn, nullptr);

    EXPECT_FALSE(conn->is_connected());
    EXPECT_FALSE(conn->is_closed());

    auto custom_sock    = hj::tcp_socket::make_shared(io);
    auto conn_with_sock = hj::tcp_conn::make_shared(io, custom_sock);
    ASSERT_NE(conn_with_sock, nullptr);
    EXPECT_FALSE(conn_with_sock->is_connected());
    EXPECT_FALSE(conn_with_sock->is_closed());

    auto conn_null_sock = hj::tcp_conn::make_shared(io, nullptr);
    ASSERT_NE(conn_null_sock, nullptr);
    EXPECT_FALSE(conn_null_sock->is_connected());
}

TEST(tcp_conn, send_boundary_conditions)
{
    hj::tcp_conn::io_t io;
    auto               conn = hj::tcp_conn::make_shared(io);

    hj::tcp_conn::msg_buffer_t empty_buf;
    conn->send(empty_buf);
    conn->send(nullptr, 0);

    hj::tcp_conn::msg_buffer_t test_buf = {'h', 'e', 'l', 'l', 'o'};
    conn->send(test_buf);
    conn->send(test_buf.data(), test_buf.size());

    conn->send(nullptr, 10);

    io.poll();
}

TEST(tcp_conn, async_connect_success_and_read)
{
    hj::tcp_conn::io_t  io;
    const std::uint16_t test_port = 10020;

    auto listener = hj::tcp_listener::make_shared(io);
    listener->listen(test_port);

    std::promise<void> accept_promise;
    auto               accept_future = accept_promise.get_future();

    std::thread server_thread([&listener, &accept_promise]() {
        hj::tcp_listener::err_t err;
        auto                    server_sock = listener->accept(err);
        ASSERT_FALSE(err.failed());
        ASSERT_NE(server_sock, nullptr);

        std::string resp = "pong";
        server_sock->write(boost::asio::buffer(resp), err);
        accept_promise.set_value();
    });

    auto                      conn = hj::tcp_conn::make_shared(io);
    std::promise<std::string> read_promise;
    auto                      read_future = read_promise.get_future();

    conn->set_read_callback([&read_promise](hj::tcp_conn::conn_ptr_t     c,
                                            hj::tcp_conn::msg_buffer_t &&buf) {
        std::string recved(buf.begin(), buf.end());
        read_promise.set_value(recved);
    });

    std::promise<hj::tcp_conn::err_t> conn_promise;
    auto                              conn_future = conn_promise.get_future();

    conn->async_connect("127.0.0.1",
                        test_port,
                        [&conn_promise](hj::tcp_conn::conn_ptr_t   c,
                                        const hj::tcp_conn::err_t &ec) {
                            conn_promise.set_value(ec);
                        });

    std::thread io_thread([&io]() { io.run(); });

    auto conn_err = conn_future.get();
    EXPECT_FALSE(conn_err.failed());
    EXPECT_TRUE(conn->is_connected());

    accept_future.wait();
    auto recved_data = read_future.get();
    EXPECT_EQ(recved_data, "pong");

    conn->close();
    listener->close();
    if(server_thread.joinable())
        server_thread.join();
    if(io_thread.joinable())
        io_thread.join();
}

TEST(tcp_conn, async_connect_failure_invalid_port)
{
    hj::tcp_conn::io_t io;
    auto               conn = hj::tcp_conn::make_shared(io);

    std::promise<hj::tcp_conn::err_t> conn_promise;
    auto                              conn_future = conn_promise.get_future();

    conn->async_connect("127.0.0.1",
                        19999,
                        [&conn_promise](hj::tcp_conn::conn_ptr_t   c,
                                        const hj::tcp_conn::err_t &ec) {
                            conn_promise.set_value(ec);
                        });

    std::thread io_thread([&io]() { io.run(); });

    auto err = conn_future.get();
    EXPECT_TRUE(err.failed());
    EXPECT_FALSE(conn->is_connected());

    if(io_thread.joinable())
        io_thread.join();
}

TEST(tcp_conn, async_send_and_receive_multi_packets)
{
    hj::tcp_conn::io_t  io;
    const std::uint16_t test_port = 10021;

    auto listener = hj::tcp_listener::make_shared(io);
    listener->listen(test_port);

    std::vector<std::string> received_by_server;
    std::promise<void>       server_done_promise;
    auto server_done_future = server_done_promise.get_future();

    std::thread server_thread(
        [&listener, &received_by_server, &server_done_promise]() {
            hj::tcp_listener::err_t err;
            auto                    server_sock = listener->accept(err);
            ASSERT_FALSE(err.failed());

            for(int i = 0; i < 3; ++i)
            {
                char        buf[128] = {0};
                std::size_t sz =
                    server_sock->read(boost::asio::buffer(buf, sizeof(buf)),
                                      err);
                if(!err.failed() && sz > 0)
                {
                    received_by_server.emplace_back(buf, sz);
                }
            }
            server_done_promise.set_value();
        });

    auto               conn = hj::tcp_conn::make_shared(io);
    std::promise<void> conn_connected_promise;
    auto conn_connected_future = conn_connected_promise.get_future();

    conn->async_connect(
        "127.0.0.1",
        test_port,
        [&conn_connected_promise](hj::tcp_conn::conn_ptr_t   c,
                                  const hj::tcp_conn::err_t &ec) {
            ASSERT_FALSE(ec.failed());
            conn_connected_promise.set_value();
        });

    std::thread io_thread([&io]() { io.run(); });

    conn_connected_future.wait();

    std::string msg1 = "packet_1_head";
    std::string msg2 = "packet_2_body";
    std::string msg3 = "packet_3_tail";

    conn->send(reinterpret_cast<const std::uint8_t *>(msg1.data()),
               msg1.size());
    conn->send(hj::tcp_conn::msg_buffer_t(msg2.begin(), msg2.end()));
    conn->send(reinterpret_cast<const std::uint8_t *>(msg3.data()),
               msg3.size());

    server_done_future.wait();

    ASSERT_EQ(received_by_server.size(), 3);
    EXPECT_EQ(received_by_server[0], msg1);
    EXPECT_EQ(received_by_server[1], msg2);
    EXPECT_EQ(received_by_server[2], msg3);

    conn->close();
    listener->close();

    if(server_thread.joinable())
        server_thread.join();
    if(io_thread.joinable())
        io_thread.join();
}

TEST(tcp_conn, server_abrupt_close_triggers_error_cb)
{
    hj::tcp_conn::io_t  io;
    const std::uint16_t test_port = 10022;

    auto listener = hj::tcp_listener::make_shared(io);
    listener->listen(test_port);

    std::thread server_thread([&listener]() {
        hj::tcp_listener::err_t err;
        auto                    server_sock = listener->accept(err);
        ASSERT_FALSE(err.failed());
        server_sock->close();
    });

    auto                              conn = hj::tcp_conn::make_shared(io);
    std::promise<hj::tcp_conn::err_t> err_promise;
    auto                              err_future = err_promise.get_future();

    conn->set_error_callback([&err_promise](hj::tcp_conn::conn_ptr_t   c,
                                            const hj::tcp_conn::err_t &ec) {
        err_promise.set_value(ec);
    });

    conn->async_connect(
        "127.0.0.1",
        test_port,
        [](hj::tcp_conn::conn_ptr_t c, const hj::tcp_conn::err_t &ec) {});

    std::thread io_thread([&io]() { io.run(); });

    auto error_code = err_future.get();
    EXPECT_TRUE(error_code.failed());
    EXPECT_TRUE(conn->is_closed());

    listener->close();
    if(server_thread.joinable())
        server_thread.join();
    if(io_thread.joinable())
        io_thread.join();
}

TEST(tcp_conn, close_idempotency_and_ops_on_closed_conn)
{
    hj::tcp_conn::io_t io;
    auto               conn = hj::tcp_conn::make_shared(io);

    EXPECT_FALSE(conn->is_closed());

    conn->close();
    EXPECT_TRUE(conn->is_closed());

    conn->close();
    EXPECT_TRUE(conn->is_closed());

    std::promise<hj::tcp_conn::err_t> conn_promise;
    auto                              conn_future = conn_promise.get_future();

    conn->async_connect("127.0.0.1",
                        10023,
                        [&conn_promise](hj::tcp_conn::conn_ptr_t   c,
                                        const hj::tcp_conn::err_t &ec) {
                            conn_promise.set_value(ec);
                        });

    io.poll();

    auto err = conn_future.get();
    EXPECT_EQ(err,
              boost::system::errc::make_error_code(
                  boost::system::errc::bad_file_descriptor));

    conn->run();
    conn->send(hj::tcp_conn::msg_buffer_t{'t', 'e', 's', 't'});
}

TEST(tcp_conn, concurrent_send_and_close_thread_safety)
{
    hj::tcp_conn::io_t  io;
    const std::uint16_t test_port = 10024;

    auto listener = hj::tcp_listener::make_shared(io);
    listener->listen(test_port);

    std::thread server_thread([&listener]() {
        hj::tcp_listener::err_t err;
        auto                    server_sock = listener->accept(err);
        if(server_sock)
        {
            char buf[512];
            while(!err.failed())
            {
                server_sock->read(boost::asio::buffer(buf), err);
            }
        }
    });

    auto               conn = hj::tcp_conn::make_shared(io);
    std::promise<void> conn_promise;
    auto               conn_future = conn_promise.get_future();

    conn->async_connect("127.0.0.1",
                        test_port,
                        [&conn_promise](hj::tcp_conn::conn_ptr_t   c,
                                        const hj::tcp_conn::err_t &ec) {
                            conn_promise.set_value();
                        });

    std::vector<std::thread> io_threads;
    for(int i = 0; i < 2; ++i)
    {
        io_threads.emplace_back([&io]() { io.run(); });
    }

    conn_future.wait();

    constexpr int            threads_count   = 4;
    constexpr int            msgs_per_thread = 100;
    std::vector<std::thread> senders;

    for(int t = 0; t < threads_count; ++t)
    {
        senders.emplace_back([conn, t, msgs_per_thread]() {
            for(int i = 0; i < msgs_per_thread; ++i)
            {
                std::string msg =
                    "thread_" + std::to_string(t) + "_msg_" + std::to_string(i);
                conn->send(reinterpret_cast<const std::uint8_t *>(msg.data()),
                           msg.size());
            }
        });
    }

    for(auto &t : senders)
    {
        if(t.joinable())
            t.join();
    }

    conn->close();
    listener->close();

    if(server_thread.joinable())
        server_thread.join();
    for(auto &t : io_threads)
    {
        if(t.joinable())
            t.join();
    }
}