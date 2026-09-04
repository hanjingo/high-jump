#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

#include <cstddef>
#include <deque>
#include <exception>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>

#include <boost/sml.hpp>

namespace hj
{

/**
 * @brief Representing current state of the error handler FSM.
 */
enum class err_status
{
    idle,
    handling,
    failed,
    success,
    unknown
};

/**
 * @brief Exception thrown when the deferred event queue reaches its capacity limit.
 */
class defer_queue_overflow : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

namespace detail
{

// event
template <typename T = std::error_code>
struct err_event
{
    T                              ec;
    std::function<void(const T &)> cb;

    explicit err_event(T e, std::function<void(const T &)> cb_fn = nullptr)
        : ec(std::move(e))
        , cb(std::move(cb_fn))
    {
    }
};

struct pass
{
};
struct resolved
{
};
struct fail
{
};
struct abort
{
};
struct reset
{
};

// state
struct idle
{
};
struct handling
{
};
struct failed
{
};
struct success
{
};

/**
 * @brief Bounded FIFO queue wrapper to manage deferred events.
 */
template <typename TElement, std::size_t Capacity = 64>
class bounded_defer_queue
{
  public:
    using container_type = std::deque<TElement>;
    using size_type      = typename container_type::size_type;

    bounded_defer_queue() = default;

    void push_back(TElement event)
    {
        if constexpr(Capacity > 0)
        {
            if(_queue.size() >= Capacity)
            {
                throw defer_queue_overflow(
                    "Defer queue overflow limit reached.");
            }
        }
        _queue.push_back(std::move(event));
    }

    void pop_front() { _queue.pop_front(); }

    TElement       &front() { return _queue.front(); }
    const TElement &front() const { return _queue.front(); }

    [[nodiscard]] bool      empty() const noexcept { return _queue.empty(); }
    [[nodiscard]] size_type size() const noexcept { return _queue.size(); }

    void clear() noexcept { _queue.clear(); }

  private:
    container_type _queue;
};

template <typename T, std::size_t MaxDeferCapacity>
struct sm_context
{
    using transition_cb = std::function<void(err_status src, err_status dst)>;
    using exception_cb  = std::function<void(const std::exception_ptr &)>;
    using event_type    = err_event<T>;

    transition_cb                                     on_transition;
    exception_cb                                      on_exception;
    bounded_defer_queue<event_type, MaxDeferCapacity> deferred_queue;

    explicit sm_context(transition_cb fn    = nullptr,
                        exception_cb  ex_fn = nullptr)
        : on_transition(std::move(fn))
        , on_exception(std::move(ex_fn))
    {
    }

    void safe_invoke(const std::function<void()> &fn) const noexcept
    {
        if(!fn)
            return;

        try
        {
            fn();
        }
        catch(...)
        {
            if(on_exception)
            {
                try
                {
                    on_exception(std::current_exception());
                }
                catch(...)
                {
                }
            }
        }
    }

    void safe_invoke_cb(const err_event<T> &e) const noexcept
    {
        if(e.cb)
        {
            safe_invoke([&] { e.cb(e.ec); });
        }
    }

    void safe_invoke_transition(err_status src, err_status dst) const noexcept
    {
        if(on_transition)
        {
            safe_invoke([&] { on_transition(src, dst); });
        }
    }

    void defer_event(const err_event<T> &e) { deferred_queue.push_back(e); }

    void replay_one_deferred()
    {
        if(!deferred_queue.empty())
        {
            auto event = std::move(deferred_queue.front());
            deferred_queue.pop_front();
            safe_invoke_cb(event);
        }
    }

    void clear_deferred() noexcept { deferred_queue.clear(); }
};

template <typename T = std::error_code, std::size_t MaxDeferCapacity = 64>
struct error_handler_impl
{
    auto operator()() const
    {
        using namespace boost::sml;

        auto is_queue_not_empty =
            [](const sm_context<T, MaxDeferCapacity> &ctx) noexcept {
                return !ctx.deferred_queue.empty();
            };

        auto is_queue_empty =
            [](const sm_context<T, MaxDeferCapacity> &ctx) noexcept {
                return ctx.deferred_queue.empty();
            };

        return make_transition_table(
            // --- IDLE STATE ---
            *state<idle>
                + event<pass> /
                      [](sm_context<T, MaxDeferCapacity> &ctx) {
                          ctx.safe_invoke_transition(err_status::idle,
                                                     err_status::success);
                      } = state<success>,
            *state<idle>
                + event<fail> /
                      [](sm_context<T, MaxDeferCapacity> &ctx) {
                          ctx.safe_invoke_transition(err_status::idle,
                                                     err_status::failed);
                      } = state<failed>,
            *state<idle>
                + event<abort> /
                      [](sm_context<T, MaxDeferCapacity> &ctx) {
                          ctx.safe_invoke_transition(err_status::idle,
                                                     err_status::failed);
                      } = state<failed>,
            *state<idle>
                + event<err_event<T>> /
                      [](sm_context<T, MaxDeferCapacity> &ctx, const auto &e) {
                          ctx.safe_invoke_cb(e);
                          ctx.safe_invoke_transition(err_status::idle,
                                                     err_status::handling);
                      }                         = state<handling>,
            *state<idle> + event<reset> / [] {} = state<idle>,

            // --- SUCCESS STATE ---
            state<success> + event<pass> / [] {}     = state<success>,
            state<success> + event<resolved> / [] {} = state<success>,
            state<success> + event<abort> / [] {}    = state<success>,
            state<success>
                + event<err_event<T>> /
                      [](sm_context<T, MaxDeferCapacity> &ctx, const auto &e) {
                          ctx.safe_invoke_cb(e);
                          ctx.safe_invoke_transition(err_status::success,
                                                     err_status::handling);
                      } = state<handling>,
            state<success>
                + event<reset> /
                      [](sm_context<T, MaxDeferCapacity> &ctx) {
                          ctx.clear_deferred();
                          ctx.safe_invoke_transition(err_status::success,
                                                     err_status::idle);
                      } = state<idle>,

            // --- HANDLING STATE ---
            state<handling>
                + event<pass> /
                      [](sm_context<T, MaxDeferCapacity> &ctx) {
                          ctx.clear_deferred();
                          ctx.safe_invoke_transition(err_status::handling,
                                                     err_status::success);
                      } = state<success>,
            state<handling>
                + event<resolved>[is_queue_not_empty] /
                      [](sm_context<T, MaxDeferCapacity> &ctx) {
                          ctx.replay_one_deferred();
                      } = state<handling>,
            state<handling>
                + event<resolved>[is_queue_empty] /
                      [](sm_context<T, MaxDeferCapacity> &ctx) {
                          ctx.safe_invoke_transition(err_status::handling,
                                                     err_status::success);
                      } = state<success>,
            state<handling>
                + event<fail> /
                      [](sm_context<T, MaxDeferCapacity> &ctx) {
                          ctx.clear_deferred();
                          ctx.safe_invoke_transition(err_status::handling,
                                                     err_status::failed);
                      } = state<failed>,
            state<handling>
                + event<abort> /
                      [](sm_context<T, MaxDeferCapacity> &ctx) {
                          ctx.clear_deferred();
                          ctx.safe_invoke_transition(err_status::handling,
                                                     err_status::failed);
                      } = state<failed>,
            state<handling>
                + event<reset> /
                      [](sm_context<T, MaxDeferCapacity> &ctx) {
                          ctx.clear_deferred();
                          ctx.safe_invoke_transition(err_status::handling,
                                                     err_status::idle);
                      } = state<idle>,
            state<handling>
                + event<err_event<T>> /
                      [](sm_context<T, MaxDeferCapacity> &ctx, const auto &e) {
                          ctx.defer_event(e);
                      } = state<handling>,

            // --- FAILED STATE (Explicit Ignored Design) ---
            state<failed> + event<pass> / [] {}         = state<failed>,
            state<failed> + event<resolved> / [] {}     = state<failed>,
            state<failed> + event<fail> / [] {}         = state<failed>,
            state<failed> + event<abort> / [] {}        = state<failed>,
            state<failed> + event<err_event<T>> / [] {} = state<failed>,
            state<failed>
                + event<reset> /
                      [](sm_context<T, MaxDeferCapacity> &ctx) {
                          ctx.clear_deferred();
                          ctx.safe_invoke_transition(err_status::failed,
                                                     err_status::idle);
                      } = state<idle>);
    }
};

} // namespace detail

/**
 * @brief Lightweight finite state machine wrapper for error handling and recovery.
 */
template <typename T                   = std::error_code,
          typename IsOkFn              = std::function<bool(const T &)>,
          std::size_t MaxDeferCapacity = 64>
class error_handler
{
    static_assert(
        std::is_invocable_r_v<bool, IsOkFn, const T &>,
        "IsOkFn must be a callable object with signature bool(const T &).");

  public:
    using is_ok_fn      = IsOkFn;
    using match_fn      = std::function<void(const T &)>;
    using transition_fn = std::function<void(err_status src, err_status dst)>;
    using exception_fn  = std::function<void(const std::exception_ptr &)>;

  private:
    using event_type   = detail::err_event<T>;
    using context_type = detail::sm_context<T, MaxDeferCapacity>;
    using sm_impl      = detail::error_handler_impl<T, MaxDeferCapacity>;
    using sm_type      = boost::sml::sm<sm_impl>;

    std::unique_ptr<sm_type> create_sm()
    {
        return std::make_unique<sm_type>(*_ctx);
    }

  public:
    error_handler()
        : _ctx{std::make_shared<context_type>(nullptr, nullptr)}
        , _sm{create_sm()}
        , _is_ok{[](const T &t) { return !t; }}
    {
    }

    static error_handler with_ok_value(T             ok_value,
                                       transition_fn on_transition = nullptr,
                                       exception_fn  on_exception  = nullptr)
    {
        return error_handler(
            [ok = std::move(ok_value)](const T &t) { return t == ok; },
            std::move(on_transition),
            std::move(on_exception));
    }

    static error_handler with_checker(is_ok_fn      is_ok,
                                      transition_fn on_transition = nullptr,
                                      exception_fn  on_exception  = nullptr)
    {
        return error_handler(std::move(is_ok),
                             std::move(on_transition),
                             std::move(on_exception));
    }

    static error_handler with_hooks(transition_fn on_transition,
                                    exception_fn  on_exception = nullptr)
    {
        return error_handler([](const T &t) { return !t; },
                             std::move(on_transition),
                             std::move(on_exception));
    }

    explicit error_handler(is_ok_fn      is_ok,
                           transition_fn on_transition = nullptr,
                           exception_fn  on_exception  = nullptr)
        : _ctx{std::make_shared<context_type>(std::move(on_transition),
                                              std::move(on_exception))}
        , _sm{create_sm()}
        , _is_ok{std::move(is_ok)}
    {
    }

    explicit error_handler(is_ok_fn is_ok)
        : error_handler(std::move(is_ok), nullptr, nullptr)
    {
    }

    ~error_handler() noexcept = default;

    error_handler(const error_handler &)            = delete;
    error_handler &operator=(const error_handler &) = delete;

    error_handler(error_handler &&other) noexcept(
        std::is_nothrow_move_constructible_v<is_ok_fn>)
        : _ctx(std::move(other._ctx))
        , _sm(std::move(other._sm))
        , _is_ok(std::move(other._is_ok))
    {
    }

    error_handler &operator=(error_handler &&other) noexcept(
        std::is_nothrow_move_assignable_v<is_ok_fn>)
    {
        if(this != &other)
        {
            _ctx   = std::move(other._ctx);
            _sm    = std::move(other._sm);
            _is_ok = std::move(other._is_ok);
        }
        return *this;
    }

    /**
     * @brief Process an incoming error.
     * @throws hj::defer_queue_overflow if the handling queue exceeds MaxDeferCapacity.
     */
    error_handler &match(const T &err, match_fn cb = nullptr)
    {
        if(_is_ok(err))
        {
            _sm->process_event(detail::pass{});
        } else
        {
            _sm->process_event(detail::err_event<T>{err, std::move(cb)});
        }

        return *this;
    }

    error_handler &pass()
    {
        _sm->process_event(detail::pass{});
        return *this;
    }

    error_handler &resolve()
    {
        _sm->process_event(detail::resolved{});
        return *this;
    }

    error_handler &fail()
    {
        _sm->process_event(detail::fail{});
        return *this;
    }

    error_handler &abort()
    {
        _sm->process_event(detail::abort{});
        return *this;
    }

    error_handler &reset()
    {
        _ctx->clear_deferred();
        _sm = create_sm();
        return *this;
    }

    [[nodiscard]] bool is_idle() const
    {
        return _sm->is(boost::sml::state<detail::idle>);
    }

    [[nodiscard]] bool is_handling() const
    {
        return _sm->is(boost::sml::state<detail::handling>);
    }

    [[nodiscard]] bool is_success() const
    {
        return _sm->is(boost::sml::state<detail::success>);
    }

    [[nodiscard]] bool is_failed() const
    {
        return _sm->is(boost::sml::state<detail::failed>);
    }

    [[nodiscard]] err_status status() const
    {
        if(_sm->is(boost::sml::state<detail::idle>))
            return err_status::idle;
        if(_sm->is(boost::sml::state<detail::handling>))
            return err_status::handling;
        if(_sm->is(boost::sml::state<detail::success>))
            return err_status::success;
        if(_sm->is(boost::sml::state<detail::failed>))
            return err_status::failed;

        return err_status::unknown;
    }

    /**
     * @brief Direct query on underlying queue. Single Source of Truth.
     */
    [[nodiscard]] std::size_t deferred_size() const noexcept
    {
        return _ctx ? _ctx->deferred_queue.size() : 0;
    }

    [[nodiscard]] static constexpr std::size_t max_defer_capacity() noexcept
    {
        return MaxDeferCapacity;
    }

  private:
    std::shared_ptr<context_type> _ctx;
    std::unique_ptr<sm_type>      _sm;
    is_ok_fn                      _is_ok;
};

} // namespace hj

#endif // ERROR_HANDLER_HPP