#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

#include <string>
#include <memory>
#include <functional>
#include <system_error>
#include <deque>

#include <boost/sml.hpp>

namespace hj
{
namespace detail
{

// event
template <typename T = std::error_code>
struct err_event
{
    T                              ec;
    std::function<void(const T &)> cb;

    explicit err_event(T e, std::function<void(const T &)> &&cb)
        : ec(e)
        , cb(std::move(cb))
    {
    }
};
struct finish
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
struct succed
{
};

// boost::sml
template <typename T = std::error_code>
struct error_handler_impl
{
    error_handler_impl(
        std::function<void(const char *src, const char *dst)> &&fn)
        : on_transition(std::move(fn))
    {
    }

    auto operator()() const
    {
        return boost::sml::make_transition_table(
            *boost::sml::state<idle>
                + boost::sml::event<finish>
                      / [this]() { on_transition("idle", "succed"); } =
                boost::sml::state<succed>,
            *boost::sml::state<idle>
                + boost::sml::event<abort>
                      / [this]() { on_transition("idle", "failed"); } =
                boost::sml::state<failed>,
            *boost::sml::state<idle>
                + boost::sml::event<err_event<T> > /
                      [this](const auto &e) {
                          e.cb(e.ec);
                          on_transition("idle", "handling");
                      } = boost::sml::state<handling>,
            *boost::sml::state<idle> + boost::sml::event<reset> / [this]() {} =
                boost::sml::state<idle>,

            boost::sml::state<succed>
                + boost::sml::event<finish> / [this]() {} =
                boost::sml::state<succed>,
            boost::sml::state<succed> + boost::sml::event<abort> / [this]() {} =
                boost::sml::state<succed>,
            boost::sml::state<succed>
                + boost::sml::event<err_event<T> > /
                      [this](const auto &e) {
                          e.cb(e.ec);
                          on_transition("succed", "handling");
                      } = boost::sml::state<handling>,
            boost::sml::state<succed>
                + boost::sml::event<reset>
                      / [this]() { on_transition("succed", "idle"); } =
                boost::sml::state<idle>,

            boost::sml::state<handling>
                + boost::sml::event<finish>
                      / [this]() { on_transition("handling", "succed"); } =
                boost::sml::state<succed>,
            boost::sml::state<handling>
                + boost::sml::event<abort>
                      / [this]() { on_transition("handling", "failed"); } =
                boost::sml::state<failed>,
            boost::sml::state<handling>
                + boost::sml::event<err_event<T> > / boost::sml::defer,

            boost::sml::state<failed>
                + boost::sml::event<reset>
                      / [this]() { on_transition("failed", "idle"); } =
                boost::sml::state<idle>);
    }

    std::function<void(const char *src, const char *dst)> on_transition;
};

} // namespace detail

template <typename T = std::error_code>
class error_handler
{
  public:
    using is_ok_fn = std::function<bool(const T &)>;
    using match_fn = std::function<void(const T &)>;
    using transaction_fn =
        std::function<void(const char *src, const char *dst)>;

  public:
    error_handler()
        : _base{[](const char *src, const char *dst) {}}
        , _sm{_base}
        , _is_ok{[](const T &t) { return !t; }}
    {
    }
    error_handler(transaction_fn &&on_transition)
        : _base{std::move(on_transition)}
        , _sm{_base}
        , _is_ok{[](const T &t) { return !t; }}
    {
    }
    error_handler(is_ok_fn &&is_ok)
        : _base{[](const char *src, const char *dst) {}}
        , _sm{_base}
        , _is_ok{std::move(is_ok)}
    {
    }
    ~error_handler() { _sm.process_event(detail::reset{}); }

    inline void match(const T &err, match_fn &&cb = [](const T &) {})
    {
        if(_is_ok(err))
            _sm.process_event(detail::finish{});
        else
            _sm.process_event(detail::err_event<T>{err, std::move(cb)});
    }

    inline void abort() { _sm.process_event(detail::abort{}); }

    inline void reset() { _sm.process_event(detail::reset{}); }

    inline const char *status() const
    {
        if(_sm.is(boost::sml::state<detail::idle>))
            return "idle";
        if(_sm.is(boost::sml::state<detail::handling>))
            return "handling";
        if(_sm.is(boost::sml::state<detail::succed>))
            return "succed";
        if(_sm.is(boost::sml::state<detail::failed>))
            return "failed";
        return "unknown";
    }

  private:
    detail::error_handler_impl<T> _base;
    boost::sml::sm<detail::error_handler_impl<T>,
                   boost::sml::defer_queue<std::deque> >
        _sm;

    std::function<bool(const T &)> _is_ok;
};

} // namespace hj

#endif