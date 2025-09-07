#ifndef GRPC_HPP
#define GRPC_HPP

#include <grpcpp/grpcpp.h>
#include <grpcpp/impl/service_type.h>
#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <iostream>

namespace libcpp
{

class grpc_server 
{
public:
    grpc_server() : _running(false), _shutdown_requested(false) {}

    ~grpc_server() 
    { 
        stop(); 
    }

    template<typename ServiceType>
    bool start(const std::string& address, ServiceType* service) 
    {
        grpc::ServerBuilder builder;
        builder.AddListeningPort(address, grpc::InsecureServerCredentials());
        builder.RegisterService(service);
        _cq = builder.AddCompletionQueue();
        _server = builder.BuildAndStart();
        if (!_server) 
            return false;

        _running = true;
        _shutdown_requested = false;
        _cq_thread = std::thread([this]() { this->cq_worker(); });
        _server_thread = std::thread([this]() { _server->Wait(); });
        return true;
    }

    void stop() 
    {
        if (_server) 
        {
            if (!_shutdown_requested.exchange(true)) 
            {
                _server->Shutdown();
                if (_cq) 
                    _cq->Shutdown();
            }
            _running = false;
            if (_server_thread.joinable())
                _server_thread.join();

            if (_cq_thread.joinable())
                _cq_thread.join();

            _cq.reset();
            _server.reset();
        }
    }

    bool is_running() const { return _running; }

private:
    void cq_worker() {
        void* tag;
        bool ok;
        while (_cq && _cq->Next(&tag, &ok)) 
        {
            // No custom async events, just drain
        }
    }

    std::unique_ptr<grpc::Server> _server;
    std::unique_ptr<grpc::ServerCompletionQueue> _cq;
    std::thread _server_thread;
    std::thread _cq_thread;
    std::atomic<bool> _running;
    std::atomic<bool> _shutdown_requested;
};

class grpc_channel 
{
public:
    grpc_channel() = default;

    bool connect(const std::string& address) 
    {
        _channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
        return (_channel != nullptr);
    }

    std::shared_ptr<grpc::Channel> get() const { return _channel; }

private:
    std::shared_ptr<grpc::Channel> _channel;
};

} // namespace libcpp

#endif // GRPC_HPP