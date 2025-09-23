#include <gtest/gtest.h>
#include <hj/net/http/ws_server.hpp>
#include <thread>
#include <chrono>

// TEST(WebSocketServerTest, Echo) {
//     hj::ws_server::io_t io;
//     hj::ws_server server(io);

//     // 监听本地9003端口
//     hj::ws_server::endpoint_t ep(hj::ws_server::address_t::from_string("127.0.0.1"), 9003);
//     bool accepted = false;
//     std::shared_ptr<hj::ws_server::ws_stream_t> ws_conn;

//     server.async_accept(ep, [&](const hj::ws_server::err_t& ec, std::shared_ptr<hj::ws_server::ws_stream_t> ws) {
//         accepted = !ec;
//         ws_conn = ws;
//     });

//     // 启动io线程
//     std::thread server_thread([&](){ io.run(); });

//     // 等待连接建立
//     for (int i = 0; i < 100 && !accepted; ++i) std::this_thread::sleep_for(std::chrono::milliseconds(10));
//     ASSERT_TRUE(accepted);
//     ASSERT_TRUE(ws_conn);

//     // 接收消息并回显
//     bool recv_ok = false, send_ok = false;
//     std::string recv_msg;
//     server.async_recv(ws_conn, [&](const hj::ws_server::err_t& ec, std::shared_ptr<hj::ws_server::ws_stream_t> ws, std::string msg){
//         recv_ok = !ec;
//         recv_msg = msg;
//         // 回显
//         server.async_send(ws, msg, [&](const hj::ws_server::err_t& ec2, std::shared_ptr<hj::ws_server::ws_stream_t>, std::size_t){
//             send_ok = !ec2;
//         });
//     });

//     // 等待消息处理
//     for (int i = 0; i < 100 && !recv_ok; ++i) std::this_thread::sleep_for(std::chrono::milliseconds(10));
//     for (int i = 0; i < 100 && !send_ok; ++i) std::this_thread::sleep_for(std::chrono::milliseconds(10));

//     ASSERT_TRUE(recv_ok);
//     ASSERT_TRUE(send_ok);
//     EXPECT_EQ(recv_msg, "hello server");

//     server.close();
//     server_thread.join();
// }

