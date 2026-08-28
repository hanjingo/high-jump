#ifndef TCP_CONN_HPP
#define TCP_CONN_HPP

#include <chrono>
#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <boost/asio.hpp>
#include <boost/version.hpp>

#include <hj/net/tcp/tcp_socket.hpp>

namespace hj
{

class tcp_conn : public std::enable_shared_from_this<tcp_conn>
{
  public:
    enum class state
    {
        idle,
        connecting,
        connected,
        closed
    };

    static constexpr std::size_t read_buffer_size = 65535;
    static constexpr std::size_t default_max_outstanding_bytes =
        64 * 1024 * 1024;

    using io_t  = boost::asio::io_context;
    using err_t = boost::system::error_code;

    using strand_t   = boost::asio::strand<io_t::executor_type>;
    using conn_ptr_t = std::shared_ptr<tcp_conn>;
    using sock_ptr_t = std::shared_ptr<hj::tcp_socket>;

    using buffer_t = std::vector<std::uint8_t>;

    using write_handler_t = std::function<void(conn_ptr_t, const buffer_t &)>;
    using read_handler_t  = std::function<void(conn_ptr_t, buffer_t &&)>;
    using err_handler_t   = std::function<void(conn_ptr_t, const err_t &)>;
    using conn_handler_t  = std::function<void(conn_ptr_t, const err_t &)>;

  private:
    struct tcp_conn_key
    {
        explicit tcp_conn_key() = default;
    };

  public:
    explicit tcp_conn(tcp_conn_key, io_t &io)
        : _io(io)
        , _strand{boost::asio::make_strand(io)}
        , _sock{nullptr}
        , _state{state::idle}
    {
    }

    explicit tcp_conn(tcp_conn_key, io_t &io, sock_ptr_t sock)
        : _io(io)
        , _strand{boost::asio::make_strand(io)}
        , _sock{std::move(sock)}
        , _state{state::idle}
    {
        if(_sock && _sock->status() == hj::tcp_socket::state::connected)
            _state = state::connected;
    }

    ~tcp_conn() noexcept
    {
        try
        {
            _state = state::closed;
            if(_sock)
            {
                _sock->close();
            }
            _send_queue.clear();
            _outstanding_write_bytes = 0;
        }
        catch(...)
        {
        }
    }

    tcp_conn(const tcp_conn &)            = delete;
    tcp_conn &operator=(const tcp_conn &) = delete;
    tcp_conn(tcp_conn &&)                 = delete;
    tcp_conn &operator=(tcp_conn &&)      = delete;

    template <typename... Args>
    static conn_ptr_t make_shared(Args &&...args)
    {
        auto conn = std::make_shared<tcp_conn>(tcp_conn_key{},
                                               std::forward<Args>(args)...);
        conn->_init();
        return conn;
    }

    void set_max_outstanding_write_bytes(std::size_t max_bytes)
    {
        conn_ptr_t self = shared_from_this();
        if(!self)
            return;

        boost::asio::dispatch(_strand,
                              [this, self = std::move(self), max_bytes]() {
                                  if(_is_closed())
                                      return;
                                  _max_outstanding_bytes = max_bytes;
                              });
    }

    void set_write_callback(write_handler_t cb)
    {
        conn_ptr_t self = shared_from_this();
        if(!self)
            return;

        boost::asio::dispatch(
            _strand,
            [this, self = std::move(self), cb = std::move(cb)]() mutable {
                if(_is_closed())
                    return;
                _write_cb = std::move(cb);
            });
    }

    void set_read_callback(read_handler_t cb)
    {
        conn_ptr_t self = shared_from_this();
        if(!self)
            return;

        boost::asio::dispatch(
            _strand,
            [this, self = std::move(self), cb = std::move(cb)]() mutable {
                if(_is_closed())
                    return;
                _read_cb = std::move(cb);
                _try_start_read();
            });
    }

    void set_error_callback(err_handler_t cb)
    {
        conn_ptr_t self = shared_from_this();
        if(!self)
            return;

        boost::asio::dispatch(
            _strand,
            [this, self = std::move(self), cb = std::move(cb)]() mutable {
                if(_is_closed())
                    return;
                _err_cb = std::move(cb);
            });
    }

    void
    async_connect(const char *ip, std::uint16_t port, conn_handler_t handler)
    {
        conn_ptr_t self = shared_from_this();
        if(!self)
            return;

        boost::asio::dispatch(
            _strand,
            [this,
             self   = std::move(self),
             ip_str = std::string(ip ? ip : ""),
             port,
             handler = std::move(handler)]() mutable {
                if(_state != state::idle)
                {
                    if(handler)
                    {
                        handler(self,
                                boost::system::errc::make_error_code(
                                    boost::system::errc::already_connected));
                    }
                    return;
                }

                _state = state::connecting;
                if(!_sock)
                    _sock = tcp_socket::make_shared(_io);

                _sock->async_connect(
                    ip_str.c_str(),
                    port,
                    boost::asio::bind_executor(
                        _strand,
                        [this, self, handler = std::move(handler)](
                            const err_t &ec) {
                            if(_is_closed())
                                return;

                            if(!ec)
                            {
                                _state = state::connected;
                                _try_start_read();
                            } else
                            {
                                _do_close();
                            }

                            if(handler)
                                handler(self, ec);
                        }));
            });
    }

    void send(buffer_t data)
    {
        conn_ptr_t self = shared_from_this();
        if(!self)
            return;

        const std::size_t data_size = data.size();

        boost::asio::dispatch(
            _strand,
            [this,
             self = std::move(self),
             data = std::move(data),
             data_size]() mutable {
                if(data.empty())
                {
                    if(_err_cb)
                        _err_cb(self,
                                boost::system::errc::make_error_code(
                                    boost::system::errc::invalid_argument));
                    return;
                }

                if(!_is_connected())
                {
                    if(_err_cb)
                        _err_cb(self,
                                boost::system::errc::make_error_code(
                                    boost::system::errc::not_connected));
                    return;
                }

                if(data_size > _max_outstanding_bytes
                   || _outstanding_write_bytes
                          > _max_outstanding_bytes - data_size)
                {
                    if(_err_cb)
                    {
                        _err_cb(self,
                                boost::system::errc::make_error_code(
                                    boost::system::errc::no_buffer_space));
                    }
                    return;
                }

                _outstanding_write_bytes += data_size;
                bool write_in_progress = !_send_queue.empty();
                _send_queue.push_back(std::move(data));

                if(!write_in_progress)
                    _write();
            });

        return;
    }

    void send(const std::uint8_t *data, std::size_t len)
    {
        if(!data || len == 0)
        {
            conn_ptr_t self = shared_from_this();
            if(!self)
                return;

            boost::asio::dispatch(
                _strand,
                [this, self = std::move(self)]() mutable {
                    if(_err_cb)
                        _err_cb(self,
                                boost::system::errc::make_error_code(
                                    boost::system::errc::invalid_argument));
                });
            return;
        }

        buffer_t buf(data, data + len);
        send(std::move(buf));
    }

    void close()
    {
        conn_ptr_t self = shared_from_this();
        if(!self)
            return;

        boost::asio::dispatch(_strand, [this, self = std::move(self)]() {
            _do_close();
        });
    }

  private:
    void _init()
    {
        if(!_sock)
            _sock = tcp_socket::make_shared(_io);
    }

    bool _is_connected() const noexcept { return _state == state::connected; }

    bool _is_closed() const noexcept { return _state == state::closed; }

    void _do_close()
    {
        if(_is_closed())
            return;

        _state = state::closed;
        if(_sock)
            _sock->close();

        _send_queue.clear();
        _outstanding_write_bytes = 0;
    }

    void _try_start_read()
    {
        if(_is_connected() && !_read_started)
        {
            _read_started = true;
            _read();
        }
    }

    void _read()
    {
        if(_is_closed())
            return;

        auto self = shared_from_this();
        _recv_buf.resize(read_buffer_size);

        _sock->async_read(
            boost::asio::buffer(_recv_buf),
            boost::asio::bind_executor(
                _strand,
                [this, self](const err_t &ec, std::size_t bytes_transferred) {
                    if(_is_closed())
                        return;

                    if(!ec && bytes_transferred > 0)
                    {
                        buffer_t payload(_recv_buf.begin(),
                                         _recv_buf.begin() + bytes_transferred);
                        if(_read_cb)
                            _read_cb(self, std::move(payload));

                        _read();
                    } else
                    {
                        _handle_error(ec);
                    }
                }));
    }

    void _write()
    {
        if(_send_queue.empty() || _is_closed())
            return;

        auto self = shared_from_this();
        auto current_buf =
            std::make_shared<buffer_t>(std::move(_send_queue.front()));
        _send_queue.pop_front();

        boost::asio::async_write(
            _sock->raw_socket(),
            boost::asio::buffer(*current_buf),
            boost::asio::bind_executor(
                _strand,
                [this, self, current_buf](const err_t &ec,
                                          std::size_t  bytes_transferred) {
                    if(_is_closed())
                        return;

                    if(_outstanding_write_bytes >= current_buf->size())
                        _outstanding_write_bytes -= current_buf->size();
                    else
                        _outstanding_write_bytes = 0;

                    if(!ec)
                    {
                        if(_write_cb)
                            _write_cb(self, *current_buf);

                        if(!_send_queue.empty() && !_is_closed())
                            _write();
                    } else
                    {
                        _handle_error(ec);
                    }
                }));
    }

    void _handle_error(const err_t &ec)
    {
        if(_is_closed())
            return;

        _do_close();
        if(_err_cb)
            _err_cb(shared_from_this(), ec);
    }

  private:
    io_t      &_io;
    strand_t   _strand;
    sock_ptr_t _sock;

    state _state{state::idle};

    bool _read_started{false};

    std::size_t _max_outstanding_bytes{default_max_outstanding_bytes};
    std::size_t _outstanding_write_bytes{0};

    std::deque<buffer_t> _send_queue;
    buffer_t             _recv_buf;

    write_handler_t _write_cb;
    read_handler_t  _read_cb;
    err_handler_t   _err_cb;
};

} // namespace hj

#endif // TCP_CONN_HPP