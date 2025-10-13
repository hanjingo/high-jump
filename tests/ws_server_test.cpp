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
        auto                client = std::make_shared<hj::ws_client>(io);
        ASSERT_TRUE(client->connect("127.0.0.1", "9003", "/"));
        ASSERT_TRUE(client->is_connected());

        std::string msg = "hello";
        ASSERT_TRUE(client->send(msg));

        std::string recv_msg;
        ASSERT_TRUE(client->recv(recv_msg));
        EXPECT_EQ(recv_msg, "world");

        client->close();
        ASSERT_FALSE(client->is_connected());
    }).detach();

    hj::ws_server::io_t  io;
    hj::ws_server::err_t err;
    auto                 ep = hj::ws_server::make_endpoint("127.0.0.1", 9003);

    auto serv = std::make_shared<hj::ws_server>(io);
    auto ws   = serv->accept(ep, err);
    ASSERT_FALSE(err);

    std::string msg = serv->recv(ws, err);
    ASSERT_FALSE(err);
    ASSERT_EQ(msg, "hello");

    size_t sz = serv->send(ws, err, std::string("world"));
    ASSERT_FALSE(err);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    serv->close();
}

TEST(ws_server, async_accept_recv_send_close)
{
    static std::atomic<bool> is_async_ws_server_running{false};
    hj::ws_server::io_t      io;
    hj::ws_server::err_t     err;
    auto                     serv   = std::make_shared<hj::ws_server>(io);
    auto                     client = std::make_shared<hj::ws_client>(io);
    client->async_connect(
        "127.0.0.1",
        "9003",
        "/",
        [client](const hj::ws_client::err_t &err) {
            ASSERT_FALSE(err);

            client->async_send(
                "async_hello",
                [client](const hj::ws_client::err_t &err, std::size_t) {
                    ASSERT_FALSE(err);

                    client->async_recv([client](const hj::ws_client::err_t &err,
                                                std::string msg) {
                        ASSERT_FALSE(err);
                        EXPECT_EQ(msg, "async_world");

                        client->async_close(
                            [](const hj::ws_client::err_t &err) {
                                ASSERT_FALSE(err);
                            });
                    });
                });
        });

    serv->async_accept(
        hj::ws_server::make_endpoint("127.0.0.1", 9003),
        [serv](const hj::ws_server::err_t    &err,
               hj::ws_server::ws_stream_ptr_t ws) {
            std::cout << "ws_server async_accept callback" << std::endl;
            if(err)
                std::cout << "ws_server async_accept error: " << err.message()
                          << std::endl;
            ASSERT_FALSE(err);

            serv->async_recv(
                ws,
                [serv](const hj::ws_server::err_t    &err,
                       hj::ws_server::ws_stream_ptr_t ws,
                       std::string                    msg) {
                    std::cout << "ws_server async_recv callback" << std::endl;
                    if(err)
                        std::cout
                            << "ws_server async_recv error: " << err.message()
                            << std::endl;
                    ASSERT_FALSE(err);
                    EXPECT_EQ(msg, "async_hello");

                    serv->async_send(
                        ws,
                        "async_world",
                        [serv](const hj::ws_server::err_t    &err,
                               hj::ws_server::ws_stream_ptr_t ws,
                               std::size_t                    n) {
                            std::cout << "ws_server async_send callback"
                                      << std::endl;
                            if(err)
                                std::cout << "ws_server async_send error: "
                                          << err.message() << std::endl;
                            ASSERT_FALSE(err);

                            std::this_thread::sleep_for(
                                std::chrono::milliseconds(10));
                            serv->async_close(
                                [](const hj::ws_server::err_t &err) {
                                    std::cout
                                        << "ws_server async_close callback"
                                        << std::endl;
                                    if(err)
                                        std::cout
                                            << "ws_server async_close error: "
                                            << err.value() << ":"
                                            << err.message() << std::endl;
                                    is_async_ws_server_running.store(true);
                                    ASSERT_FALSE(err);
                                });
                        });
                });
        });

    io.run();
    ASSERT_TRUE(is_async_ws_server_running);
}

TEST(ws_server_ssl, connect_recv_send_close)
{
    std::thread([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        hj::ws_client_ssl::io_t io;
        auto ssl_ctx = hj::ws_client_ssl::make_ctx("client.crt", "client.key");
        auto client  = std::make_shared<hj::ws_client_ssl>(io, ssl_ctx);
        ASSERT_TRUE(client->connect("127.0.0.1", "9004", "/"));
        ASSERT_TRUE(client->is_connected());

        std::string msg = "hello_ssl";
        ASSERT_TRUE(client->send(msg));

        std::string recv_msg;
        ASSERT_TRUE(client->recv(recv_msg));
        EXPECT_EQ(recv_msg, "world_ssl");

        client->close();
        ASSERT_FALSE(client->is_connected());
    }).detach();

    hj::ws_server_ssl::io_t io;
    auto ssl_ctx = hj::ws_server_ssl::make_ctx("server.crt", "server.key");
    hj::ws_server_ssl::err_t err;
    auto              ep = hj::ws_server_ssl::make_endpoint("127.0.0.1", 9004);
    hj::ws_server_ssl serv(io, ssl_ctx);
    auto              ws = serv.accept(ep, err);
    ASSERT_FALSE(err);

    std::string msg = serv.recv(ws, err);
    ASSERT_FALSE(err);
    ASSERT_EQ(msg, "hello_ssl");

    size_t sz = serv.send(ws, err, std::string("world_ssl"));
    ASSERT_FALSE(err);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    serv.close();
}

TEST(ws_server_ssl, async_connect_recv_send_close)
{
    static bool             is_async_ws_server_ssl_running = false;
    hj::ws_server_ssl::io_t io;
    auto ssl_ctx    = hj::ws_server_ssl::make_ctx("server.crt", "server.key");
    auto serv       = std::make_shared<hj::ws_server_ssl>(io, ssl_ctx);
    auto client_ctx = hj::ws_client_ssl::make_ctx("client.crt", "client.key");
    auto client     = std::make_shared<hj::ws_client_ssl>(io, client_ctx);
    client->async_connect(
        "127.0.0.1",
        "9004",
        "/",
        [client](const hj::ws_client_ssl::err_t &err) {
            ASSERT_FALSE(err);

            client->async_send(
                "async_hello_ssl",
                [client](const hj::ws_client_ssl::err_t &err, std::size_t) {
                    ASSERT_FALSE(err);

                    client->async_recv(
                        [client](const hj::ws_client_ssl::err_t &err,
                                 std::string                     msg) {
                            ASSERT_FALSE(err);
                            EXPECT_EQ(msg, "async_world_ssl");

                            client->async_close(
                                [](const hj::ws_client_ssl::err_t &err) {
                                    ASSERT_FALSE(err);
                                });
                        });
                });
        });

    serv->async_accept(
        hj::ws_server_ssl::make_endpoint("127.0.0.1", 9004),
        [serv](const hj::ws_server_ssl::err_t    &err,
               hj::ws_server_ssl::ws_stream_ptr_t ws) {
            std::cout << "ws_server_ssl async_accept callback" << std::endl;
            if(err)
                std::cout << "ws_server_ssl async_accept error: "
                          << err.message() << std::endl;
            ASSERT_FALSE(err);

            serv->async_recv(
                ws,
                [serv](const hj::ws_server_ssl::err_t    &err,
                       hj::ws_server_ssl::ws_stream_ptr_t ws,
                       std::string                        msg) {
                    std::cout << "ws_server_ssl async_recv callback"
                              << std::endl;
                    if(err)
                        std::cout << "ws_server_ssl async_recv error: "
                                  << err.message() << std::endl;
                    ASSERT_FALSE(err);
                    EXPECT_EQ(msg, "async_hello_ssl");

                    serv->async_send(
                        ws,
                        "async_world_ssl",
                        [serv](const hj::ws_server_ssl::err_t    &err,
                               hj::ws_server_ssl::ws_stream_ptr_t ws,
                               std::size_t                        n) {
                            std::cout << "ws_server_ssl async_send callback"
                                      << std::endl;
                            if(err)
                                std::cout << "ws_server_ssl async_send error: "
                                          << err.message() << std::endl;
                            ASSERT_FALSE(err);

                            serv->async_close([](const hj::ws_server_ssl::err_t
                                                     &err) {
                                std::cout
                                    << "ws_server_ssl async_close callback"
                                    << std::endl;
                                if(err)
                                    std::cout
                                        << "ws_server_ssl async_close error: "
                                        << err.value() << ":" << err.message()
                                        << std::endl;
                                is_async_ws_server_ssl_running = true;
                                ASSERT_FALSE(err);
                            });
                        });
                });
        });

    io.run();

    ASSERT_TRUE(is_async_ws_server_ssl_running);
}