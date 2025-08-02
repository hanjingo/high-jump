#include <gtest/gtest.h>
#include <libcpp/misc/grpc.hpp>
// #include <grpcpp/grpcpp.h>
// #include <thread>
// #include <chrono>

// // Dummy gRPC service definition for test
// class TestServiceImpl final : public grpc::Service {};

// TEST(GRPCTest, ServerStartStop) {
//     libcpp::grpc_server server;
//     TestServiceImpl service;
//     ASSERT_TRUE(server.start("0.0.0.0:50051", &service));
//     ASSERT_TRUE(server.is_running());
//     server.stop();
//     ASSERT_FALSE(server.is_running());
// }

// TEST(GRPCTest, ChannelConnect) {
//     libcpp::grpc_channel channel;
//     ASSERT_TRUE(channel.connect("localhost:50051"));
//     auto ch = channel.get();
//     ASSERT_TRUE(ch != nullptr);
// }

