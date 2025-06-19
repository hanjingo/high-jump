// Origin:     https://github.com/progschj/ThreadPool
// Altered By: hehehunanchina@live.com

#ifndef CODEC_HPP
#define CODEC_HPP

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

#include <initializer_list>

#if defined(_WIN32)
#include <windows.h>
#elif __APPLE__
#include <sys/param.h>
#include <sys/sysctl.h>
#elif __linux__
#include <unistd.h>
#include <pthread.h>
#else
#endif

namespace libcpp
{

class codec
{    
public:
    using task_t = std::function<void()>;
    using encode_handler_t = std::function<unsigned long(const void*, unsigned char*, const unsigned long)>;
    using decode_handler_t = std::function<unsigned long(const unsigned char*, const unsigned long, void*)>;

public:
    codec(const unsigned long nthread = std::thread::hardware_concurrency(),
          const encode_handler_t fn_encode = encode_handler_t(),
          const decode_handler_t fn_decode = decode_handler_t())
        : is_stop_{false}
        , fn_encode_{fn_encode}
        , fn_decode_{fn_decode}
    {
        unsigned long ncore = (nthread < 1) ? 1 : nthread;
        for (size_t i = 0; i < ncore; ++i)
            init_work_();
    }

    codec(const std::initializer_list<unsigned int> cores,
          const encode_handler_t fn_encode = encode_handler_t(),
          const decode_handler_t fn_decode = decode_handler_t())
        : is_stop_{false}
        , fn_encode_{fn_encode}
        , fn_decode_{fn_decode}
    {
        for (auto core : cores)
            init_work_(core);
    }

    ~codec()
    {
        // lock begin
        {
            std::unique_lock<std::mutex> lock(mu_);
            is_stop_ = true;
        }
        // lock end

        cond_.notify_all();
        for (std::thread& worker : workers_)
            worker.join();
    }

    inline std::size_t size() { return workers_.size(); }

    template<typename F>
    inline void set_encode_handler(F&& fn)
    {
        std::unique_lock<std::mutex> lock(mu_);
        fn_encode_ = std::forward<F>(fn);
    }

    template<typename F>
    inline void set_decode_handler(F&& fn)
    {
        std::unique_lock<std::mutex> lock(mu_);
        fn_decode_ = std::forward<F>(fn);
    }

    inline void cancel() { clear_(); }

    template<typename T>
    std::future<unsigned long> encode(const T& obj, unsigned char* data, const unsigned long len)
    {
        std::unique_lock<std::mutex> lock(mu_);
        if (!fn_encode_ || is_stop_)
            return std::future<unsigned long>();

        return enqueue_(fn_encode_, static_cast<const void*>(&obj), data, len);
    }

    template<typename T>
    std::future<unsigned long> decode(const unsigned char* data, const unsigned long len, T& obj)
    {
        std::unique_lock<std::mutex> lock(mu_);
        if (!fn_decode_ || is_stop_)
            return std::future<unsigned long>();

        return enqueue_(fn_decode_, data, len, static_cast<void*>(&obj));
    }

private:
    // NOTE: This function is not thread-safe.
    template<class F, class... Args>
    auto enqueue_(F&& f, Args&& ... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;
        auto task = std::make_shared< std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<return_type> res = task->get_future();

        // don't allow enqueueing after stopping the pool
        if (is_stop_)
            throw std::runtime_error("enqueue on stopped thread_pool");

        tasks_.emplace([task](){ (*task)(); });
        cond_.notify_one();
        return res;
    }

    void clear_()
    {
        // lock begin
        {
            std::unique_lock<std::mutex> lock(mu_);
            is_stop_ = true;
        }
        // lock end

        cond_.notify_all();
        for (std::thread& worker : workers_)
            worker.join();

        is_stop_ = false;
    }

    void init_work_(const int core = -1)
    {
        workers_.emplace_back([this, core](){
            // Set the thread affinity to the specified core
            if (core > -1 && !bind_core_(core))
                throw std::runtime_error("Failed to bind thread to core " + std::to_string(core));

            for (;;)
            {
                std::function<void()> task;
                    
                // lock begin
                {
                    std::unique_lock<std::mutex> lock(this->mu_);
                    this->cond_.wait(lock, [this](){ return this->is_stop_ || !this->tasks_.empty(); });
                    if (this->is_stop_ && this->tasks_.empty())
                        return;

                    task = std::move(this->tasks_.front());
                    this->tasks_.pop();
                }
                // lock end

                task();
            }
        });
    }

    bool bind_core_(const unsigned int core)
    {
#if defined(_WIN32)
        HANDLE hThread = GetCurrentThread();
        DWORD_PTR mask = SetThreadAffinityMask(hThread, (DWORD_PTR)(1LLU << core));
        return (mask != 0);
#elif __linux__
        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(core, &mask);
        return (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) >= 0);
#else
        return false;
#endif
    }

private:
    std::vector<std::thread> workers_;
    std::queue<task_t>       tasks_;
    std::mutex               mu_;
    std::condition_variable  cond_;
    bool                     is_stop_ = false;

    encode_handler_t         fn_encode_;
    decode_handler_t         fn_decode_;
};

}

#endif // CODEC_HPP