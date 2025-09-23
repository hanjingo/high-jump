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

namespace libcpp
{

class timer;

#if BOOST_VERSION < 108700
static boost::asio::io_service io_global;
static boost::asio::io_service::work worker_global{io_global};
#else
static boost::asio::io_context io_global;
static boost::asio::executor_work_guard<boost::asio::io_context::executor_type> worker_global{io_global.get_executor()};
#endif

static std::once_flag io_run_once;
static std::unordered_map<int64_t, std::shared_ptr<libcpp::timer>> tm_map_global{};
static std::mutex tm_map_mu;
static std::atomic<int64_t> tm_id_tag;

class timer
{
public:
    template <typename F>
    timer(unsigned long long us, F&& f, int64_t id = -1)
        : id_{id}
        , tm_{io_global, boost::posix_time::microseconds(us)}
        , fn_{}
    {
        auto fn = std::move(f);
        fn_ = std::bind([ = ](const boost::system::error_code & err) {
            if (id != -1) {
                tm_map_mu.lock();
                tm_map_global.erase(id);
                tm_map_mu.unlock();
            }

            if (err.failed()) {
                return;
            }
            fn();
        }, std::placeholders::_1);

        libcpp::timer::run();

        tm_.async_wait(fn_);
    }

    timer(timer&& rhs)
        : id_{std::move(rhs.id_)}
        , tm_{std::move(rhs.tm_)}
        , fn_{std::move(rhs.fn_)}
    {}

    ~timer() {}

    inline int64_t id()
    {
        return id_;
    }

    inline void cancel()
    {
        tm_.cancel();

        if (id_ != -1) {
            tm_map_mu.lock();
            tm_map_global.erase(id_);
            tm_map_mu.unlock();
        }
    }

    inline void reset(unsigned long long us)
    {
        tm_.expires_at(boost::posix_time::microsec_clock::universal_time()
                       + boost::posix_time::microseconds(us));
        tm_.async_wait(fn_);
    }

    template<typename F>
    inline void reset(unsigned long long us, F&& f)
    {
        auto fn = std::move(f);
        fn_ = std::bind([ = ](const boost::system::error_code & err) {
            if (err.failed()) {
                return;
            }
            fn();
        }, std::placeholders::_1);

        reset(us);
    }

    inline void wait()
    {
        tm_.wait();
    }

    template <typename F>
    static std::shared_ptr<timer> create_global(unsigned long long us, F&& f)
    {
        auto tm = std::make_shared<libcpp::timer>(us, std::move(f), tm_id_tag++);

        tm_map_mu.lock();
        tm_map_global.insert(std::make_pair(tm->id(), tm));
        tm_map_mu.unlock();

        return tm;
    }

    static void run(bool async = true)
    {
        std::call_once(io_run_once, [ = ]() {
            if (async) {
                std::thread([]() {
                    io_global.run();
                }).detach();
                return;
            }

            io_global.run();
        });
    }

private:
    int64_t                                                   id_{-1};
    boost::asio::deadline_timer                               tm_;
    std::function<void(const boost::system::error_code& err)> fn_;
};
}

#endif