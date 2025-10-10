#include <gtest/gtest.h>
#include <hj/net/http/ws_client.hpp>
#include <hj/net/http/ws_server.hpp>
#include <thread>
#include <chrono>
#include <iostream> // Include iostream for std::cout

TEST(ws_client, connect_send_recv_close)
{
    std::thread([]() {
        hj::ws_server::io_t  io;
        hj::ws_server::err_t err;
        auto ep = hj::ws_server::make_endpoint("127.0.0.1", 9003);

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
    }).detach();

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
}

TEST(ws_client, async_connect_send_recv_close)
{
    std::thread t([]() {
        hj::ws_server::io_t  io;
        hj::ws_server::err_t err;
        auto ep = hj::ws_server::make_endpoint("127.0.0.1", 12345);

        hj::ws_server serv(io);
        auto          ws = serv.accept(ep, err);
        ASSERT_FALSE(err);

        std::string msg = serv.recv(ws, err);
        ASSERT_FALSE(err);

        size_t sz = serv.send(ws, err, std::string("async_world"));
        ASSERT_FALSE(err);

        serv.recv(ws, err);
        serv.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    hj::ws_client::io_t io;
    hj::ws_client       client(io);
    client.async_connect(
        "127.0.0.1",
        "12345",
        "/",
        [&client](const hj::ws_client::err_t &err) {
            ASSERT_FALSE(err);

            client.async_send(
                "async_hello",
                [&client](const hj::ws_client::err_t &err, std::size_t) {
                    ASSERT_FALSE(err);

                    client.async_recv([&client](const hj::ws_client::err_t &err,
                                                std::string msg) {
                        ASSERT_FALSE(err);
                        EXPECT_EQ(msg, "async_world");

                        client.async_close([](const hj::ws_client::err_t &err) {
                            ASSERT_FALSE(err);
                        });
                    });
                });
        });

    io.run();
    t.join();
}

TEST(ws_client_ssl, connect_send_recv_close)
{
    std::thread([]() {
        hj::ws_server_ssl::io_t io;
        auto ssl_ctx = hj::ws_server_ssl::make_ctx("server.crt", "server.key");
        hj::ws_server_ssl        serv(io, ssl_ctx);
        hj::ws_server_ssl::err_t err;
        auto ep = hj::ws_server_ssl::make_endpoint("127.0.0.1", 9004);
        auto ws = serv.accept(ep, err);
        ASSERT_FALSE(err);

        std::string msg = serv.recv(ws, err);
        ASSERT_FALSE(err);
        ASSERT_EQ(msg, "hello_ssl");

        size_t sz = serv.send(ws, err, std::string("world_ssl"));
        ASSERT_FALSE(err);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        serv.close();
    }).detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::ws_client_ssl::io_t io;
    auto ssl_ctx = hj::ws_client_ssl::make_ctx("client.crt", "client.key");
    hj::ws_client_ssl client(io, ssl_ctx);
    ASSERT_TRUE(client.connect("127.0.0.1", "9004", "/"));
    ASSERT_TRUE(client.is_connected());

    std::string msg = "hello_ssl";
    ASSERT_TRUE(client.send(msg));

    std::string recv_msg;
    ASSERT_TRUE(client.recv(recv_msg));
    EXPECT_EQ(recv_msg, "world_ssl");

    client.close();
    ASSERT_FALSE(client.is_connected());
}

TEST(ws_client_ssl, async_connect_send_recv_close)
{
    std::thread t([]() {
        hj::ws_server_ssl::io_t io;
        auto ssl_ctx = hj::ws_server_ssl::make_ctx("server.crt", "server.key");
        hj::ws_server_ssl        serv(io, ssl_ctx);
        hj::ws_server_ssl::err_t err;
        auto ep = hj::ws_server_ssl::make_endpoint("127.0.0.1", 12346);
        auto ws = serv.accept(ep, err);
        if(err)
            std::cout << "ws_server_ssl accept error: " << err.message()
                      << std::endl;
        ASSERT_FALSE(err);

        std::string msg = serv.recv(ws, err);
        if(err)
            std::cout << "ws_server_ssl recv error: " << err.message()
                      << std::endl;
        ASSERT_FALSE(err);

        size_t sz = serv.send(ws, err, std::string("async_world_ssl"));
        if(err)
            std::cout << "ws_server_ssl send error: " << err.message()
                      << std::endl;
        ASSERT_FALSE(err);

        serv.recv(ws, err);
        serv.close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    hj::ws_client_ssl::io_t io;
    auto ssl_ctx = hj::ws_client_ssl::make_ctx("client.crt", "client.key");
    hj::ws_client_ssl client(io, ssl_ctx);
    client.async_connect(
        "127.0.0.1",
        "12346",
        "/",
        [&client](const hj::ws_client_ssl::err_t &err) {
            if(err)
                std::cout << "ws_client_ssl async_connect error: "
                          << err.message() << std::endl;
            ASSERT_FALSE(err);

            client.async_send(
                "async_hello_ssl",
                [&client](const hj::ws_client_ssl::err_t &err, std::size_t) {
                    if(err)
                        std::cout << "ws_client_ssl async_send error: "
                                  << err.message() << std::endl;
                    ASSERT_FALSE(err);

                    client.async_recv([&client](
                                          const hj::ws_client_ssl::err_t &err,
                                          std::string                     msg) {
                        if(err)
                            std::cout << "ws_client_ssl async_recv error: "
                                      << err.message() << std::endl;
                        ASSERT_FALSE(err);
                        EXPECT_EQ(msg, "async_world_ssl");

                        client.async_close(
                            [](const hj::ws_client_ssl::err_t &err) {
                                if(err)
                                    std::cout
                                        << "ws_client_ssl async_close error: "
                                        << err.value() << ":" << err.message()
                                        << std::endl;
                                ASSERT_FALSE(err);
                            });
                    });
                });
        });

    io.run();
    t.join();
}