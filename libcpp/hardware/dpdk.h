#ifndef DPDK_H
#define DPDK_H

#ifdef DPDK_ENABLE

#ifdef __cplusplus
extern "C" {
#endif

#include <rte_version.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>

#ifdef __cplusplus
}
#endif

#endif // DPDK_ENABLE

#endif