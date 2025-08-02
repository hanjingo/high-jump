#ifndef GRPC_HPP
#define GRPC_HPP

// #include <grpcpp/grpcpp.h>
// #include <string>
// #include <memory>
// #include <functional>
// #include <thread>
// #include <atomic>
// #include <iostream>

// namespace libcpp
// {

// // Simple gRPC server wrapper
// class grpc_server {
// public:
//     grpc_server() : running_(false) {}

//     // Start the server with a given service and address
//     template<typename ServiceType>
//     bool start(const std::string& address, ServiceType* service) {
//         grpc::ServerBuilder builder;
//         builder.AddListeningPort(address, grpc::InsecureServerCredentials());
//         builder.RegisterService(service);
//         server_ = builder.BuildAndStart();
//         if (!server_) return false;
//         running_ = true;
//         server_thread_ = std::thread([this]() { server_->Wait(); });
//         return true;
//     }

//     // Stop the server
//     void stop() {
//         if (server_) {
//             server_->Shutdown();
//             running_ = false;
//             if (server_thread_.joinable())
//                 server_thread_.join();
//         }
//     }

//     bool is_running() const { return running_; }

//     ~grpc_server() { stop(); }

// private:
//     std::unique_ptr<grpc::Server> server_;
//     std::thread server_thread_;
//     std::atomic<bool> running_;
// };

// // Simple gRPC client wrapper
// class grpc_channel {
// public:
//     grpc_channel() = default;

//     // Create a channel to the given address
//     bool connect(const std::string& address) {
//         channel_ = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
//         return (channel_ != nullptr);
//     }

//     std::shared_ptr<grpc::Channel> get() const { return channel_; }

// private:
//     std::shared_ptr<grpc::Channel> channel_;
// };

// } // namespace libcpp

#endif // GRPC_HPP