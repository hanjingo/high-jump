#ifndef WEBSOCKET_HPP
#define WEBSOCKET_HPP

// #include <boost/asio.hpp>
// #include <boost/beast/core.hpp>
// #include <boost/beast/websocket.hpp>
// #include <functional>
// #include <string>
// #include <memory>
// #include <atomic>
// #include <thread>
// #include <mutex>
// #include <queue>
// #include <condition_variable>

// namespace libcpp
// {

// class websocket
// {
// public:
//     using io_t = boost::asio::io_context;
//     using tcp_t = boost::asio::ip::tcp;
//     using ws_stream_t = boost::beast::websocket::stream<tcp_t::socket>;
//     using err_t = boost::system::error_code;
//     using message_handler_t = std::function<void(const std::string&)>;
//     using error_handler_t = std::function<void(const err_t&)>;

//     websocket(io_t& io)
//         : io_(io), ws_(std::make_unique<ws_stream_t>(io)), connected_(false)
//     {}

//     ~websocket()
//     {
//         close();
//     }

//     // Connect to a WebSocket server (blocking)
//     bool connect(const std::string& host, const std::string& port, const
//     std::string& target = "/")
//     {
//         err_t ec;
//         tcp_t::resolver resolver(io_);
//         auto const results = resolver.resolve(host, port, ec);
//         if (ec) return false;

//         boost::asio::connect(ws_->next_layer(), results.begin(),
//         results.end(), ec); if (ec) return false;

//         ws_->handshake(host, target, ec);
//         if (ec) return false;

//         connected_ = true;
//         return true;
//     }

//     // Asynchronously connect to a WebSocket server
//     void async_connect(const std::string& host, const std::string& port,
//     const std::string& target,
//                       std::function<void(const err_t&)> handler)
//     {
//         auto resolver = std::make_shared<tcp_t::resolver>(io_);
//         resolver->async_resolve(host, port,
//             [this, resolver, host, target, handler](const err_t& ec,
//             tcp_t::resolver::results_type results) {
//                 if (ec) { handler(ec); return; }
//                 boost::asio::async_connect(ws_->next_layer(),
//                 results.begin(), results.end(),
//                     [this, host, target, handler](const err_t& ec, const
//                     tcp_t::endpoint&) {
//                         if (ec) { handler(ec); return; }
//                         ws_->async_handshake(host, target,
//                             [this, handler](const err_t& ec) {
//                                 if (!ec) connected_ = true;
//                                 handler(ec);
//                             });
//                     });
//             });
//     }

//     // Send a text message (blocking)
//     bool send(const std::string& msg)
//     {
//         if (!connected_) return false;
//         err_t ec;
//         ws_->write(boost::asio::buffer(msg), ec);
//         return !ec;
//     }

//     // Asynchronously send a text message
//     void async_send(const std::string& msg, std::function<void(const err_t&,
//     std::size_t)> handler)
//     {
//         if (!connected_) {
//             handler(boost::asio::error::not_connected, 0);
//             return;
//         }
//         ws_->async_write(boost::asio::buffer(msg), handler);
//     }

//     // Receive a text message (blocking)
//     bool recv(std::string& out)
//     {
//         if (!connected_) return false;
//         boost::beast::flat_buffer buffer;
//         err_t ec;
//         ws_->read(buffer, ec);
//         if (ec) return false;
//         out = boost::beast::buffers_to_string(buffer.data());
//         return true;
//     }

//     // Asynchronously receive a text message
//     void async_recv(std::function<void(const err_t&, std::string)> handler)
//     {
//         if (!connected_) {
//             handler(boost::asio::error::not_connected, "");
//             return;
//         }
//         auto buffer = std::make_shared<boost::beast::flat_buffer>();
//         ws_->async_read(*buffer, [buffer, handler](const err_t& ec,
//         std::size_t) {
//             std::string msg;
//             if (!ec)
//                 msg = boost::beast::buffers_to_string(buffer->data());
//             handler(ec, msg);
//         });
//     }

//     // Close the websocket connection
//     void close()
//     {
//         if (!connected_) return;
//         err_t ec;
//         ws_->close(boost::beast::websocket::close_code::normal, ec);
//         connected_ = false;
//     }

//     // Check if connected
//     bool is_connected() const { return connected_; }

//     ws_stream_t& stream() { return *ws_; }

// private:
//     io_t& io_;
//     std::unique_ptr<ws_stream_t> ws_;
//     std::atomic<bool> connected_;
// };

// } // namespace libcpp

#endif // WEBSOCKET_HPP