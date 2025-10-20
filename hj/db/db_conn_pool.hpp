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

// NOTIC: This file is written in C++11 by myself.
//  I am not sure if it is performing well and stable.
//  If you need an industrial level code, please use the following libraries:
//      1.https://github.com/SOCI/soci/blob/master/include/soci/connection-pool.h

#include <queue>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include <functional>
#include <stdexcept>
#include <cassert>
#include <optional>

namespace hj
{

template <typename Conn>
class db_conn_pool
{
  public:
    using conn_ptr_t   = std::shared_ptr<Conn>;
    using make_conn_t  = std::function<conn_ptr_t()>;
    using check_conn_t = std::function<bool(conn_ptr_t)>;

  public:
    explicit db_conn_pool(const std::size_t capa, make_conn_t &&make)
        : _pool{}
        , _cond{}
        , _mu{}
        , _closed{false}
        , _capa{capa}
        , _make{std::move(make)}
        , _check{[](conn_ptr_t) -> bool { return true; }}
    {
        for(std::size_t i = 0; i < capa; ++i)
            _pool.push(_make());
    }

    explicit db_conn_pool(const std::size_t capa,
                          make_conn_t     &&make,
                          check_conn_t    &&check)
        : _pool{}
        , _cond{}
        , _mu{}
        , _closed{false}
        , _capa{capa}
        , _make{std::move(make)}
        , _check{std::move(check)}
    {
        for(std::size_t i = 0; i < capa; ++i)
            _pool.push(_make());
    }

    ~db_conn_pool() { close(); }

    inline bool        is_closed() const { return _closed; }
    inline std::size_t capa() const { return _capa; }
    inline std::size_t size() const
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _pool.size();
    }

    conn_ptr_t acquire(const int timeout_ms = 0)
    {
        std::unique_lock<std::mutex> lock{_mu};
        if(_closed)
            return nullptr;

        if(timeout_ms > 0)
        {
            // wait until pool is non-empty or pool is closed
            if(!_cond.wait_for(
                   lock,
                   std::chrono::milliseconds(timeout_ms),
                   [this] { return !this->_pool.empty() || this->_closed; }))
                return nullptr;

            if(_closed)
                return nullptr;
        } else
        {
            if(_pool.empty())
                return nullptr;
        }

        while(!_pool.empty())
        {
            if(_closed)
                return nullptr;

            conn_ptr_t conn_ptr = _pool.front();
            _pool.pop();

            if(_closed)
                return nullptr;

            if(!_check(conn_ptr))
            {
                // replace bad connection with a new one (only if not closed)
                if(!_closed)
                    _pool.push(_make());

                continue;
            }

            // auto give_back
            return conn_ptr_t(conn_ptr.get(), [this, conn_ptr](Conn *) {
                this->_release(conn_ptr);
            });
        }

        return nullptr;
    }

    void close()
    {
        std::lock_guard<std::mutex> lock{_mu};
        _closed = true;
        while(!_pool.empty())
            _pool.pop();

        _cond.notify_all();
    }

  private:
    void _release(conn_ptr_t conn)
    {
        std::lock_guard<std::mutex> lock{_mu};
        if(!_closed)
        {
            _pool.push(conn);
            _cond.notify_one();
        }
    }

  private:
    std::queue<conn_ptr_t>  _pool;
    std::condition_variable _cond;
    mutable std::mutex      _mu;
    std::atomic<bool>       _closed;
    std::size_t             _capa;

    make_conn_t  _make;
    check_conn_t _check;
};

};

#endif