#include <gtest/gtest.h>
#include <libcpp/net/websocket.hpp>
// #include <thread>
// #include <chrono>

// // Note: This test requires a running echo WebSocket server on localhost:9002

// TEST(WebSocketTest, ConnectSendRecvClose) {
//     boost::asio::io_context io;
//     libcpp::websocket ws(io);

//     // Connect (blocking)
//     ASSERT_TRUE(ws.connect("127.0.0.1", "9002", "/"));
//     ASSERT_TRUE(ws.is_connected());

//     // Send message
//     std::string msg = "hello websocket";
//     ASSERT_TRUE(ws.send(msg));

//     // Receive echo
//     std::string recv_msg;
//     ASSERT_TRUE(ws.recv(recv_msg));
//     EXPECT_EQ(recv_msg, msg);

//     // Close
//     ws.close();
//     ASSERT_FALSE(ws.is_connected());
// }

// TEST(WebSocketTest, AsyncConnectSendRecv) {
//     boost::asio::io_context io;
//     libcpp::websocket ws(io);

//     bool connected = false;
//     ws.async_connect("127.0.0.1", "9002", "/", [&](const
//     boost::system::error_code& ec) {
//         connected = !ec;
//     });
//     io.run();
//     ASSERT_TRUE(connected);
//     ASSERT_TRUE(ws.is_connected());

//     // Async send
//     bool send_ok = false;
//     ws.async_send("async test", [&](const boost::system::error_code& ec,
//     std::size_t){
//         send_ok = !ec;
//     });
//     io.restart();
//     io.run();
//     ASSERT_TRUE(send_ok);

//     // Async receive
//     bool recv_ok = false;
//     std::string recv_msg;
//     ws.async_recv([&](const boost::system::error_code& ec, std::string msg){
//         recv_ok = !ec;
//         recv_msg = msg;
//     });
//     io.restart();
//     io.run();
//     ASSERT_TRUE(recv_ok);
//     EXPECT_EQ(recv_msg, "async test");

//     ws.close();
//     ASSERT_FALSE(ws.is_connected());
// }