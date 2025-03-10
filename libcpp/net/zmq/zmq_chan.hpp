#ifndef ZMQ_CHAN_HPP
#define ZMQ_CHAN_HPP

#include <string>
#include <sstream>

#include <zmq.h>

namespace libcpp
{

class zmq_chan
{
public:
    explicit zmq_chan(void* ctx)
        : _ctx{ctx}
        , _sock_in{zmq_socket(ctx, ZMQ_PAIR)}
        , _sock_out{zmq_socket(ctx, ZMQ_PAIR)}
    {
        std::ostringstream ss;
        ss << std::hex << this;
        std::string id = "inproc://" + ss.str();

        int result = zmq_bind(_sock_in, id.c_str());
	    if (0 != result)
	    {
            zmq_close(_sock_in);
            _sock_in = nullptr;
	    }

        result = zmq_connect(_sock_out, id.c_str());
	    if (0 != result)
	    {
            zmq_msg_close(&_buf_out);
            zmq_close(_sock_out);
		    _sock_out = nullptr;
	    }
    }
    ~zmq_chan()
    {
        zmq_close(_sock_in);
        _sock_in = nullptr;

        zmq_msg_close(&_buf_out);
        zmq_close(_sock_out);
		_sock_out = nullptr;
    }

    inline bool operator>>(std::string& dst)
    {
        int result = zmq_msg_recv(&_buf_out, _sock_out, ZMQ_DONTWAIT);
        if (result < 0 || EAGAIN == zmq_errno() || EINTR == zmq_errno())
            return false;

        dst.reserve(zmq_msg_size(&_buf_out));
        dst.assign(static_cast<char*>(zmq_msg_data(&_buf_out)), zmq_msg_size(&_buf_out));
        return true;
    }

    inline bool operator <<(const std::string& src)
    {
        return zmq_send(_sock_in, src.c_str(), src.size(), ZMQ_DONTWAIT) >= 0;
    }

private:
    void*     _ctx;
    void*     _sock_in;
    void*     _sock_out;
    zmq_msg_t _buf_out;
};

}

#endif