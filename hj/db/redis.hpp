#ifndef REDIS_HPP
#define REDIS_HPP

#include <hiredis/hiredis.h>
#include <string>
#include <stdexcept>

#if defined(_WIN32)
#include <winsock2.h>
#else
#include <sys/time.h>
#endif

namespace hj
{

class redis
{
  public:
    redis(const std::string &host, int port = 6379, int timeout_ms = 2000)
        : _ctx(nullptr)
    {
        struct timeval timeout = {timeout_ms / 1000,
                                  (timeout_ms % 1000) * 1000};
        _ctx = redisConnectWithTimeout(host.c_str(), port, timeout);
        if(!_ctx || _ctx->err)
        {
            std::string err =
                _ctx ? _ctx->errstr : "Can't allocate redis context";
            if(_ctx)
            {
                redisFree(_ctx);
                _ctx = nullptr;
            }
            throw std::runtime_error("Redis connect failed: " + err);
        }
    }
    ~redis()
    {
        if(_ctx)
            redisFree(_ctx);
    }

    inline bool is_connected() const { return _ctx && _ctx->err == 0; }

    bool set(const std::string &key, const std::string &value)
    {
        redisReply *reply = (redisReply *)
            redisCommand(_ctx, "SET %s %s", key.c_str(), value.c_str());
        bool ok = reply && reply->type == REDIS_REPLY_STATUS
                  && std::string(reply->str) == "OK";
        if(reply)
            freeReplyObject(reply);

        return ok;
    }

    std::string get(const std::string &key)
    {
        redisReply *reply =
            (redisReply *) redisCommand(_ctx, "GET %s", key.c_str());
        std::string val;
        if(reply && reply->type == REDIS_REPLY_STRING)
            val = reply->str;

        if(reply)
            freeReplyObject(reply);

        return val;
    }

  private:
    redisContext *_ctx;
};

} // namespace hj

#endif