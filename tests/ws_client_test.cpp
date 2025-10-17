#include <gtest/gtest.h>
#include <hj/net/http/ws_client.hpp>
#include <hj/net/http/ws_server.hpp>
#include <thread>
#include <chrono>
#include <iostream> // Include iostream for std::cout
#include <filesystem>

TEST(ws_client, connect_send_recv_close)
{
    std::thread([]() {
        hj::ws_server::io_t  io;
        hj::ws_server::err_t err;
        auto ep = hj::ws_server::make_endpoint("127.0.0.1", 20001);

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
    }).detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    hj::ws_client::io_t io;
    auto                client = std::make_shared<hj::ws_client>(io);
    ASSERT_TRUE(client->connect("127.0.0.1", "20001", "/"));
    ASSERT_TRUE(client->is_connected());

    std::string msg = "hello";
    ASSERT_TRUE(client->send(msg));

    std::string recv_msg;
    ASSERT_TRUE(client->recv(recv_msg));
    EXPECT_EQ(recv_msg, "world");

    client->close();
    ASSERT_FALSE(client->is_connected());
}

TEST(ws_client, async_connect_send_recv_close)
{
    static bool is_async_ws_client_running = false;
    std::thread t([]() {
        hj::ws_server::io_t  io;
        hj::ws_server::err_t err;
        auto ep = hj::ws_server::make_endpoint("127.0.0.1", 20002);

        auto serv = std::make_shared<hj::ws_server>(io);
        auto ws   = serv->accept(ep, err);
        ASSERT_FALSE(err);

        std::string msg = serv->recv(ws, err);
        ASSERT_FALSE(err);

        size_t sz = serv->send(ws, err, std::string("async_world"));
        ASSERT_FALSE(err);

        serv->recv(ws, err);
        serv->close();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    hj::ws_client::io_t io;
    auto                client = std::make_shared<hj::ws_client>(io);
    client->async_connect(
        "127.0.0.1",
        "20002",
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
                                is_async_ws_client_running = true;
                            });
                    });
                });
        });

    io.run();
    t.join();
    ASSERT_TRUE(is_async_ws_client_running);
}

TEST(ws_client_ssl, connect_send_recv_close)
{
    std::string server_crt = "./server.crt";
    std::string server_key = "./server.key";
    std::string client_crt = "./client.crt";
    std::string client_key = "./client.key";
    if(!std::filesystem::exists(server_crt)
       || !std::filesystem::exists(server_key)
       || !std::filesystem::exists(client_crt)
       || !std::filesystem::exists(client_key))
    {
        GTEST_SKIP() << "skip test ws_client_ssl connect_send_recv_close "
                        "with file not exist: "
                     << server_crt << " " << server_key << " " << client_crt
                     << " " << client_key;
    }

    std::thread([server_crt, server_key]() {
        hj::ws_server_ssl::io_t io;


        auto ssl_ctx = hj::ws_server_ssl::make_ctx(server_crt, server_key);
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
    auto ssl_ctx = hj::ws_client_ssl::make_ctx(client_crt, client_key);
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
}

TEST(ws_client_ssl, async_connect_send_recv_close)
{
    std::string server_crt2 = "./server.crt";
    std::string server_key2 = "./server.key";
    std::string client_crt2 = "./client.crt";
    std::string client_key2 = "./client.key";
    if(!std::filesystem::exists(server_crt2)
       || !std::filesystem::exists(server_key2)
       || !std::filesystem::exists(client_crt2)
       || !std::filesystem::exists(client_key2))
    {
        GTEST_SKIP() << "skip test ws_client_ssl async_connect_send_recv_close "
                        "with file not exist: "
                     << server_crt2 << " " << server_key2 << " " << client_crt2
                     << " " << client_key2;
    }

    static bool is_async_ws_client_ssl_running = false;
    std::thread t([server_crt2, server_key2]() {
        hj::ws_server_ssl::io_t io;
        auto ssl_ctx = hj::ws_server_ssl::make_ctx(server_crt2, server_key2);
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
    auto ssl_ctx2 = hj::ws_client_ssl::make_ctx(client_crt2, client_key2);
    auto client   = std::make_shared<hj::ws_client_ssl>(io, ssl_ctx2);
    client->async_connect(
        "127.0.0.1",
        "12346",
        "/",
        [client](const hj::ws_client_ssl::err_t &err) {
            if(err)
                std::cout << "ws_client_ssl async_connect error: "
                          << err.message() << std::endl;
            ASSERT_FALSE(err);

            client->async_send(
                "async_hello_ssl",
                [client](const hj::ws_client_ssl::err_t &err, std::size_t) {
                    if(err)
                        std::cout << "ws_client_ssl async_send error: "
                                  << err.message() << std::endl;
                    ASSERT_FALSE(err);

                    client->async_recv([client](
                                           const hj::ws_client_ssl::err_t &err,
                                           std::string msg) {
                        if(err)
                            std::cout << "ws_client_ssl async_recv error: "
                                      << err.message() << std::endl;
                        ASSERT_FALSE(err);
                        EXPECT_EQ(msg, "async_world_ssl");

                        client->async_close(
                            [](const hj::ws_client_ssl::err_t &err) {
                                if(err)
                                    std::cout
                                        << "ws_client_ssl async_close error: "
                                        << err.value() << ":" << err.message()
                                        << std::endl;
                                ASSERT_FALSE(err);
                                is_async_ws_client_ssl_running = true;
                            });
                    });
                });
        });

    io.run();
    t.join();
    ASSERT_TRUE(is_async_ws_client_ssl_running);
}