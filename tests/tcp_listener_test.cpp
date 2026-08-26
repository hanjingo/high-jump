#include <gtest/gtest.h>
#include <hj/net/tcp.hpp>
#include <thread>
#include <chrono>
#include <atomic>

TEST(tcp_listener, is_closed)
{
    hj::tcp_listener::io_t io;
    auto                   li = hj::tcp_listener::create(io);
    ASSERT_FALSE(li->is_closed());
    li->close();
    ASSERT_TRUE(li->is_closed());
}

TEST(tcp_listener, state_transition)
{
    hj::tcp_listener::io_t io;
    auto                   li = hj::tcp_listener::create(io);

    ASSERT_EQ(li->status(), hj::tcp_listener::state::init);
    ASSERT_FALSE(li->is_listening());
    ASSERT_FALSE(li->is_closed());

    ASSERT_FALSE(li->open().failed());
    ASSERT_EQ(li->status(), hj::tcp_listener::state::opened);

    ASSERT_FALSE(li->listen(11999).failed());
    ASSERT_EQ(li->status(), hj::tcp_listener::state::listening);
    ASSERT_TRUE(li->is_listening());

    li->close();
    ASSERT_EQ(li->status(), hj::tcp_listener::state::closed);
    ASSERT_TRUE(li->is_closed());
}

TEST(tcp_listener, set_option)
{
    std::atomic<bool> listening{false};

    std::thread t([&listening]() {
        hj::tcp_listener::io_t io;
        auto                   li = hj::tcp_listener::create(io);

        ASSERT_TRUE(
            li->set_option(hj::tcp_listener::opt_reuse_addr(true)).failed());

        ASSERT_FALSE(li->open().failed());
        ASSERT_FALSE(
            li->set_option(hj::tcp_listener::opt_reuse_addr(true)).failed());

        ASSERT_FALSE(li->listen(12000).failed());
        li->async_accept([](const hj::tcp_listener::err_t  &err,
                            std::shared_ptr<hj::tcp_socket> sock) {
            ASSERT_FALSE(err.failed());
            ASSERT_NE(sock, nullptr);
            sock->close();
        });

        listening.store(true);

        io.run();
        li->close();
    });

    while(!listening.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};
    ASSERT_FALSE(sock.connect("127.0.0.1", 12000).failed());

    t.join();
}

TEST(tcp_listener, accept)
{
    std::atomic<bool> listening{false};

    std::thread t([&listening]() {
        hj::tcp_socket::io_t io;
        auto                 li = hj::tcp_listener::create(io);

        hj::tcp_listener::err_t unlisten_err;
        ASSERT_EQ(li->accept(unlisten_err), nullptr);
        ASSERT_TRUE(unlisten_err.failed());

        ASSERT_FALSE(li->listen(12001).failed());
        listening.store(true);

        for(int i = 0; i < 2; i++)
        {
            hj::tcp_listener::err_t err;
            auto                    sock = li->accept(err);
            ASSERT_TRUE(sock);
            ASSERT_FALSE(err.failed());
            sock->close();
        }
        li->close();
    });

    while(!listening.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    hj::tcp_socket::io_t io;

    hj::tcp_socket sock0{io};
    ASSERT_FALSE(sock0.connect("127.0.0.1", 12001).failed());

    hj::tcp_socket sock1{io};
    ASSERT_FALSE(sock1.connect("127.0.0.1", 12001).failed());

    t.join();
}

TEST(tcp_listener, accept_endpoint_error_semantics_threaded)
{
    hj::tcp_listener::io_t io;

    auto l1 = hj::tcp_listener::create(io);
    ASSERT_FALSE(l1->listen(12005).failed());

    auto l2 = hj::tcp_listener::create(io);

    hj::tcp_listener::err_t err = boost::system::errc::make_error_code(
        boost::system::errc::permission_denied);

    auto sock = l2->accept(12005, err);

    ASSERT_EQ(sock, nullptr);
    ASSERT_TRUE(err.failed());
    ASSERT_EQ(err, boost::asio::error::address_in_use);

    l2->close();
    l1->close();
}

TEST(tcp_listener, async_accept)
{
    static std::atomic<int> async_accept_times1{0};
    static std::atomic<int> async_accept_times2{0};

    std::thread t1([]() {
        hj::tcp_socket::io_t io;
        auto                 li = hj::tcp_listener::create(io);

        ASSERT_FALSE(li->listen(12002).failed());

        for(int i = 0; i < 2; i++)
        {
            li->async_accept([](const hj::tcp_listener::err_t  &err,
                                std::shared_ptr<hj::tcp_socket> sock) {
                ASSERT_FALSE(err.failed());
                ASSERT_EQ(sock->status(), hj::tcp_socket::state::connected);
                async_accept_times1++;
            });
        }

        io.run();
    });

    std::thread t2([]() {
        hj::tcp_socket::io_t io;
        auto                 li = hj::tcp_listener::create(io);

        ASSERT_FALSE(li->listen(12003).failed());

        for(int i = 0; i < 2; i++)
        {
            li->async_accept([](const hj::tcp_listener::err_t  &err,
                                std::shared_ptr<hj::tcp_socket> sock) {
                ASSERT_FALSE(err.failed());
                ASSERT_EQ(sock->status(), hj::tcp_socket::state::connected);
                async_accept_times2++;
            });
        }

        io.run();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    hj::tcp_socket::io_t io;
    hj::tcp_socket       sock{io};
    sock.connect("127.0.0.1", 12002);

    hj::tcp_socket sock1{io};
    sock1.connect("127.0.0.1", 12002);

    hj::tcp_socket sock2{io};
    sock2.connect("127.0.0.1", 12003);

    hj::tcp_socket sock3{io};
    sock3.connect("127.0.0.1", 12003);

    t1.join();
    t2.join();
    ASSERT_EQ(async_accept_times1.load(), 2);
    ASSERT_EQ(async_accept_times2.load(), 2);
}

TEST(tcp_listener, close)
{
    hj::tcp_socket::io_t io;
    auto                 li = hj::tcp_listener::create(io);
    ASSERT_FALSE(li->is_closed());
    li->close();
    ASSERT_TRUE(li->is_closed());
    li->close();
    ASSERT_TRUE(li->is_closed());
}

TEST(tcp_listener, concurrent_close_safety)
{
    hj::tcp_socket::io_t io;
    auto                 li = hj::tcp_listener::create(io);
    ASSERT_FALSE(li->listen(12006).failed());

    std::atomic<bool> stop{false};

    std::thread t1([&]() {
        while(!stop.load())
        {
            li->async_accept([](const hj::tcp_listener::err_t &, auto) {});
            std::this_thread::yield();
        }
    });

    std::thread t2([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        li->close();
        stop.store(true);
    });

    t1.join();
    t2.join();

    ASSERT_TRUE(li->is_closed());
}

TEST(tcp_listener, port_conflict)
{
    hj::tcp_listener::io_t io;
    auto                   l1 = hj::tcp_listener::create(io);
    ASSERT_FALSE(l1->listen(12000).failed());

    auto                    l2  = hj::tcp_listener::create(io);
    hj::tcp_listener::err_t err = l2->listen(12000);

    ASSERT_TRUE(err.failed());
    ASSERT_EQ(err, boost::asio::error::address_in_use);
}

TEST(tcp_listener, invalid_ip_parse)
{
    hj::tcp_listener::io_t io;
    auto                   li = hj::tcp_listener::create(io);

    std::atomic<bool> called{false};
    li->async_accept("999.999.999.999",
                     12000,
                     [&](const hj::tcp_listener::err_t &err, auto sock) {
                         called.store(true);
                         ASSERT_TRUE(err.failed());
                         ASSERT_EQ(sock, nullptr);
                         ASSERT_EQ(err, boost::asio::error::invalid_argument);
                     });

    ASSERT_FALSE(called.load());

    io.poll();

    ASSERT_TRUE(called.load());
}

TEST(tcp_listener, close_pending_accept)
{
    hj::tcp_listener::io_t io;
    auto                   li = hj::tcp_listener::create(io);
    ASSERT_FALSE(li->listen(12010).failed());

    std::atomic<bool> callback_executed{false};

    li->async_accept([&](const hj::tcp_listener::err_t  &err,
                         std::shared_ptr<hj::tcp_socket> sock) {
        callback_executed.store(true);
        ASSERT_TRUE(err.failed());
        ASSERT_EQ(err, boost::asio::error::operation_aborted);
        ASSERT_EQ(sock, nullptr);
    });

    li->close();

    io.run();
    ASSERT_TRUE(callback_executed.load());
}

TEST(tcp_listener, destructor_with_pending_accept)
{
    hj::tcp_listener::io_t io;
    std::atomic<bool>      callback_executed{false};

    {
        auto li = hj::tcp_listener::create(io);
        ASSERT_FALSE(li->listen(12011).failed());

        li->async_accept([&](const hj::tcp_listener::err_t  &err,
                             std::shared_ptr<hj::tcp_socket> sock) {
            callback_executed.store(true);
            ASSERT_TRUE(err.failed());
            ASSERT_EQ(err, boost::asio::error::operation_aborted);
            ASSERT_EQ(sock, nullptr);
        });
    }

    io.run();
    ASSERT_TRUE(callback_executed.load());
}

TEST(tcp_listener, stack_object_lifecycle)
{
    hj::tcp_listener::io_t io;
    std::atomic<bool>      callback_executed{false};

    {
        hj::tcp_listener li{io};
        ASSERT_FALSE(li.listen(12012).failed());

        ASSERT_NO_THROW(
            li.async_accept([&](const hj::tcp_listener::err_t  &err,
                                std::shared_ptr<hj::tcp_socket> sock) {
                callback_executed.store(true);
                ASSERT_TRUE(err.failed());
                ASSERT_EQ(err, boost::asio::error::operation_aborted);
                ASSERT_EQ(sock, nullptr);
            }));
    }

    io.run();
    ASSERT_TRUE(callback_executed.load());
}

TEST(tcp_listener, concurrent_close_while_running)
{
    hj::tcp_listener::io_t io;
    auto                   work_guard = boost::asio::make_work_guard(io);

    std::thread io_thread([&io]() { io.run(); });

    auto li = hj::tcp_listener::create(io);
    ASSERT_FALSE(li->listen(12020).failed());

    std::atomic<bool> stop{false};
    std::atomic<int>  executed_count{0};

    std::thread accept_thread([&]() {
        while(!stop.load())
        {
            li->async_accept([&executed_count](const hj::tcp_listener::err_t &,
                                               auto) { executed_count++; });
            std::this_thread::yield();
        }
    });

    std::thread close_thread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        li->close();
        stop.store(true);
    });

    accept_thread.join();
    close_thread.join();

    work_guard.reset();
    io_thread.join();

    ASSERT_TRUE(li->is_closed());
}

TEST(tcp_listener, multithread_mixed_stress)
{
    hj::tcp_listener::io_t io;

    auto                     work_guard = boost::asio::make_work_guard(io);
    std::vector<std::thread> io_workers;
    for(int i = 0; i < 4; ++i)
    {
        io_workers.emplace_back([&io]() { io.run(); });
    }

    auto              li = hj::tcp_listener::create(io);
    std::atomic<bool> stop{false};

    std::thread t_accept([&]() {
        while(!stop.load())
        {
            li->async_accept(12021,
                             [](const hj::tcp_listener::err_t &, auto) {});
            std::this_thread::yield();
        }
    });

    std::thread t_cancel([&]() {
        while(!stop.load())
        {
            li->cancel();
            std::this_thread::yield();
        }
    });

    std::thread t_listen([&]() {
        while(!stop.load())
        {
            li->listen(12021);
            std::this_thread::yield();
        }
    });

    std::thread t_close([&]() {
        for(int i = 0; i < 50; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            li->close();
            li->open();
        }
        stop.store(true);
    });

    t_accept.join();
    t_cancel.join();
    t_listen.join();
    t_close.join();

    li->close();
    work_guard.reset();
    for(auto &worker : io_workers)
    {
        if(worker.joinable())
            worker.join();
    }

    ASSERT_TRUE(li->is_closed());
}