#include <gtest/gtest.h>
#include <hj/net/grpc.hpp>
#include <grpcpp/grpcpp.h>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include "grpc/grpc_test.grpc.pb.h"

namespace
{

using GrpcLibrary::GrpcService;
using GrpcLibrary::HelloReply;
using GrpcLibrary::HelloRequest;

class TestGrpcServiceImpl : public GrpcService::Service
{
  public:
    ::grpc::Status SayHello(::grpc::ServerContext *context,
                            const HelloRequest    *request,
                            HelloReply            *response) override
    {
        (void) context;
        response->set_message("Hello, " + request->name());
        return ::grpc::Status::OK;
    }
};

class GrpcTestFixture : public ::testing::Test
{
  protected:
    TestGrpcServiceImpl service;
};

} // namespace

TEST_F(GrpcTestFixture, say_hello_unary_call)
{
    hj::grpc_server server;
    std::string     address = "127.0.0.1:50062";

    auto err = server.start(address, &service);
    ASSERT_FALSE(err);

    hj::grpc_channel channel;
    ASSERT_TRUE(channel.init(address));
    channel.connect();

    ASSERT_FALSE(channel.wait_until_ready(std::chrono::seconds(2)));

    auto stub = GrpcService::NewStub(channel.get());

    HelloRequest req;
    req.set_name("World");
    HelloReply            reply;
    ::grpc::ClientContext ctx;
    ::grpc::Status        status = stub->SayHello(&ctx, req, &reply);

    EXPECT_TRUE(status.ok());
    EXPECT_EQ(reply.message(), "Hello, World");

    EXPECT_FALSE(server.stop());
}

TEST_F(GrpcTestFixture, start_null_service)
{
    hj::grpc_server server;
    std::string     address = "127.0.0.1:50063";

    TestGrpcServiceImpl *null_service = nullptr;
    auto err = server.start<TestGrpcServiceImpl>(address, null_service);

    EXPECT_EQ(err, hj::make_error_code(hj::grpc_errc::invalid_argument));
    EXPECT_FALSE(server.is_running());
}

TEST_F(GrpcTestFixture, duplicate_start)
{
    hj::grpc_server server;
    std::string     address = "127.0.0.1:50064";

    auto err1 = server.start(address, &service);
    EXPECT_FALSE(err1);

    auto err2 = server.start(address, &service);
    EXPECT_EQ(err2, hj::make_error_code(hj::grpc_errc::already_started));

    server.stop();
}

TEST_F(GrpcTestFixture, idempotent_stop)
{
    hj::grpc_server server;
    std::string     address = "127.0.0.1:50065";

    EXPECT_FALSE(server.start(address, &service));

    EXPECT_FALSE(server.stop());
    EXPECT_FALSE(server.is_running());

    EXPECT_FALSE(server.stop());
}

TEST_F(GrpcTestFixture, bind_conflict)
{
    hj::grpc_server server1;
    hj::grpc_server server2;
    std::string     address = "127.0.0.1:50066";

    ASSERT_FALSE(server1.start(address, &service));

    auto err = server2.start(address, &service);
    EXPECT_EQ(err, hj::make_error_code(hj::grpc_errc::bind_failed));

    server1.stop();
}

TEST_F(GrpcTestFixture, rpc_after_server_stop)
{
    hj::grpc_server server;
    std::string     address = "127.0.0.1:50067";

    ASSERT_FALSE(server.start(address, &service));

    hj::grpc_channel channel;
    ASSERT_TRUE(channel.init(address));
    auto stub = GrpcService::NewStub(channel.get());

    server.stop();

    HelloRequest req;
    req.set_name("World");
    HelloReply            reply;
    ::grpc::ClientContext ctx;
    ctx.set_deadline(std::chrono::system_clock::now()
                     + std::chrono::milliseconds(500));

    ::grpc::Status status = stub->SayHello(&ctx, req, &reply);

    EXPECT_FALSE(status.ok());
}

TEST_F(GrpcTestFixture, channel_uninitialized_state)
{
    hj::grpc_channel channel;

    EXPECT_FALSE(channel.is_ready());
    EXPECT_EQ(channel.get(), nullptr);

    auto err = channel.wait_until_ready(std::chrono::milliseconds(100));
    EXPECT_EQ(err, hj::make_error_code(hj::grpc_errc::channel_not_initialized));
}

TEST_F(GrpcTestFixture, invalid_address_rpc)
{
    std::string bad_address = "255.255.255.255:65535";

    hj::grpc_channel channel(bad_address);
    channel.connect();

    auto stub = GrpcService::NewStub(channel.get());

    HelloRequest req;
    req.set_name("Test");
    HelloReply            reply;
    ::grpc::ClientContext ctx;
    ctx.set_deadline(std::chrono::system_clock::now()
                     + std::chrono::milliseconds(200));

    ::grpc::Status status = stub->SayHello(&ctx, req, &reply);

    EXPECT_FALSE(status.ok());
    EXPECT_TRUE(status.error_code() == ::grpc::StatusCode::UNAVAILABLE
                || status.error_code()
                       == ::grpc::StatusCode::DEADLINE_EXCEEDED);
}

TEST_F(GrpcTestFixture, concurrent_start_stop_state)
{
    hj::grpc_server   server;
    std::string       address = "127.0.0.1:50068";
    std::atomic<bool> stop_flag{false};

    std::thread reader([&]() {
        while(!stop_flag.load())
        {
            (void) server.is_running();
            (void) server.get_state();
            std::this_thread::yield();
        }
    });

    std::vector<std::thread> workers;
    for(int i = 0; i < 10; ++i)
    {
        workers.emplace_back([&]() {
            if(!server.start(address, &service))
            {
                while(server.get_state() == hj::grpc_server::state::starting)
                {
                    std::this_thread::yield();
                }
                server.stop();
            }
        });
    }

    for(auto &t : workers)
    {
        if(t.joinable())
            t.join();
    }

    stop_flag.store(true);
    if(reader.joinable())
        reader.join();

    EXPECT_FALSE(server.is_running());
    EXPECT_EQ(server.get_state(), hj::grpc_server::state::stopped);
}

TEST_F(GrpcTestFixture, concurrent_rpc_requests)
{
    hj::grpc_server server;
    std::string     address = "127.0.0.1:50069";
    ASSERT_FALSE(server.start(address, &service));

    hj::grpc_channel channel;
    ASSERT_TRUE(channel.init(address));
    channel.connect();

    ASSERT_FALSE(channel.wait_until_ready(std::chrono::seconds(2)));

    const int                thread_count = 50;
    std::vector<std::thread> threads;
    std::atomic<int>         success_count{0};

    for(int i = 0; i < thread_count; ++i)
    {
        threads.emplace_back([&channel, &success_count, i]() {
            auto         stub = GrpcService::NewStub(channel.get());
            HelloRequest req;
            req.set_name("Thread " + std::to_string(i));
            HelloReply            reply;
            ::grpc::ClientContext ctx;

            ::grpc::Status status = stub->SayHello(&ctx, req, &reply);
            if(status.ok()
               && reply.message() == "Hello, Thread " + std::to_string(i))
            {
                success_count.fetch_add(1);
            }
        });
    }

    for(auto &t : threads)
    {
        if(t.joinable())
            t.join();
    }

    EXPECT_EQ(success_count.load(), thread_count);
    server.stop();
}

TEST_F(GrpcTestFixture, server_lifecycle_stress_test)
{
    hj::grpc_server server;
    std::string     address    = "127.0.0.1:50070";
    const int       iterations = 100;

    for(int i = 0; i < iterations; ++i)
    {
        ASSERT_FALSE(server.start(address, &service));
        EXPECT_TRUE(server.is_running());

        ASSERT_FALSE(server.stop());
        EXPECT_FALSE(server.is_running());
    }
}