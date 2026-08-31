#include <gtest/gtest.h>
#include <hj/net/udp/udp_socket.hpp>
#include <thread>
#include <chrono>

TEST(udp_socket, basic_construct_and_close)
{
    boost::asio::io_context io;
    auto                    s = hj::udp::socket::create(io.get_executor());
    EXPECT_TRUE(s->is_open());
    s->close();
    EXPECT_FALSE(s->is_open());
}

TEST(udp_socket, bind_and_set_option)
{
    boost::asio::io_context   io;
    auto                      s = hj::udp::socket::create(io.get_executor());
    boost::system::error_code ec;

    s->bind(3001, ec);
    EXPECT_FALSE(ec);

    s->set_option(hj::udp::opt::reuse_addr(true), ec);
    EXPECT_FALSE(ec);
}

TEST(udp_socket, send_recv_sync)
{
    boost::asio::io_context   io;
    boost::system::error_code ec;
    auto                      p1 = hj::udp::socket::create(io.get_executor());
    auto                      p2 = hj::udp::socket::create(io.get_executor());

    char   send_buf[16] = "hello_udp";
    char   recv_buf[16] = {};
    size_t len          = sizeof(send_buf);

    auto ep = hj::udp::socket::endpoint("127.0.0.1", 3002);

    p1->bind(3002, ec);
    EXPECT_FALSE(ec);

    size_t sent = p2->send_to(send_buf, len, ep, ec);
    EXPECT_EQ(sent, len);
    EXPECT_FALSE(ec);

    hj::udp::socket::endpoint_t sender;
    size_t recvd = p1->receive_from(recv_buf, len, sender, ec);
    EXPECT_EQ(recvd, len);
    EXPECT_FALSE(ec);

    // Buffer Sequence send/receive
    std::vector<hj::udp::socket::const_buffer_t> bufs = {
        boost::asio::buffer(send_buf, len)};
    sent = p2->send_to(bufs, ep, ec);
    EXPECT_EQ(sent, len);
    EXPECT_FALSE(ec);

    std::vector<hj::udp::socket::multi_buffer_t> mbufs = {
        boost::asio::buffer(recv_buf, len)};
    recvd = p1->receive_from(mbufs, sender, ec);
    EXPECT_EQ(recvd, len);
    EXPECT_FALSE(ec);
}

TEST(udp_socket, async_send_recv_safe)
{
    boost::asio::io_context   io;
    boost::system::error_code ec;
    auto                      p1 = hj::udp::socket::create(io.get_executor());
    auto                      p2 = hj::udp::socket::create(io.get_executor());

    char   buf[16] = "async_data";
    size_t len     = sizeof(buf);
    auto   ep      = hj::udp::socket::endpoint("127.0.0.1", 3003);

    static bool is_udp_send_called = false, is_udp_recv_called = false;

    p1->bind(3003, ec);

    p1->async_receive_from(
        boost::asio::buffer(buf, len),
        ep,
        [len](const boost::system::error_code &ec, std::size_t sz) {
            is_udp_recv_called = true;
            EXPECT_FALSE(ec);
            EXPECT_EQ(sz, len);
        });

    p2->async_send_to(
        boost::asio::buffer(buf, len),
        ep,
        [len](const boost::system::error_code &ec, std::size_t sz) {
            is_udp_send_called = true;
            EXPECT_FALSE(ec);
            EXPECT_EQ(sz, len);
        });

    io.run();
    EXPECT_TRUE(is_udp_send_called);
    EXPECT_TRUE(is_udp_recv_called);
}

TEST(udp_socket, executor_and_socket_reuse)
{
    boost::asio::io_context io;
    hj::udp::socket::sock_t raw_sock(io.get_executor());
    raw_sock.open(boost::asio::ip::udp::v4());

    auto s = hj::udp::socket::create(std::move(raw_sock));
    EXPECT_TRUE(s->is_open());
    s->close();
    EXPECT_FALSE(s->is_open());
}