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
    explicit zmq_chan(zmq::ctx_t* ctx)
        : _ctx{ctx}
        , _in{zmq_socket(ctx, ZMQ_PAIR)}
        , _out{zmq_socket(ctx, ZMQ_PAIR)}
    {
        std::ostringstream ss;
        ss << std::hex << this;
        std::string id = "inproc://" + ss.str();

        int result = zmq_bind(_in, id.c_str());
	    if (0 != result)
	    {
            zmq_msg_close(&_in_recv_buf);
            zmq_close(_in);
            _in = nullptr;
	    }

        result = zmq_connect(_out, id.c_str());
	    if (0 != result)
	    {
            zmq_msg_close(&_out_recv_buf);
            zmq_close(_out);
		    _out = nullptr;
	    }
    }
    ~zmq_chan()
    {
        zmq_msg_close(&_in_recv_buf);
        zmq_close(_in);
        _in = nullptr;

        zmq_msg_close(&_out_recv_buf);
        zmq_close(_out);
		_out = nullptr;
    }

    inline bool operator>>(std::string& dst)
    {
        int result = zmq_msg_recv(&_out_recv_buf, _out, ZMQ_DONTWAIT);
        if (result < 0 || EAGAIN == zmq_errno() || EINTR == zmq_errno())
            return false;

        dst.reserve(zmq_msg_size(&_out_recv_buf));
        string.assign(static_cast<char*>(zmq_msg_data(&_out_recv_buf)), zmq_msg_size(&_out_recv_buf));
        return true;
    }

    inline bool operator <<(const std::string& src)
    {
        return zmq_send(_in, src.c_str(), src.size(), ZMQ_DONTWAIT) >= 0;
    }

private:
    zmq::ctx_t*         _ctx;
    zmq::socket_base_t* _in;
    zmq::socket_base_t* _out;
    zmq::zmq_msg_t      _out_recv_buf;
};

}

#endif