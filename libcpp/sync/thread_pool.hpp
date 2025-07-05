// Origin:     https://github.com/progschj/ThreadPool
// Altered By: hehehunanchina@live.com

#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <string>

#include <unordered_set>

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

class thread_pool
{
public:
    using task_t = std::function<void()>;

public:
    explicit thread_pool(unsigned long nthread = std::thread::hardware_concurrency())
        : is_stop_{false}
    {
        nthread = (nthread < 1) ? 1 : nthread;
        for (size_t i = 0; i < nthread; ++i)
            init_work_();
    }

    explicit thread_pool(const std::unordered_set<unsigned int> cores)
        : is_stop_{false}
    {
        for (auto core : cores)
            init_work_(core);
    }

    inline std::size_t size() { return workers_.size(); }

    // add new work item to the pool
    template<class F, class... Args>
#if __cplusplus >= 201703L
    auto enqueue(F&& f, Args&& ... args) -> std::future<typename std::invoke_result<F, Args...>::type>
    {
        using return_type = typename std::invoke_result<F, Args...>::type;
#else
    auto enqueue(F&& f, Args&& ... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;
#endif

        auto task = std::make_shared< std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();

        // lock begin
        {
            std::unique_lock<std::mutex> lock(mu_);

            // don't allow enqueueing after stopping the pool
            if (is_stop_)
                return std::future<return_type>();

            tasks_.emplace([task]() { (*task)(); });
        }
        // lock end

        cond_.notify_one();
        return res;
    }

    void clear()
    {
        // lock begin
        {
            std::unique_lock<std::mutex> lock(mu_);
            is_stop_ = true;
        }
        // lock end

        cond_.notify_all();
        for (std::thread& worker : workers_)
            if (worker.joinable()) { worker.join(); }

        is_stop_ = false;
    }

    // the destructor joins all threads
    inline ~thread_pool()
    {
        // lock begin
        {
            std::unique_lock<std::mutex> lock(mu_);
            is_stop_ = true;
        }
        // lock end

        cond_.notify_all();
        for (std::thread& worker : workers_)
            if (worker.joinable()) { worker.join(); }
    }

private:
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

                try {
                    task();
                } catch(...) {
                    std::cerr << "RUN THREAD POOL TASK EXCEPTION" << std::endl;
                }
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
    thread_pool(const thread_pool&) = delete;
    thread_pool& operator=(const thread_pool&) = delete;

private:
    std::vector<std::thread> workers_;
    std::queue<task_t>       tasks_;
    std::mutex               mu_;
    std::condition_variable  cond_;
    bool                     is_stop_ = false;
};

}

#endif