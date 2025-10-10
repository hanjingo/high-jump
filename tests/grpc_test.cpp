#include <gtest/gtest.h>

#ifdef GRPC_ENABLE
#include <hj/net/grpc.hpp>
#include <grpcpp/grpcpp.h>
#include <thread>
#include <chrono>
#include "grpc/grpc_test.grpc.pb.h"

using namespace GrpcLibrary;

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

TestGrpcServiceImpl *g_service = new TestGrpcServiceImpl();

TEST(GRPCTest, SayHelloUnaryCall)
{
    hj::grpc_server server;
    std::string     address = "127.0.0.1:50051";
    ASSERT_TRUE(server.start(address, g_service));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    hj::grpc_channel channel;
    ASSERT_TRUE(channel.connect(address));
    auto stub = GrpcService::NewStub(channel.get());

    HelloRequest req;
    req.set_name("World");
    HelloReply          reply;
    grpc::ClientContext ctx;
    grpc::Status        status = stub->SayHello(&ctx, req, &reply);

    EXPECT_TRUE(status.ok());
    EXPECT_EQ(reply.message(), "Hello, World");

    server.stop();
}

#endif // GRPC_ENABLE