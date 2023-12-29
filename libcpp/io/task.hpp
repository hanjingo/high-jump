#ifndef TASK_HPP
#define TASK_HPP

#include <atomic>
#include <chrono>
#include <functional>

#include <boost/asio.hpp>

namespace libcpp
{

struct task {
public:
    enum cmd {
        cmd_pause,
        cmd_continue,
        cmd_skip_next,
        cmd_skip_all,
        cmd_stop
    };

public:
    task(uint64_t timer_interval_ms = 100)
        : timer_interval_{std::chrono::milliseconds(timer_interval_ms)}
    {}
    ~task()
    {
        io_.stop();
    }

    static inline task* create()
    {
        return new libcpp::task();
    }

    template<typename F, typename... Types>
    static inline task* create(F&& f, Types&& ... args)
    {
        auto p = new libcpp::task();
        p->then(std::move(f), std::forward<Types>(args)...);
        return p;
    }

    template<typename F, typename... Types>
    inline task* then(F&& f, Types&& ... args)
    {
        for (auto old = count_.load(); !count_.compare_exchange_strong(old, old + 1); old = count_.load()) {
            continue;
        }

        io_.post(std::bind([](F f, std::atomic<int>& cmd, std::atomic<int>& count, std::chrono::milliseconds interval, Types... args) {
            for (auto old = count.load(); !count.compare_exchange_strong(old, old - 1); old = count.load()) {
                continue;
            }
            while (true) {
                if (cmd.load() == cmd_pause) {
                    std::this_thread::sleep_for(interval);
                    continue;
                }

                break;
            }

            if (cmd.load() == cmd_continue) {
                f(std::forward<Types>(args)...);
                return;
            }

            if (cmd.load() == cmd_skip_next) {
                cmd.store(cmd_continue);
                return;
            }
        }, std::move(f), std::ref(cmd_), std::ref(count_), timer_interval_, std::forward<Types>(args)...));

        return this;
    }

    inline bool skip_next()
    {
        int old = cmd_.load();
        if (!cmd_.compare_exchange_strong(old, cmd_skip_next)) {
            return false;
        }
        last_cmd_.store(old);
        return true;
    }

    inline bool skip_all()
    {
        int old = cmd_.load();
        if (!cmd_.compare_exchange_strong(old, cmd_skip_all)) {
            return false;
        }
        last_cmd_.store(old);
        return true;
    }

    inline bool pause()
    {
        int old = cmd_.load();
        if (!cmd_.compare_exchange_strong(old, cmd_pause)) {
            return false;
        }
        last_cmd_.store(old);
        return true;
    }

    inline bool resume()
    {
        int old = cmd_.load();
        int last_cmd = last_cmd_.load();
        return cmd_.compare_exchange_strong(old, last_cmd);
    }

    inline void stop()
    {
        if (io_.stopped() || cmd_.load() == cmd_stop) {
            return;
        }

        int old = cmd_.load();
        if (!cmd_.compare_exchange_strong(old, cmd_stop)) {
            return;
        }
        last_cmd_.store(old);
        io_.stop();
    }

    inline void run()
    {
        io_.run();
    }

    inline void run_one()
    {
        io_.run_one();
    }

    inline void run_for(const int64_t ms)
    {
        io_.run_for(std::chrono::milliseconds(ms));
    }

    inline std::future<void> async_run()
    {
        return std::async(std::bind(&task::run, this));
    }

    inline int size()
    {
        return count_.load();
    }

private:
    boost::asio::io_service   io_;
    std::atomic<int>          count_{0};
    std::atomic<int>          cmd_{cmd_continue};
    std::atomic<int>          last_cmd_{cmd_continue};
    std::chrono::milliseconds timer_interval_;
};

}

#endif