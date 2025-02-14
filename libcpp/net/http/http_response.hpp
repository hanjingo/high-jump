#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <httplib.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#include <Windows.h>
#endif

namespace libcpp
{

using http_response = httplib::Response;

}

#endif