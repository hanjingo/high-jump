#include <gtest/gtest.h>
#include <libcpp/net/http/ws_client.hpp>
#include <libcpp/net/http/ws_server.hpp>
#include <thread>
#include <chrono>

TEST(WebSocketClientTest, ConnectSendRecvClose) {
    std::thread([](){
        libcpp::ws_server::io_t io;
        libcpp::ws_server::err_t err;
        auto ep = libcpp::ws_server::make_endpoint("127.0.0.1", 9003);

        libcpp::ws_server serv(io);
        auto ws = serv.accept(ep, err);
        ASSERT_FALSE(err);

        std::string msg = serv.recv(ws, err);
        ASSERT_FALSE(err);
        ASSERT_EQ(msg, "hello");

        size_t sz = serv.send(ws, err, std::string("world"));
        ASSERT_FALSE(err);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        serv.close();
    }).detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    libcpp::ws_client::io_t io;
    libcpp::ws_client client(io);
    ASSERT_TRUE(client.connect("127.0.0.1", "9003", "/"));
    ASSERT_TRUE(client.is_connected());

    std::string msg = "hello";
    ASSERT_TRUE(client.send(msg));

    std::string recv_msg;
    ASSERT_TRUE(client.recv(recv_msg));
    EXPECT_EQ(recv_msg, "world");

    client.close();
    ASSERT_FALSE(client.is_connected());
}

TEST(WebSocketClientTest, AsyncConnectSendRecvClose) {
    std::thread t([](){
        libcpp::ws_server::io_t io;
        libcpp::ws_server::err_t err;
        auto ep = libcpp::ws_server::make_endpoint("127.0.0.1", 12345);

        libcpp::ws_server serv(io);
        auto ws = serv.accept(ep, err);
        ASSERT_FALSE(err);

        std::string msg = serv.recv(ws, err);
        ASSERT_FALSE(err);

        size_t sz = serv.send(ws, err, std::string("async_world"));
        ASSERT_FALSE(err);

        serv.recv(ws, err);
        serv.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    libcpp::ws_client::io_t io;
    libcpp::ws_client client(io);
    client.async_connect("127.0.0.1", "12345", "/", [&client](const libcpp::ws_client::err_t& err){
        ASSERT_FALSE(err);

        client.async_send("async_hello", [&client](const libcpp::ws_client::err_t& err, std::size_t){
            ASSERT_FALSE(err);
        
            client.async_recv([&client](const libcpp::ws_client::err_t& err, std::string msg){
                ASSERT_FALSE(err);
                EXPECT_EQ(msg, "async_world");
        
                client.async_close([](const libcpp::ws_client::err_t& err){
                    ASSERT_FALSE(err);
                });
            });
        });
    });

    io.run();
    t.join();
}