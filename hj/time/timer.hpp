// for more information, please see:
// https://blog.51cto.com/u_15346415/5109520
// https://blog.51cto.com/u_15346415/5109505
// https://blog.51cto.com/u_15346415/5109502
// https://blog.51cto.com/u_15346415/5109245
#ifndef TIMER_HPP
#define TIMER_HPP

#include <future>
#include <chrono>
#include <memory>
#include <atomic>
#include <mutex>
#include <unordered_map>

#include <boost/version.hpp>
#include <boost/asio.hpp>

namespace hj
{

class timer;

#if BOOST_VERSION < 108700
static boost::asio::io_service       io_global;
static boost::asio::io_service::work worker_global{io_global};
#else
static boost::asio::io_context io_global;
static boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
    worker_global{io_global.get_executor()};
#endif

static std::once_flag                                           io_run_once;
static std::unordered_map<int64_t, std::shared_ptr<hj::timer> > tm_map_global{};
static std::mutex                                               tm_map_mu;
static std::atomic<int64_t>                                     tm_id_tag;

class timer
{
  public:
    template <typename F>
    timer(unsigned long long us, F &&f, int64_t id = -1)
        : _id{id}
        , _tm{io_global, boost::posix_time::microseconds(us)}
        , _fn{}
    {
        auto fn = std::move(f);
        _fn     = std::bind(
            [=](const boost::system::error_code &err) {
                if(id != -1)
                {
                    tm_map_mu.lock();
                    tm_map_global.erase(id);
                    tm_map_mu.unlock();
                }

                if(err.failed())
                {
                    return;
                }
                fn();
            },
            std::placeholders::_1);

        hj::timer::run();

        _tm.async_wait(_fn);
    }

    timer(timer &&rhs)
        : _id{std::move(rhs._id)}
        , _tm{std::move(rhs._tm)}
        , _fn{std::move(rhs._fn)}
    {
    }

    ~timer() {}

    inline int64_t id() { return _id; }

    inline void cancel()
    {
        _tm.cancel();

        if(_id != -1)
        {
            tm_map_mu.lock();
            tm_map_global.erase(_id);
            tm_map_mu.unlock();
        }
    }

    inline void reset(unsigned long long us)
    {
        _tm.expires_at(boost::posix_time::microsec_clock::universal_time()
                       + boost::posix_time::microseconds(us));
        _tm.async_wait(_fn);
    }

    template <typename F>
    inline void reset(unsigned long long us, F &&f)
    {
        auto fn = std::move(f);
        _fn     = std::bind(
            [=](const boost::system::error_code &err) {
                if(err.failed())
                {
                    return;
                }
                fn();
            },
            std::placeholders::_1);

        reset(us);
    }

    inline void wait() { _tm.wait(); }

    template <typename F>
    static std::shared_ptr<timer> create_global(unsigned long long us, F &&f)
    {
        auto tm = std::make_shared<hj::timer>(us, std::move(f), tm_id_tag++);

        tm_map_mu.lock();
        tm_map_global.insert(std::make_pair(tm->id(), tm));
        tm_map_mu.unlock();

        return tm;
    }

    static void run(bool async = true)
    {
        std::call_once(io_run_once, [=]() {
            if(async)
            {
                std::thread([]() { io_global.run(); }).detach();
                return;
            }

            io_global.run();
        });
    }

  private:
    int64_t                                                   _id{-1};
    boost::asio::deadline_timer                               _tm;
    std::function<void(const boost::system::error_code &err)> _fn;
};
}

#endif