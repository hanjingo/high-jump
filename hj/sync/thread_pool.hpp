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

namespace hj
{

class thread_pool
{
  public:
    using task_t = std::function<void()>;

  public:
    explicit thread_pool(
        unsigned long nthread = std::thread::hardware_concurrency())
        : _is_stop{false}
    {
        nthread = (nthread < 1) ? 1 : nthread;
        for(size_t i = 0; i < nthread; ++i)
            _init_work();
    }

    explicit thread_pool(const std::unordered_set<unsigned int> cores)
        : _is_stop{false}
    {
        for(auto core : cores)
            _init_work(core);
    }

    inline std::size_t size() { return _workers.size(); }

    // add new work item to the pool
    template <class F, class... Args>
#if __cplusplus >= 201703L || (defined(_MSC_VER) && _MSC_VER >= 1910)
    auto enqueue(F &&f, Args &&...args)
        -> std::future<typename std::invoke_result<F, Args...>::type>
    {
        using return_type = typename std::invoke_result<F, Args...>::type;
#else
    auto enqueue(F &&f, Args &&...args)
        -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;
#endif

        auto task = std::make_shared<std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();

        // lock begin
        {
            std::unique_lock<std::mutex> lock(_mu);

            // don't allow enqueueing after stopping the pool
            if(_is_stop)
                return std::future<return_type>();

            _tasks.emplace([task]() { (*task)(); });
        }
        // lock end

        _cond.notify_one();
        return res;
    }

    void clear()
    {
        // lock begin
        {
            std::unique_lock<std::mutex> lock(_mu);
            _is_stop = true;
        }
        // lock end

        _cond.notify_all();
        for(std::thread &worker : _workers)
            if(worker.joinable())
            {
                worker.join();
            }

        _is_stop = false;
    }

    // the destructor joins all threads
    inline ~thread_pool()
    {
        // lock begin
        {
            std::unique_lock<std::mutex> lock(_mu);
            _is_stop = true;
        }
        // lock end

        _cond.notify_all();
        for(std::thread &worker : _workers)
            if(worker.joinable())
            {
                worker.join();
            }
    }

  private:
    void _init_work(const int core = -1)
    {
        _workers.emplace_back([this, core]() {
            // Set the thread affinity to the specified core
            if(core > -1 && !_bind_core(core))
                throw std::runtime_error("Failed to bind thread to core "
                                         + std::to_string(core));

            for(;;)
            {
                std::function<void()> task;

                // lock begin
                {
                    std::unique_lock<std::mutex> lock(this->_mu);
                    this->_cond.wait(lock, [this]() {
                        return this->_is_stop || !this->_tasks.empty();
                    });
                    if(this->_is_stop && this->_tasks.empty())
                        return;

                    task = std::move(this->_tasks.front());
                    this->_tasks.pop();
                }
                // lock end

                try
                {
                    task();
                }
                catch(...)
                {
                    std::cerr << "RUN THREAD POOL TASK EXCEPTION" << std::endl;
                }
            }
        });
    }

    bool _bind_core(const unsigned int core)
    {
#if defined(_WIN32)
        HANDLE    hThread = GetCurrentThread();
        DWORD_PTR mask =
            SetThreadAffinityMask(hThread, (DWORD_PTR) (1LLU << core));
        return (mask != 0);
#elif __linux__
        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(core, &mask);
        return (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask)
                >= 0);
#else
    return false;
#endif
    }

  private:
    thread_pool(const thread_pool &)            = delete;
    thread_pool &operator=(const thread_pool &) = delete;

  private:
    std::vector<std::thread> _workers;
    std::queue<task_t>       _tasks;
    std::mutex               _mu;
    std::condition_variable  _cond;
    bool                     _is_stop = false;
};

}

#endif