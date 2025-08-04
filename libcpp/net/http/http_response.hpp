#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <httplib.h>

#if defined(_WIN32)
#include <Windows.h>
#endif

namespace libcpp {

using http_response = httplib::Response;

}

#endif