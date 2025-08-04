#ifndef KERNEL_H
#define KERNEL_H

#include <cstdint>
#include <string>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__APPLE__)
#include <unistd.h>

#include <sys/sysctl.h>
#include <sys/utsname.h>
#elif defined(__linux__)
#include <unistd.h>

#include <sys/utsname.h>
#endif

namespace libcpp {

class kernel
{
  public:
    // Get kernel name (e.g., "Linux", "Darwin", "Windows")
    static std::string name()
    {
#if defined(_WIN32) || defined(_WIN64)
        return "Windows";
#elif defined(__APPLE__)
        return "Darwin";
#elif defined(__linux__)
        return "Linux";
#else
        return "Unknown";
#endif
    }

    // Get kernel version string
    static std::string version()
    {
#if defined(_WIN32) || defined(_WIN64)
        OSVERSIONINFOEX info = { 0 };
        info.dwOSVersionInfoSize = sizeof(info);
        if (GetVersionEx((OSVERSIONINFO*)&info))
        {
            return std::to_string(info.dwMajorVersion) + "." +
                   std::to_string(info.dwMinorVersion) + "." +
                   std::to_string(info.dwBuildNumber);
        }
        return "Unknown";
#elif defined(__APPLE__) || defined(__linux__)
        struct utsname buf;
        if (uname(&buf) == 0)
        {
            return std::string(buf.release);
        }
        return "Unknown";
#else
        return "Unknown";
#endif
    }

    // Get kernel architecture (e.g., "x86_64", "arm64")
    static std::string arch()
    {
#if defined(_WIN32) || defined(_WIN64)
        SYSTEM_INFO si;
        GetNativeSystemInfo(&si);
        switch (si.wProcessorArchitecture)
        {
            case PROCESSOR_ARCHITECTURE_AMD64:
                return "x86_64";
            case PROCESSOR_ARCHITECTURE_INTEL:
                return "x86";
            case PROCESSOR_ARCHITECTURE_ARM:
                return "arm";
            case PROCESSOR_ARCHITECTURE_ARM64:
                return "arm64";
            default:
                return "unknown";
        }
#elif defined(__APPLE__) || defined(__linux__)
        struct utsname buf;
        if (uname(&buf) == 0)
        {
            return std::string(buf.machine);
        }
        return "Unknown";
#else
        return "Unknown";
#endif
    }

    // Get number of logical CPUs
    static int cpu_count()
    {
#if defined(_WIN32) || defined(_WIN64)
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        return static_cast<int>(si.dwNumberOfProcessors);
#elif defined(__APPLE__)
        int nm[2] = { CTL_HW, HW_NCPU };
        int count = 0;
        size_t len = sizeof(count);
        if (sysctl(nm, 2, &count, &len, NULL, 0) == 0)
            return count;
        return 1;
#elif defined(__linux__)
        long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
        return nprocs > 0 ? static_cast<int>(nprocs) : 1;
#else
        return 1;
#endif
    }

    // Get system uptime in seconds
    static uint64_t uptime()
    {
#if defined(_WIN32) || defined(_WIN64)
        return GetTickCount64() / 1000;
#elif defined(__APPLE__)
        struct timeval boottime;
        size_t len = sizeof(boottime);
        int mib[2] = { CTL_KERN, KERN_BOOTTIME };
        time_t now = time(NULL);
        if (sysctl(mib, 2, &boottime, &len, NULL, 0) == 0 &&
            boottime.tv_sec != 0)
        {
            return static_cast<uint64_t>(now - boottime.tv_sec);
        }
        return 0;
#elif defined(__linux__)
        struct sysinfo info;
        if (sysinfo(&info) == 0)
        {
            return static_cast<uint64_t>(info.uptime);
        }
        return 0;
#else
        return 0;
#endif
    }
};

}  // namespace libcpp

#endif  // KERNEL_H