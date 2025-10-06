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
    redis()
        : _ctx(nullptr)
    {
    }

    explicit redis(const std::string &host,
                   const int          port       = 6379,
                   const int          timeout_ms = 2000,
                   const std::string &password   = "",
                   const int          db         = 0)
        : _ctx(nullptr)
    {
        connect(host, port, timeout_ms, password, db);
    }

    redis(const redis &)            = delete;
    redis &operator=(const redis &) = delete;
    redis(redis &&other) noexcept
        : _ctx(other._ctx)
    {
        other._ctx = nullptr;
    }
    redis &operator=(redis &&other) noexcept
    {
        if(this != &other)
        {
            _close();
            _ctx       = other._ctx;
            other._ctx = nullptr;
        }
        return *this;
    }

    ~redis() { _close(); }

    inline bool is_connected() const { return _ctx && _ctx->err == 0; }

    bool connect(const std::string &host,
                 const int          port       = 6379,
                 const int          timeout_ms = 2000,
                 const std::string &password   = "",
                 const int          db         = 0) noexcept
    {
        struct timeval timeout = {timeout_ms / 1000,
                                  (timeout_ms % 1000) * 1000};
        try
        {
            _close();
            _ctx = redisConnectWithTimeout(host.c_str(), port, timeout);
        }
        catch(...)
        {
            return false;
        }

        if(!_ctx || _ctx->err)
        {
            if(_ctx)
            {
                redisFree(_ctx);
                _ctx = nullptr;
            }

            return false;
        }

        if(!password.empty())
        {
            auto reply = cmd("AUTH %s", password.c_str());
            if(!reply || reply->type == REDIS_REPLY_ERROR)
                return false;
        }

        if(db > 0)
        {
            auto reply = cmd("SELECT %d", db);
            if(!reply || reply->type == REDIS_REPLY_ERROR)
                return false;
        }

        return true;
    }

    template <typename... Args>
    std::unique_ptr<redisReply, void (*)(void *)> cmd(const char *fmt,
                                                      Args &&...args)
    {
        if(!_ctx)
            return std::unique_ptr<redisReply, void (*)(void *)>(
                nullptr,
                freeReplyObject);

        redisReply *reply =
            (redisReply *) redisCommand(_ctx, fmt, std::forward<Args>(args)...);
        return std::unique_ptr<redisReply, void (*)(void *)>(reply,
                                                             freeReplyObject);
    }

    bool set(const std::string &key, const std::string &value)
    {
        auto reply = cmd("SET %s %s", key.c_str(), value.c_str());
        return reply && reply->type == REDIS_REPLY_STATUS
               && std::string(reply->str) == "OK";
    }

    std::string get(const std::string &key)
    {
        auto reply = cmd("GET %s", key.c_str());
        if(reply && reply->type == REDIS_REPLY_STRING)
            return reply->str;

        return std::string();
    }

  private:
    void _close()
    {
        if(_ctx)
        {
            redisFree(_ctx);
            _ctx = nullptr;
        }
    }

  private:
    redisContext *_ctx;
};

} // namespace hj

#endif