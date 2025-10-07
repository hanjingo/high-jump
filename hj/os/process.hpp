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

#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <functional>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#if defined(_WIN32)
#include <windows.h>
#include <tlhelp32.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#endif

namespace hj
{
namespace process
{

#if defined(_WIN32)
using pid_t = DWORD;
#else
using pid_t = pid_t;
#endif

using exit_code_t   = int;
using error_code_t  = int;
using child_t       = pid_t;
using path_t        = std::string;
using list_match_cb = std::function<bool(const std::vector<std::string> &)>;

inline pid_t getpid()
{
#if defined(_WIN32)
    return GetCurrentProcessId();
#else
    return ::getpid();
#endif
}

inline pid_t getppid()
{
#if defined(_WIN32)
    DWORD  ppid  = 0;
    DWORD  pid   = GetCurrentProcessId();
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hSnap == INVALID_HANDLE_VALUE)
        return ppid;

    PROCESSENTRY32 pe = {sizeof(pe)};
    if(Process32First(hSnap, &pe))
    {
        do
        {
            if(pe.th32ProcessID == pid)
            {
                ppid = pe.th32ParentProcessID;
                break;
            }
        } while(Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
    return ppid;
#else
    return ::getppid();
#endif
}

inline int system(const std::string &cmd)
{
    return std::system(cmd.c_str());
}

inline child_t child(const std::string &cmd)
{
#if defined(_WIN32)
    STARTUPINFOA        si  = {sizeof(si)};
    PROCESS_INFORMATION pi  = {};
    BOOL                ret = CreateProcessA(nullptr,
                              const_cast<char *>(cmd.c_str()),
                              nullptr,
                              nullptr,
                              FALSE,
                              0,
                              nullptr,
                              nullptr,
                              &si,
                              &pi);
    if(ret)
    {
        CloseHandle(pi.hThread);
        return pi.dwProcessId;
    }
    return 0;

#else
    pid_t pid = fork();
    if(pid == 0)
    {
        execl("/bin/sh", "sh", "-c", cmd.c_str(), (char *) nullptr);
        std::exit(127);
    }
    return pid;

#endif
}

inline void spawn(const std::string &cmd)
{
    child(cmd);
}

inline void daemon(const std::string &cmd)
{
#if defined(_WIN32)
    STARTUPINFOA        si  = {sizeof(si)};
    PROCESS_INFORMATION pi  = {};
    BOOL                ret = CreateProcessA(nullptr,
                              const_cast<char *>(cmd.c_str()),
                              nullptr,
                              nullptr,
                              FALSE,
                              DETACHED_PROCESS,
                              nullptr,
                              nullptr,
                              &si,
                              &pi);
    if(ret)
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

#else
    pid_t pid = fork();
    if(pid < 0)
        return;

    if(pid > 0)
        return;

    setsid();
    pid = fork();
    if(pid < 0)
        exit(EXIT_FAILURE);

    if(pid > 0)
        exit(EXIT_SUCCESS);

    chdir("/");
    for(int fd = 0; fd < 1024; ++fd)
        close(fd);

    execl("/bin/sh", "sh", "-c", cmd.c_str(), (char *) nullptr);
    exit(EXIT_SUCCESS);

#endif
}

inline void list(
    std::vector<pid_t> &result,
    list_match_cb match = [](const std::vector<std::string> &) { return true; })
{
#if defined(_WIN32)
    FILE *fp = _popen("tasklist /FO CSV /NH", "r");
    if(!fp)
        return;

    char buf[1024];
    while(fgets(buf, sizeof(buf), fp))
    {
        std::string              line(buf);
        std::vector<std::string> vec;
        size_t                   pos = 0;
        while((pos = line.find(',')) != std::string::npos)
        {
            vec.push_back(line.substr(0, pos));
            line.erase(0, pos + 1);
        }
        vec.push_back(line);

        for(auto &v : vec)
        {
            v.erase(std::remove(v.begin(), v.end(), '\"'), v.end());
            v.erase(
                std::remove_if(v.begin(),
                               v.end(),
                               [](unsigned char c) { return std::isspace(c); }),
                v.end());
        }

        if(vec.size() < 2 || !match(vec))
            continue;

        try
        {
            if(!vec[1].empty()
               && std::all_of(vec[1].begin(), vec[1].end(), ::isdigit))
                result.push_back(static_cast<pid_t>(std::stoi(vec[1])));
        }
        catch(...)
        {
        }
    }
    _pclose(fp);

#else
    FILE *fp = popen("ps -eo comm,pid", "r");
    if(!fp)
        return;
    char buf[1024];
    bool is_first_line = true;
    while(fgets(buf, sizeof(buf), fp))
    {
        if(is_first_line)
        {
            is_first_line = false;
            continue;
        }

        std::string              line(buf);
        std::vector<std::string> vec;
        size_t                   pos = line.find_last_of(' ');
        if(pos != std::string::npos)
        {
            vec.push_back(line.substr(0, pos));
            vec.push_back(line.substr(pos + 1));
        }

        for(auto &v : vec)
            v.erase(std::remove_if(v.begin(), v.end(), ::isspace), v.end());

        if(vec.size() < 2 || !match(vec))
            continue;

        try
        {
            if(!vec[1].empty()
               && std::all_of(vec[1].begin(), vec[1].end(), ::isdigit))
                result.push_back(static_cast<pid_t>(std::stoi(vec[1])));
        }
        catch(...)
        {
        }
    }
    pclose(fp);
#endif
}

inline void kill(const pid_t pid)
{
#if defined(_WIN32)
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if(hProcess)
    {
        TerminateProcess(hProcess, 0);
        CloseHandle(hProcess);
    }

#else
    ::kill(pid, SIGKILL);

#endif
}

} // namespace process
} // namespace hj

#endif