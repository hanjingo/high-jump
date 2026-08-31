#ifndef HJ_GRPC_HPP
#define HJ_GRPC_HPP

#include <grpcpp/grpcpp.h>
#include <grpcpp/impl/service_type.h>
#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <system_error>
#include <iostream>

namespace hj
{

enum class grpc_errc
{
    success = 0,
    invalid_argument,
    null_server,
    already_started,
    not_running,
};

namespace detail
{
class grpc_category final : public std::error_category
{
  public:
    const char *name() const noexcept override { return "hj::grpc"; }

    std::string message(int ev) const override
    {
        switch(static_cast<grpc_errc>(ev))
        {
            case grpc_errc::success:
                return "Success";
            case grpc_errc::invalid_argument:
                return "Invalid argument (nullptr or illegal parameter)";
            case grpc_errc::null_server:
                return "Underlying grpc::Server pointer is nullptr";
            case grpc_errc::already_started:
                return "Server is already running";
            case grpc_errc::not_running:
                return "Server is not currently running";
            default:
                return "Unknown gRPC error code";
        }
    }
};

inline const std::error_category &grpc_category_instance() noexcept
{
    static grpc_category inst;
    return inst;
}
} // namespace hj::detail

inline std::error_code make_error_code(grpc_errc e) noexcept
{
    return std::error_code(static_cast<int>(e),
                           detail::grpc_category_instance());
}
} // namespace hj

namespace std
{
template <>
struct is_error_code_enum<hj::grpc_errc> : std::true_type
{
};
} // namespace std

namespace hj
{
class grpc_server
{
  public:
    grpc_server()
        : _running(false)
        , _shutdown_requested(false)
    {
    }

    ~grpc_server() { stop(); }

    grpc_server(const grpc_server &)            = delete;
    grpc_server &operator=(const grpc_server &) = delete;
    grpc_server(grpc_server &&)                 = delete;
    grpc_server &operator=(grpc_server &&)      = delete;

    template <typename ServiceType>
    std::error_code
    start(const std::string                       &address,
          ServiceType                             *service,
          std::shared_ptr<grpc::ServerCredentials> credentials =
              grpc::InsecureServerCredentials(),
          const grpc::ChannelArguments &args = grpc::ChannelArguments())
    {
        if(!service)
            return make_error_code(grpc_errc::invalid_argument);

        std::lock_guard<std::mutex> lock(_mutex);
        if(_running)
            return make_error_code(grpc_errc::already_started);

        grpc::ServerBuilder builder;
        builder.AddListeningPort(address, credentials);
        builder.RegisterService(service);

        _server = builder.BuildAndStart();
        if(!_server)
            return make_error_code(grpc_errc::null_server);

        _shutdown_requested = false;
        _running            = true;

        _server_thread = std::thread([this]() { _server->Wait(); });

        return {};
    }

    std::error_code stop()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if(!_running && !_server)
            return {};

        if(!_shutdown_requested.exchange(true))
        {
            if(_server)
            {
                _server->Shutdown();
            }
        }

        _running = false;

        if(_server_thread.joinable())
        {
            _server_thread.join();
        }

        _server.reset();
        _shutdown_requested = false;
        return {};
    }

    bool is_running() const { return _running; }

  private:
    std::unique_ptr<grpc::Server> _server;
    std::thread                   _server_thread;
    std::atomic<bool>             _running;
    std::atomic<bool>             _shutdown_requested;
    mutable std::mutex            _mutex;
};

class grpc_channel
{
  public:
    grpc_channel() = default;

    bool connect(const std::string                        &address,
                 std::shared_ptr<grpc::ChannelCredentials> credentials =
                     grpc::InsecureChannelCredentials(),
                 const grpc::ChannelArguments &args = grpc::ChannelArguments())
    {
        _channel = grpc::CreateCustomChannel(address, credentials, args);
        return (_channel != nullptr);
    }

    std::shared_ptr<grpc::Channel> get() const { return _channel; }

    bool is_connected() const
    {
        if(!_channel)
            return false;
        auto state = _channel->GetState(false);
        return (state == GRPC_CHANNEL_READY || state == GRPC_CHANNEL_IDLE);
    }

  private:
    std::shared_ptr<grpc::Channel> _channel;
};

} // namespace hj

#endif // HJ_GRPC_HPP