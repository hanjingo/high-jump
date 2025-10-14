#include <gtest/gtest.h>
#include <hj/net/udp/udp_socket.hpp>
#include <thread>
#include <chrono>

TEST(udp_socket, basic_construct_and_close)
{
    boost::asio::io_context io;
    hj::udp::socket         s(io.get_executor());
    EXPECT_TRUE(s.is_open());
    s.close();
    EXPECT_FALSE(s.is_open());
}

TEST(udp_socket, bind_and_set_option)
{
    boost::asio::io_context io;
    hj::udp::socket         s(io.get_executor());
    std::error_code         ec;

    s.bind(3001, ec);
    EXPECT_FALSE(ec);

    s.set_option(hj::udp::opt::reuse_addr(true), ec);
    EXPECT_FALSE(ec);
}

TEST(udp_socket, send_recv_error)
{
    boost::asio::io_context     io;
    std::error_code             ec;
    hj::udp::socket             p1(io.get_executor());
    hj::udp::socket             p2(io.get_executor());
    char                        buf[16] = {};
    size_t                      len     = sizeof(buf);
    hj::udp::socket::endpoint_t ep =
        hj::udp::socket::endpoint("127.0.0.1", 3002);

    p1.bind(3002, ec);
    EXPECT_FALSE(ec);

    size_t sent = p2.send(buf, len, ep, ec);
    EXPECT_EQ(sent, len);
    EXPECT_FALSE(ec);

    hj::udp::socket::endpoint_t sender;
    size_t                      recvd = p1.recv(buf, len, sender, ec);
    EXPECT_EQ(recvd, len);
    EXPECT_FALSE(ec);

    // buffer_sequence send/recv
    std::vector<hj::udp::socket::const_buffer_t> bufs = {
        boost::asio::buffer(buf, len)};
    sent = p2.send(bufs, ep, ec);
    EXPECT_EQ(sent, len);
    EXPECT_FALSE(ec);

    std::vector<hj::udp::socket::multi_buffer_t> mbufs = {
        boost::asio::mutable_buffer(buf, len)};
    recvd = p1.recv(mbufs, sender, ec);
    EXPECT_EQ(recvd, len);
    EXPECT_FALSE(ec);
}

TEST(udp_socket, async_send_recv_error)
{
    boost::asio::io_context     io;
    std::error_code             ec;
    hj::udp::socket             p1(io.get_executor(), 0);
    hj::udp::socket             p2(io.get_executor(), 0);
    char                        buf[16] = {};
    size_t                      len     = sizeof(buf);
    hj::udp::socket::endpoint_t ep =
        hj::udp::socket::endpoint("127.0.0.1", 3003);
    static bool is_udp_send_called = false, is_udp_recv_called = false;

    p1.bind(3003, ec);
    p1.async_recv(buf,
                  len,
                  ep,
                  [len](const std::error_code &ec, std::size_t sz) {
                      is_udp_recv_called = true;
                      EXPECT_FALSE(ec);
                      EXPECT_EQ(sz, len);
                  });

    p2.async_send(buf,
                  len,
                  ep,
                  [len](const std::error_code &ec, std::size_t sz) {
                      is_udp_send_called = true;
                      EXPECT_FALSE(ec);
                      EXPECT_EQ(sz, len);
                  });

    io.run();
    EXPECT_TRUE(is_udp_send_called);
    EXPECT_TRUE(is_udp_recv_called);
}

TEST(udp_socket, invalid_argument)
{
    boost::asio::io_context     io;
    hj::udp::socket             s(io.get_executor());
    std::error_code             ec;
    char                       *buf = nullptr;
    size_t                      len = 0;
    hj::udp::socket::endpoint_t ep =
        hj::udp::socket::endpoint("127.0.0.1", 3004);
    size_t sent = s.send(buf, len, ep, ec);
    EXPECT_EQ(sent, 0);
    EXPECT_EQ(ec, hj::udp::make_err(hj::udp::error_code::invalid_argument));

    s.async_send(
        buf,
        len,
        ep,
        [len](const std::error_code &ec, std::size_t sz) {
            EXPECT_EQ(ec,
                      hj::udp::make_err(hj::udp::error_code::invalid_argument));
            EXPECT_EQ(sz, 0);
        });
}

TEST(udp_socket, executor_and_socket_reuse)
{
    boost::asio::io_context io;
    hj::udp::socket::sock_t raw_sock(io.get_executor());
    raw_sock.open(boost::asio::ip::udp::v4());
    std::unique_ptr<hj::udp::socket::sock_t> uptr_sock =
        std::make_unique<hj::udp::socket::sock_t>(std::move(raw_sock));
    hj::udp::socket s(io.get_executor(), std::move(uptr_sock));
    EXPECT_TRUE(s.is_open());
    s.close();
    EXPECT_FALSE(s.is_open());
}