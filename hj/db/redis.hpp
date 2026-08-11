/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2026 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef REDIS_HPP
#define REDIS_HPP

#include "db_conn.hpp"

#include <hiredis/hiredis.h>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#ifdef _WIN32
#if defined(_WIN32) && !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <winsock2.h>
#else
#include <sys/time.h>
#endif

namespace hj
{

class redis : public db_conn
{
  public:
    using reply_ptr = std::unique_ptr<redisReply, void (*)(void *)>;

    redis() = default;

    explicit redis(const std::string &host,
                   const int          port       = 6379,
                   const int          timeout_ms = 2000,
                   const std::string &password   = "",
                   const int          db         = 0)
    {
        connect(host, port, timeout_ms, password, db);
    }

    ~redis() override { close(); }

    redis(const redis &)            = delete;
    redis &operator=(const redis &) = delete;

    redis(redis &&other) noexcept
        : _ctx(std::exchange(other._ctx, nullptr))
        , _last_err(std::move(other._last_err))
    {
    }

    redis &operator=(redis &&other) noexcept
    {
        if(this != &other)
        {
            close();
            _ctx      = std::exchange(other._ctx, nullptr);
            _last_err = std::move(other._last_err);
        }
        return *this;
    }

    bool is_connected() const { return _ctx != nullptr && _ctx->err == 0; }

    bool connect(const std::string &host,
                 const int          port       = 6379,
                 const int          timeout_ms = 2000,
                 const std::string &password   = "",
                 const int          db         = 0) noexcept
    {
        close();
        _last_err.clear();

        struct timeval timeout = {timeout_ms / 1000,
                                  (timeout_ms % 1000) * 1000};
        _ctx = redisConnectWithTimeout(host.c_str(), port, timeout);

        if(!_ctx || _ctx->err)
        {
            if(_ctx)
            {
                _last_err = _ctx->errstr;
                redisFree(_ctx);
                _ctx = nullptr;
            } else
            {
                _last_err = "Failed to allocate redis context";
            }
            return false;
        }

        if(!password.empty())
        {
            auto res = exec("AUTH ?", password);
            if(res.ec)
                return false;
        }

        if(db > 0)
        {
            auto res = exec("SELECT ?", static_cast<int64_t>(db));
            if(res.ec)
                return false;
        }

        return true;
    }

    void close()
    {
        if(_ctx)
        {
            redisFree(_ctx);
            _ctx = nullptr;
        }
    }

    std::string get_last_error() const { return _last_err; }

  protected:
    err_t begin_impl() override
    {
        auto res = exec("MULTI");
        return res.ec;
    }

    err_t commit_impl() override
    {
        auto res = exec("EXEC");
        return res.ec;
    }

    err_t rollback_impl() override
    {
        auto res = exec("DISCARD");
        return res.ec;
    }

    exec_result exec_impl(std::string_view sql,
                          const val_in_t  *args,
                          std::size_t      count) override
    {
        exec_result res{};
        if(!is_connected())
        {
            res.ec = std::make_error_code(std::errc::not_connected);
            return res;
        }

        reply_ptr reply = _exec(sql, args, count);
        if(!reply)
        {
            res.ec = std::make_error_code(std::errc::io_error);
            return res;
        }

        if(reply->type == REDIS_REPLY_ERROR)
        {
            _last_err = reply->str ? reply->str : "Redis execution error";
            res.ec    = std::make_error_code(std::errc::bad_message);
            return res;
        }

        res.ec = err_t();
        if(reply->type == REDIS_REPLY_INTEGER)
        {
            res.affected_rows = reply->integer;
        } else if(reply->type == REDIS_REPLY_STATUS)
        {
            res.affected_rows =
                (reply->str && std::strcmp(reply->str, "OK") == 0) ? 1 : 0;
        }

        return res;
    }

    err_t query_impl(ret_t           &outs,
                     std::string_view sql,
                     const val_in_t  *args,
                     std::size_t      count) override
    {
        outs.clear();

        if(!is_connected())
            return std::make_error_code(std::errc::not_connected);

        reply_ptr reply = _exec(sql, args, count);
        if(!reply)
            return std::make_error_code(std::errc::io_error);

        if(reply->type == REDIS_REPLY_ERROR)
        {
            _last_err = reply->str ? reply->str : "Redis query error";
            return std::make_error_code(std::errc::bad_message);
        }

        _parse_reply_to_ret(reply.get(), outs);
        return err_t();
    }

  private:
    reply_ptr
    _exec(std::string_view cmd_tmpl, const val_in_t *args, std::size_t count)
    {
        std::vector<std::string>  arg_buffers;
        std::vector<const char *> argv;
        std::vector<size_t>       argvlen;

        std::size_t arg_idx = 0;
        std::size_t start   = 0;
        for(std::size_t i = 0; i <= cmd_tmpl.size(); ++i)
        {
            if(i != cmd_tmpl.size() && cmd_tmpl[i] != ' ')
                continue;

            if(i > start)
            {
                std::string_view token = cmd_tmpl.substr(start, i - start);
                if(token == "?" || token == "%s")
                {
                    if(arg_idx < count)
                        _append_val_in(args[arg_idx++],
                                       arg_buffers,
                                       argv,
                                       argvlen);
                } else
                {
                    argv.push_back(token.data());
                    argvlen.push_back(token.size());
                }
            }
            start = i + 1;
        }

        if(argv.empty())
        {
            _last_err = "Empty command";
            return reply_ptr(nullptr, freeReplyObject);
        }

        redisReply *reply = static_cast<redisReply *>(
            redisCommandArgv(_ctx,
                             static_cast<int>(argv.size()),
                             argv.data(),
                             argvlen.data()));

        if(!reply)
        {
            if(_ctx)
                _last_err = _ctx->errstr;
        }

        return reply_ptr(reply, freeReplyObject);
    }

    void _append_val_in(const val_in_t            &val,
                        std::vector<std::string>  &arg_buffers,
                        std::vector<const char *> &argv,
                        std::vector<size_t>       &argvlen)
    {
        std::visit(
            [&](auto &&arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr(std::is_same_v<T, std::nullptr_t>)
                {
                    argv.push_back("");
                    argvlen.push_back(0);
                } else if constexpr(std::is_same_v<T, int64_t>)
                {
                    arg_buffers.push_back(std::to_string(arg));
                    argv.push_back(arg_buffers.back().data());
                    argvlen.push_back(arg_buffers.back().size());
                } else if constexpr(std::is_same_v<T, double>)
                {
                    arg_buffers.push_back(std::to_string(arg));
                    argv.push_back(arg_buffers.back().data());
                    argvlen.push_back(arg_buffers.back().size());
                } else if constexpr(std::is_same_v<T, std::string_view>)
                {
                    argv.push_back(arg.data());
                    argvlen.push_back(arg.size());
                } else if constexpr(std::is_same_v<T, blob_view>)
                {
                    argv.push_back(reinterpret_cast<const char *>(arg.data));
                    argvlen.push_back(arg.size);
                }
            },
            val);
    }

    void _parse_reply_to_ret(const redisReply *reply, ret_t &outs)
    {
        if(!reply)
            return;

        if(reply->type == REDIS_REPLY_ARRAY)
        {
            outs.set_dim(reply->elements, 1);
            outs.reserve(reply->elements);
            for(size_t i = 0; i < reply->elements; ++i)
            {
                redisReply *elem = reply->element[i];
                if(elem->type == REDIS_REPLY_STRING
                   || elem->type == REDIS_REPLY_STATUS)
                {
                    outs.emplace_back(std::string(elem->str, elem->len));
                } else if(elem->type == REDIS_REPLY_INTEGER)
                {
                    outs.emplace_back(static_cast<int64_t>(elem->integer));
                } else
                {
                    outs.emplace_back(nullptr);
                }
            }
        } else
        {
            outs.set_dim(1, 1);
            outs.reserve(1);
            if(reply->type == REDIS_REPLY_STRING
               || reply->type == REDIS_REPLY_STATUS)
            {
                outs.emplace_back(std::string(reply->str, reply->len));
            } else if(reply->type == REDIS_REPLY_INTEGER)
            {
                outs.emplace_back(static_cast<int64_t>(reply->integer));
            } else
            {
                outs.emplace_back(nullptr);
            }
        }
    }

  private:
    redisContext *_ctx{nullptr};
    std::string   _last_err;
};

} // namespace hj

#endif // REDIS_HPP