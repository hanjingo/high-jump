#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#if defined(OPENSSL_ENABLE)
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include <httplib.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#include <Windows.h>
#endif

namespace libcpp
{

using http_server = httplib::Server;

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
using http_ssl_server = httplib::SSLServer;
#endif

}

#endif