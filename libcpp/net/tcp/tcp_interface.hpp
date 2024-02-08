#ifndef TCP_INTERFACE_HPP
#define TCP_INTERFACE_HPP

namespace libcpp
{
    class message
    {
    public:
        virtual std::size_t size() = 0;
        virtual std::size_t encode(unsigned char* buf, std::size_t len) = 0;
        virtual bool decode(const unsigned char* buf, std::size_t len)  = 0;
    };

    class session
    {
    public:
        virtual bool connect(const char* ip, uint16_t port, std::chrono::milliseconds timeout, int retry_times) = 0;
        virtual void disconnect() = 0;
        virtual std::size_t send(const unsigned char* buf, const std::size_t len) = 0;
        virtual std::size_t recv(const unsigned char* buf, const std::size_t len) = 0;
        virtual void close() = 0;
    };

    class listener
    {
    public:
        virtual session* accept(const uint16_t port) = 0;
        virtual void close() = 0;
    };
}

#endif