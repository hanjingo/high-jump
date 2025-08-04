#ifndef ZMQ_CHAN_HPP
#define ZMQ_CHAN_HPP

#include <assert.h>
#include <string.h>
#include <zmq.h>

#include <sstream>
#include <string>

namespace libcpp {

class zmq_chan
{
  public:
    explicit zmq_chan(void* ctx, int flags = ZMQ_DONTWAIT)
        : flags_{ flags },
          ctx_{ ctx },
          sock_in_{ zmq_socket(ctx, ZMQ_PAIR) },
          sock_out_{ zmq_socket(ctx, ZMQ_PAIR) }
    {
        std::ostringstream ss;
        ss << std::hex << this;
        std::string id = "inproc://" + ss.str();

        if (0 != zmq_bind(sock_in_, id.c_str()) ||
            0 != zmq_connect(sock_out_, id.c_str()))
        {
            zmq_close(sock_in_);
            sock_in_ = nullptr;

            zmq_close(sock_out_);
            sock_out_ = nullptr;

            assert(false);
        }
    }
    ~zmq_chan()
    {
        if (sock_in_ != nullptr)
        {
            zmq_close(sock_in_);
            sock_in_ = nullptr;
        }
        if (sock_out_ != nullptr)
        {
            zmq_close(sock_out_);
            sock_out_ = nullptr;
        }
        zmq_msg_close(&buf_in_);
        zmq_msg_close(&buf_in_);
    }

    inline bool operator>>(zmq_msg_t& dst)
    {
        return zmq_msg_recv(&dst, sock_out_, flags_) >= 0;
    }

    inline bool operator>>(std::string& dst)
    {
        int nbytes = zmq_msg_recv(&buf_out_, sock_out_, flags_);
        if (nbytes < 0)
            return false;

        dst.reserve(nbytes);
        dst.assign(static_cast<char*>(zmq_msg_data(&buf_out_)), nbytes);
        return true;
    }

    inline bool operator<<(zmq_msg_t& src)
    {
        return zmq_msg_send(&src, sock_in_, flags_) >= 0;
    }

    inline bool operator<<(const std::string& src)
    {
        if (zmq_msg_init_size(&buf_in_, src.size()) != 0)
            return false;

        memcpy(zmq_msg_data(&buf_in_), src.data(), src.size());
        return zmq_msg_send(&buf_in_, sock_in_, flags_) >= 0;
    }

  private:
    int flags_;
    void* ctx_;
    void* sock_in_;
    void* sock_out_;
    zmq_msg_t buf_out_;
    zmq_msg_t buf_in_;
};

}  // namespace libcpp

#endif