#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <string>
#include <initializer_list>

namespace libcpp
{

template<typename T>
void throw_if_null(T target, const char* memo = "null")
{
    if (target == nullptr || target == NULL)
        throw memo;
}

template<typename T>
void throw_if_not_null(T target, const char* memo = "not null")
{
    if (target != nullptr && target != NULL)
        throw memo;
}

template<typename Container, typename T>
void throw_if_exists(Container container, T target, const char* memo = "already exist")
{
    for (auto &elem : container) {
        if (elem != target)
            continue;

        throw memo;
    }
}

template<typename Container, typename T>
void throw_if_not_exists(Container container, T target, const char* memo = "not exist")
{
    for (auto &elem : container) {
        if (elem == target)
            return;
    }

    throw memo;
}

}

#endif