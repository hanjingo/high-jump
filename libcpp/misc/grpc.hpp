#ifndef GRPC_HPP
#define GRPC_HPP

// #include <grpc/grpc.h>
// #include <string>
// #include <memory>
// #include <functional>
// #include <thread>
// #include <atomic>
// #include <iostream>

// namespace libcpp
// {

// class grpc_server 
// {
// public:
//     grpc_server() : _running(false) {}
//     ~grpc_server() {}

//     template<typename ServiceType>
//     bool start(const std::string& address, ServiceType* service) 
//     {
//         grpc::ServerBuilder builder;
//         builder.AddListeningPort(address, grpc::InsecureServerCredentials());
//         builder.RegisterService(service);
//         _server = builder.BuildAndStart();
//         if (!_server) 
//             return false;

//         _running = true;
//         _server_thread = std::thread([this]() { _server->Wait(); });
//         return true;
//     }

//     void stop() 
//     {
//         if (_server) 
//         {
//             _server->Shutdown();
//             _running = false;
//             if (_server_thread.joinable())
//                 _server_thread.join();
//         }
//     }

//     bool is_running() const { return _running; }

//     ~grpc_server() { stop(); }

// private:
//     std::unique_ptr<grpc::Server> _server;
//     std::thread _server_thread;
//     std::atomic<bool> _running;
// };

// class grpc_channel 
// {
// public:
//     grpc_channel() = default;

//     bool connect(const std::string& address) 
//     {
//         _channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
//         return (_channel != nullptr);
//     }

//     std::shared_ptr<grpc::Channel> get() const { return _channel; }

// private:
//     std::shared_ptr<grpc::Channel> _channel;
// };

} // namespace libcpp

#endif // GRPC_HPP