/*
 * This file is part of high-jump(hj).
 *
 * Copyright 2025 hanjingo <hehehunanchina@live.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <algorithm>
#include <cctype>
#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <tlhelp32.h>
#else
#include <cerrno>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#if defined(__linux__)
#include <sys/syscall.h>
#endif
#endif

namespace hj::os
{

#if defined(_WIN32)
using pid_t                                     = DWORD;
using native_handle_t                           = HANDLE;
inline constexpr native_handle_t invalid_handle = nullptr;
#else
using pid_t                                     = ::pid_t;
using native_handle_t                           = ::pid_t;
inline constexpr native_handle_t invalid_handle = -1;
#endif

struct exit_status
{
    bool exited_normally{false};
    int  exit_code{-1};

    bool signaled{false};
    int  termsig{0};

    [[nodiscard]] bool success() const noexcept
    {
        return exited_normally && exit_code == 0;
    }

    [[nodiscard]] int code() const noexcept
    {
        if(exited_normally)
            return exit_code;
        if(signaled)
            return 128 + termsig;
        return exit_code;
    }

    explicit operator int() const noexcept { return code(); }
};

enum class process_policy
{
    detach_on_destroy,
    wait_on_destroy,
    terminate_on_destroy,
    kill_on_destroy,
    manual
};

struct daemon_options
{
    std::string working_directory = "/";
    std::string pid_file;
    std::string user;
    std::string group;
#if !defined(_WIN32)
    mode_t umask_value = 0;
#endif
    bool redirect_stdio = true;
    bool auto_close_fds = true;
};

struct process_info
{
    pid_t                      pid{0};
    pid_t                      ppid{0};
    std::string                name;
    std::optional<std::string> cmdline;
};

using list_match_cb = std::function<bool(const process_info &)>;

inline pid_t getpid() noexcept;
inline pid_t getppid() noexcept;
inline bool  terminate(pid_t pid) noexcept;
inline bool  kill(pid_t pid) noexcept;

inline bool is_alive(pid_t pid) noexcept
{
    if(pid <= 0)
        return false;

#if defined(_WIN32)
    HANDLE h = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if(!h)
        return false;

    DWORD exit_code = 0;
    BOOL  ok = ::GetExitCodeProcess(h, &exit_code);
    ::CloseHandle(h);
    return ok != FALSE && exit_code == STILL_ACTIVE;
#else
    if(::kill(pid, 0) == 0)
        return true;
    return errno == EPERM;
#endif
}

namespace detail
{
#if !defined(_WIN32)
class async_reaper
{
  public:
    static async_reaper &instance()
    {
        static async_reaper reaper;
        return reaper;
    }

    void add_pid(pid_t pid)
    {
        if(pid <= 0)
            return;
        std::lock_guard<std::mutex> lock(_mutex);
        _pids.push_back(pid);
        if(!_worker.joinable())
        {
            _stop   = false;
            _worker = std::thread([this]() { run(); });
        }
    }

    ~async_reaper()
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _stop = true;
        }
        if(_worker.joinable())
            _worker.join();
    }

  private:
    async_reaper() = default;

    void run()
    {
        while(true)
        {
            std::vector<pid_t> current_pids;
            {
                std::lock_guard<std::mutex> lock(_mutex);
                if(_pids.empty() && _stop)
                    break;
                current_pids = _pids;
            }

            if(current_pids.empty())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            std::vector<pid_t> remaining;
            for(auto pid : current_pids)
            {
                int   status = 0;
                pid_t res    = 0;
                do
                {
                    res = ::waitpid(pid, &status, WNOHANG);
                } while(res < 0 && errno == EINTR);

                if(res == 0)
                    remaining.push_back(pid);
            }

            {
                std::lock_guard<std::mutex> lock(_mutex);
                std::vector<pid_t>          new_pids;
                for(auto pid : _pids)
                {
                    if(std::find(remaining.begin(), remaining.end(), pid)
                       != remaining.end())
                        new_pids.push_back(pid);
                }
                _pids = std::move(new_pids);
                if(_pids.empty() && _stop)
                    break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    std::mutex         _mutex;
    std::vector<pid_t> _pids;
    std::thread        _worker;
    bool               _stop{false};
};

inline void close_all_fds_above(int min_fd, int keep_fd = -1) noexcept
{
#if defined(__linux__) && defined(__NR_close_range)
    if(keep_fd >= min_fd)
    {
        bool success = true;
        if(keep_fd > min_fd)
        {
            if(::syscall(__NR_close_range, min_fd, keep_fd - 1, 0) < 0)
                success = false;
        }
        if(static_cast<unsigned int>(keep_fd) < ~0U)
        {
            if(::syscall(__NR_close_range, keep_fd + 1, ~0U, 0) < 0)
                success = false;
        }
        if(success)
            return;
    } else
    {
        if(::syscall(__NR_close_range, min_fd, ~0U, 0) == 0)
            return;
    }
#endif
    DIR *dir = ::opendir("/proc/self/fd");
    if(dir)
    {
        int            dir_fd = ::dirfd(dir);
        struct dirent *entry  = nullptr;
        while((entry = ::readdir(dir)) != nullptr)
        {
            if(entry->d_name[0] == '.')
                continue;

            int fd = 0;
            auto [ptr, ec] =
                std::from_chars(entry->d_name,
                                entry->d_name + std::strlen(entry->d_name),
                                fd);
            if(ec == std::errc{} && fd >= min_fd && fd != dir_fd
               && fd != keep_fd)
                ::close(fd);
        }
        ::closedir(dir);
        return;
    }

    long max_fd = ::sysconf(_SC_OPEN_MAX);
    if(max_fd < 0 || max_fd > 65536)
        max_fd = 65536;

    for(int fd = min_fd; fd < max_fd; ++fd)
    {
        if(fd != keep_fd)
            ::close(fd);
    }
}

inline std::string &global_pid_file_path()
{
    static std::string path;
    return path;
}

inline void remove_pid_file_safe() noexcept
{
    auto &path = global_pid_file_path();
    if(!path.empty())
        ::unlink(path.c_str());
}

inline void daemon_signal_handler(int sig)
{
    remove_pid_file_safe();
    ::_exit(128 + sig);
}

inline void register_pid_file_cleanup(const std::string &path)
{
    global_pid_file_path() = path;
    std::atexit([]() { remove_pid_file_safe(); });
    struct sigaction sa{};
    sa.sa_handler = daemon_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    ::sigaction(SIGTERM, &sa, nullptr);
    ::sigaction(SIGINT, &sa, nullptr);
    ::sigaction(SIGHUP, &sa, nullptr);
}

#else
inline bool utf8_to_utf16(const std::string &str,
                          std::wstring      &out,
                          std::error_code   &ec) noexcept
{
    ec.clear();
    out.clear();
    if(str.empty())
        return true;

    int size_needed = ::MultiByteToWideChar(CP_UTF8,
                                            MB_ERR_INVALID_CHARS,
                                            str.c_str(),
                                            static_cast<int>(str.size()),
                                            nullptr,
                                            0);
    if(size_needed <= 0)
    {
        ec = std::error_code(static_cast<int>(::GetLastError()),
                             std::system_category());
        return false;
    }

    out.resize(size_needed);
    int ret = ::MultiByteToWideChar(CP_UTF8,
                                    MB_ERR_INVALID_CHARS,
                                    str.c_str(),
                                    static_cast<int>(str.size()),
                                    &out[0],
                                    size_needed);
    if(ret <= 0)
    {
        ec = std::error_code(static_cast<int>(::GetLastError()),
                             std::system_category());
        out.clear();
        return false;
    }

    return true;
}

inline std::string utf16_to_utf8(const std::wstring &wstr) noexcept
{
    if(wstr.empty())
        return "";
    int size_needed = ::WideCharToMultiByte(CP_UTF8,
                                            0,
                                            wstr.c_str(),
                                            static_cast<int>(wstr.size()),
                                            nullptr,
                                            0,
                                            nullptr,
                                            nullptr);
    if(size_needed <= 0)
        return "";

    std::string strTo(size_needed, 0);
    ::WideCharToMultiByte(CP_UTF8,
                          0,
                          wstr.c_str(),
                          static_cast<int>(wstr.size()),
                          &strTo[0],
                          size_needed,
                          nullptr,
                          nullptr);
    return strTo;
}

inline std::wstring escape_win_arg(const std::string &arg,
                                   std::error_code   &ec) noexcept
{
    std::wstring warg;
    if(!utf8_to_utf16(arg, warg, ec) || ec)
        return L"";

    if(warg.empty())
        return L"\"\"";
    if(warg.find_first_of(L" \t\n\v\"") == std::wstring::npos)
        return warg;

    std::wstring result = L"\"";
    for(auto it = warg.begin();; ++it)
    {
        unsigned num_backslashes = 0;
        while(it != warg.end() && *it == L'\\')
        {
            ++it;
            ++num_backslashes;
        }

        if(it == warg.end())
        {
            result.append(num_backslashes * 2, L'\\');
            break;
        } else if(*it == L'"')
        {
            result.append(num_backslashes * 2 + 1, L'\\');
            result.push_back(*it);
        } else
        {
            result.append(num_backslashes, L'\\');
            result.push_back(*it);
        }
    }
    result.push_back(L'"');
    return result;
}

struct win_enum_param
{
    DWORD pid;
    bool  sent;
};

inline BOOL CALLBACK enum_windows_close_cb(HWND hwnd, LPARAM lparam)
{
    win_enum_param *param      = reinterpret_cast<win_enum_param *>(lparam);
    DWORD           process_id = 0;
    GetWindowThreadProcessId(hwnd, &process_id);
    if(process_id == param->pid && GetWindow(hwnd, GW_OWNER) == nullptr)
    {
        PostMessageW(hwnd, WM_CLOSE, 0, 0);
        param->sent = true;
    }
    return TRUE;
}
#endif
} // namespace detail

inline pid_t getpid() noexcept
{
#if defined(_WIN32)
    return ::GetCurrentProcessId();
#else
    return ::getpid();
#endif
}

inline pid_t getppid() noexcept
{
#if defined(_WIN32)
    DWORD  ppid  = 0;
    DWORD  pid   = GetCurrentProcessId();
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hSnap == INVALID_HANDLE_VALUE)
        return ppid;

    PROCESSENTRY32W pe = {sizeof(pe)};
    if(Process32FirstW(hSnap, &pe))
    {
        do
        {
            if(pe.th32ProcessID == pid)
            {
                ppid = pe.th32ParentProcessID;
                break;
            }
        } while(Process32NextW(hSnap, &pe));
    }
    CloseHandle(hSnap);
    return ppid;
#else
    return ::getppid();
#endif
}

inline bool terminate(pid_t pid) noexcept
{
    if(pid <= 0)
        return false;

#if defined(_WIN32)
    detail::win_enum_param param{pid, false};
    EnumWindows(detail::enum_windows_close_cb,
                reinterpret_cast<LPARAM>(&param));
    if(param.sent)
        return true;

    if(::GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, pid))
        return true;

    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if(!hProcess)
        return false;

    BOOL ret = TerminateProcess(hProcess, 1);
    CloseHandle(hProcess);
    return ret != FALSE;
#else
    return ::kill(pid, SIGTERM) == 0;
#endif
}

inline bool kill(pid_t pid) noexcept
{
    if(pid <= 0)
        return false;

#if defined(_WIN32)
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if(!hProcess)
        return false;

    BOOL ret = TerminateProcess(hProcess, 1);
    CloseHandle(hProcess);
    return ret != FALSE;
#else
    return ::kill(pid, SIGKILL) == 0;
#endif
}

class process
{
  public:
    struct options
    {
        std::string              command;
        std::vector<std::string> args;
        std::string              working_directory;
        bool                     detached            = false;
        bool                     redirect_stdin_null = false;
        process_policy           policy = process_policy::detach_on_destroy;
    };

    process() = default;

    ~process() noexcept { clean_up(); }

    process(const process &)            = delete;
    process &operator=(const process &) = delete;

    process(process &&other) noexcept
        : _pid(std::exchange(other._pid, 0))
        , _handle(std::exchange(other._handle, invalid_handle))
        , _detached(std::exchange(other._detached, true))
        , _policy(other._policy)
        , _exit_status(other._exit_status)
    {
    }

    process &operator=(process &&other) noexcept
    {
        if(this != &other)
        {
            clean_up();

            _pid         = std::exchange(other._pid, 0);
            _handle      = std::exchange(other._handle, invalid_handle);
            _detached    = std::exchange(other._detached, true);
            _policy      = other._policy;
            _exit_status = other._exit_status;
        }
        return *this;
    }

    bool start(options opts, std::error_code &ec)
    {
        clean_up();
        ec.clear();

        if(opts.command.empty())
        {
            ec = std::make_error_code(std::errc::invalid_argument);
            return false;
        }

        _detached    = opts.detached;
        _policy      = opts.policy;
        _exit_status = std::nullopt;

#if defined(_WIN32)
        std::wstring cmdline = detail::escape_win_arg(opts.command, ec);
        if(ec)
            return false;

        for(const auto &arg : opts.args)
        {
            cmdline += L" ";
            cmdline += detail::escape_win_arg(arg, ec);
            if(ec)
                return false;
        }

        STARTUPINFOW        si = {sizeof(si)};
        PROCESS_INFORMATION pi = {};

        HANDLE hNullInput = INVALID_HANDLE_VALUE;
        if(opts.redirect_stdin_null)
        {
            SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES),
                                      nullptr,
                                      TRUE};
            hNullInput = CreateFileW(L"NUL",
                                     GENERIC_READ,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     &sa,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     nullptr);
            if(hNullInput == INVALID_HANDLE_VALUE)
            {
                ec = std::error_code(static_cast<int>(::GetLastError()),
                                     std::system_category());
                return false;
            }

            si.dwFlags |= STARTF_USESTDHANDLES;
            si.hStdInput  = hNullInput;
            si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
        }

        DWORD creation_flags = CREATE_NEW_PROCESS_GROUP;
        if(opts.detached)
        {
            creation_flags |= DETACHED_PROCESS;
        }

        std::wstring wcwd;
        if(!opts.working_directory.empty())
        {
            if(!detail::utf8_to_utf16(opts.working_directory, wcwd, ec) || ec)
            {
                if(hNullInput != INVALID_HANDLE_VALUE)
                    CloseHandle(hNullInput);
                return false;
            }
        }
        LPCWSTR pcwd = wcwd.empty() ? nullptr : wcwd.c_str();

        std::vector<wchar_t> cmd_buf(cmdline.begin(), cmdline.end());
        cmd_buf.push_back(L'\0');

        BOOL ret = CreateProcessW(nullptr,
                                  cmd_buf.data(),
                                  nullptr,
                                  nullptr,
                                  FALSE,
                                  creation_flags,
                                  nullptr,
                                  pcwd,
                                  &si,
                                  &pi);

        if(hNullInput != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hNullInput);
        }

        if(!ret)
        {
            ec = std::error_code(static_cast<int>(::GetLastError()),
                                 std::system_category());
            return false;
        }

        CloseHandle(pi.hThread);
        _pid    = pi.dwProcessId;
        _handle = pi.hProcess;
        return true;

#else
        int err_pipe[2];

#if defined(__linux__) && defined(O_CLOEXEC)
        if(::pipe2(err_pipe, O_CLOEXEC) < 0)
        {
            ec = std::error_code(errno, std::generic_category());
            return false;
        }
#else
        if(::pipe(err_pipe) < 0)
        {
            ec = std::error_code(errno, std::generic_category());
            return false;
        }

        if(::fcntl(err_pipe[0], F_SETFD, FD_CLOEXEC) == -1
           || ::fcntl(err_pipe[1], F_SETFD, FD_CLOEXEC) == -1)
        {
            ec = std::error_code(errno, std::generic_category());
            ::close(err_pipe[0]);
            ::close(err_pipe[1]);
            return false;
        }
#endif
        if(opts.detached)
        {
            pid_t p1 = ::fork();
            if(p1 < 0)
            {
                ec = std::error_code(errno, std::generic_category());
                ::close(err_pipe[0]);
                ::close(err_pipe[1]);
                return false;
            }

            if(p1 == 0)
            {
                ::close(err_pipe[0]);
                ::setsid();

                pid_t p2 = ::fork();
                if(p2 < 0)
                {
                    int                   err = errno;
                    [[maybe_unused]] auto w =
                        ::write(err_pipe[1], &err, sizeof(err));
                    ::_exit(127);
                }

                if(p2 > 0)
                    ::_exit(0);

                if(!opts.working_directory.empty())
                {
                    if(::chdir(opts.working_directory.c_str()) != 0)
                    {
                        int                   err = errno;
                        [[maybe_unused]] auto w =
                            ::write(err_pipe[1], &err, sizeof(err));
                        ::_exit(127);
                    }
                }

                if(opts.redirect_stdin_null)
                {
                    int null_fd = ::open("/dev/null", O_RDWR);
                    if(null_fd != -1)
                    {
                        ::dup2(null_fd, STDIN_FILENO);
                        ::close(null_fd);
                    }
                }

                std::vector<std::string> arg_storage;
                arg_storage.push_back(opts.command);
                arg_storage.insert(arg_storage.end(),
                                   opts.args.begin(),
                                   opts.args.end());

                std::vector<char *> c_args;
                c_args.reserve(arg_storage.size() + 1);
                for(auto &s : arg_storage)
                    c_args.push_back(s.data());
                c_args.push_back(nullptr);

                ::execvp(c_args[0], c_args.data());

                int                   err = errno;
                [[maybe_unused]] auto w =
                    ::write(err_pipe[1], &err, sizeof(err));
                ::_exit(127);
            }

            ::close(err_pipe[1]);
            int status = 0;
            while(::waitpid(p1, &status, 0) < 0 && errno == EINTR)
                ;

            int     child_errno = 0;
            ssize_t n           = 0;
            do
            {
                n = ::read(err_pipe[0], &child_errno, sizeof(child_errno));
            } while(n < 0 && errno == EINTR);

            ::close(err_pipe[0]);

            if(n == sizeof(child_errno))
            {
                ec = std::error_code(child_errno, std::generic_category());
                return false;
            } else if(n < 0)
            {
                ec = std::error_code(errno, std::generic_category());
                return false;
            }

            _pid      = 0;
            _handle   = invalid_handle;
            _detached = true;
            return true;
        }

        pid_t pid = ::fork();
        if(pid < 0)
        {
            ec = std::error_code(errno, std::generic_category());
            ::close(err_pipe[0]);
            ::close(err_pipe[1]);
            return false;
        }

        if(pid == 0)
        {
            ::close(err_pipe[0]);

            if(!opts.working_directory.empty())
            {
                if(::chdir(opts.working_directory.c_str()) != 0)
                {
                    int                   err = errno;
                    [[maybe_unused]] auto w =
                        ::write(err_pipe[1], &err, sizeof(err));
                    ::_exit(127);
                }
            }

            if(opts.redirect_stdin_null)
            {
                int null_fd = ::open("/dev/null", O_RDWR);
                if(null_fd != -1)
                {
                    ::dup2(null_fd, STDIN_FILENO);
                    ::close(null_fd);
                }
            }

            std::vector<std::string> arg_storage;
            arg_storage.push_back(opts.command);
            arg_storage.insert(arg_storage.end(),
                               opts.args.begin(),
                               opts.args.end());

            std::vector<char *> c_args;
            c_args.reserve(arg_storage.size() + 1);
            for(auto &s : arg_storage)
                c_args.push_back(s.data());

            c_args.push_back(nullptr);

            ::execvp(c_args[0], c_args.data());

            int                   err = errno;
            [[maybe_unused]] auto w   = ::write(err_pipe[1], &err, sizeof(err));
            ::_exit(127);
        }

        ::close(err_pipe[1]);

        int     child_errno = 0;
        ssize_t n           = 0;
        do
        {
            n = ::read(err_pipe[0], &child_errno, sizeof(child_errno));
        } while(n < 0 && errno == EINTR);

        ::close(err_pipe[0]);

        if(n == sizeof(child_errno))
        {
            ec         = std::error_code(child_errno, std::generic_category());
            int status = 0;
            while(::waitpid(pid, &status, 0) < 0 && errno == EINTR)
                ;
            return false;
        } else if(n < 0)
        {
            ec         = std::error_code(errno, std::generic_category());
            int status = 0;
            while(::waitpid(pid, &status, 0) < 0 && errno == EINTR)
                ;
            return false;
        }

        _pid    = pid;
        _handle = pid;
        return true;
#endif
    }

    bool start(options opts)
    {
        std::error_code ec;
        return start(std::move(opts), ec);
    }

    void set_policy(process_policy policy) noexcept { _policy = policy; }

    process_policy get_policy() const noexcept { return _policy; }

    bool is_valid() const noexcept { return _pid > 0; }

    pid_t id() const noexcept { return _pid; }

    void detach() noexcept
    {
        if(is_valid())
        {
#if !defined(_WIN32)
            if(_pid > 0 && !_detached)
            {
                int   status = 0;
                pid_t res    = 0;
                do
                {
                    res = ::waitpid(_pid, &status, WNOHANG);
                } while(res < 0 && errno == EINTR);

                if(res == 0)
                {
                    detail::async_reaper::instance().add_pid(_pid);
                }
            }
#endif
            _detached = true;
            release_handle();
            _pid = 0;
        }
    }

    bool terminate() noexcept
    {
        if(!is_valid())
            return false;

        return hj::os::terminate(_pid);
    }

    bool kill() noexcept
    {
        if(!is_valid())
            return false;

        return hj::os::kill(_pid);
    }

    std::optional<exit_status> wait(std::error_code &ec)
    {
        ec.clear();

        if(_exit_status.has_value())
            return _exit_status;

        if(!is_valid() || _detached)
        {
            ec = std::make_error_code(std::errc::invalid_argument);
            return std::nullopt;
        }

#if defined(_WIN32)
        if(_handle == invalid_handle)
        {
            ec = std::make_error_code(std::errc::invalid_argument);
            return std::nullopt;
        }

        DWORD wait_ret = WaitForSingleObject(_handle, INFINITE);
        if(wait_ret == WAIT_FAILED)
        {
            ec = std::error_code(static_cast<int>(::GetLastError()),
                                 std::system_category());
            return std::nullopt;
        }

        if(wait_ret == WAIT_OBJECT_0)
        {
            DWORD exit_code = 0;
            BOOL  got_code  = GetExitCodeProcess(_handle, &exit_code);

            release_handle();
            _pid = 0;

            if(got_code)
            {
                exit_status st;
                st.exited_normally = true;
                st.exit_code       = static_cast<int>(exit_code);
                _exit_status       = st;
                return _exit_status;
            }

            ec = std::error_code(static_cast<int>(::GetLastError()),
                                 std::system_category());
            return std::nullopt;
        }

        ec = std::make_error_code(std::errc::state_not_recoverable);
        return std::nullopt;

#else
        int status = 0;
        while(true)
        {
            pid_t res = ::waitpid(_pid, &status, 0);

            if(res == _pid)
            {
                _pid    = 0;
                _handle = invalid_handle;

                exit_status st;
                if(WIFEXITED(status))
                {
                    st.exited_normally = true;
                    st.exit_code       = WEXITSTATUS(status);
                } else if(WIFSIGNALED(status))
                {
                    st.signaled = true;
                    st.termsig  = WTERMSIG(status);
                }
                _exit_status = st;
                return _exit_status;
            }

            if(res < 0)
            {
                if(errno == EINTR)
                    continue;

                ec = std::error_code(errno, std::generic_category());

                if(errno == ECHILD)
                {
                    _pid    = 0;
                    _handle = invalid_handle;
                }

                return std::nullopt;
            }
        }
#endif
    }

    std::optional<exit_status> wait()
    {
        std::error_code ec;
        auto            res = wait(ec);
        if(ec)
        {
            throw std::system_error(ec, "Failed to wait for process");
        }
        return res;
    }

    bool is_running()
    {
        if(!is_valid())
            return false;

        if(_exit_status.has_value())
            return false;

#if defined(_WIN32)
        if(_handle == invalid_handle)
            return false;

        DWORD wait_res = WaitForSingleObject(_handle, 0);
        if(wait_res == WAIT_TIMEOUT)
        {
            return true;
        } else if(wait_res == WAIT_OBJECT_0)
        {
            DWORD exit_code = 0;
            if(GetExitCodeProcess(_handle, &exit_code))
            {
                exit_status st;
                st.exited_normally = true;
                st.exit_code       = static_cast<int>(exit_code);
                _exit_status       = st;
            }

            release_handle();
            _pid = 0;
            return false;
        }

        return false;
#else
        int status = 0;

        while(true)
        {
            pid_t res = ::waitpid(_pid, &status, WNOHANG);

            if(res == 0)
                return true;

            if(res == _pid)
            {
                exit_status st;
                if(WIFEXITED(status))
                {
                    st.exited_normally = true;
                    st.exit_code       = WEXITSTATUS(status);
                } else if(WIFSIGNALED(status))
                {
                    st.signaled = true;
                    st.termsig  = WTERMSIG(status);
                }
                _exit_status = st;
                _pid         = 0;
                _handle      = invalid_handle;
                return false;
            }

            if(res < 0)
            {
                if(errno == EINTR)
                    continue;

                if(errno == ECHILD)
                {
                    _pid    = 0;
                    _handle = invalid_handle;
                    return false;
                }

                return true;
            }
        }
#endif
    }

  private:
    void release_handle() noexcept
    {
#if defined(_WIN32)
        if(_handle != invalid_handle)
        {
            CloseHandle(_handle);
            _handle = invalid_handle;
        }
#else
        _handle = invalid_handle;
#endif
    }

    void clean_up() noexcept
    {
        if(is_valid() && !_detached)
        {
            switch(_policy)
            {
                case process_policy::detach_on_destroy:
                    detach();
                    break;
                case process_policy::wait_on_destroy:
                    wait();
                    break;
                case process_policy::terminate_on_destroy:
                    terminate();
                    wait();
                    break;
                case process_policy::kill_on_destroy:
                    kill();
                    wait();
                    break;
                case process_policy::manual:
                    break;
            }
        }

        release_handle();
        _pid = 0;
    }

    pid_t                      _pid{0};
    native_handle_t            _handle{invalid_handle};
    bool                       _detached{false};
    process_policy             _policy{process_policy::detach_on_destroy};
    std::optional<exit_status> _exit_status{std::nullopt};
};

inline process spawn(process::options opts, std::error_code &ec) noexcept
{
    process p;
    p.start(std::move(opts), ec);
    return p;
}

inline process spawn(process::options opts)
{
    std::error_code ec;
    process         p = spawn(std::move(opts), ec);
    if(ec)
    {
        throw std::system_error(ec, "Failed to spawn process");
    }
    return p;
}

inline bool spawn_detached(const std::string              &executable,
                           const std::vector<std::string> &args,
                           std::error_code                &ec) noexcept
{
    ec.clear();
#if defined(_WIN32)
    process::options opts;
    opts.command  = executable;
    opts.args     = args;
    opts.detached = true;
    opts.policy   = process_policy::detach_on_destroy;
    auto p        = spawn(std::move(opts), ec);
    return p.is_valid() && !ec;
#else
    if(executable.empty())
    {
        ec = std::make_error_code(std::errc::invalid_argument);
        return false;
    }

    int err_pipe[2];
#if defined(__linux__) && defined(O_CLOEXEC)
    if(::pipe2(err_pipe, O_CLOEXEC) < 0)
    {
        ec = std::error_code(errno, std::generic_category());
        return false;
    }
#else
    if(::pipe(err_pipe) < 0)
    {
        ec = std::error_code(errno, std::generic_category());
        return false;
    }
    ::fcntl(err_pipe[0], F_SETFD, FD_CLOEXEC);
    ::fcntl(err_pipe[1], F_SETFD, FD_CLOEXEC);
#endif

    pid_t pid = ::fork();
    if(pid < 0)
    {
        ec = std::error_code(errno, std::generic_category());
        ::close(err_pipe[0]);
        ::close(err_pipe[1]);
        return false;
    }

    if(pid > 0)
    {
        ::close(err_pipe[1]);
        int     child_errno = 0;
        ssize_t n = ::read(err_pipe[0], &child_errno, sizeof(child_errno));
        ::close(err_pipe[0]);

        int status = 0;
        ::waitpid(pid, &status, 0);

        if(n > 0)
        {
            ec = std::error_code(child_errno, std::generic_category());
            return false;
        }

        return true;
    }

    ::close(err_pipe[0]);
    if(::setsid() < 0)
    {
        int                   err = errno;
        [[maybe_unused]] auto w   = ::write(err_pipe[1], &err, sizeof(err));
        ::_exit(EXIT_FAILURE);
    }

    pid_t grandchild = ::fork();
    if(grandchild < 0)
    {
        int                   err = errno;
        [[maybe_unused]] auto w   = ::write(err_pipe[1], &err, sizeof(err));
        ::_exit(EXIT_FAILURE);
    }

    if(grandchild > 0)
        ::_exit(EXIT_SUCCESS);

    int dev_null = ::open("/dev/null", O_RDWR);
    if(dev_null != -1)
    {
        ::dup2(dev_null, STDIN_FILENO);
        ::dup2(dev_null, STDOUT_FILENO);
        ::dup2(dev_null, STDERR_FILENO);
        if(dev_null > 2)
            ::close(dev_null);
    }

    std::vector<std::string> arg_storage;
    arg_storage.push_back(executable);
    arg_storage.insert(arg_storage.end(), args.begin(), args.end());

    std::vector<char *> c_args;
    c_args.reserve(arg_storage.size() + 1);
    for(auto &s : arg_storage)
        c_args.push_back(s.data());

    c_args.push_back(nullptr);
    ::execvp(c_args[0], c_args.data());
    int                   err = errno;
    [[maybe_unused]] auto w   = ::write(err_pipe[1], &err, sizeof(err));
    ::_exit(EXIT_FAILURE);
#endif
}

inline bool spawn_detached(const std::string              &executable,
                           const std::vector<std::string> &args = {})
{
    std::error_code ec;
    bool            res = spawn_detached(executable, args, ec);
    if(ec)
    {
        throw std::system_error(ec,
                                "Failed to spawn detached process: "
                                    + executable);
    }
    return res;
}

inline bool daemonize(const daemon_options &opts, std::error_code &ec)
{
    ec.clear();
#if defined(_WIN32)
    (void) opts;
    ec = std::make_error_code(std::errc::not_supported);
    return false;
#else
    int err_pipe[2];
#if defined(__linux__) && defined(O_CLOEXEC)
    if(::pipe2(err_pipe, O_CLOEXEC) < 0)
    {
        ec = std::error_code(errno, std::generic_category());
        return false;
    }
#else
    if(::pipe(err_pipe) < 0)
    {
        ec = std::error_code(errno, std::generic_category());
        return false;
    }
    ::fcntl(err_pipe[0], F_SETFD, FD_CLOEXEC);
    ::fcntl(err_pipe[1], F_SETFD, FD_CLOEXEC);
#endif

    ::umask(opts.umask_value);

    pid_t pid = ::fork();
    if(pid < 0)
    {
        ec = std::error_code(errno, std::generic_category());
        ::close(err_pipe[0]);
        ::close(err_pipe[1]);
        return false;
    }

    if(pid > 0)
    {
        ::close(err_pipe[1]);
        int     child_errno = 0;
        ssize_t n = ::read(err_pipe[0], &child_errno, sizeof(child_errno));
        ::close(err_pipe[0]);

        int status = 0;
        ::waitpid(pid, &status, 0);

        if(n > 0)
        {
            ec = std::error_code(child_errno, std::generic_category());
            return false;
        }
        ::_exit(EXIT_SUCCESS);
    }

    ::close(err_pipe[0]);

    if(::setsid() < 0)
    {
        int                   err = errno;
        [[maybe_unused]] auto w   = ::write(err_pipe[1], &err, sizeof(err));
        ::_exit(EXIT_FAILURE);
    }

    ::signal(SIGHUP, SIG_IGN);
    pid = ::fork();
    if(pid < 0)
    {
        int                   err = errno;
        [[maybe_unused]] auto w   = ::write(err_pipe[1], &err, sizeof(err));
        ::_exit(EXIT_FAILURE);
    }

    if(pid > 0)
    {
        ::_exit(EXIT_SUCCESS);
    }

    if(!opts.working_directory.empty())
    {
        if(::chdir(opts.working_directory.c_str()) < 0)
        {
            int                   err = errno;
            [[maybe_unused]] auto w   = ::write(err_pipe[1], &err, sizeof(err));
            ::_exit(EXIT_FAILURE);
        }
    }

    gid_t target_gid = ::getgid();
    uid_t target_uid = ::getuid();
    bool  has_group  = false;
    bool  has_user   = false;

    if(!opts.group.empty())
    {
        struct group *gr = ::getgrnam(opts.group.c_str());
        if(!gr)
        {
            int                   err = ENOENT;
            [[maybe_unused]] auto w   = ::write(err_pipe[1], &err, sizeof(err));
            ::_exit(EXIT_FAILURE);
        }
        target_gid = gr->gr_gid;
        has_group  = true;
    }

    struct passwd *pw = nullptr;
    if(!opts.user.empty())
    {
        pw = ::getpwnam(opts.user.c_str());
        if(!pw)
        {
            int                   err = ENOENT;
            [[maybe_unused]] auto w   = ::write(err_pipe[1], &err, sizeof(err));
            ::_exit(EXIT_FAILURE);
        }
        target_uid = pw->pw_uid;
        has_user   = true;
        if(!has_group)
        {
            target_gid = pw->pw_gid;
        }
    }

    if(::geteuid() == 0)
    {
        if(has_user && pw)
        {
            if(::initgroups(pw->pw_name, target_gid) < 0)
            {
                int                   err = errno;
                [[maybe_unused]] auto w =
                    ::write(err_pipe[1], &err, sizeof(err));
                ::_exit(EXIT_FAILURE);
            }
        } else if(has_group)
        {
            if(::setgroups(0, nullptr) < 0)
            {
                int                   err = errno;
                [[maybe_unused]] auto w =
                    ::write(err_pipe[1], &err, sizeof(err));
                ::_exit(EXIT_FAILURE);
            }
        }
    }

    if(has_group || has_user)
    {
        if(::setgid(target_gid) < 0)
        {
            int                   err = errno;
            [[maybe_unused]] auto w   = ::write(err_pipe[1], &err, sizeof(err));
            ::_exit(EXIT_FAILURE);
        }
    }

    if(has_user)
    {
        if(::setuid(target_uid) < 0)
        {
            int                   err = errno;
            [[maybe_unused]] auto w   = ::write(err_pipe[1], &err, sizeof(err));
            ::_exit(EXIT_FAILURE);
        }
    }

    if(!opts.pid_file.empty())
    {
        int pid_fd = ::open(opts.pid_file.c_str(), O_RDWR | O_CREAT, 0644);
        if(pid_fd < 0)
        {
            int                   err = errno;
            [[maybe_unused]] auto w   = ::write(err_pipe[1], &err, sizeof(err));
            ::_exit(EXIT_FAILURE);
        }

        struct ::flock fl{};
        fl.l_type   = F_WRLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start  = 0;
        fl.l_len    = 0;

        if(::fcntl(pid_fd, F_SETLK, &fl) < 0)
        {
            int err = (errno == EAGAIN || errno == EACCES) ? EBUSY : errno;
            [[maybe_unused]] auto w = ::write(err_pipe[1], &err, sizeof(err));
            ::close(pid_fd);
            ::_exit(EXIT_FAILURE);
        }

        if(::ftruncate(pid_fd, 0) < 0)
        {
            int                   err = errno;
            [[maybe_unused]] auto w   = ::write(err_pipe[1], &err, sizeof(err));
            ::close(pid_fd);
            ::_exit(EXIT_FAILURE);
        }

        std::string pid_str      = std::to_string(::getpid()) + "\n";
        size_t      written      = 0;
        bool        write_failed = false;

        while(written < pid_str.size())
        {
            ssize_t n = ::write(pid_fd,
                                pid_str.data() + written,
                                pid_str.size() - written);
            if(n < 0)
            {
                if(errno == EINTR)
                    continue;
                write_failed = true;
                break;
            }
            written += static_cast<size_t>(n);
        }

        if(write_failed || ::fsync(pid_fd) < 0)
        {
            int                   err = errno;
            [[maybe_unused]] auto w   = ::write(err_pipe[1], &err, sizeof(err));
            ::close(pid_fd);
            ::_exit(EXIT_FAILURE);
        }

        detail::register_pid_file_cleanup(opts.pid_file);
    }

    if(opts.auto_close_fds)
        detail::close_all_fds_above(3, err_pipe[1]);

    if(opts.redirect_stdio)
    {
        int dev_null = ::open("/dev/null", O_RDWR);
        if(dev_null != -1)
        {
            ::dup2(dev_null, STDIN_FILENO);
            ::dup2(dev_null, STDOUT_FILENO);
            ::dup2(dev_null, STDERR_FILENO);
            if(dev_null > 2 && dev_null != err_pipe[1])
                ::close(dev_null);
        }
    }

    ::close(err_pipe[1]);
    return true;
#endif
}

inline bool daemonize(const daemon_options &opts = {})
{
    std::error_code ec;
    bool            res = daemonize(opts, ec);
    if(ec)
        throw std::system_error(ec, "Failed to daemonize process");

    return res;
}

inline void list(
    std::vector<process_info> &result,
    list_match_cb match = [](const process_info &) { return true; })
{
    result.clear();

#if defined(_WIN32)
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hSnap == INVALID_HANDLE_VALUE)
        return;

    PROCESSENTRY32W pe = {sizeof(pe)};
    if(Process32FirstW(hSnap, &pe))
    {
        do
        {
            process_info info;
            info.pid     = pe.th32ProcessID;
            info.ppid    = pe.th32ParentProcessID;
            info.name    = detail::utf16_to_utf8(pe.szExeFile);
            info.cmdline = std::nullopt;

            if(match(info))
            {
                result.push_back(std::move(info));
            }
        } while(Process32NextW(hSnap, &pe));
    }
    CloseHandle(hSnap);

#else
    namespace fs = std::filesystem;
    std::error_code ec;

    fs::directory_iterator iter("/proc", ec);
    if(ec)
        return;

    for(const auto &entry : iter)
    {
        std::error_code entry_ec;
        if(!entry.is_directory(entry_ec) || entry_ec)
            continue;

        std::string filename = entry.path().filename().string();
        if(filename.empty())
            continue;

        pid_t pid           = 0;
        auto [ptr, conv_ec] = std::from_chars(filename.data(),
                                              filename.data() + filename.size(),
                                              pid);
        if(conv_ec != std::errc{} || ptr != filename.data() + filename.size())
            continue;

        process_info info;
        info.pid = pid;

        std::ifstream comm_file(entry.path() / "comm");
        if(comm_file.is_open())
            std::getline(comm_file, info.name);

        std::ifstream cmd_file(entry.path() / "cmdline", std::ios::binary);
        if(cmd_file.is_open())
        {
            std::string parsed_cmdline;
            std::string arg;
            while(std::getline(cmd_file, arg, '\0'))
            {
                if(!parsed_cmdline.empty())
                    parsed_cmdline += " ";

                parsed_cmdline += arg;
            }

            if(!parsed_cmdline.empty())
                info.cmdline = parsed_cmdline;
            else
                info.cmdline = std::nullopt;

        } else
        {
            info.cmdline = std::nullopt;
        }

        if(info.name.empty() && info.cmdline.has_value()
           && !info.cmdline->empty())
            info.name = *info.cmdline;

        if(match(info))
            result.push_back(std::move(info));
    }
#endif
}

} // namespace hj::os

#endif // PROCESS_HPP