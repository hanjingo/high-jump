#ifndef ENV_HPP
#define ENV_HPP

#include <stdlib.h>

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