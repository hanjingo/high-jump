/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SIGNAL_HPP
#define SIGNAL_HPP

#include <vector>
#include <atomic>
#include <functional>
#include <initializer_list>
#include <csignal>
#include <mutex>
#include <unordered_map>
#include <iostream>

namespace hj
{

using sig_t = int;

class sighandler
{
  public:
    sighandler()
        : _sig_last(0)
    {
    }
    ~sighandler() = default;

    sighandler(const sighandler &)            = delete;
    sighandler &operator=(const sighandler &) = delete;
    sighandler(sighandler &&)                 = delete;
    sighandler &operator=(sighandler &&)      = delete;

    static sighandler &instance()
    {
        static sighandler inst;
        return inst;
    }

    inline int  last_signal() const noexcept { return _sig_last.load(); }
    inline bool is_registered(sig_t sig) noexcept
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _callbacks.find(sig) != _callbacks.end();
    }
    inline bool is_one_shoot(sig_t sig) noexcept
    {
        std::lock_guard<std::mutex> lock(_mu);
        return _one_shoot.find(sig) != _one_shoot.end() && _one_shoot.at(sig);
    }

    void
    sigcatch(sig_t sig, std::function<void(sig_t)> cb, bool one_shoot = false)
    {
        std::lock_guard<std::mutex> lock(_mu);
        _callbacks[sig] = cb;
        _one_shoot[sig] = one_shoot;
        ::signal(sig, &sighandler::_handle);
    }

    void sigcatch(const std::vector<sig_t>  &sigs,
                  std::function<void(sig_t)> cb,
                  bool                       one_shoot = false)
    {
        for(auto sig : sigs)
            sigcatch(sig, cb, one_shoot);
    }

    template <typename... Args>
    bool sigraise(Args... args)
    {
        std::initializer_list<sig_t> li{args...};
        for(sig_t sig : li)
        {
            if(::raise(sig) != 0)
                return false;
        }
        return true;
    }

    // NOTICE: windows only support: SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM
    // POSIX support more signals, such as: SIGHUP, SIGQUIT, SIGKILL, SIGUSR1, SIGUSR2, etc.
    void sigignore(const std::vector<sig_t> &sigs)
    {
        for(auto sig : sigs)
            ::signal(sig, SIG_IGN);
    }

    void poll()
    {
        sig_t sig = _sig_last.exchange(0);
        if(sig <= 0)
            return;

        std::lock_guard<std::mutex> lock(_mu);
        auto                        it = _callbacks.find(sig);
        if(it == _callbacks.end())
            return;

        try
        {
            it->second(sig);
        }
        catch(...)
        {
            std::cerr << "Signal callback exception for sig " << sig
                      << std::endl;
        }

        if(_one_shoot[sig])
        {
            _callbacks.erase(sig);
            _one_shoot.erase(sig);
        } else
        {
            ::signal(sig, &sighandler::_handle);
        }
    }

  private:
    static void _handle(int sig)
    {
        sighandler::instance()._sig_last.store(sig);
    }

    std::atomic<sig_t>                                    _sig_last;
    std::mutex                                            _mu;
    std::unordered_map<sig_t, std::function<void(sig_t)>> _callbacks;
    std::unordered_map<sig_t, bool>                       _one_shoot;
};

} // namespace hj

#endif