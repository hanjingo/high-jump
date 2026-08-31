#ifndef HJ_GRPC_HPP
#define HJ_GRPC_HPP

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <system_error>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

namespace hj
{

enum class grpc_errc
{
    success = 0,
    invalid_argument,
    already_started,
    channel_not_initialized,
    connection_timeout,
    channel_shutdown,
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
            case grpc_errc::channel_shutdown:
                return "gRPC channel is in SHUTDOWN state";
            case grpc_errc::bind_failed:
                return "Failed to bind address or port (port already in use or "
                       "invalid address)";
            case grpc_errc::server_build_failed:
                return "Failed to build and start gRPC server (internal gRPC "
                       "failure or invalid option)";
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
} // namespace detail

inline std::error_code make_error_code(grpc_errc e) noexcept
{
    return std::error_code(static_cast<int>(e),
                           detail::grpc_category_instance());
}

struct grpc_server_diagnostic
{
    std::string     address;
    int             selected_port;
    std::error_code error;
    std::string     details;

    std::string to_string() const
    {
        std::ostringstream oss;
        oss << "[gRPC Server Diagnostic] target address: '" << address
            << "', bound port: " << selected_port
            << ", error: " << error.message() << " (" << error.value() << ")";
        if(!details.empty())
        {
            oss << ", details: " << details;
        }
        return oss.str();
    }
};

inline std::ostream &operator<<(std::ostream                 &os,
                                const grpc_server_diagnostic &diag)
{
    return os << diag.to_string();
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

struct grpc_argument
{
    std::string                    key;
    std::variant<int, std::string> value;
};

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

    int keepalive_time_ms              = -1;
    int keepalive_timeout_ms           = -1;
    int keepalive_permit_without_calls = -1;

    grpc_compression_algorithm compression_algorithm = GRPC_COMPRESS_NONE;

    std::vector<grpc_argument> custom_arguments;

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
        custom_arguments.push_back(grpc_argument{key, value});
        return *this;
    }

    grpc_server_options &add_argument(const std::string &key,
                                      const std::string &value)
    {
        custom_arguments.push_back(grpc_argument{key, value});
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

    template <typename ServiceType>
    std::error_code
    start(const std::string                       &address,
          ServiceType                             *service,
          std::shared_ptr<grpc::ServerCredentials> credentials =
              grpc::InsecureServerCredentials(),
          const grpc_server_options &options = grpc_server_options())
    {
        if(!service)
        {
            set_diagnostic(address,
                           0,
                           make_error_code(grpc_errc::invalid_argument),
                           "Service pointer is null");
            return make_error_code(grpc_errc::invalid_argument);
        }

        std::thread old_thread;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if(_state != state::stopped)
            {
                set_diagnostic(address,
                               0,
                               make_error_code(grpc_errc::already_started),
                               "Server is not in stopped state");
                return make_error_code(grpc_errc::already_started);
            }

            if(_server_thread.joinable())
            {
                old_thread = std::move(_server_thread);
            }

            _server.reset();
            _state = state::starting;
        }

        if(old_thread.joinable())
        {
            old_thread.join();
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
            std::visit(
                [&builder, &arg](auto &&val) {
                    builder.AddChannelArgument(arg.key, val);
                },
                arg.value);
        }

        // hook for testing purposes to allow manipulation of the builder before building the server
        std::function<void()> hook_to_call;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            hook_to_call = _before_build_hook;
        }
        if(hook_to_call)
        {
            hook_to_call();
        }
        // hook end

        auto temp_server = builder.BuildAndStart();
        if(!temp_server)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _state = state::stopped;
            _cv.notify_all();

            std::error_code err;
            std::string     detail_msg;

            if(selected_port == 0)
            {
                err = make_error_code(grpc_errc::bind_failed);
                detail_msg =
                    "AddListeningPort failed for address: " + address
                    + " (port in use, illegal IP, or permission denied)";
            } else
            {
                err = make_error_code(grpc_errc::server_build_failed);
                detail_msg =
                    "BuildAndStart returned null despite valid port binding ("
                    + std::to_string(selected_port) + ")";
            }

            set_diagnostic(address, selected_port, err, detail_msg);
            return err;
        }

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _server = std::move(temp_server);
            _state  = state::running;

            set_diagnostic(address,
                           selected_port,
                           {},
                           "Server running successfully");
            _cv.notify_all();

            _server_thread = std::thread([server = _server.get(), this]() {
                server->Wait();

                std::lock_guard<std::mutex> lock(_mutex);
                if(_state == state::running)
                {
                    _state = state::stopped;
                }
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
            std::unique_lock<std::mutex> lock(_mutex);

            _cv.wait(lock, [this] { return _state != state::starting; });

            if(_state == state::stopping)
            {
                return {};
            }

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

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _state = state::stopped;
            _cv.notify_all();
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

    grpc_server_diagnostic last_diagnostic() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _diag;
    }

    void set_before_build_hook(std::function<void()> hook)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _before_build_hook = std::move(hook);
    }

  private:
    void set_diagnostic(const std::string &address,
                        int                port,
                        std::error_code    err,
                        const std::string &details)
    {
        _diag = grpc_server_diagnostic{address, port, err, details};
    }

    std::unique_ptr<grpc::Server> _server;
    std::thread                   _server_thread;
    state                         _state;
    mutable std::mutex            _mutex;
    std::condition_variable       _cv;
    grpc_server_diagnostic        _diag;
    std::function<void()>         _before_build_hook;
};

struct grpc_channel_options
{
    int max_receive_message_size = -1;
    int max_send_message_size    = -1;

    int keepalive_time_ms              = -1;
    int keepalive_timeout_ms           = -1;
    int keepalive_permit_without_calls = -1;

    grpc_compression_algorithm compression_algorithm = GRPC_COMPRESS_NONE;

    std::vector<grpc_argument> custom_arguments;

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
        custom_arguments.push_back(grpc_argument{key, value});
        return *this;
    }

    grpc_channel_options &add_argument(const std::string &key,
                                       const std::string &value)
    {
        custom_arguments.push_back(grpc_argument{key, value});
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
            std::visit(
                [&args, &arg](auto &&val) {
                    using T = std::decay_t<decltype(val)>;
                    if constexpr(std::is_same_v<T, int>)
                    {
                        args.SetInt(arg.key, val);
                    } else if constexpr(std::is_same_v<T, std::string>)
                    {
                        args.SetString(arg.key, val);
                    }
                },
                arg.value);
        }
        return args;
    }
};

class grpc_channel
{
  private:
    struct grpc_channel_key
    {
        explicit grpc_channel_key() = default;
    };

  public:
    explicit grpc_channel(grpc_channel_key) {}

    explicit grpc_channel(
        grpc_channel_key,
        std::string                               target,
        std::shared_ptr<grpc::ChannelCredentials> credentials,
        const grpc_channel_options &options = grpc_channel_options())
    {
        init(std::move(target), std::move(credentials), options);
    }

    static std::shared_ptr<grpc_channel> make_shared()
    {
        return std::make_shared<hj::grpc_channel>(grpc_channel_key{});
    }

    static std::shared_ptr<grpc_channel>
    make_shared(const std::string                        &target,
                std::shared_ptr<grpc::ChannelCredentials> credentials,
                const grpc_channel_options &options = grpc_channel_options())
    {
        auto ch = std::make_shared<grpc_channel>(grpc_channel_key{},
                                                 target,
                                                 credentials,
                                                 options);
        if(!target.empty())
            ch->init(target, credentials, options);

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

    bool is_ready() const
    {
        if(!_channel)
            return false;

        return state(false) == GRPC_CHANNEL_READY;
    }

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
                return make_error_code(grpc_errc::channel_shutdown);
            }

            if(!_channel->WaitForStateChange(current_state, deadline))
            {
                auto last_state = _channel->GetState(false);
                if(last_state == GRPC_CHANNEL_READY)
                {
                    return {};
                }

                if(last_state == GRPC_CHANNEL_SHUTDOWN)
                {
                    return make_error_code(grpc_errc::channel_shutdown);
                }

                return make_error_code(grpc_errc::connection_timeout);
            }
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