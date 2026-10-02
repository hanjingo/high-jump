#ifndef NET_HPP
#define NET_HPP

#ifdef HJ_ENABLE_GRPC
#include <hj/net/grpc.hpp>
#endif

#if defined(HJ_ENABLE_HTTP) || defined(HJ_ENABLE_HTTPS)
#include <hj/net/http.hpp>
#endif

#include <hj/net/tcp.hpp>

#include <hj/net/udp.hpp>

#ifdef HJ_ENABLE_ZMQ
#include <hj/net/zmq.hpp>
#endif

#endif