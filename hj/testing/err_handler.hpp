#ifndef ERR_HANDLER_HPP
#define ERR_HANDLER_HPP

#include <string>
#include <memory>
#include <functional>
#include <system_error>
#include <unordered_map>
#include <boost/sml.hpp>

namespace hj 
{
namespace detail 
{

// event
template<typename T = std::error_code>
struct err_event 
{
	T ec;
	std::function<void(const T&)> cb;

	explicit err_event(T e, std::function<void(const T&)>&& cb) : ec(e), cb(std::move(cb)) {}
};
struct finish {};
struct abort {};
struct reset {};

// state
struct idle {};
struct handling {};
struct failed {};
struct succed {};

// boost::sml
template<typename T = std::error_code>
struct err_handler_impl
{
	err_handler_impl() = default;

	auto operator()() const 
	{
		using namespace boost::sml;
		return make_transition_table(
			*state<idle> + event<finish> = state<succed>,
			*state<idle> + event<abort> = state<failed>,
			*state<idle> + event<err_event<T>> / [](const auto& e){ e.cb(e.ec); } = state<handling>,

			*state<succed> + event<finish> = state<succed>,
			*state<succed> + event<abort> = state<succed>,
			*state<succed> + event<err_event<T>> / [](const auto& e){ e.cb(e.ec); } = state<handling>,

			state<handling> + event<finish> = state<succed>,
			state<handling> + event<abort> = state<failed>,
			state<handling> + event<err_event<T>> / [](const auto& e){ e.cb(e.ec); } = state<handling>,

			state<succed> + event<reset> = state<idle>,
			state<failed> + event<reset> = state<idle>,
			state<idle> + event<reset> = state<idle>
		);
	}
};

} // namespace detail

enum class err_state
{
	unknown,
	idle,
	handling,
	succed,
	failed
};

template<typename T = std::error_code>
class err_handler
{
public:
	err_handler()
		: _base{}
		, _sm{_base}
		, _is_ok{[](const T& t){ return !t; }}
	{}
	err_handler(std::function<bool(const T&)>&& is_ok)
		: _base{}
		, _sm{_base}
		, _is_ok{std::move(is_ok)}
	{}
	~err_handler() = default;

	inline void match(const T& err, std::function<void(const T&)> cb = [](const T&){})
	{
		if (_is_ok(err))
			_sm.process_event(detail::finish{});
		else
			_sm.process_event(detail::err_event<T>{err, std::move(cb)});
	}

	inline void abort()
	{
		_sm.process_event(detail::abort{});
	}

	inline void reset()
	{
		_sm.process_event(detail::reset{});
	}

	inline err_state status() const
	{
		using namespace boost::sml;
		if (_sm.is(state<detail::idle>))    return err_state::idle;
		if (_sm.is(state<detail::handling>)) return err_state::handling;
		if (_sm.is(state<detail::succed>))  return err_state::succed;
		if (_sm.is(state<detail::failed>))  return err_state::failed;
		return err_state::unknown;
	}

private:
	detail::err_handler_impl<T> _base;
	boost::sml::sm<detail::err_handler_impl<T>> _sm;

	std::function<bool(const T&)> _is_ok;
};

} // namespace hj

#endif