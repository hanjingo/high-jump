#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include <httplib.h>

#if defined(_WIN32)
#include <Windows.h>
#endif

namespace hj
{

using http_client = httplib::Client;

}

#endif