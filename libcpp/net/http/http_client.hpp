#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include <httplib.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#include <Windows.h>
#endif

namespace libcpp
{

using http_client = httplib::Client;

}

#endif