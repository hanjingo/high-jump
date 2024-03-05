#ifndef ENV_HPP
#define ENV_HPP

#include <stdlib.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#define __WINDOWS__
#endif

namespace libcpp
{

void setenv(const char* key, const char* value, bool override = true)
{
    setenv(key, value, override);
}

char* getenv(const char* key)
{
    return std::getenv(key);
}

}

#endif