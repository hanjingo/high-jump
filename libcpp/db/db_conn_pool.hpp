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

namespace libcpp
{

template<typename Conn>
class db_conn_pool
{
public:
    using conn_ptr_t   = std::shared_ptr<Conn>;
    using make_conn_t  = std::function<conn_ptr_t()>;
    using check_conn_t = std::function<bool(conn_ptr_t)>;
    
public:
    explicit db_conn_pool(const std::size_t capa, make_conn_t&& make)
        : _pool{}
        , _cond{}
        , _mu{}
        , _capa{capa}
        , _make{std::move(make)}
        , _check{[](conn_ptr_t)->bool{ return true; }}
    {
        for (std::size_t i = 0; i < capa; ++i)
            _pool.push(_make());
    }
    explicit db_conn_pool(const std::size_t capa, make_conn_t&& make, check_conn_t&& check)
        : _pool{}
        , _cond{}
        , _mu{}
        , _capa{capa}
        , _make{std::move(make)}
        , _check{std::move(check)}
    {
        for (std::size_t i = 0; i < capa; ++i)
            _pool.push(_make());
    }
    ~db_conn_pool()
    {
        _cond.notify_all();
    }

    inline std::size_t capa() { return _capa; }

    inline std::size_t size() { return _pool.size(); }

    conn_ptr_t acquire(const int timeout_ms = 0)
    {
        if (timeout_ms > 0)
        {
            std::unique_lock<std::mutex> lock{_mu};
            bool ok = _cond.wait_for(
                lock, 
                std::chrono::milliseconds(timeout_ms), 
                [this]{ return !this->_pool.empty(); });
            if (!ok)
                return nullptr;
        }

        std::unique_lock<std::mutex> lock{_mu};
        if (_pool.empty())
            return nullptr;

        do {
            conn_ptr_t conn_ptr = _pool.front();
            _pool.pop();
            if (!_check(conn_ptr))
            {
                _pool.push(_make());
                continue;
            }

            // auto give_back
            return conn_ptr_t(conn_ptr.get(), [this, conn_ptr](Conn*){
                this->_release(conn_ptr);
            });
            break;
        } while (true);
    }

private:
    void _release(conn_ptr_t conn)
    {
        std::lock_guard<std::mutex> lock{_mu};
        _pool.push(conn);
        _cond.notify_one();
    }

private:
    std::queue<conn_ptr_t>  _pool;
    std::condition_variable _cond;
    std::mutex              _mu;
    std::size_t             _capa;

    make_conn_t             _make;
    check_conn_t            _check;
};

};

#endif