/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025-2026 hanjingo <hehehunanchina@live.com>
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

#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <algorithm>
#include <cctype>
#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
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

namespace detail
{
#if defined(_WIN32)
inline std::wstring utf8_to_utf16(const std::string &str)
{
    if(str.empty())
        return L"";
    int          size_needed = MultiByteToWideChar(CP_UTF8,
                                                   0,
                                                   str.c_str(),
                                                   static_cast<int>(str.size()),
                                                   nullptr,
                                                   0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8,
                        0,
                        str.c_str(),
                        static_cast<int>(str.size()),
                        &wstrTo[0],
                        size_needed);
    return wstrTo;
}

inline std::string utf16_to_utf8(const std::wstring &wstr)
{
    if(wstr.empty())
        return "";
    int         size_needed = WideCharToMultiByte(CP_UTF8,
                                                  0,
                                                  wstr.c_str(),
                                                  static_cast<int>(wstr.size()),
                                                  nullptr,
                                                  0,
                                                  nullptr,
                                                  nullptr);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8,
                        0,
                        wstr.c_str(),
                        static_cast<int>(wstr.size()),
                        &strTo[0],
                        size_needed,
                        nullptr,
                        nullptr);
    return strTo;
}

inline std::wstring escape_win_arg(const std::string &arg)
{
    std::wstring warg = utf8_to_utf16(arg);
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

#else

inline void close_all_fds_above(int min_fd, int keep_fd = -1) noexcept
{
#if defined(__linux__) && defined(__NR_close_range)
    if(::syscall(__NR_close_range, min_fd, ~0U, 0) == 0)
    {
        if(keep_fd >= min_fd)
        {
        } else
        {
            return;
        }
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
    // Note: Windows does not maintain a dynamic parent-child relationship like Unix.
    // 'th32ParentProcessID' stores the static PID of the parent process at the moment
    // of creation. If the parent exits, this PID may be reassigned to an unrelated process.
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

    return GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, pid) != FALSE;
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

    explicit process(options opts)
    {
        std::error_code ec;
        start(std::move(opts), ec);
    }

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

    bool start(options opts, std::error_code &ec) noexcept
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
        std::wstring cmdline = detail::escape_win_arg(opts.command);
        for(const auto &arg : opts.args)
        {
            cmdline += L" ";
            cmdline += detail::escape_win_arg(arg);
        }

        STARTUPINFOW        si = {sizeof(si)};
        PROCESS_INFORMATION pi = {};
        DWORD creation_flags   = opts.detached ? DETACHED_PROCESS : 0;

        std::wstring wcwd = detail::utf8_to_utf16(opts.working_directory);
        LPCWSTR      pcwd = wcwd.empty() ? nullptr : wcwd.c_str();

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
        if(::pipe(err_pipe) < 0)
        {
            ec = std::error_code(errno, std::generic_category());
            return false;
        }

        ::fcntl(err_pipe[0], F_SETFD, FD_CLOEXEC);
        ::fcntl(err_pipe[1], F_SETFD, FD_CLOEXEC);

        pid_t pid = fork();
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
        ssize_t n = ::read(err_pipe[0], &child_errno, sizeof(child_errno));
        ::close(err_pipe[0]);

        if(n > 0)
        {
            ec         = std::error_code(child_errno, std::generic_category());
            int status = 0;
            ::waitpid(pid, &status, 0);
            return false;
        }

        _pid    = pid;
        _handle = pid;
        return true;
#endif
    }

    bool start(options opts) noexcept
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
        _detached = true;
        release_handle();
        _pid = 0;
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

    std::optional<exit_status> wait() noexcept
    {
        if(_exit_status.has_value())
            return _exit_status;

        if(!is_valid() || _detached)
            return std::nullopt;

#if defined(_WIN32)
        if(_handle == invalid_handle)
            return std::nullopt;

        WaitForSingleObject(_handle, INFINITE);
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
        return std::nullopt;
#else
        int   status = 0;
        pid_t res    = ::waitpid(_pid, &status, 0);

        _pid    = 0;
        _handle = invalid_handle;

        if(res > 0)
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
            return _exit_status;
        }
        return std::nullopt;
#endif
    }

    bool is_running() noexcept
    {
        if(!is_valid())
            return false;

        if(_exit_status.has_value())
            return false;

#if defined(_WIN32)
        if(_handle == invalid_handle)
            return false;
        DWORD exit_code = 0;
        if(GetExitCodeProcess(_handle, &exit_code))
        {
            if(exit_code == STILL_ACTIVE)
                return true;

            release_handle();
            _pid = 0;

            exit_status st;
            st.exited_normally = true;
            st.exit_code       = static_cast<int>(exit_code);
            _exit_status       = st;

            return false;
        }
        return false;
#else
        int   status = 0;
        pid_t res    = ::waitpid(_pid, &status, WNOHANG);

        if(res == 0)
        {
            return true;
        } else if(res == _pid)
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
        } else if(res == -1 && errno == ECHILD)
        {
            bool alive = (::kill(_pid, 0) == 0);
            if(!alive)
            {
                _pid    = 0;
                _handle = invalid_handle;
            }
            return alive;
        }

        return false;
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

inline process
spawn(const std::string              &executable,
      const std::vector<std::string> &args,
      std::error_code                &ec,
      const std::string              &work_dir = {},
      process_policy policy = process_policy::detach_on_destroy) noexcept
{
    process::options opts;
    opts.command           = executable;
    opts.args              = args;
    opts.working_directory = work_dir;
    opts.detached          = false;
    opts.policy            = policy;

    process p;
    p.start(opts, ec);
    return p;
}

inline process spawn(const std::string              &executable,
                     const std::vector<std::string> &args     = {},
                     const std::string              &work_dir = {},
                     process_policy policy = process_policy::detach_on_destroy)
{
    std::error_code ec;
    process         p = spawn(executable, args, ec, work_dir, policy);
    if(ec)
    {
        throw std::system_error(ec, "Failed to spawn process: " + executable);
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
    process p;
    return p.start(opts, ec);
#else
    if(executable.empty())
    {
        ec = std::make_error_code(std::errc::invalid_argument);
        return false;
    }

    int err_pipe[2];
    if(::pipe(err_pipe) < 0)
    {
        ec = std::error_code(errno, std::generic_category());
        return false;
    }

    ::fcntl(err_pipe[0], F_SETFD, FD_CLOEXEC);
    ::fcntl(err_pipe[1], F_SETFD, FD_CLOEXEC);

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
    {
        ::_exit(EXIT_SUCCESS);
    }

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
    {
        c_args.push_back(s.data());
    }
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

inline bool daemonize(const daemon_options &opts, std::error_code &ec) noexcept
{
    ec.clear();
#if defined(_WIN32)
    (void) opts;
    return true;
#else
    ::umask(opts.umask_value);

    pid_t pid = ::fork();
    if(pid < 0)
    {
        ec = std::error_code(errno, std::generic_category());
        return false;
    }
    if(pid > 0)
    {
        ::_exit(EXIT_SUCCESS);
    }

    if(::setsid() < 0)
    {
        ec = std::error_code(errno, std::generic_category());
        ::_exit(EXIT_FAILURE);
    }

    ::signal(SIGHUP, SIG_IGN);

    pid = ::fork();
    if(pid < 0)
    {
        ec = std::error_code(errno, std::generic_category());
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
            ec = std::error_code(errno, std::generic_category());
            return false;
        }
    }

    if(!opts.group.empty())
    {
        struct group *gr = ::getgrnam(opts.group.c_str());
        if(gr && ::setgid(gr->gr_gid) < 0)
        {
            ec = std::error_code(errno, std::generic_category());
            return false;
        }
    }

    if(!opts.user.empty())
    {
        struct passwd *pw = ::getpwnam(opts.user.c_str());
        if(pw && ::setuid(pw->pw_uid) < 0)
        {
            ec = std::error_code(errno, std::generic_category());
            return false;
        }
    }

    if(opts.auto_close_fds)
    {
        detail::close_all_fds_above(3);
    }

    if(opts.redirect_stdio)
    {
        int dev_null = ::open("/dev/null", O_RDWR);
        if(dev_null != -1)
        {
            ::dup2(dev_null, STDIN_FILENO);
            ::dup2(dev_null, STDOUT_FILENO);
            ::dup2(dev_null, STDERR_FILENO);
            if(dev_null > 2)
                ::close(dev_null);
        }
    }

    if(!opts.pid_file.empty())
    {
        int pid_fd = ::open(opts.pid_file.c_str(), O_RDWR | O_CREAT, 0644);
        if(pid_fd < 0)
        {
            ec = std::error_code(errno, std::generic_category());
            return false;
        }
        std::string           pid_str = std::to_string(::getpid()) + "\n";
        [[maybe_unused]] auto w =
            ::write(pid_fd, pid_str.c_str(), pid_str.size());
        ::close(pid_fd);
    }

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
    list_match_cb match = [](const process_info &) { return true; }) noexcept
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
        if(filename.empty()
           || !std::all_of(filename.begin(), filename.end(), ::isdigit))
            continue;

        pid_t pid           = 0;
        auto [ptr, conv_ec] = std::from_chars(filename.data(),
                                              filename.data() + filename.size(),
                                              pid);
        if(conv_ec != std::errc{})
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