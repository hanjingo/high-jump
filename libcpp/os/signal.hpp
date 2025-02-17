#ifndef SIGNAL_HPP
#define SIGNAL_HPP

#include <atomic>
#include <mutex>
#include <signal.h>
#include <functional>

#if defined(NSIG) && !defined(_NSIG)
#define _NSIG NSIG
#endif

namespace libcpp
{
static std::once_flag _sig_once;
static std::mutex _sig_mu;
static int _sig_last;
static std::function<void(int)> _sighandler_set[_NSIG];

static void _on_sig(int sig)
{
    std::lock_guard<std::mutex> lock(_sig_mu);
    std::function<void(int)> fn = _sighandler_set[sig];
    fn(sig);

    _sig_last = sig;
}

static void _on_sig_once(int sig)
{
    std::lock_guard<std::mutex> lock(_sig_mu);
    std::function<void(int)> fn = _sighandler_set[sig];
    fn(sig);

    _sig_last = sig;
    sigignore(sig);
}

std::function<void(int)> sigcatch(int sig, const std::function<void(int)>&& cb, const bool one_shoot = false)
{
    std::call_once(_sig_once, [&](){
        for (int i = 0; i < _NSIG; ++i)
            _sighandler_set[i] = std::bind(_on_sig, std::placeholders::_1);
    });

    std::lock_guard<std::mutex> lock(_sig_mu);
    std::function<void(int)> ret = _sighandler_set[sig];
    _sighandler_set[sig] = std::move(cb);
    if (one_shoot) {
        signal(sig, _on_sig);
    }
    else {
        signal(sig, _on_sig_once);
    }
    return ret;
}

static inline int last_signal() { return _sig_last; }

static inline int sig_raise(const int sig_no) { return raise(sig_no); };

}

#endif