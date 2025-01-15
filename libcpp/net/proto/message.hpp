#ifndef MESSAGE_HPP
#define MESSAGE_HPP

namespace libcpp
{
    class message
    {
    public:
        virtual std::size_t size() = 0;
        virtual std::size_t encode(unsigned char* buf, std::size_t len) = 0;
        virtual bool decode(const unsigned char* buf, std::size_t len)  = 0;
    };
}

#endif