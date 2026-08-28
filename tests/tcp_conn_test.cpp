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

constexpr auto TEST_TIMEOUT = std::chrono::seconds(5);

template <typename T>
T get_future_with_timeout(std::future<T>      &fut,
                          std::chrono::seconds timeout = TEST_TIMEOUT)
{
    if(fut.wait_for(timeout) != std::future_status::ready)
    {
        ADD_FAILURE() << "Timed out waiting for future (limit: "
                      << timeout.count() << "s)";
        return T{};
    }
    return fut.get();
}

inline void
wait_future_with_timeout(std::future<void>   &fut,
                         std::chrono::seconds timeout = TEST_TIMEOUT)
{
    if(fut.wait_for(timeout) != std::future_status::ready)
    {
        ADD_FAILURE() << "Timed out waiting for future (limit: "
                      << timeout.count() << "s)";
        return;
    }
    fut.get();
}

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

    auto custom_sock    = hj::tcp_socket::make_shared(io);
    auto conn_with_sock = hj::tcp_conn::make_shared(io, custom_sock);
    ASSERT_NE(conn_with_sock, nullptr);

    auto conn_null_sock = hj::tcp_conn::make_shared(io, nullptr);
    ASSERT_NE(conn_null_sock, nullptr);
}

TEST(tcp_conn, send_boundary_conditions)
{
    hj::tcp_conn::io_t io;
    auto               conn = hj::tcp_conn::make_shared(io);

    hj::tcp_conn::buffer_t empty_buf;
    conn->send(empty_buf);
    conn->send(nullptr, 0);

    hj::tcp_conn::buffer_t test_buf = {'h', 'e', 'l', 'l', 'o'};
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

    std::promise<void> server_done_promise;
    auto               server_done_future = server_done_promise.get_future();

    std::thread server_thread([&listener, &server_done_promise]() {
        hj::tcp_listener::err_t err;
        auto                    server_sock = listener->accept(err);

        EXPECT_FALSE(err.failed());
        EXPECT_NE(server_sock, nullptr);

        if(!err.failed() && server_sock)
        {
            std::string resp = "pong";
            server_sock->write(boost::asio::buffer(resp), err);

            char buf[128] = {0};
            server_sock->read(boost::asio::buffer(buf, sizeof(buf)), err);
        }
        server_done_promise.set_value();
    });

    auto                      conn = hj::tcp_conn::make_shared(io);
    std::promise<std::string> write_promise;
    auto                      write_future = write_promise.get_future();
    std::promise<std::string> read_promise;
    auto                      read_future = read_promise.get_future();

    std::atomic<bool> write_set{false};
    std::atomic<bool> read_set{false};

    conn->set_write_callback(
        [&write_promise, &write_set](hj::tcp_conn::conn_ptr_t      c,
                                     const hj::tcp_conn::buffer_t &buf) {
            if(!write_set.exchange(true))
            {
                std::string sended(buf.begin(), buf.end());
                write_promise.set_value(sended);
            }
        });

    conn->set_read_callback(
        [&read_promise, &read_set](hj::tcp_conn::conn_ptr_t c,
                                   hj::tcp_conn::buffer_t &&buf) {
            if(!read_set.exchange(true))
            {
                std::string recved(buf.begin(), buf.end());
                read_promise.set_value(recved);
            }
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

    auto conn_err = get_future_with_timeout(conn_future);
    EXPECT_FALSE(conn_err.failed());

    std::string send_msg = "ping";
    conn->send(reinterpret_cast<const std::uint8_t *>(send_msg.data()),
               send_msg.size());

    auto recved_data = get_future_with_timeout(read_future);
    EXPECT_EQ(recved_data, "pong");

    auto sended_data = get_future_with_timeout(write_future);
    EXPECT_EQ(sended_data, "ping");

    wait_future_with_timeout(server_done_future);

    conn->close();
    listener->close();

    if(server_thread.joinable())
        server_thread.join();

    io.stop();
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

    auto err = get_future_with_timeout(conn_future);
    EXPECT_TRUE(err.failed());

    if(io_thread.joinable())
        io_thread.join();
}

TEST(tcp_conn, async_send_and_receive_multi_packets)
{
    hj::tcp_conn::io_t io;
    auto               work_guard = boost::asio::make_work_guard(io);

    const std::uint16_t test_port = 10021;

    auto listener = hj::tcp_listener::make_shared(io);
    listener->listen(test_port);

    std::string        total_received;
    std::promise<void> server_done_promise;
    auto               server_done_future = server_done_promise.get_future();

    std::string       msg1 = "packet_1_head";
    std::string       msg2 = "packet_2_body";
    std::string       msg3 = "packet_3_tail";
    const std::size_t expected_total_len =
        msg1.size() + msg2.size() + msg3.size();

    std::thread server_thread([&listener,
                               &total_received,
                               expected_total_len,
                               &server_done_promise]() {
        hj::tcp_listener::err_t err;
        auto                    server_sock = listener->accept(err);
        EXPECT_FALSE(err.failed());

        if(server_sock)
        {
            while(total_received.size() < expected_total_len && !err.failed())
            {
                char        buf[128] = {0};
                std::size_t sz =
                    server_sock->read(boost::asio::buffer(buf, sizeof(buf)),
                                      err);
                if(!err.failed() && sz > 0)
                {
                    total_received.append(buf, sz);
                }
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
            EXPECT_FALSE(ec.failed());
            conn_connected_promise.set_value();
        });

    std::thread io_thread([&io]() { io.run(); });

    wait_future_with_timeout(conn_connected_future);

    conn->send(reinterpret_cast<const std::uint8_t *>(msg1.data()),
               msg1.size());
    conn->send(hj::tcp_conn::buffer_t(msg2.begin(), msg2.end()));
    conn->send(reinterpret_cast<const std::uint8_t *>(msg3.data()),
               msg3.size());

    wait_future_with_timeout(server_done_future);

    EXPECT_EQ(total_received, msg1 + msg2 + msg3);

    conn->close();
    listener->close();
    work_guard.reset();

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
        EXPECT_FALSE(err.failed());
        if(server_sock)
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
        [conn](hj::tcp_conn::conn_ptr_t c, const hj::tcp_conn::err_t &ec) {});

    std::thread io_thread([&io]() { io.run(); });

    auto error_code = get_future_with_timeout(err_future);
    EXPECT_TRUE(error_code.failed());

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

    conn->close();
    conn->close();

    std::promise<hj::tcp_conn::err_t> conn_promise;
    auto                              conn_future = conn_promise.get_future();

    conn->async_connect("127.0.0.1",
                        10023,
                        [&conn_promise](hj::tcp_conn::conn_ptr_t   c,
                                        const hj::tcp_conn::err_t &ec) {
                            conn_promise.set_value(ec);
                        });

    io.poll();

    auto err = get_future_with_timeout(conn_future);
    EXPECT_TRUE(err.failed());

    conn->send(hj::tcp_conn::buffer_t{'t', 'e', 's', 't'});
}

TEST(tcp_conn, concurrent_send_and_close_thread_safety)
{
    hj::tcp_conn::io_t io;
    auto               work_guard = boost::asio::make_work_guard(io);

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
    for(int i = 0; i < 4; ++i)
    {
        io_threads.emplace_back([&io]() { io.run(); });
    }

    wait_future_with_timeout(conn_future);

    constexpr int            send_threads_count = 3;
    constexpr int            msgs_per_thread    = 500;
    std::vector<std::thread> threads;

    for(int t = 0; t < send_threads_count; ++t)
    {
        threads.emplace_back([conn, t, msgs_per_thread]() {
            for(int i = 0; i < msgs_per_thread; ++i)
            {
                std::string msg = "msg_" + std::to_string(i);
                conn->send(reinterpret_cast<const std::uint8_t *>(msg.data()),
                           msg.size());
            }
        });
    }

    threads.emplace_back([conn]() {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        conn->close();
    });

    for(auto &t : threads)
    {
        if(t.joinable())
            t.join();
    }

    listener->close();
    work_guard.reset();

    if(server_thread.joinable())
        server_thread.join();
    for(auto &t : io_threads)
    {
        if(t.joinable())
            t.join();
    }
}

TEST(tcp_conn, concurrent_callback_setting_and_invocation)
{
    hj::tcp_conn::io_t  io;
    const std::uint16_t test_port = 10025;

    auto listener = hj::tcp_listener::make_shared(io);
    listener->listen(test_port);

    std::atomic<bool> server_running{true};
    std::thread       server_thread([&listener, &server_running]() {
        hj::tcp_listener::err_t err;
        auto                    server_sock = listener->accept(err);
        if(!err.failed() && server_sock)
        {
            std::string payload = "data_stream_chunk";
            while(server_running.load())
            {
                server_sock->write(boost::asio::buffer(payload), err);
                if(err.failed())
                    break;
                std::this_thread::sleep_for(std::chrono::microseconds(200));
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

    std::thread io_thread([&io]() { io.run(); });

    wait_future_with_timeout(conn_future);

    std::atomic<int> cb_counter{0};
    std::thread      setter_thread([conn, &cb_counter]() {
        for(int i = 0; i < 200; ++i)
        {
            conn->set_read_callback([&cb_counter](hj::tcp_conn::conn_ptr_t,
                                                  hj::tcp_conn::buffer_t &&) {
                cb_counter.fetch_add(1, std::memory_order_relaxed);
            });
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
    });

    if(setter_thread.joinable())
        setter_thread.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    server_running.store(false);
    conn->close();
    listener->close();

    if(server_thread.joinable())
        server_thread.join();
    if(io_thread.joinable())
        io_thread.join();

    EXPECT_GT(cb_counter.load(), 0);
}

TEST(tcp_conn, duplicate_async_connect)
{
    hj::tcp_conn::io_t  io;
    const std::uint16_t test_port = 10026;

    auto listener = hj::tcp_listener::make_shared(io);
    listener->listen(test_port);

    std::thread server_thread([&listener]() {
        hj::tcp_listener::err_t err;
        auto                    server_sock = listener->accept(err);
    });

    auto conn = hj::tcp_conn::make_shared(io);

    std::promise<hj::tcp_conn::err_t> first_conn_promise;
    std::promise<hj::tcp_conn::err_t> second_conn_promise;

    auto first_conn_future  = first_conn_promise.get_future();
    auto second_conn_future = second_conn_promise.get_future();

    conn->async_connect("127.0.0.1",
                        test_port,
                        [&first_conn_promise](hj::tcp_conn::conn_ptr_t,
                                              const hj::tcp_conn::err_t &ec) {
                            first_conn_promise.set_value(ec);
                        });

    conn->async_connect("127.0.0.1",
                        test_port,
                        [&second_conn_promise](hj::tcp_conn::conn_ptr_t,
                                               const hj::tcp_conn::err_t &ec) {
                            second_conn_promise.set_value(ec);
                        });

    std::thread io_thread([&io]() { io.run(); });

    auto first_err  = get_future_with_timeout(first_conn_future);
    auto second_err = get_future_with_timeout(second_conn_future);

    EXPECT_FALSE(first_err.failed());
    EXPECT_TRUE(second_err.failed());
    EXPECT_EQ(second_err,
              boost::system::errc::make_error_code(
                  boost::system::errc::already_connected));

    conn->close();
    listener->close();

    if(server_thread.joinable())
        server_thread.join();
    if(io_thread.joinable())
        io_thread.join();
}

TEST(tcp_conn, concurrent_send_close_reset_race)
{
    hj::tcp_conn::io_t io;
    auto               work_guard = boost::asio::make_work_guard(io);

    const std::uint16_t test_port = 10027;

    auto listener = hj::tcp_listener::make_shared(io);
    listener->listen(test_port);

    std::thread server_thread([&listener]() {
        hj::tcp_listener::err_t err;
        auto                    server_sock = listener->accept(err);
        if(server_sock)
        {
            char buf[128];
            while(!err.failed())
            {
                server_sock->read(boost::asio::buffer(buf), err);
            }
        }
    });

    auto               conn = hj::tcp_conn::make_shared(io);
    std::promise<void> conn_promise;
    auto               conn_future = conn_promise.get_future();

    conn->async_connect(
        "127.0.0.1",
        test_port,
        [&conn_promise](hj::tcp_conn::conn_ptr_t, const hj::tcp_conn::err_t &) {
            conn_promise.set_value();
        });

    std::thread io_thread([&io]() { io.run(); });
    wait_future_with_timeout(conn_future);

    std::thread thread_a([conn]() {
        for(int i = 0; i < 300; ++i)
        {
            std::string msg = "race_" + std::to_string(i);
            conn->send(reinterpret_cast<const std::uint8_t *>(msg.data()),
                       msg.size());
        }
    });

    std::thread thread_b([conn]() {
        std::this_thread::sleep_for(std::chrono::microseconds(50));
        conn->close();
    });

    std::thread thread_c([conn]() mutable {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        conn.reset();
    });

    if(thread_a.joinable())
        thread_a.join();
    if(thread_b.joinable())
        thread_b.join();
    if(thread_c.joinable())
        thread_c.join();

    conn.reset();

    listener->close();
    if(server_thread.joinable())
        server_thread.join();

    work_guard.reset();

    if(io_thread.joinable())
        io_thread.join();

    SUCCEED();
}

TEST(tcp_conn, destruction_with_outstanding_async_ops)
{
    hj::tcp_conn::io_t  io;
    const std::uint16_t test_port = 10028;

    auto listener = hj::tcp_listener::make_shared(io);
    listener->listen(test_port);

    std::thread server_thread([&listener]() {
        hj::tcp_listener::err_t err;
        auto                    server_sock = listener->accept(err);
        if(server_sock)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            server_sock->close();
        }
    });

    std::weak_ptr<hj::tcp_conn> weak_conn;

    {
        auto conn = hj::tcp_conn::make_shared(io);
        weak_conn = conn;

        std::promise<void> connected_promise;
        auto               connected_future = connected_promise.get_future();

        conn->async_connect("127.0.0.1",
                            test_port,
                            [&connected_promise](hj::tcp_conn::conn_ptr_t,
                                                 const hj::tcp_conn::err_t &) {
                                connected_promise.set_value();
                            });

        std::thread io_thread([&io]() { io.run(); });
        wait_future_with_timeout(connected_future);

        std::string payload = "outstanding_data_test";
        conn->send(reinterpret_cast<const std::uint8_t *>(payload.data()),
                   payload.size());

        conn.reset();

        if(io_thread.joinable())
            io_thread.join();
    }

    EXPECT_TRUE(weak_conn.expired());

    listener->close();
    if(server_thread.joinable())
        server_thread.join();
}

TEST(tcp_conn, packet_coalescing_and_fragmentation)
{
    hj::tcp_conn::io_t io;
    auto               work_guard = boost::asio::make_work_guard(io);

    const std::uint16_t test_port = 10030;

    auto listener = hj::tcp_listener::make_shared(io);
    listener->listen(test_port);

    std::promise<std::shared_ptr<hj::tcp_socket>> server_sock_promise;
    auto server_sock_future = server_sock_promise.get_future();

    std::thread server_thread([&listener, &server_sock_promise]() {
        hj::tcp_listener::err_t err;
        auto                    server_sock = listener->accept(err);
        EXPECT_FALSE(err.failed());
        server_sock_promise.set_value(server_sock);
    });

    auto               conn = hj::tcp_conn::make_shared(io);
    std::promise<void> conn_promise;
    auto               conn_future = conn_promise.get_future();

    conn->async_connect("127.0.0.1",
                        test_port,
                        [&conn_promise](hj::tcp_conn::conn_ptr_t,
                                        const hj::tcp_conn::err_t &ec) {
                            EXPECT_FALSE(ec.failed());
                            conn_promise.set_value();
                        });

    std::thread io_thread([&io]() { io.run(); });

    wait_future_with_timeout(conn_future);
    auto server_sock = get_future_with_timeout(server_sock_future);
    ASSERT_NE(server_sock, nullptr);

    boost::asio::ip::tcp::no_delay option(true);
    server_sock->raw_socket().set_option(option);

    std::vector<std::string> received_chunks;
    std::promise<void>       all_done_promise;
    auto                     all_done_future = all_done_promise.get_future();
    const std::string        expected_all =
        "ABCDEFGHIJ"; // 3("ABC") + 4("DEFG") + 3("HIJ")

    conn->set_read_callback([&received_chunks, &all_done_promise, expected_all](
                                hj::tcp_conn::conn_ptr_t,
                                hj::tcp_conn::buffer_t &&buf) {
        received_chunks.emplace_back(buf.begin(), buf.end());

        std::size_t total_len = 0;
        for(const auto &c : received_chunks)
            total_len += c.size();

        if(total_len >= expected_all.size())
        {
            all_done_promise.set_value();
        }
    });

    hj::tcp_socket::err_t err;

    server_sock->write(boost::asio::buffer("ABC", 3), err);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    server_sock->write(boost::asio::buffer("DEFG", 4), err);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    server_sock->write(boost::asio::buffer("HIJ", 3), err);

    wait_future_with_timeout(all_done_future);

    EXPECT_GT(received_chunks.size(), 1u);

    std::string full_reconstructed;
    for(const auto &chunk : received_chunks)
    {
        full_reconstructed += chunk;
    }
    EXPECT_EQ(full_reconstructed, expected_all);

    conn->close();
    listener->close();
    work_guard.reset();

    if(server_thread.joinable())
        server_thread.join();
    if(io_thread.joinable())
        io_thread.join();
}