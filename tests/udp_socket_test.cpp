#include <gtest/gtest.h>
#include <hj/net/udp/udp_socket.hpp>
#include <thread>
#include <chrono>
#include <vector>
#include <string_view>
#include <atomic>

TEST(udp_socket, basic_construct_and_close)
{
    boost::asio::io_context io;
    auto                    s = hj::udp_socket::make_shared(io.get_executor());
    EXPECT_TRUE(s->is_open());
    s->close();
    EXPECT_FALSE(s->is_open());
}

TEST(udp_socket, close_twice)
{
    boost::asio::io_context   io;
    boost::system::error_code ec;
    auto s = hj::udp_socket::make_shared(io.get_executor());

    s->close(ec);
    EXPECT_FALSE(ec);
    EXPECT_FALSE(s->is_open());

    s->close(ec);
    s->close();
    EXPECT_FALSE(s->is_open());
}

TEST(udp_socket, bind_same_port_failure)
{
    boost::asio::io_context   io;
    boost::system::error_code ec;

    auto s1 = hj::udp_socket::make_shared(io.get_executor());
    auto s2 = hj::udp_socket::make_shared(io.get_executor());

    s1->bind(4001, ec);
    EXPECT_FALSE(ec);

    s2->bind(4001, ec);
    EXPECT_TRUE(ec);
}

TEST(udp_socket, invalid_ip_handling)
{
    boost::system::error_code ec;

#if BOOST_VERSION < 108700
    auto addr = boost::asio::ip::address::from_string("invalid.ip.address", ec);
    EXPECT_TRUE(ec);
#else
    auto addr = boost::asio::ip::make_address("invalid.ip.address", ec);
    EXPECT_TRUE(ec);
#endif
}

TEST(udp_socket, async_send_receive_payload_validation)
{
    boost::asio::io_context   io;
    boost::system::error_code ec;

    auto receiver = hj::udp_socket::make_shared(io.get_executor());
    auto sender   = hj::udp_socket::make_shared(io.get_executor());

    receiver->bind(4011, ec);
    EXPECT_FALSE(ec);

    constexpr std::string_view msg          = "async_udp_test_payload";
    char                       recv_buf[64] = {0};
    hj::udp_socket::endpoint_t from_ep;

    bool send_called = false;
    bool recv_called = false;

    receiver->async_receive_from(
        boost::asio::buffer(recv_buf),
        from_ep,
        [&](const boost::system::error_code &err, std::size_t recvd) {
            EXPECT_FALSE(err);
            EXPECT_EQ(recvd, msg.size());
            EXPECT_EQ(std::string_view(recv_buf, recvd), msg);
            recv_called = true;
        });

    auto target_ep = hj::udp_socket::endpoint("127.0.0.1", 4011);

    sender->async_send_to(
        std::string(msg),
        target_ep,
        [&](const boost::system::error_code &err, std::size_t sent) {
            EXPECT_FALSE(err);
            EXPECT_EQ(sent, msg.size());
            send_called = true;
        });

    io.run();

    EXPECT_TRUE(send_called);
    EXPECT_TRUE(recv_called);
}

TEST(udp_socket, connected_udp_send_receive_payload_validation)
{
    boost::asio::io_context   io;
    boost::system::error_code ec;

    auto s1 = hj::udp_socket::make_shared(io.get_executor());
    auto s2 = hj::udp_socket::make_shared(io.get_executor());

    s1->bind(4002, ec);
    EXPECT_FALSE(ec);

    s2->bind(4003, ec);
    EXPECT_FALSE(ec);

    s2->connect("127.0.0.1", 4002, ec);
    EXPECT_FALSE(ec);

    constexpr std::string_view msg          = "connected_udp_data";
    char                       recv_buf[64] = {0};

    size_t sent = s2->send(boost::asio::buffer(msg.data(), msg.size()), ec);
    EXPECT_FALSE(ec);
    EXPECT_EQ(sent, msg.size());

    hj::udp_socket::endpoint_t from_ep;
    size_t recvd = s1->receive_from(boost::asio::buffer(recv_buf), from_ep, ec);
    EXPECT_FALSE(ec);

    EXPECT_EQ(recvd, msg.size());
    EXPECT_EQ(std::string_view(recv_buf, recvd), msg);
}

TEST(udp_socket, zero_length_datagram)
{
    boost::asio::io_context   io;
    boost::system::error_code ec;

    auto receiver = hj::udp_socket::make_shared(io.get_executor());
    auto sender   = hj::udp_socket::make_shared(io.get_executor());

    receiver->bind(4004, ec);
    EXPECT_FALSE(ec);

    auto   ep   = hj::udp_socket::endpoint("127.0.0.1", 4004);
    size_t sent = sender->send_to(boost::asio::buffer("", 0), ep, ec);
    EXPECT_FALSE(ec);
    EXPECT_EQ(sent, 0u);

    char                       recv_buf[16] = {'X'};
    hj::udp_socket::endpoint_t from_ep;
    size_t                     recvd =
        receiver->receive_from(boost::asio::buffer(recv_buf), from_ep, ec);

    EXPECT_FALSE(ec);
    EXPECT_EQ(recvd, 0u);
    EXPECT_EQ(recv_buf[0], 'X');
}

TEST(udp_socket, datagram_truncation_behavior)
{
    boost::asio::io_context   io;
    boost::system::error_code ec;

    auto receiver = hj::udp_socket::make_shared(io.get_executor());
    auto sender   = hj::udp_socket::make_shared(io.get_executor());

    receiver->bind(4005, ec);
    EXPECT_FALSE(ec);

    std::string large_payload(4096, 'A');
    auto        ep = hj::udp_socket::endpoint("127.0.0.1", 4005);
    sender->send_to(boost::asio::buffer(large_payload), ep, ec);
    EXPECT_FALSE(ec);

    char                       small_recv_buf[1024] = {0};
    hj::udp_socket::endpoint_t from_ep;
    size_t recvd = receiver->receive_from(boost::asio::buffer(small_recv_buf),
                                          from_ep,
                                          ec);

    if(ec)
    {
        EXPECT_EQ(ec, boost::asio::error::message_size);
    } else
    {
        EXPECT_EQ(recvd, sizeof(small_recv_buf));
    }

    EXPECT_EQ(std::string_view(small_recv_buf, sizeof(small_recv_buf)),
              std::string_view(large_payload.data(), sizeof(small_recv_buf)));
}

TEST(udp_socket, large_datagram_within_mtu)
{
    boost::asio::io_context   io;
    boost::system::error_code ec;

    auto receiver = hj::udp_socket::make_shared(io.get_executor());
    auto sender   = hj::udp_socket::make_shared(io.get_executor());

    receiver->bind(4006, ec);

    std::string       send_payload(8192, 'B');
    std::vector<char> recv_buf(8192, 0);

    auto ep = hj::udp_socket::endpoint("127.0.0.1", 4006);
    sender->send_to(boost::asio::buffer(send_payload), ep, ec);
    EXPECT_FALSE(ec);

    hj::udp_socket::endpoint_t from_ep;
    size_t                     recvd =
        receiver->receive_from(boost::asio::buffer(recv_buf), from_ep, ec);
    EXPECT_FALSE(ec);
    EXPECT_EQ(recvd, send_payload.size());
    EXPECT_EQ(std::string_view(recv_buf.data(), recvd), send_payload);
}

TEST(udp_socket, broadcast_send_recv_payload_validation)
{
    boost::asio::io_context   io;
    boost::system::error_code ec;

    auto receiver = hj::udp_socket::make_shared(io.get_executor());
    receiver->set_option(hj::udp_socket::opt_reuse_addr(true), ec);
    receiver->bind(4007, ec);
    EXPECT_FALSE(ec);

    auto sender = hj::udp_socket::make_shared(io.get_executor());
    sender->set_option(hj::udp_socket::opt_broadcast(true), ec);
    EXPECT_FALSE(ec);

    constexpr std::string_view msg          = "broadcast_payload";
    char                       recv_buf[64] = {0};

    auto bcast_ep = hj::udp_socket::endpoint("255.255.255.255", 4007);
    sender->send_to(msg.data(), msg.size(), bcast_ep, ec);

    if(!ec)
    {
        hj::udp_socket::endpoint_t from_ep;
        size_t                     recvd =
            receiver->receive_from(boost::asio::buffer(recv_buf), from_ep, ec);
        if(!ec)
        {
            EXPECT_EQ(recvd, msg.size());
            EXPECT_EQ(std::string_view(recv_buf, recvd), msg);
        }
    }
}

TEST(udp_socket, cancel_pending_async_receive)
{
    boost::asio::io_context   io;
    boost::system::error_code ec;

    auto receiver = hj::udp_socket::make_shared(io.get_executor());
    receiver->bind(4008, ec);

    char                       recv_buf[16];
    hj::udp_socket::endpoint_t from_ep;

    bool                      handler_called = false;
    boost::system::error_code async_ec;

    receiver->async_receive_from(
        boost::asio::buffer(recv_buf),
        from_ep,
        [&](const boost::system::error_code &err, std::size_t) {
            handler_called = true;
            async_ec       = err;
        });

    receiver->close();
    io.run();

    EXPECT_TRUE(handler_called);
    EXPECT_EQ(async_ec, boost::asio::error::operation_aborted);
}

TEST(udp_socket, socket_destroyed_during_async_operation)
{
    boost::asio::io_context io;

    bool                      handler_called = false;
    boost::system::error_code async_ec;

    char                       recv_buf[16];
    hj::udp_socket::endpoint_t from_ep;

    {
        auto sock = hj::udp_socket::make_shared(io.get_executor());
        boost::system::error_code ec;
        sock->bind(4009, ec);

        sock->async_receive_from(
            boost::asio::buffer(recv_buf),
            from_ep,
            [&handler_called, &async_ec](const boost::system::error_code &err,
                                         std::size_t) {
                handler_called = true;
                async_ec       = err;
            });

        sock->close();
    }

    io.run();

    EXPECT_TRUE(handler_called);
    EXPECT_EQ(async_ec, boost::asio::error::operation_aborted);
}

TEST(udp_socket, multithreaded_io_context_payload_validation)
{
    boost::asio::io_context   io;
    boost::system::error_code ec;

    const int                  total_packets = 100;
    std::atomic<int>           received_count{0};
    constexpr std::string_view expected_msg = "mt_test_payload";

    auto receiver = hj::udp_socket::make_shared(io.get_executor());
    receiver->bind(4010, ec);
    EXPECT_FALSE(ec);

    auto sender = hj::udp_socket::make_shared(io.get_executor());

    auto recv_buf = std::make_shared<std::array<char, 64>>();
    auto from_ep  = std::make_shared<hj::udp_socket::endpoint_t>();

    std::function<void()> do_receive = [&]() {
        receiver->async_receive_from(
            boost::asio::buffer(*recv_buf),
            *from_ep,
            [&, recv_buf, from_ep](const boost::system::error_code &err,
                                   std::size_t                      recvd) {
                if(!err)
                {
                    EXPECT_EQ(std::string_view(recv_buf->data(), recvd),
                              expected_msg);
                    if(++received_count < total_packets)
                    {
                        do_receive();
                    }
                }
            });
    };

    do_receive();

    std::vector<std::thread> threads;
    for(int i = 0; i < 4; ++i)
    {
        threads.emplace_back([&io]() { io.run(); });
    }

    auto target_ep = hj::udp_socket::endpoint("127.0.0.1", 4010);
    for(int i = 0; i < total_packets; ++i)
    {
        sender->send_to(expected_msg.data(),
                        expected_msg.size(),
                        target_ep,
                        ec);
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    for(auto &t : threads)
    {
        if(t.joinable())
            t.join();
    }

    EXPECT_EQ(received_count.load(), total_packets);
}