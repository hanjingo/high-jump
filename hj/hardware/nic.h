/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef NIC_H
#define NIC_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HJ_NIC_API
#if defined(HJ_NIC_STATIC)
#define HJ_NIC_API static inline
#else
#define HJ_NIC_API extern
#endif
#endif

/* ----------------------------- Constants and Limits ------------------------------- */
#define HJ_NIC_MAX_INTERFACES 32
#define HJ_NIC_MAX_NAME_LENGTH 256
#define HJ_NIC_MAX_DESCRIPTION_LENGTH 512
#define HJ_NIC_MAC_ADDRESS_LENGTH 6
#define HJ_NIC_IP_ADDRESS_LENGTH 16
#define HJ_NIC_IPV6_ADDRESS_LENGTH 40

/* ----------------------------- Error Codes ------------------------------------ */
typedef enum
{
    HJ_NIC_SUCCESS                 = 0,
    HJ_NIC_ERR_INVALID_PARAMETER   = -1,
    HJ_NIC_ERR_ACCESS_DENIED       = -2,
    HJ_NIC_ERR_NOT_FOUND           = -3,
    HJ_NIC_ERR_INSUFFICIENT_BUFFER = -4,
    HJ_NIC_ERR_SYSTEM_ERROR        = -5,
    HJ_NIC_ERR_NOT_SUPPORTED       = -6,
    HJ_NIC_ERR_TIMEOUT             = -7,
    HJ_NIC_ERR_INVALID_STATE       = -8,
    HJ_NIC_ERR_INSUFFICIENT_MEMORY = -9
} hj_nic_err_t;

/* ------------------------ Interface Types -------------- */
typedef enum
{
    HJ_NIC_TYPE_UNKNOWN  = 0,
    HJ_NIC_TYPE_ETHERNET = 1,
    HJ_NIC_TYPE_WIRELESS = 2,
    HJ_NIC_TYPE_LOOPBACK = 3,
    HJ_NIC_TYPE_PPP      = 4,
    HJ_NIC_TYPE_TUNNEL   = 5,
    HJ_NIC_TYPE_VIRTUAL  = 6
} hj_nic_type_t;

/* ----------------------------- Interface Status ------------------------------------ */
typedef enum
{
    HJ_NIC_STATUS_UNKNOWN          = 0,
    HJ_NIC_STATUS_UP               = 1,
    HJ_NIC_STATUS_DOWN             = 2,
    HJ_NIC_STATUS_TESTING          = 3,
    HJ_NIC_STATUS_DORMANT          = 4,
    HJ_NIC_STATUS_NOT_PRESENT      = 5,
    HJ_NIC_STATUS_LOWER_LAYER_DOWN = 6
} hj_nic_status_t;

/* ----------------------------- Duplex Mode ------------------------------------ */
typedef enum
{
    HJ_NIC_DUPLEX_UNKNOWN = 0,
    HJ_NIC_DUPLEX_HALF    = 1,
    HJ_NIC_DUPLEX_FULL    = 2
} hj_nic_duplex_t;

/* ----------------------------- Structures ------------------------------------ */

typedef struct
{
    uint8_t bytes[HJ_NIC_MAC_ADDRESS_LENGTH];
} hj_nic_mac_addr_t;

typedef struct
{
    union
    {
        uint8_t  bytes[4];
        uint32_t addr;
    } ipv4;
    char str[HJ_NIC_IP_ADDRESS_LENGTH];
} hj_nic_ip_addr_t;

typedef struct
{
    uint8_t bytes[16];
    char    str[HJ_NIC_IPV6_ADDRESS_LENGTH];
} hj_nic_ipv6_addr_t;

/**
 * @brief Network Interface Information Structure
 * All fields are populated across supported platforms (Windows, Linux, macOS)
 * where underlying OS capability exists.
 */
typedef struct
{
    char name
        [HJ_NIC_MAX_NAME_LENGTH]; /* Interface name (e.g., GUID or "eth0", "wlan0") */
    char description[HJ_NIC_MAX_DESCRIPTION_LENGTH]; /* Interface description */
    char friendly_name
        [HJ_NIC_MAX_NAME_LENGTH];    /* Human-readable friendly name */
    hj_nic_mac_addr_t  mac_address;  /* MAC address */
    hj_nic_type_t      type;         /* Interface type */
    hj_nic_status_t    status;       /* Operational status */
    uint32_t           index;        /* Interface index */
    uint32_t           mtu;          /* Maximum Transmission Unit */
    uint64_t           speed;        /* Interface speed in bits per second */
    hj_nic_duplex_t    duplex;       /* Duplex mode */
    bool               dhcp_enabled; /* True if DHCP is enabled */
    bool               is_virtual;   /* True if interface is virtual */
    bool               is_wireless;  /* True if interface is wireless */
    hj_nic_ip_addr_t   ip_address;   /* Primary IPv4 address */
    hj_nic_ip_addr_t   subnet_mask;  /* Subnet mask for IPv4 */
    hj_nic_ip_addr_t   gateway;      /* Default gateway IPv4 */
    hj_nic_ipv6_addr_t ipv6_address; /* Primary IPv6 address */
    uint32_t           ipv6_prefix_length; /* IPv6 prefix length */
} hj_nic_info_t;

typedef struct
{
    uint64_t bytes_sent;       /* Total bytes sent */
    uint64_t bytes_received;   /* Total bytes received */
    uint64_t packets_sent;     /* Total packets sent */
    uint64_t packets_received; /* Total packets received */
    uint64_t errors_sent;      /* Send errors */
    uint64_t errors_received;  /* Receive errors */
    uint64_t drops_sent;       /* Sent packets dropped */
    uint64_t drops_received;   /* Received packets dropped */
} hj_nic_statistics_t;

HJ_NIC_API hj_nic_err_t hj_nic_init(void);
HJ_NIC_API void         hj_nic_cleanup(void);
HJ_NIC_API hj_nic_err_t hj_nic_get_interface_count(uint32_t *count);
HJ_NIC_API hj_nic_err_t hj_nic_enumerate_interfaces(hj_nic_info_t *interfaces,
                                                    uint32_t  max_interfaces,
                                                    uint32_t *actual_count);
HJ_NIC_API hj_nic_err_t hj_nic_get_interface_info(const char    *interface_name,
                                                  hj_nic_info_t *info);
HJ_NIC_API hj_nic_err_t hj_nic_get_statistics(const char *interface_name,
                                              hj_nic_statistics_t *stats);
HJ_NIC_API hj_nic_err_t hj_nic_enable_interface(const char *interface_name);
HJ_NIC_API hj_nic_err_t hj_nic_disable_interface(const char *interface_name);

#ifdef __cplusplus
}
#endif

#endif // NIC_H

/* ----------------------------- API Implement ------------------------- */
#if (defined(HJ_NIC_IMPL) || defined(HJ_NIC_STATIC))                           \
    && !defined(HJ_NIC_IMPL_DONE)
#define HJ_NIC_IMPL_DONE

#if defined(_WIN32) || defined(_WIN64)
#define HJ_NIC_PLATFORM_WINDOWS 1
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <netioapi.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#elif defined(__linux__)
#define HJ_NIC_PLATFORM_LINUX 1
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include <unistd.h>

#elif defined(__APPLE__)
#define HJ_NIC_PLATFORM_MACOS 1
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/route.h>
#include <ifaddrs.h>
#include <sys/sysctl.h>

#else
#define HJ_NIC_PLATFORM_UNKNOWN 1

#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------- Internal Safe Utility Helpers ---------------- */
static inline void
hj_nic_safe_strcpy(char *dest, size_t dest_size, const char *src)
{
    if(!dest || dest_size == 0)
        return;
    if(!src)
    {
        dest[0] = '\0';
        return;
    }
    snprintf(dest, dest_size, "%s", src);
}

#if defined(HJ_NIC_PLATFORM_WINDOWS)
static inline void
hj_nic_wchar_to_utf8(char *dest, size_t dest_size, const wchar_t *src)
{
    if(!dest || dest_size == 0)
        return;
    if(!src)
    {
        dest[0] = '\0';
        return;
    }
    WideCharToMultiByte(CP_UTF8, 0, src, -1, dest, (int) dest_size, NULL, NULL);
    dest[dest_size - 1] = '\0';
}
#endif

static inline hj_nic_err_t hj_nic_init(void)
{
#if defined(HJ_NIC_PLATFORM_WINDOWS)
    WSADATA wsaData;
    int     result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    return (result == 0) ? HJ_NIC_SUCCESS : HJ_NIC_ERR_SYSTEM_ERROR;
#else
    return HJ_NIC_SUCCESS;
#endif
}

static inline void hj_nic_cleanup(void)
{
#if defined(HJ_NIC_PLATFORM_WINDOWS)
    WSACleanup();
#endif
}

static inline hj_nic_err_t hj_nic_get_interface_count(uint32_t *count)
{
    if(!count)
        return HJ_NIC_ERR_INVALID_PARAMETER;

#if defined(HJ_NIC_PLATFORM_WINDOWS)
    ULONG size  = 0;
    DWORD flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST
                  | GAA_FLAG_SKIP_DNS_SERVER;
    if(GetAdaptersAddresses(AF_UNSPEC, flags, NULL, NULL, &size)
       != ERROR_BUFFER_OVERFLOW)
    {
        *count = 0;
        return HJ_NIC_SUCCESS;
    }

    PIP_ADAPTER_ADDRESSES adapters = (PIP_ADAPTER_ADDRESSES) malloc(size);
    if(!adapters)
        return HJ_NIC_ERR_INSUFFICIENT_MEMORY;

    if(GetAdaptersAddresses(AF_UNSPEC, flags, NULL, adapters, &size)
       != ERROR_SUCCESS)
    {
        free(adapters);
        return HJ_NIC_ERR_SYSTEM_ERROR;
    }

    uint32_t              interface_count = 0;
    PIP_ADAPTER_ADDRESSES adapter         = adapters;
    while(adapter)
    {
        interface_count++;
        adapter = adapter->Next;
    }

    free(adapters);
    *count = interface_count;
    return HJ_NIC_SUCCESS;

#elif defined(HJ_NIC_PLATFORM_LINUX) || defined(HJ_NIC_PLATFORM_MACOS)
    struct ifaddrs *ifaddrs_ptr;
    if(getifaddrs(&ifaddrs_ptr) == -1)
        return HJ_NIC_ERR_SYSTEM_ERROR;

    uint32_t        interface_count = 0;
    struct ifaddrs *ifa             = ifaddrs_ptr;
    while(ifa)
    {
#if defined(HJ_NIC_PLATFORM_LINUX)
        if(ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_PACKET)
#else
        if(ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_LINK)
#endif
            interface_count++;

        ifa = ifa->ifa_next;
    }

    freeifaddrs(ifaddrs_ptr);
    *count = interface_count;
    return HJ_NIC_SUCCESS;
#else
    *count = 0;
    return HJ_NIC_ERR_NOT_SUPPORTED;
#endif
}

/* Helper function to find or append an interface entry by name in POSIX enumeration */
static inline hj_nic_info_t *
hj_nic_find_or_add_interface(hj_nic_info_t *interfaces,
                             uint32_t       max_interfaces,
                             uint32_t      *count,
                             const char    *name,
                             bool          *truncated)
{
    for(uint32_t i = 0; i < *count; i++)
    {
        if(strcmp(interfaces[i].name, name) == 0)
        {
            return &interfaces[i];
        }
    }
    if(*count >= max_interfaces)
    {
        if(truncated)
            *truncated = true;
        return NULL;
    }

    hj_nic_info_t *nic = &interfaces[*count];
    memset(nic, 0, sizeof(hj_nic_info_t));
    hj_nic_safe_strcpy(nic->name, sizeof(nic->name), name);
    nic->index = if_nametoindex(name);
    (*count)++;
    return nic;
}

static inline hj_nic_err_t hj_nic_enumerate_interfaces(
    hj_nic_info_t *interfaces, uint32_t max_interfaces, uint32_t *actual_count)
{
    if(!actual_count)
        return HJ_NIC_ERR_INVALID_PARAMETER;
    if(!interfaces && max_interfaces > 0)
        return HJ_NIC_ERR_INVALID_PARAMETER;

    uint32_t     total_count = 0;
    hj_nic_err_t count_err   = hj_nic_get_interface_count(&total_count);
    if(count_err != HJ_NIC_SUCCESS)
        return count_err;

    if(max_interfaces < total_count)
    {
        *actual_count = total_count;
        return HJ_NIC_ERR_INSUFFICIENT_BUFFER;
    }

    *actual_count = 0;

#if defined(HJ_NIC_PLATFORM_WINDOWS)
    ULONG size  = 0;
    DWORD flags = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_ANYCAST
                  | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
    GetAdaptersAddresses(AF_UNSPEC, flags, NULL, NULL, &size);
    if(size == 0)
        return HJ_NIC_SUCCESS;

    PIP_ADAPTER_ADDRESSES adapters = (PIP_ADAPTER_ADDRESSES) malloc(size);
    if(!adapters)
        return HJ_NIC_ERR_INSUFFICIENT_MEMORY;

    if(GetAdaptersAddresses(AF_UNSPEC, flags, NULL, adapters, &size)
       != ERROR_SUCCESS)
    {
        free(adapters);
        return HJ_NIC_ERR_SYSTEM_ERROR;
    }

    PIP_ADAPTER_ADDRESSES adapter = adapters;
    uint32_t              count   = 0;
    while(adapter)
    {
        if(count >= max_interfaces)
        {
            adapter   = adapter->Next;
            continue;
        }

        hj_nic_info_t *nic = &interfaces[count];
        memset(nic, 0, sizeof(hj_nic_info_t));

        /* Adapter Name, Description & Friendly Name */
        hj_nic_safe_strcpy(nic->name, sizeof(nic->name), adapter->AdapterName);
        hj_nic_wchar_to_utf8(nic->description,
                             sizeof(nic->description),
                             adapter->Description);
        hj_nic_wchar_to_utf8(nic->friendly_name,
                             sizeof(nic->friendly_name),
                             adapter->FriendlyName);

        /* MAC Address */
        if(adapter->PhysicalAddressLength == HJ_NIC_MAC_ADDRESS_LENGTH)
        {
            memcpy(nic->mac_address.bytes,
                   adapter->PhysicalAddress,
                   HJ_NIC_MAC_ADDRESS_LENGTH);
        }

        nic->index = adapter->IfIndex;
        nic->mtu   = adapter->Mtu;
        nic->speed = adapter->TransmitLinkSpeed;

        /* Operational Status */
        switch(adapter->OperStatus)
        {
            case IfOperStatusUp:
                nic->status = HJ_NIC_STATUS_UP;
                break;
            case IfOperStatusDown:
                nic->status = HJ_NIC_STATUS_DOWN;
                break;
            case IfOperStatusTesting:
                nic->status = HJ_NIC_STATUS_TESTING;
                break;
            case IfOperStatusDormant:
                nic->status = HJ_NIC_STATUS_DORMANT;
                break;
            case IfOperStatusNotPresent:
                nic->status = HJ_NIC_STATUS_NOT_PRESENT;
                break;
            case IfOperStatusLowerLayerDown:
                nic->status = HJ_NIC_STATUS_LOWER_LAYER_DOWN;
                break;
            default:
                nic->status = HJ_NIC_STATUS_UNKNOWN;
                break;
        }

        /* Interface Type */
        switch(adapter->IfType)
        {
            case IF_TYPE_ETHERNET_CSMACD:
                nic->type = HJ_NIC_TYPE_ETHERNET;
                break;
            case IF_TYPE_IEEE80211:
                nic->type        = HJ_NIC_TYPE_WIRELESS;
                nic->is_wireless = true;
                break;
            case IF_TYPE_PPP:
                nic->type = HJ_NIC_TYPE_PPP;
                break;
            case IF_TYPE_SOFTWARE_LOOPBACK:
                nic->type = HJ_NIC_TYPE_LOOPBACK;
                break;
            case IF_TYPE_TUNNEL:
            case IF_TYPE_ISO88025_TOKENRING:
            case IF_TYPE_IEEE1394:
                nic->type       = HJ_NIC_TYPE_VIRTUAL;
                nic->is_virtual = true;
                break;
            default:
                nic->type = HJ_NIC_TYPE_UNKNOWN;
                break;
        }

        nic->dhcp_enabled = (adapter->Dhcpv4Enabled != 0);

        /* Extract Unicast IPv4 & IPv6 Addresses */
        PIP_ADAPTER_UNICAST_ADDRESS unicast = adapter->FirstUnicastAddress;
        while(unicast)
        {
            LPSOCKADDR sa = unicast->Address.lpSockaddr;
            if(sa)
            {
                if(sa->sa_family == AF_INET)
                {
                    struct sockaddr_in *sin = (struct sockaddr_in *) sa;
                    inet_ntop(AF_INET,
                              &sin->sin_addr,
                              nic->ip_address.str,
                              sizeof(nic->ip_address.str));
                    nic->ip_address.ipv4.addr = sin->sin_addr.s_addr;

                    /* Convert Prefix Length to IPv4 Subnet Mask */
                    uint8_t prefix = unicast->OnLinkPrefixLength;
                    if(prefix <= 32)
                    {
                        uint32_t mask =
                            prefix ? htonl(~((1U << (32 - prefix)) - 1)) : 0;
                        nic->subnet_mask.ipv4.addr = mask;
                        struct in_addr mask_addr;
                        mask_addr.s_addr = mask;
                        inet_ntop(AF_INET,
                                  &mask_addr,
                                  nic->subnet_mask.str,
                                  sizeof(nic->subnet_mask.str));
                    }
                } else if(sa->sa_family == AF_INET6)
                {
                    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;
                    inet_ntop(AF_INET6,
                              &sin6->sin6_addr,
                              nic->ipv6_address.str,
                              sizeof(nic->ipv6_address.str));
                    memcpy(nic->ipv6_address.bytes, &sin6->sin6_addr, 16);
                    nic->ipv6_prefix_length = unicast->OnLinkPrefixLength;
                }
            }
            unicast = unicast->Next;
        }

        /* Extract Gateway Address */
        PIP_ADAPTER_GATEWAY_ADDRESS gateway = adapter->FirstGatewayAddress;
        while(gateway)
        {
            LPSOCKADDR sa = gateway->Address.lpSockaddr;
            if(sa && sa->sa_family == AF_INET)
            {
                struct sockaddr_in *sin = (struct sockaddr_in *) sa;
                inet_ntop(AF_INET,
                          &sin->sin_addr,
                          nic->gateway.str,
                          sizeof(nic->gateway.str));
                nic->gateway.ipv4.addr = sin->sin_addr.s_addr;
                break;
            }
            gateway = gateway->Next;
        }

        count++;
        adapter = adapter->Next;
    }

    free(adapters);
    *actual_count = count;
    return HJ_NIC_SUCCESS;

#elif defined(HJ_NIC_PLATFORM_LINUX) || defined(HJ_NIC_PLATFORM_MACOS)
    struct ifaddrs *ifaddrs_ptr;
    if(getifaddrs(&ifaddrs_ptr) == -1)
        return HJ_NIC_ERR_SYSTEM_ERROR;

    uint32_t        count = 0;
    struct ifaddrs *ifa   = ifaddrs_ptr;
    while(ifa)
    {
        if(ifa->ifa_name && ifa->ifa_addr)
        {
            hj_nic_info_t *nic = hj_nic_find_or_add_interface(interfaces,
                                                              max_interfaces,
                                                              &count,
                                                              ifa->ifa_name,
                                                              &truncated);
            if(nic)
            {
                nic->status = (ifa->ifa_flags & IFF_UP) ? HJ_NIC_STATUS_UP
                                                        : HJ_NIC_STATUS_DOWN;

                if(nic->type == HJ_NIC_TYPE_UNKNOWN)
                {
                    if(strncmp(ifa->ifa_name, "lo", 2) == 0)
                    {
                        nic->type = HJ_NIC_TYPE_LOOPBACK;
                    } else if(strncmp(ifa->ifa_name, "eth", 3) == 0
                              || strncmp(ifa->ifa_name, "en", 2) == 0)
                    {
                        nic->type = HJ_NIC_TYPE_ETHERNET;
                    } else if(strncmp(ifa->ifa_name, "wlan", 4) == 0
                              || strncmp(ifa->ifa_name, "wifi", 4) == 0)
                    {
                        nic->type        = HJ_NIC_TYPE_WIRELESS;
                        nic->is_wireless = true;
                    }
                }

                /* Hardware Address Extraction */
#if defined(HJ_NIC_PLATFORM_LINUX)
                if(ifa->ifa_addr->sa_family == AF_PACKET)
                {
                    struct sockaddr_ll *s =
                        (struct sockaddr_ll *) ifa->ifa_addr;
                    if(s->sll_halen == HJ_NIC_MAC_ADDRESS_LENGTH)
                    {
                        memcpy(nic->mac_address.bytes,
                               s->sll_addr,
                               HJ_NIC_MAC_ADDRESS_LENGTH);
                    }
                }
#elif defined(HJ_NIC_PLATFORM_MACOS)
                if(ifa->ifa_addr->sa_family == AF_LINK)
                {
                    struct sockaddr_dl *sdl =
                        (struct sockaddr_dl *) ifa->ifa_addr;
                    if(sdl->sdl_type == IFT_ETHER
                       && sdl->sdl_alen == HJ_NIC_MAC_ADDRESS_LENGTH)
                    {
                        memcpy(nic->mac_address.bytes,
                               LLADDR(sdl),
                               HJ_NIC_MAC_ADDRESS_LENGTH);
                    }
                }
#endif

                /* IP Address Extraction */
                if(ifa->ifa_addr->sa_family == AF_INET)
                {
                    struct sockaddr_in *sin =
                        (struct sockaddr_in *) ifa->ifa_addr;
                    inet_ntop(AF_INET,
                              &sin->sin_addr,
                              nic->ip_address.str,
                              sizeof(nic->ip_address.str));
                    nic->ip_address.ipv4.addr = sin->sin_addr.s_addr;
                    if(ifa->ifa_netmask)
                    {
                        struct sockaddr_in *netmask =
                            (struct sockaddr_in *) ifa->ifa_netmask;
                        inet_ntop(AF_INET,
                                  &netmask->sin_addr,
                                  nic->subnet_mask.str,
                                  sizeof(nic->subnet_mask.str));
                        nic->subnet_mask.ipv4.addr = netmask->sin_addr.s_addr;
                    }
                } else if(ifa->ifa_addr->sa_family == AF_INET6)
                {
                    struct sockaddr_in6 *sin6 =
                        (struct sockaddr_in6 *) ifa->ifa_addr;
                    inet_ntop(AF_INET6,
                              &sin6->sin6_addr,
                              nic->ipv6_address.str,
                              sizeof(nic->ipv6_address.str));
                    memcpy(nic->ipv6_address.bytes, &sin6->sin6_addr, 16);
                }
            }
        }
        ifa = ifa->ifa_next;
    }

    freeifaddrs(ifaddrs_ptr);
    *actual_count = count;
    return HJ_NIC_SUCCESS;
#else
    return HJ_NIC_ERR_NOT_SUPPORTED;
#endif
}

static inline hj_nic_err_t hj_nic_get_interface_info(const char *interface_name,
                                                     hj_nic_info_t *info)
{
    if(!interface_name || !info)
        return HJ_NIC_ERR_INVALID_PARAMETER;

    uint32_t     total_count = 0;
    hj_nic_err_t count_err   = hj_nic_get_interface_count(&total_count);
    if(count_err != HJ_NIC_SUCCESS)
        return count_err;

    if(total_count == 0)
        return HJ_NIC_ERR_NOT_FOUND;

    hj_nic_info_t *interfaces =
        (hj_nic_info_t *) malloc(sizeof(hj_nic_info_t) * total_count);
    if(!interfaces)
        return HJ_NIC_ERR_INSUFFICIENT_MEMORY;

    uint32_t     actual_count = 0;
    hj_nic_err_t result =
        hj_nic_enumerate_interfaces(interfaces, total_count, &actual_count);
    if(result != HJ_NIC_SUCCESS && result != HJ_NIC_ERR_INSUFFICIENT_BUFFER)
    {
        free(interfaces);
        return result;
    }

    hj_nic_err_t ret = HJ_NIC_ERR_NOT_FOUND;
    for(uint32_t i = 0; i < actual_count; i++)
    {
        if(strcmp(interfaces[i].name, interface_name) == 0)
        {
            *info = interfaces[i];
            ret   = HJ_NIC_SUCCESS;
            break;
        }
    }

    free(interfaces);
    return ret;
}

static inline hj_nic_err_t hj_nic_get_statistics(const char *interface_name,
                                                 hj_nic_statistics_t *stats)
{
    if(!interface_name || !stats)
        return HJ_NIC_ERR_INVALID_PARAMETER;
    memset(stats, 0, sizeof(hj_nic_statistics_t));

#if defined(HJ_NIC_PLATFORM_WINDOWS)
    ULONG size = 0;
    GetIfTable(NULL, &size, FALSE);
    if(size == 0)
        return HJ_NIC_ERR_SYSTEM_ERROR;

    PMIB_IFTABLE ifTable = (PMIB_IFTABLE) malloc(size);
    if(!ifTable)
        return HJ_NIC_ERR_INSUFFICIENT_MEMORY;

    DWORD result = GetIfTable(ifTable, &size, FALSE);
    if(result != NO_ERROR)
    {
        free(ifTable);
        return HJ_NIC_ERR_SYSTEM_ERROR;
    }

    PIP_ADAPTER_ADDRESSES adapters     = NULL;
    ULONG                 adapter_size = 0;
    DWORD flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST
                  | GAA_FLAG_SKIP_DNS_SERVER;
    GetAdaptersAddresses(AF_UNSPEC, flags, NULL, NULL, &adapter_size);
    if(adapter_size > 0)
    {
        adapters = (PIP_ADAPTER_ADDRESSES) malloc(adapter_size);
        if(adapters
           && GetAdaptersAddresses(AF_UNSPEC,
                                   flags,
                                   NULL,
                                   adapters,
                                   &adapter_size)
                  != ERROR_SUCCESS)
        {
            free(adapters);
            adapters = NULL;
        }
    }

    BOOL found = FALSE;
    for(DWORD i = 0; i < ifTable->dwNumEntries; i++)
    {
        MIB_IFROW *row                              = &ifTable->table[i];
        char       name_buf[HJ_NIC_MAX_NAME_LENGTH] = {0};

        PIP_ADAPTER_ADDRESSES adapter = adapters;
        while(adapter)
        {
            if(adapter->IfIndex == row->dwIndex)
            {
                hj_nic_safe_strcpy(name_buf,
                                   sizeof(name_buf),
                                   adapter->AdapterName);
                break;
            }
            adapter = adapter->Next;
        }

        if(strcmp(name_buf, interface_name) == 0)
        {
            stats->bytes_sent     = row->dwOutOctets;
            stats->bytes_received = row->dwInOctets;
            stats->packets_sent   = row->dwOutUcastPkts + row->dwOutNUcastPkts;
            stats->packets_received = row->dwInUcastPkts + row->dwInNUcastPkts;
            stats->errors_sent      = row->dwOutErrors;
            stats->errors_received  = row->dwInErrors;
            stats->drops_sent       = row->dwOutDiscards;
            stats->drops_received   = row->dwInDiscards;
            found                   = TRUE;
            break;
        }
    }

    if(adapters)
        free(adapters);
    free(ifTable);
    return found ? HJ_NIC_SUCCESS : HJ_NIC_ERR_NOT_FOUND;

#elif defined(HJ_NIC_PLATFORM_LINUX)
    FILE *file = fopen("/proc/net/dev", "r");
    if(!file)
        return HJ_NIC_ERR_SYSTEM_ERROR;

    char line[512];
    bool found = false;
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    while(fgets(line, sizeof(line), file))
    {
        char          name[64];
        unsigned long rx_bytes, rx_packets, rx_errs, rx_drop;
        unsigned long tx_bytes, tx_packets, tx_errs, tx_drop;
        if(sscanf(line,
                  "%63[^:]: %lu %lu %lu %lu %*u %*u %*u %*u %lu %lu %lu %lu",
                  name,
                  &rx_bytes,
                  &rx_packets,
                  &rx_errs,
                  &rx_drop,
                  &tx_bytes,
                  &tx_packets,
                  &tx_errs,
                  &tx_drop)
           == 9)
        {
            char *trimmed_name = name;
            while(*trimmed_name == ' ')
                trimmed_name++;

            if(strcmp(trimmed_name, interface_name) != 0)
                continue;

            stats->bytes_received   = rx_bytes;
            stats->packets_received = rx_packets;
            stats->errors_received  = rx_errs;
            stats->drops_received   = rx_drop;
            stats->bytes_sent       = tx_bytes;
            stats->packets_sent     = tx_packets;
            stats->errors_sent      = tx_errs;
            stats->drops_sent       = tx_drop;
            found                   = true;
            break;
        }
    }

    fclose(file);
    return found ? HJ_NIC_SUCCESS : HJ_NIC_ERR_NOT_FOUND;

#elif defined(HJ_NIC_PLATFORM_MACOS)
    int    mib[6] = {CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST2, 0};
    size_t len    = 0;
    if(sysctl(mib, 6, NULL, &len, NULL, 0) < 0)
        return HJ_NIC_ERR_SYSTEM_ERROR;

    char *buf = (char *) malloc(len);
    if(!buf)
        return HJ_NIC_ERR_INSUFFICIENT_MEMORY;

    if(sysctl(mib, 6, buf, &len, NULL, 0) < 0)
    {
        free(buf);
        return HJ_NIC_ERR_SYSTEM_ERROR;
    }

    bool  found = false;
    char *lim   = buf + len;
    char *next  = buf;
    while(next < lim)
    {
        struct if_msghdr *ifm = (struct if_msghdr *) next;
        next += ifm->ifm_msglen;
        if(ifm->ifm_type == RTM_IFINFO2)
        {
            struct if_msghdr2  *if2m     = (struct if_msghdr2 *) ifm;
            struct sockaddr_dl *sdl      = (struct sockaddr_dl *) (if2m + 1);
            char                name[32] = {0};
            if(sdl->sdl_nlen > 0 && sdl->sdl_nlen < sizeof(name))
            {
                memcpy(name, sdl->sdl_data, sdl->sdl_nlen);
                if(strcmp(name, interface_name) == 0)
                {
                    stats->bytes_sent       = if2m->ifm_data.ifi_obytes;
                    stats->bytes_received   = if2m->ifm_data.ifi_ibytes;
                    stats->packets_sent     = if2m->ifm_data.ifi_opackets;
                    stats->packets_received = if2m->ifm_data.ifi_ipackets;
                    stats->errors_sent      = if2m->ifm_data.ifi_oerrors;
                    stats->errors_received  = if2m->ifm_data.ifi_ierrors;
                    stats->drops_sent       = if2m->ifm_data.ifi_collisions;
                    stats->drops_received   = if2m->ifm_data.ifi_iqdrops;
                    found                   = true;
                    break;
                }
            }
        }
    }
    free(buf);
    return found ? HJ_NIC_SUCCESS : HJ_NIC_ERR_NOT_FOUND;
#else
    return HJ_NIC_ERR_NOT_SUPPORTED;
#endif
}

/* Helper function to set network interface status securely using OS Native APIs */
static inline hj_nic_err_t
hj_nic_set_interface_state(const char *interface_name, bool enable)
{
    if(!interface_name)
        return HJ_NIC_ERR_INVALID_PARAMETER;

#if defined(HJ_NIC_PLATFORM_LINUX)
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0)
        return HJ_NIC_ERR_SYSTEM_ERROR;

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    hj_nic_safe_strcpy(ifr.ifr_name, sizeof(ifr.ifr_name), interface_name);

    if(ioctl(fd, SIOCGIFFLAGS, &ifr) < 0)
    {
        close(fd);
        return HJ_NIC_ERR_NOT_FOUND;
    }

    if(enable)
        ifr.ifr_flags |= IFF_UP;
    else
        ifr.ifr_flags &= ~IFF_UP;

    if(ioctl(fd, SIOCSIFFLAGS, &ifr) < 0)
    {
        close(fd);
        return HJ_NIC_ERR_ACCESS_DENIED;
    }

    close(fd);
    return HJ_NIC_SUCCESS;

#elif defined(HJ_NIC_PLATFORM_WINDOWS)
    NET_LUID luid;
    DWORD    index = 0;
    if(ConvertInterfaceNameToLuidA(interface_name, &luid) == NO_ERROR)
    {
        NET_IFINDEX if_idx = 0;
        if(ConvertInterfaceLuidToIndex(&luid, &if_idx) == NO_ERROR)
        {
            index = (DWORD) if_idx;
        }
    }

    if(index == 0)
    {
        return HJ_NIC_ERR_NOT_FOUND;
    }

    MIB_IFROW row;
    memset(&row, 0, sizeof(row));
    row.dwIndex = index;
    row.dwAdminStatus =
        enable ? MIB_IF_ADMIN_STATUS_UP : MIB_IF_ADMIN_STATUS_DOWN;

    if(SetIfEntry(&row) != NO_ERROR)
    {
        return HJ_NIC_ERR_ACCESS_DENIED;
    }

    return HJ_NIC_SUCCESS;

#else
    return HJ_NIC_ERR_NOT_SUPPORTED;
#endif
}

static inline hj_nic_err_t hj_nic_enable_interface(const char *interface_name)
{
    return hj_nic_set_interface_state(interface_name, true);
}

static inline hj_nic_err_t hj_nic_disable_interface(const char *interface_name)
{
    return hj_nic_set_interface_state(interface_name, false);
}

#ifdef __cplusplus
}
#endif

#endif // NIC_H