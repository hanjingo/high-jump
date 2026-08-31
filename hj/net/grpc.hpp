#ifndef HJ_GRPC_HPP
#define HJ_GRPC_HPP

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/impl/service_type.h>
#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <system_error>
#include <chrono>
#include <variant>
#include <vector>
#include <iostream>

namespace hj
{

enum class grpc_errc
{
    success = 0,
    invalid_argument,
    already_started,
    channel_not_initialized,
    connection_timeout,
    bind_failed,
    server_build_failed,
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
            case grpc_errc::already_started:
                return "Server is already running or starting";
            case grpc_errc::channel_not_initialized:
                return "gRPC channel is not initialized";
            case grpc_errc::connection_timeout:
                return "Timed out waiting for gRPC channel to become ready";
            case grpc_errc::bind_failed:
                return "Failed to bind address or port (port already in use or "
                       "invalid address)";
            case grpc_errc::server_build_failed:
                return "Failed to build and start gRPC server (internal gRPC "
                       "failure)";
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

inline std::shared_ptr<grpc::ServerCredentials>
make_tls_server_credentials(const std::string &pem_root_certs,
                            const std::string &pem_cert_chain,
                            const std::string &pem_private_key,
                            bool               require_client_cert = false)
{
    grpc::SslServerCredentialsOptions::PemKeyCertPair key_cert_pair = {
        pem_private_key,
        pem_cert_chain};

    grpc::SslServerCredentialsOptions options(
        require_client_cert
            ? GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY
            : GRPC_SSL_DONT_REQUEST_CLIENT_CERTIFICATE);
    options.pem_root_certs = pem_root_certs;
    options.pem_key_cert_pairs.push_back(key_cert_pair);

    return grpc::SslServerCredentials(options);
}

struct grpc_server_options
{
    int max_receive_message_size = -1;
    int max_send_message_size    = -1;

    int keepalive_time_ms    = -1;
    int keepalive_timeout_ms = -1;
    int keepalive_permit_without_calls =

        grpc_compression_algorithm compression_algorithm = GRPC_COMPRESS_NONE;

    using custom_arg_t = std::variant<std::pair<std::string, int>,
                                      std::pair<std::string, std::string>>;
    std::vector<custom_arg_t> custom_arguments;

    grpc_server_options &set_max_receive_message_size(int size)
    {
        max_receive_message_size = size;
        return *this;
    }

    grpc_server_options &set_max_send_message_size(int size)
    {
        max_send_message_size = size;
        return *this;
    }

    grpc_server_options &set_keepalive(int  time_ms,
                                       int  timeout_ms,
                                       bool permit_without_calls = false)
    {
        keepalive_time_ms              = time_ms;
        keepalive_timeout_ms           = timeout_ms;
        keepalive_permit_without_calls = permit_without_calls ? 1 : 0;
        return *this;
    }

    grpc_server_options &
    set_compression_algorithm(grpc_compression_algorithm algo)
    {
        compression_algorithm = algo;
        return *this;
    }

    grpc_server_options &add_argument(const std::string &key, int value)
    {
        custom_arguments.emplace_back(
            std::in_place_type<std::pair<std::string, int>>,
            key,
            value);
        return *this;
    }

    grpc_server_options &add_argument(const std::string &key,
                                      const std::string &value)
    {
        custom_arguments.emplace_back(
            std::in_place_type<std::pair<std::string, std::string>>,
            key,
            value);
        return *this;
    }
};

class grpc_server
{
  public:
    enum class state
    {
        stopped,
        starting,
        running,
        stopping
    };

    grpc_server()
        : _state(state::stopped)
    {
    }

    ~grpc_server() { stop(); }

    grpc_server(const grpc_server &)            = delete;
    grpc_server &operator=(const grpc_server &) = delete;
    grpc_server(grpc_server &&)                 = delete;
    grpc_server &operator=(grpc_server &&)      = delete;

    // Insecure credentials are intended for trusted/internal/test environments only.
    template <typename ServiceType>
    std::error_code
    start(const std::string                       &address,
          ServiceType                             *service,
          std::shared_ptr<grpc::ServerCredentials> credentials =
              grpc::InsecureServerCredentials(),
          const grpc_server_options &options = grpc_server_options())
    {
        if(!service)
            return make_error_code(grpc_errc::invalid_argument);

        {
            std::lock_guard<std::mutex> lock(_mutex);
            if(_state != state::stopped)
                return make_error_code(grpc_errc::already_started);
            _state = state::starting;
        }

        grpc::ServerBuilder builder;
        int                 selected_port = 0;
        builder.AddListeningPort(address, credentials, &selected_port);
        builder.RegisterService(service);

        if(options.max_receive_message_size >= 0)
            builder.SetMaxReceiveMessageSize(options.max_receive_message_size);

        if(options.max_send_message_size >= 0)
            builder.SetMaxSendMessageSize(options.max_send_message_size);

        if(options.compression_algorithm != GRPC_COMPRESS_NONE)
            builder.SetDefaultCompressionAlgorithm(
                options.compression_algorithm);

        if(options.keepalive_time_ms >= 0)
            builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS,
                                       options.keepalive_time_ms);
        if(options.keepalive_timeout_ms >= 0)
            builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIMEOUT_MS,
                                       options.keepalive_timeout_ms);
        if(options.keepalive_permit_without_calls >= 0)
            builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS,
                                       options.keepalive_permit_without_calls);

        for(const auto &arg : options.custom_arguments)
        {
            if(std::holds_alternative<std::pair<std::string, int>>(arg))
            {
                const auto &[key, val] =
                    std::get<std::pair<std::string, int>>(arg);
                builder.AddChannelArgument(key, val);
            } else if(std::holds_alternative<
                          std::pair<std::string, std::string>>(arg))
            {
                const auto &[key, val] =
                    std::get<std::pair<std::string, std::string>>(arg);
                builder.AddChannelArgument(key, val);
            }
        }

        auto temp_server = builder.BuildAndStart();
        if(!temp_server)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _state = state::stopped;
            _cv.notify_all();
            if(selected_port == 0)
            {
                return make_error_code(grpc_errc::bind_failed);
            }
            return make_error_code(grpc_errc::server_build_failed);
        }

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _server        = std::move(temp_server);
            _state         = state::running;
            _server_thread = std::thread([server = _server.get(), this]() {
                server->Wait();
                std::lock_guard<std::mutex> lock(_mutex);
                _state = state::stopped;
                _cv.notify_all();
            });
        }

        return {};
    }

    std::error_code stop()
    {
        std::unique_ptr<grpc::Server> local_server;
        std::thread                   local_thread;

        {
            std::lock_guard<std::mutex> lock(_mutex);
            if(_state == state::stopped || _state == state::stopping)
                return {};

            _state       = state::stopping;
            local_server = std::move(_server);
            local_thread = std::move(_server_thread);
        }

        if(local_server)
        {
            local_server->Shutdown();
        }

        if(local_thread.joinable())
        {
            local_thread.join();
        }

        return {};
    }

    void wait()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _cv.wait(lock, [this] { return _state == state::stopped; });
    }

    bool is_running() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _state == state::running;
    }

    state get_state() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _state;
    }

  private:
    std::unique_ptr<grpc::Server> _server;
    std::thread                   _server_thread;
    state                         _state;
    mutable std::mutex            _mutex;
    std::condition_variable       _cv;
};

struct grpc_channel_options
{
    int max_receive_message_size = -1;
    int max_send_message_size    = -1;

    int keepalive_time_ms              = -1;
    int keepalive_timeout_ms           = -1;
    int keepalive_permit_without_calls = -1;

    grpc_compression_algorithm compression_algorithm = GRPC_COMPRESS_NONE;

    using custom_arg_t = std::variant<std::pair<std::string, int>,
                                      std::pair<std::string, std::string>>;
    std::vector<custom_arg_t> custom_arguments;

    grpc_channel_options &set_max_receive_message_size(int size)
    {
        max_receive_message_size = size;
        return *this;
    }

    grpc_channel_options &set_max_send_message_size(int size)
    {
        max_send_message_size = size;
        return *this;
    }

    grpc_channel_options &set_keepalive(int  time_ms,
                                        int  timeout_ms,
                                        bool permit_without_calls = false)
    {
        keepalive_time_ms              = time_ms;
        keepalive_timeout_ms           = timeout_ms;
        keepalive_permit_without_calls = permit_without_calls ? 1 : 0;
        return *this;
    }

    grpc_channel_options &
    set_compression_algorithm(grpc_compression_algorithm algo)
    {
        compression_algorithm = algo;
        return *this;
    }

    grpc_channel_options &add_argument(const std::string &key, int value)
    {
        custom_arguments.emplace_back(
            std::in_place_type<std::pair<std::string, int>>,
            key,
            value);
        return *this;
    }

    grpc_channel_options &add_argument(const std::string &key,
                                       const std::string &value)
    {
        custom_arguments.emplace_back(
            std::in_place_type<std::pair<std::string, std::string>>,
            key,
            value);
        return *this;
    }

    grpc::ChannelArguments to_grpc_args() const
    {
        grpc::ChannelArguments args;
        if(max_receive_message_size >= 0)
            args.SetMaxReceiveMessageSize(max_receive_message_size);
        if(max_send_message_size >= 0)
            args.SetMaxSendMessageSize(max_send_message_size);
        if(compression_algorithm != GRPC_COMPRESS_NONE)
            args.SetCompressionAlgorithm(compression_algorithm);

        if(keepalive_time_ms >= 0)
            args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, keepalive_time_ms);
        if(keepalive_timeout_ms >= 0)
            args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, keepalive_timeout_ms);
        if(keepalive_permit_without_calls >= 0)
            args.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS,
                        keepalive_permit_without_calls);

        for(const auto &arg : custom_arguments)
        {
            if(std::holds_alternative<std::pair<std::string, int>>(arg))
            {
                const auto &[k, v] = std::get<std::pair<std::string, int>>(arg);
                args.SetInt(k, v);
            } else if(std::holds_alternative<
                          std::pair<std::string, std::string>>(arg))
            {
                const auto &[k, v] =
                    std::get<std::pair<std::string, std::string>>(arg);
                args.SetString(k, v);
            }
        }
        return args;
    }
};

class grpc_channel : public std::enable_shared_from_this<grpc_channel>
{
  public:
    grpc_channel() = default;

    explicit grpc_channel(
        std::string                               target,
        std::shared_ptr<grpc::ChannelCredentials> credentials =
            grpc::InsecureChannelCredentials(),
        const grpc_channel_options &options = grpc_channel_options())
    {
        init(std::move(target), std::move(credentials), options);
    }

    static std::shared_ptr<grpc_channel>
    create(const std::string                        &target,
           std::shared_ptr<grpc::ChannelCredentials> credentials =
               grpc::InsecureChannelCredentials(),
           const grpc_channel_options &options = grpc_channel_options())
    {
        auto ch = std::make_shared<grpc_channel>();
        if(!ch->init(target, credentials, options))
        {
            return nullptr;
        }
        return ch;
    }

    bool init(std::string                               target,
              std::shared_ptr<grpc::ChannelCredentials> credentials =
                  grpc::InsecureChannelCredentials(),
              const grpc_channel_options &options = grpc_channel_options())
    {
        _target   = std::move(target);
        auto args = options.to_grpc_args();
        _channel  = grpc::CreateCustomChannel(_target, credentials, args);
        return (_channel != nullptr);
    }

    bool connect()
    {
        if(!_channel)
            return false;
        _channel->GetState(true /* try_to_connect */);
        return true;
    }

    const std::string &target() const noexcept { return _target; }

    grpc_connectivity_state state(bool try_to_connect = false) const
    {
        if(!_channel)
            return GRPC_CHANNEL_SHUTDOWN;
        return _channel->GetState(try_to_connect);
    }

    bool is_ready() const { return state(false) == GRPC_CHANNEL_READY; }

    template <typename Rep, typename Period>
    std::error_code
    wait_until_ready(const std::chrono::duration<Rep, Period> &timeout)
    {
        if(!_channel)
            return make_error_code(grpc_errc::channel_not_initialized);

        auto deadline = std::chrono::system_clock::now() + timeout;

        while(true)
        {
            auto current_state = _channel->GetState(true /* try_to_connect */);
            if(current_state == GRPC_CHANNEL_READY)
            {
                return {};
            }
            if(current_state == GRPC_CHANNEL_SHUTDOWN)
            {
                return make_error_code(grpc_errc::connection_timeout);
            }
            if(!_channel->WaitForStateChange(current_state, deadline))
            {
                if(_channel->GetState(false) == GRPC_CHANNEL_READY)
                {
                    return {};
                }
                return make_error_code(grpc_errc::connection_timeout);
            }
        }
    }

    void reset_connection()
    {
        if(_channel)
        {
            _channel->GetState(true /* try_to_connect */);
        }
    }

    std::shared_ptr<grpc::Channel> get() const { return _channel; }

    operator std::shared_ptr<grpc::Channel>() const { return _channel; }

  private:
    std::string                    _target;
    std::shared_ptr<grpc::Channel> _channel;
};

} // namespace hj

#endif // HJ_GRPC_HPP