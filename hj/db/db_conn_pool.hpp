/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
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

#ifndef DB_CONN_POOL_HPP
#define DB_CONN_POOL_HPP

// NOTIC: This file is written in C++17 by myself.
//  I am not sure if it is performing well and stable.
//  If you need an industrial level code, please use the following libraries:
//      1.https://github.com/SOCI/soci/blob/master/include/soci/connection-pool.h

#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional>
#include <utility>
#include <atomic>

namespace hj
{

template <typename Conn>
class db_conn_pool : public std::enable_shared_from_this<db_conn_pool<Conn>>
{
  private:
    struct private_tag
    {
        explicit private_tag() = default;
    };

  public:
    using conn_ptr_t    = std::shared_ptr<Conn>;
    using deleter_t     = std::function<void(Conn *)>;
    using conn_handle_t = std::unique_ptr<Conn, deleter_t>;
    using make_conn_t   = std::function<conn_ptr_t()>;
    using check_conn_t  = std::function<bool(conn_ptr_t)>;

  public:
    explicit db_conn_pool(private_tag,
                          std::size_t    capa,
                          std::size_t    min_size,
                          make_conn_t  &&make,
                          check_conn_t &&check)
        : _capa{capa}
        , _min_size{(min_size > capa) ? capa : min_size}
        , _make{std::move(make)}
        , _check{std::move(check)}
        , _closed{false}
        , _total_cnt{0}
    {
    }

    ~db_conn_pool() { close(); }

    static std::shared_ptr<db_conn_pool> create(std::size_t  capa,
                                                std::size_t  min_size,
                                                make_conn_t  make,
                                                check_conn_t check = nullptr)
    {
        auto pool = std::make_shared<db_conn_pool>(private_tag{},
                                                   capa,
                                                   min_size,
                                                   std::move(make),
                                                   std::move(check));
        pool->_init();
        return pool;
    }

    inline bool is_closed() const
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _closed.load();
    }

    inline bool is_empty() const
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _pool.empty();
    }

    inline std::size_t capa() const { return _capa; }
    inline std::size_t min_size() const { return _min_size; }

    inline std::size_t idle() const
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _pool.size();
    }

    inline std::size_t active() const
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _total_cnt - _pool.size();
    }

    inline std::size_t total() const
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _total_cnt;
    }

    conn_handle_t acquire(int timeout_ms = -1)
    {
        auto start_time   = std::chrono::steady_clock::now();
        int  remaining_ms = timeout_ms;

        while(true)
        {
            conn_ptr_t conn            = nullptr;
            bool       need_create_new = false;

            {
                std::unique_lock<std::mutex> lock(_mu);
                auto                         wait_cond = [this] {
                    return !_pool.empty() || _closed.load()
                           || _total_cnt < _capa;
                };

                if(remaining_ms < 0)
                {
                    _cond.wait(lock, wait_cond);
                } else if(remaining_ms > 0)
                {
                    if(!_cond.wait_for(lock,
                                       std::chrono::milliseconds(remaining_ms),
                                       wait_cond))
                        return nullptr;
                } else
                {
                    if(_pool.empty() && _total_cnt >= _capa)
                        return nullptr;
                }

                if(_closed.load())
                    return nullptr;

                if(!_pool.empty())
                {
                    conn = std::move(_pool.front());
                    _pool.pop();
                } else if(_total_cnt < _capa)
                {
                    _total_cnt++;
                    need_create_new = true;
                } else
                {
                    return nullptr;
                }
            }

            if(need_create_new)
            {
                try
                {
                    conn = _make();
                }
                catch(...)
                {
                    conn = nullptr;
                }

                if(!conn)
                {
                    {
                        std::lock_guard<std::mutex> lock(_mu);
                        _total_cnt--;
                    }
                    _cond.notify_one();

                    if(timeout_ms >= 0)
                    {
                        auto elapsed =
                            std::chrono::duration_cast<
                                std::chrono::milliseconds>(
                                std::chrono::steady_clock::now() - start_time)
                                .count();
                        remaining_ms = timeout_ms - static_cast<int>(elapsed);
                        if(remaining_ms <= 0)
                            return nullptr;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
            }

            bool valid = false;
            if(_check && conn)
            {
                try
                {
                    valid = _check(conn);
                }
                catch(...)
                {
                    valid = false;
                }
            } else
            {
                valid = (conn != nullptr);
            }

            if(!valid)
            {
                {
                    std::lock_guard<std::mutex> lock(_mu);
                    _total_cnt--;
                }
                _cond.notify_one();

                if(timeout_ms >= 0)
                {
                    auto elapsed =
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now() - start_time)
                            .count();
                    remaining_ms = timeout_ms - static_cast<int>(elapsed);
                    if(remaining_ms <= 0)
                        return nullptr;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            {
                std::lock_guard<std::mutex> lock(_mu);
                if(_closed.load())
                {
                    if(_total_cnt > 0)
                        _total_cnt--;

                    return nullptr;
                }
            }

            std::weak_ptr<db_conn_pool> weak_self = this->shared_from_this();
            return conn_handle_t(
                conn.get(),
                [weak_self, internal_conn = conn](Conn *) mutable {
                    if(auto self = weak_self.lock())
                        self->_release(std::move(internal_conn));
                });
        }
    }

    void close()
    {
        std::queue<conn_ptr_t> to_destroy;
        {
            bool expected = false;
            if(!_closed.compare_exchange_strong(expected, true))
                return;

            std::lock_guard<std::mutex> lock(_mu);
            _total_cnt -= _pool.size();
            _pool.swap(to_destroy);
        }
        _cond.notify_all();
    }

  private:
    void _init()
    {
        for(std::size_t i = 0; i < _min_size; ++i)
        {
            try
            {
                auto conn = _make();
                if(conn)
                {
                    std::lock_guard<std::mutex> lock(_mu);
                    _pool.push(std::move(conn));
                    _total_cnt++;
                }
            }
            catch(...)
            {
            }
        }
    }

    void _release(conn_ptr_t conn)
    {
        if(!conn)
            return;

        conn_ptr_t to_destroy;
        bool       need_notify = false;

        {
            std::lock_guard<std::mutex> lock{_mu};
            if(!_closed.load())
            {
                if(_pool.size() < _min_size)
                {
                    _pool.push(std::move(conn));
                    need_notify = true;
                } else
                {
                    _total_cnt--;
                    need_notify = true;
                    to_destroy  = std::move(conn);
                }
            } else
            {
                if(_total_cnt > 0)
                    _total_cnt--;

                to_destroy = std::move(conn);
            }
        }

        if(need_notify)
            _cond.notify_one();
    }

  private:
    mutable std::mutex      _mu;
    std::queue<conn_ptr_t>  _pool;
    std::condition_variable _cond;

    const std::size_t        _capa;
    const std::size_t        _min_size;
    std::atomic<bool>        _closed{false};
    std::atomic<std::size_t> _total_cnt{0};

    make_conn_t  _make;
    check_conn_t _check;
};

} // namespace hj

#endif