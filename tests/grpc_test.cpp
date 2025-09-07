#include <gtest/gtest.h>
#include <libcpp/misc/grpc.hpp>
#include <grpcpp/grpcpp.h>
#include <thread>
#include <chrono>
#include "grpc_test.grpc.pb.h"

using namespace GrpcLibrary;

// 服务实现补充
class TestGrpcServiceImpl : public GrpcService::Service {
public:
    ::grpc::Status SayHello(::grpc::ServerContext* context,
                            const HelloRequest* request,
                            HelloReply* response) override {
        (void)context;
        response->set_message("Hello, " + request->name());
        return ::grpc::Status::OK;
    }
};

// 用 new 分配，永不 delete
TestGrpcServiceImpl* g_service = new TestGrpcServiceImpl();

TEST(GRPCTest, SayHelloUnaryCall) {
    libcpp::grpc_server server;
    std::string address = "127.0.0.1:50051";
    ASSERT_TRUE(server.start(address, g_service));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    libcpp::grpc_channel channel;
    ASSERT_TRUE(channel.connect(address));
    auto stub = GrpcService::NewStub(channel.get());

    HelloRequest req;
    req.set_name("World");
    HelloReply reply;
    grpc::ClientContext ctx;
    grpc::Status status = stub->SayHello(&ctx, req, &reply);

    EXPECT_TRUE(status.ok());
    EXPECT_EQ(reply.message(), "Hello, World");

    server.stop();
    // 不 delete g_service，进程退出时由操作系统回收
}