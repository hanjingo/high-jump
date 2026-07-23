#ifndef NET_HPP
#define NET_HPP

#ifdef HJ_ENABLE_GRPC
#include <hj/net/grpc.hpp>
#endif

#ifdef HJ_ENABLE_HTTP
#include <hj/net/http.hpp>
#endif

#include <hj/net/tcp.hpp>

#include <hj/net/udp.hpp>

#include <hj/net/winsock.h>

#ifdef HJ_ENABLE_ZMQ
#include <hj/net/zmq.hpp>
#endif

#endif