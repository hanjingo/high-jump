#ifndef DURATION_HPP
#define DURATION_HPP

#include <chrono>

namespace libcpp
{
    static std::chrono::time_point<std::chrono::high_resolution_clock> tm_start = std::chrono::high_resolution_clock::now();
}

#define TIME_START() \
    libcpp::tm_start = std::chrono::high_resolution_clock::now();

#define TIME_PASSED() \
    (std::chrono::high_resolution_clock::now() - libcpp::tm_start)

#define TIME_PASSED_NS() \
    std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - libcpp::tm_start).count()

#define TIME_PASSED_MS() \
    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - libcpp::tm_start).count()

#define TIME_PASSED_SEC() \
    std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - libcpp::tm_start).count()

#endif