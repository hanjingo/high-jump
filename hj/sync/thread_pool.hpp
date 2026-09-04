// Origin:     https://github.com/progschj/ThreadPool
// Refactored for Industrial Standards with Modern Const Correctness and Precise Thread Metrics

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
#include <exception>
#include <string>
#include <unordered_set>
#include <utility>
#include <type_traits>
#include <algorithm>
#include <cstddef>
#include <atomic>
#include <tuple>
#include <new>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#endif

namespace hj
{

// SBO (Small Buffer Optimization) move-only task wrapper
class move_task
{
    static constexpr std::size_t SBO_SIZE = 64;

    struct vtable_t
    {
        void (*call)(void *);
        void (*destroy)(void *) noexcept;
        void (*move)(void *src, void *dst) noexcept;
    };

    template <typename F>
    struct sbo_handler
    {
        static void call(void *ptr) { (*static_cast<F *>(ptr))(); }
        static void destroy(void *ptr) noexcept { static_cast<F *>(ptr)->~F(); }
        static void move(void *src, void *dst) noexcept
        {
            new(dst) F(std::move(*static_cast<F *>(src)));
            destroy(src);
        }
    };

    template <typename F>
    struct heap_handler
    {
        static void call(void *ptr) { (**static_cast<F **>(ptr))(); }
        static void destroy(void *ptr) noexcept
        {
            delete *static_cast<F **>(ptr);
        }
        static void move(void *src, void *dst) noexcept
        {
            *static_cast<F **>(dst) = *static_cast<F **>(src);
            *static_cast<F **>(src) = nullptr;
        }
    };

    const vtable_t *_vptr = nullptr;
    alignas(std::max_align_t) char _buffer[SBO_SIZE];

  public:
    move_task() noexcept = default;

    template <typename F,
              typename = typename std::enable_if<
                  !std::is_same<typename std::decay<F>::type,
                                move_task>::value>::type>
    move_task(F &&f)
    {
        using DecayedF = typename std::decay<F>::type;

        constexpr bool fits_sbo =
            (sizeof(DecayedF) <= SBO_SIZE)
            && (alignof(DecayedF) <= alignof(std::max_align_t))
            && std::is_nothrow_move_constructible_v<DecayedF>;

        if(fits_sbo)
        {
            static const vtable_t vtable = {&sbo_handler<DecayedF>::call,
                                            &sbo_handler<DecayedF>::destroy,
                                            &sbo_handler<DecayedF>::move};
            new(_buffer) DecayedF(std::forward<F>(f));
            _vptr = &vtable;
        } else
        {
            static const vtable_t vtable = {&heap_handler<DecayedF>::call,
                                            &heap_handler<DecayedF>::destroy,
                                            &heap_handler<DecayedF>::move};
            *reinterpret_cast<DecayedF **>(_buffer) =
                new DecayedF(std::forward<F>(f));
            _vptr = &vtable;
        }
    }

    ~move_task()
    {
        if(_vptr)
        {
            _vptr->destroy(_buffer);
        }
    }

    move_task(move_task &&other) noexcept
    {
        if(other._vptr)
        {
            _vptr = other._vptr;
            _vptr->move(other._buffer, _buffer);
            other._vptr = nullptr;
        }
    }

    move_task &operator=(move_task &&other) noexcept
    {
        if(this != &other)
        {
            if(_vptr)
            {
                _vptr->destroy(_buffer);
            }
            _vptr = other._vptr;
            if(_vptr)
            {
                _vptr->move(other._buffer, _buffer);
                other._vptr = nullptr;
            }
        }
        return *this;
    }

    move_task(const move_task &)            = delete;
    move_task &operator=(const move_task &) = delete;

    void operator()()
    {
        if(_vptr)
        {
            _vptr->call(_buffer);
        }
    }

    explicit operator bool() const noexcept { return _vptr != nullptr; }
};

/**
 * @brief Thread pool managing worker threads for concurrent task execution.
 *
 * @note **EXCEPTION POLICY & CONTRACT**:
 *  - When a task executed inside the pool throws an exception, the exception is stored in the
 *    returned `std::future`. Caller can observe it by calling `future.get()`.
 *  - **CRITICAL**: If a task throws an exception AND no exception handler is installed via 
 *    `set_exception_handler()` / constructor, `std::terminate()` will be invoked immediately.
 *  - Ignoring the returned `std::future` without calling `.get()` on a throwing task **WILL** terminate 
 *    the process unless a fallback `exception_handler_t` is set. Unobserved task exceptions are fatal!
 */
class thread_pool
{
  public:
    using exception_handler_t = std::function<void(const std::exception_ptr &)>;

  public:
    /**
     * @brief Constructs a thread pool with a specified number of threads.
     * @param nthread The number of worker threads to create. Defaults to hardware concurrency.
     * @param handler Optional global exception callback for unhandled task exceptions.
     */
    explicit thread_pool(
        unsigned long       nthread = std::thread::hardware_concurrency(),
        exception_handler_t handler = nullptr)
        : _configured_threads{(nthread < 1) ? 1 : nthread}
        , _is_stop{false}
        , _exception_handler(std::move(handler))
    {
        _workers.reserve(_configured_threads);

        try
        {
            for(size_t i = 0; i < _configured_threads; ++i)
            {
                _init_work(-1);
            }
        }
        catch(...)
        {
            shutdown();
            throw;
        }
    }

    /**
     * @brief Constructs a thread pool with core affinity binding.
     * @param cores A set of global logical core IDs to bind worker threads to.
     * @param handler Optional global exception callback for unhandled task exceptions.
     */
    explicit thread_pool(const std::unordered_set<unsigned int> &cores,
                         exception_handler_t handler = nullptr)
        : _configured_threads{cores.size()}
        , _is_stop{false}
        , _exception_handler(std::move(handler))
    {
        if(cores.empty())
        {
            throw std::invalid_argument("cores set cannot be empty");
        }

        _workers.reserve(cores.size());

        try
        {
            for(auto core : cores)
            {
                _init_work(static_cast<int>(core));
            }
        }
        catch(...)
        {
            shutdown();
            throw;
        }
    }

    thread_pool(const thread_pool &)            = delete;
    thread_pool &operator=(const thread_pool &) = delete;
    thread_pool(thread_pool &&)                 = delete;
    thread_pool &operator=(thread_pool &&)      = delete;

    ~thread_pool() { shutdown(); }

    std::size_t worker_count() const noexcept { return _configured_threads; }
    std::size_t size() const noexcept { return worker_count(); }
    std::size_t active_thread_count() const noexcept
    {
        return _active_threads.load(std::memory_order_relaxed);
    }

    bool is_shutdown() const noexcept
    {
        return _is_stop.load(std::memory_order_acquire);
    }

    /**
     * @brief Sets or updates the global exception handler for unhandled task exceptions.
     * @param handler Callback to receive `std::exception_ptr` when a task throws.
     */
    void set_exception_handler(exception_handler_t handler)
    {
        std::unique_lock<std::mutex> lock(_mu);
        _exception_handler = std::move(handler);
    }

    /**
     * @brief Enqueues a callable for execution by the thread pool.
     * @tparam F Callable type.
     * @tparam Args Argument types.
     * @param f Function or lambda to execute.
     * @param args Arguments to pass to the function.
     * @return std::future holding the asynchronous result or exception.
     *
     * @warning If the callable throws an exception during execution, it will be stored in 
     *          the returned `std::future`. If `future.get()` is not called and no custom 
     *          `exception_handler_t` was configured, `std::terminate()` will be called!
     */
    template <class F, class... Args>
    auto enqueue(F &&f, Args &&...args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        using return_type = std::invoke_result_t<F, Args...>;
        auto p            = std::make_shared<std::promise<return_type>>();
        std::future<return_type> res = p->get_future();
        auto task_wrapper = [func = std::forward<F>(f),
                             args_tuple =
                                 std::make_tuple(std::forward<Args>(args)...),
                             p]() mutable {
            try
            {
                if constexpr(std::is_void_v<return_type>)
                {
                    std::apply(std::move(func), std::move(args_tuple));
                    p->set_value();
                } else
                {
                    p->set_value(
                        std::apply(std::move(func), std::move(args_tuple)));
                }
            }
            catch(...)
            {
                p->set_exception(std::current_exception());
                throw;
            }
        };

        {
            std::unique_lock<std::mutex> lock(_mu);
            if(_is_stop.load(std::memory_order_relaxed))
                return std::future<return_type>();

            _tasks.emplace(move_task(std::move(task_wrapper)));
        }

        _cond.notify_one();
        return res;
    }

    std::size_t cancel_pending()
    {
        std::unique_lock<std::mutex> lock(_mu);
        std::size_t                  count = _tasks.size();
        std::queue<move_task>        empty;
        std::swap(_tasks, empty);
        return count;
    }

    [[deprecated("Use cancel_pending() instead to explicitly reflect "
                 "broken_promise semantics.")]]
    std::size_t clear_pending_tasks()
    {
        return cancel_pending();
    }

    bool is_worker_thread() const noexcept
    {
        const auto                  current_id = std::this_thread::get_id();
        std::lock_guard<std::mutex> lock(_mu);
        return _worker_ids.find(current_id) != _worker_ids.end();
    }

    void shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(_mu);
            if(_worker_ids.find(std::this_thread::get_id())
               != _worker_ids.end())
            {
                throw std::logic_error(
                    "thread_pool::shutdown() cannot be called from a worker "
                    "thread inside the pool.");
            }

            if(_is_stop.load(std::memory_order_relaxed))
                return;

            _is_stop.store(true, std::memory_order_release);
        }

        _cond.notify_all();
        for(std::thread &worker : _workers)
        {
            if(worker.joinable())
                worker.join();
        }

        {
            std::lock_guard<std::mutex> lock(_mu);
            _workers.clear();
            _worker_ids.clear();
        }
    }

  private:
    void _init_work(const int core)
    {
        std::promise<void> init_promise;
        std::future<void>  init_future = init_promise.get_future();

        _workers.emplace_back([this,
                               core,
                               p = std::move(init_promise)]() mutable {
            struct active_guard
            {
                std::atomic<std::size_t> &counter;
                active_guard(std::atomic<std::size_t> &c)
                    : counter(c)
                {
                    counter.fetch_add(1, std::memory_order_relaxed);
                }
                ~active_guard()
                {
                    counter.fetch_sub(1, std::memory_order_relaxed);
                }
            } guard(this->_active_threads);

            {
                std::lock_guard<std::mutex> lock(this->_mu);
                this->_worker_ids.insert(std::this_thread::get_id());
            }

            if(core > -1)
            {
                if(!_bind_core(static_cast<unsigned int>(core)))
                {
                    p.set_exception(std::make_exception_ptr(
                        std::runtime_error("Failed to bind thread to core: "
                                           + std::to_string(core))));
                    return;
                }
            }

            p.set_value();

            for(;;)
            {
                move_task task;
                {
                    std::unique_lock<std::mutex> lock(this->_mu);
                    this->_cond.wait(lock, [this]() {
                        return this->_is_stop.load(std::memory_order_relaxed)
                               || !this->_tasks.empty();
                    });

                    if(this->_is_stop.load(std::memory_order_relaxed)
                       && this->_tasks.empty())
                        return;

                    task = std::move(this->_tasks.front());
                    this->_tasks.pop();
                }

                if(task)
                {
                    try
                    {
                        task();
                    }
                    catch(...)
                    {
                        exception_handler_t handler_copy;
                        {
                            std::lock_guard<std::mutex> lock(this->_mu);
                            handler_copy = this->_exception_handler;
                        }

                        if(!handler_copy)
                            std::terminate();

                        try
                        {
                            handler_copy(std::current_exception());
                        }
                        catch(...)
                        {
                            std::terminate();
                        }
                    }
                }
            }
        });

        init_future.get();
    }

    bool _bind_core(const unsigned int global_core)
    {
#if defined(_WIN32)
        HANDLE hThread = GetCurrentThread();

        WORD  num_groups       = GetActiveProcessorGroupCount();
        DWORD accumulated_cpus = 0;
        WORD  target_group     = 0;
        BYTE  relative_core    = 0;
        bool  found            = false;

        for(WORD g = 0; g < num_groups; ++g)
        {
            DWORD group_cpus = GetActiveProcessorCount(g);
            if(global_core < accumulated_cpus + group_cpus)
            {
                target_group = g;
                relative_core =
                    static_cast<BYTE>(global_core - accumulated_cpus);
                found = true;
                break;
            }
            accumulated_cpus += group_cpus;
        }

        if(!found)
        {
            return false;
        }

        GROUP_AFFINITY groupAffinity;
        ZeroMemory(&groupAffinity, sizeof(GROUP_AFFINITY));
        groupAffinity.Group = target_group;
        groupAffinity.Mask  = static_cast<KAFFINITY>(1ULL << relative_core);
        return SetThreadGroupAffinity(hThread, &groupAffinity, NULL) != FALSE;

#elif defined(__linux__)
        if(global_core >= CPU_SETSIZE)
        {
            return false;
        }

        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(global_core, &mask);
        return (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask)
                == 0);

#else
        (void) global_core;
        return true;
#endif
    }

  private:
    const std::size_t                   _configured_threads;
    std::atomic<std::size_t>            _active_threads{0};
    std::vector<std::thread>            _workers;
    std::unordered_set<std::thread::id> _worker_ids;
    std::queue<move_task>               _tasks;
    mutable std::mutex                  _mu;
    std::condition_variable             _cond;
    std::atomic<bool>                   _is_stop{false};
    exception_handler_t                 _exception_handler;
};

} // namespace hj

#endif // THREAD_POOL_HPP