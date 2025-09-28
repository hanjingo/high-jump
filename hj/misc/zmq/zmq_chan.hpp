#ifndef ZMQ_CHAN_HPP
#define ZMQ_CHAN_HPP

#include <string>
#include <sstream>
#include <string.h>
#include <assert.h>

#include <zmq.h>

namespace hj
{

class zmq_chan
{
  public:
    explicit zmq_chan(void *ctx, int flags = ZMQ_DONTWAIT)
        : _flags{flags}
        , _ctx{ctx}
        , _sock_in{zmq_socket(ctx, ZMQ_PAIR)}
        , _sock_out{zmq_socket(ctx, ZMQ_PAIR)}
    {
        std::ostringstream ss;
        ss << std::hex << this;
        std::string id = "inproc://" + ss.str();

        if(0 != zmq_bind(_sock_in, id.c_str())
           || 0 != zmq_connect(_sock_out, id.c_str()))
        {
            zmq_close(_sock_in);
            _sock_in = nullptr;

            zmq_close(_sock_out);
            _sock_out = nullptr;

            assert(false);
        }
    }
    ~zmq_chan()
    {
        if(_sock_in != nullptr)
        {
            zmq_close(_sock_in);
            _sock_in = nullptr;
        }
        if(_sock_out != nullptr)
        {
            zmq_close(_sock_out);
            _sock_out = nullptr;
        }
        zmq_msg_close(&_buf_in);
        zmq_msg_close(&_buf_in);
    }

    inline bool operator>>(zmq_msg_t &dst)
    {
        return zmq_msg_recv(&dst, _sock_out, _flags) >= 0;
    }

    inline bool operator>>(std::string &dst)
    {
        int nbytes = zmq_msg_recv(&_buf_out, _sock_out, _flags);
        if(nbytes < 0)
            return false;

        dst.reserve(nbytes);
        dst.assign(static_cast<char *>(zmq_msg_data(&_buf_out)), nbytes);
        return true;
    }

    inline bool operator<<(zmq_msg_t &src)
    {
        return zmq_msg_send(&src, _sock_in, _flags) >= 0;
    }

    inline bool operator<<(const std::string &src)
    {
        if(zmq_msg_init_size(&_buf_in, src.size()) != 0)
            return false;

        memcpy(zmq_msg_data(&_buf_in), src.data(), src.size());
        return zmq_msg_send(&_buf_in, _sock_in, _flags) >= 0;
    }

  private:
    int       _flags;
    void     *_ctx;
    void     *_sock_in;
    void     *_sock_out;
    zmq_msg_t _buf_out;
    zmq_msg_t _buf_in;
};

}

#endif