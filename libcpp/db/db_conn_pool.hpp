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
    explicit db_conn_pool(const std::size_t sz, make_conn_t&& make)
        : make_{std::move(make)}
        , check_{[](conn_ptr_t)->bool{ return true; }}
        , capa_{sz}
    {
        for (int i = 0; i < sz; ++i)
            pool_.push(make_());
    }
    explicit db_conn_pool(const std::size_t sz, make_conn_t&& make, check_conn_t&& check)
        : make_{std::move(make)}
        , check_{std::move(check)}
        , capa_{sz}
    {
        for (int i = 0; i < sz; ++i)
            pool_.push(make_());
    }
    ~db_conn_pool()
    {
        cond_.notify_all();
    }

    inline std::size_t capa() { return capa_; }

    conn_ptr_t acquire(const int timeout_ms = 0)
    {
        if (timeout_ms > 0)
        {
            std::unique_lock<std::mutex> lock{mu_};
            bool ok = cond_.wait_for(
                lock, 
                std::chrono::milliseconds(timeout_ms), 
                [this]{ return !this->pool_.empty(); });
            if (!ok)
                return nullptr;
        }

        std::unique_lock<std::mutex> lock{mu_};
        if (pool_.empty())
            return nullptr;

        do {
            conn_ptr_t conn_ptr = pool_.front();
            pool_.pop();
            if (!check_(conn_ptr))
            {
                pool_.push(make_());
                continue;
            }

            // auto give_back
            return conn_ptr_t(conn_ptr.get(), [this, conn_ptr](Conn*){
                this->release_(conn_ptr);
            });
            break;
        } while (true);
    }

private:
    void release_(conn_ptr_t conn)
    {
        std::lock_guard<std::mutex> lock{mu_};
        pool_.push(conn);
        cond_.notify_one();
    }

private:
    std::queue<conn_ptr_t>  pool_;
    std::condition_variable cond_;
    std::mutex              mu_;
    std::size_t             capa_;

    make_conn_t             make_;
    check_conn_t            check_;
};

};

#endif