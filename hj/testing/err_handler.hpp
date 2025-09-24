#ifndef ERR_HANDLER_HPP
#define ERR_HANDLER_HPP

#include <string>
#include <memory>
#include <functional>
#include <boost/sml.hpp>

namespace hj 
{
namespace detail 
{

// event
template<typename T>
struct err_event 
{
	T ec;
	explicit err_event(T e) : ec(e) {}
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
template<typename T>
struct err_handler_impl
{
	err_handler_impl(std::function<void(const T&)>&& f_err)
		: on_err(std::move(f_err))
	{}

	std::function<void(const T&)> on_err;
	auto operator()() const 
	{
		using namespace boost::sml;
		return make_transition_table(
			*state<idle> + event<finish> = state<succed>,
			*state<idle> + event<abort> = state<failed>,
			*state<idle> + event<err_event<T>> / [this](const err_event<T>& e) { on_err(e.ec); } = state<handling>,

			*state<succed> + event<finish> = state<succed>,
			*state<succed> + event<abort> = state<succed>,
			*state<succed> + event<err_event<T>> / [this](const err_event<T>& e) { on_err(e.ec); } = state<handling>,

			state<handling> + event<finish> = state<succed>,
			state<handling> + event<abort> = state<failed>,
			state<handling> + event<err_event<T>> / [this](const err_event<T>& e) { on_err(e.ec); } = state<handling>,

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

template<typename T>
class err_handler
{
public:
	err_handler(std::function<void(const T&)>&& err_cb)
		: _base{std::move(err_cb)}
		, _sm{_base}
	{}
	err_handler(std::function<void(const T&)>&& err_cb, 
				std::function<bool(const T&)>&& is_ok)
		: _base{std::move(err_cb)}
		, _sm{_base}
		, _is_ok{std::move(is_ok)}
	{}
	~err_handler() = default;

	inline void match(const T& err)
	{
		if (_is_ok(err))
			_sm.process_event(detail::finish{});
		else
			_sm.process_event(detail::err_event<T>{err});
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

	std::function<bool(const T&)> _is_ok = [](const T& t){ return !t; };
};

} // namespace hj

#endif