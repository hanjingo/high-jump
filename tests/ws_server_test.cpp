#include <gtest/gtest.h>
#include <hj/net/http/ws_client.hpp>
#include <hj/net/http/ws_server.hpp>
#include <thread>
#include <chrono>
#include <iostream> // Include iostream for std::cout

TEST(ws_server, accept_recv_send_close)
{
    std::thread([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        hj::ws_client::io_t io;
        hj::ws_client       client(io);
        ASSERT_TRUE(client.connect("127.0.0.1", "9003", "/"));
        ASSERT_TRUE(client.is_connected());

        std::string msg = "hello";
        ASSERT_TRUE(client.send(msg));

        std::string recv_msg;
        ASSERT_TRUE(client.recv(recv_msg));
        EXPECT_EQ(recv_msg, "world");

        client.close();
        ASSERT_FALSE(client.is_connected());
    }).detach();

    hj::ws_server::io_t  io;
    hj::ws_server::err_t err;
    auto                 ep = hj::ws_server::make_endpoint("127.0.0.1", 9003);

    hj::ws_server serv(io);
    auto          ws = serv.accept(ep, err);
    ASSERT_FALSE(err);

    std::string msg = serv.recv(ws, err);
    ASSERT_FALSE(err);
    ASSERT_EQ(msg, "hello");

    size_t sz = serv.send(ws, err, std::string("world"));
    ASSERT_FALSE(err);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    serv.close();
}

// TEST(ws_server, async_accept_recv_send_close)
// {
//     hj::ws_server::io_t  io;
//     hj::ws_server::err_t err;
//     hj::ws_server        serv(io);
//     hj::ws_client        client(io);
//     client.async_connect(
//         "127.0.0.1",
//         "12345",
//         "/",
//         [&client](const hj::ws_client::err_t &err) {
//             ASSERT_FALSE(err);

//             client.async_send(
//                 "async_hello",
//                 [&client](const hj::ws_client::err_t &err, std::size_t) {
//                     ASSERT_FALSE(err);

//                     client.async_recv([&client](const hj::ws_client::err_t &err,
//                                                 std::string msg) {
//                         ASSERT_FALSE(err);
//                         EXPECT_EQ(msg, "async_world");

//                         client.async_close([](const hj::ws_client::err_t &err) {
//                             ASSERT_FALSE(err);
//                         });
//                     });
//                 });
//         });

//     serv.async_accept(
//         hj::ws_server::make_endpoint("127.0.0.1", 9003),
//         [&serv](const hj::ws_server::err_t    &err,
//                 hj::ws_server::ws_stream_ptr_t ws) {
//             if(err)
//                 std::cout << "ws_server async_accept error: " << err.message()
//                           << std::endl;
//             ASSERT_FALSE(err);

//             serv.async_recv(
//                 ws,
//                 [&serv](const hj::ws_server::err_t    &err,
//                         hj::ws_server::ws_stream_ptr_t ws,
//                         std::string                    msg) {
//                     if(err)
//                         std::cout
//                             << "ws_server async_recv error: " << err.message()
//                             << std::endl;
//                     ASSERT_FALSE(err);
//                     EXPECT_EQ(msg, "async_hello");

//                     serv.async_send(
//                         ws,
//                         "async_world",
//                         [&serv](const hj::ws_server::err_t    &err,
//                                 hj::ws_server::ws_stream_ptr_t ws,
//                                 std::size_t                    n) {
//                             if(err)
//                                 std::cout << "ws_server async_send error: "
//                                           << err.message() << std::endl;
//                             ASSERT_FALSE(err);

//                             std::this_thread::sleep_for(
//                                 std::chrono::milliseconds(10));
//                             serv.async_close(
//                                 [](const hj::ws_server::err_t &err) {
//                                     if(err)
//                                         std::cout
//                                             << "ws_server async_close error: "
//                                             << err.value() << ":"
//                                             << err.message() << std::endl;
//                                     ASSERT_FALSE(err);
//                                 });
//                         });
//                 });
//         });

//     io.run();
// }