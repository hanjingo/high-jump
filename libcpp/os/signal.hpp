#ifndef SIGNAL_HPP
#define SIGNAL_HPP

#include <vector>
#include <atomic>
#include <mutex>
#include <signal.h>
#include <functional>
#include <initializer_list>

#if defined(NSIG) && !defined(_NSIG)
#define _NSIG NSIG
#endif

namespace libcpp
{
using sig_t = int;

static std::mutex _sig_mu;
static sig_t _sig_last;
static std::function<void(sig_t)> _sighandler_set[_NSIG];

static void _on_sig(sig_t sig)
{
    std::lock_guard<std::mutex> lock(_sig_mu);
    std::function<void(sig_t)> fn = _sighandler_set[sig];
    fn(sig);

    _sig_last = sig;
}

static void _on_sig_once(sig_t sig)
{
    std::lock_guard<std::mutex> lock(_sig_mu);
    std::function<void(sig_t)> fn = _sighandler_set[sig];
    fn(sig);

    _sig_last = sig;
    sigignore(sig);
}

static std::function<void(sig_t)> _sigcatch(sig_t sig, const std::function<void(sig_t)>&& cb, const bool one_shoot = false)
{
    std::lock_guard<std::mutex> lock(_sig_mu);
    std::function<void(sig_t)> old = _sighandler_set[sig];
    _sighandler_set[sig] = std::move(cb);
    if (one_shoot) {
        signal(sig, _on_sig_once);
    }
    else {
        signal(sig, _on_sig);
    }
    return old;
}

static bool sigcatch(
    const std::initializer_list<sig_t>& sigs, 
    const std::function<void(sig_t)>& cb, 
    const bool one_shoot = false)
{
    for (sig_t sig : sigs) {
        std::function<void(sig_t)> copy_cb = cb;
        _sigcatch(sig, std::move(cb), one_shoot);
    }
}

static inline int last_signal() { return _sig_last; }

template <typename... Args>
static bool sig_raise(Args... args) 
{ 
    std::initializer_list<sig_t> li{std::forward<Args>(args)...};
    for (sig_t sig : li) {
        if (raise(sig) != sig)
            return false;
    } 

    return true;
};

}

#endif