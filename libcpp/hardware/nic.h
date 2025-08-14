#ifndef NIC_H
#define NIC_H

// #include <stdint.h>
// #include <stdbool.h>
// #include <stddef.h>

// #ifdef __cplusplus
// extern "C" {
// #endif

// /* ----------------------------- Platform Detection ------------------------------------ */
// #if defined(_WIN32) || defined(_WIN64)
//     #define NIC_PLATFORM_WINDOWS 1
//     #include <winsock2.h>
//     #include <ws2tcpip.h>
//     #include <iphlpapi.h>
//     #include <netioapi.h>
//     #pragma comment(lib, "iphlpapi.lib")
//     #pragma comment(lib, "ws2_32.lib")
// #elif defined(__linux__)
//     #define NIC_PLATFORM_LINUX 1
//     #include <sys/socket.h>
//     #include <netinet/in.h>
//     #include <arpa/inet.h>
//     #include <net/if.h>
//     #include <ifaddrs.h>
//     #include <linux/ethtool.h>
//     #include <linux/sockios.h>
//     #include <sys/ioctl.h>
//     #include <unistd.h>
// #elif defined(__APPLE__)
//     #define NIC_PLATFORM_MACOS 1
//     #include <sys/socket.h>
//     #include <netinet/in.h>
//     #include <arpa/inet.h>
//     #include <net/if.h>
//     #include <ifaddrs.h>
//     #include <sys/sysctl.h>
// #else
//     #define NIC_PLATFORM_UNKNOWN 1
// #endif

// /* ----------------------------- Constants and Limits ------------------------------------ */
// #define NIC_MAX_INTERFACES          32
// #define NIC_MAX_NAME_LENGTH         256
// #define NIC_MAX_DESCRIPTION_LENGTH  512
// #define NIC_MAC_ADDRESS_LENGTH      6
// #define NIC_IP_ADDRESS_LENGTH       16
// #define NIC_IPV6_ADDRESS_LENGTH     40

// /* ----------------------------- Error Codes ------------------------------------ */
// typedef enum {
//     NIC_SUCCESS = 0,
//     NIC_ERROR_INVALID_PARAMETER = -1,
//     NIC_ERROR_ACCESS_DENIED = -2,
//     NIC_ERROR_NOT_FOUND = -3,
//     NIC_ERROR_INSUFFICIENT_BUFFER = -4,
//     NIC_ERROR_SYSTEM_ERROR = -5,
//     NIC_ERROR_NOT_SUPPORTED = -6,
//     NIC_ERROR_TIMEOUT = -7,
//     NIC_ERROR_INVALID_STATE = -8,
//     NIC_ERROR_INSUFFICIENT_MEMORY = -9
// } nic_error_t;

// /* ----------------------------- Interface Types ------------------------------------ */
// typedef enum {
//     NIC_TYPE_UNKNOWN = 0,
//     NIC_TYPE_ETHERNET = 1,
//     NIC_TYPE_WIRELESS = 2,
//     NIC_TYPE_LOOPBACK = 3,
//     NIC_TYPE_PPP = 4,
//     NIC_TYPE_TUNNEL = 5,
//     NIC_TYPE_VIRTUAL = 6
// } nic_type_t;

// /* ----------------------------- Interface Status ------------------------------------ */
// typedef enum {
//     NIC_STATUS_UNKNOWN = 0,
//     NIC_STATUS_UP = 1,
//     NIC_STATUS_DOWN = 2,
//     NIC_STATUS_TESTING = 3,
//     NIC_STATUS_DORMANT = 4,
//     NIC_STATUS_NOT_PRESENT = 5,
//     NIC_STATUS_LOWER_LAYER_DOWN = 6
// } nic_status_t;

// /* ----------------------------- Duplex Mode ------------------------------------ */
// typedef enum {
//     NIC_DUPLEX_UNKNOWN = 0,
//     NIC_DUPLEX_HALF = 1,
//     NIC_DUPLEX_FULL = 2
// } nic_duplex_t;

// /* ----------------------------- Structures ------------------------------------ */

// /* MAC Address structure */
// typedef struct {
//     uint8_t bytes[NIC_MAC_ADDRESS_LENGTH];
// } nic_mac_address_t;

// /* IP Address structure */
// typedef struct {
//     union {
//         uint8_t bytes[4];
//         uint32_t addr;
//     } ipv4;
//     char str[NIC_IP_ADDRESS_LENGTH];
// } nic_ip_address_t;

// /* IPv6 Address structure */
// typedef struct {
//     uint8_t bytes[16];
//     char str[NIC_IPV6_ADDRESS_LENGTH];
// } nic_ipv6_address_t;

// /* Network Interface Card information */
// typedef struct {
//     char name[NIC_MAX_NAME_LENGTH];                 /* Interface name (e.g., "eth0", "wlan0") */
//     char description[NIC_MAX_DESCRIPTION_LENGTH];   /* Interface description */
//     char friendly_name[NIC_MAX_NAME_LENGTH];        /* Friendly name (Windows) */
//     nic_mac_address_t mac_address;                  /* MAC address */
//     nic_type_t type;                                /* Interface type */
//     nic_status_t status;                            /* Current status */
//     uint32_t index;                                 /* Interface index */
//     uint32_t mtu;                                   /* Maximum Transmission Unit */
//     uint64_t speed;                                 /* Interface speed in bps */
//     nic_duplex_t duplex;                            /* Duplex mode */
//     bool dhcp_enabled;                              /* DHCP enabled */
//     bool is_virtual;                                /* Is virtual interface */
//     bool is_wireless;                               /* Is wireless interface */
//     nic_ip_address_t ip_address;                    /* Primary IPv4 address */
//     nic_ip_address_t subnet_mask;                   /* Subnet mask */
//     nic_ip_address_t gateway;                       /* Default gateway */
//     nic_ipv6_address_t ipv6_address;                /* Primary IPv6 address */
//     uint32_t ipv6_prefix_length;                    /* IPv6 prefix length */
// } nic_info_t;

// /* Network statistics */
// typedef struct {
//     uint64_t bytes_sent;                            /* Total bytes sent */
//     uint64_t bytes_received;                        /* Total bytes received */
//     uint64_t packets_sent;                          /* Total packets sent */
//     uint64_t packets_received;                      /* Total packets received */
//     uint64_t errors_sent;                           /* Send errors */
//     uint64_t errors_received;                       /* Receive errors */
//     uint64_t drops_sent;                            /* Sent packets dropped */
//     uint64_t drops_received;                        /* Received packets dropped */
// } nic_statistics_t;

// /* Wireless information (for wireless interfaces) */
// typedef struct {
//     char ssid[64];                                  /* SSID of connected network */
//     char bssid[18];                                 /* BSSID (MAC of access point) */
//     int signal_strength;                            /* Signal strength in dBm */
//     int signal_quality;                             /* Signal quality percentage */
//     uint32_t frequency;                             /* Frequency in MHz */
//     char security_type[32];                         /* Security type (WPA, WEP, etc.) */
//     bool is_connected;                              /* Connected to network */
// } nic_wireless_info_t;

// /* ----------------------------- Core Functions ------------------------------------ */

// /**
//  * Initialize NIC subsystem
//  * @return NIC_SUCCESS on success, error code on failure
//  */
// static inline nic_error_t nic_init(void) {
// #if defined(NIC_PLATFORM_WINDOWS)
//     WSADATA wsaData;
//     int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
//     return (result == 0) ? NIC_SUCCESS : NIC_ERROR_SYSTEM_ERROR;
// #else
//     // Linux/macOS don't need explicit initialization
//     return NIC_SUCCESS;
// #endif
// }

// /**
//  * Cleanup NIC subsystem
//  */
// static inline void nic_cleanup(void) {
// #if defined(NIC_PLATFORM_WINDOWS)
//     WSACleanup();
// #else
//     // Linux/macOS don't need explicit cleanup
// #endif
// }

// /**
//  * Get error message string
//  * @param error Error code
//  * @return Pointer to error message string
//  */
// static inline const char* nic_get_error_string(nic_error_t error) {
//     switch (error) {
//         case NIC_SUCCESS: return "Success";
//         case NIC_ERROR_INVALID_PARAMETER: return "Invalid parameter";
//         case NIC_ERROR_ACCESS_DENIED: return "Access denied";
//         case NIC_ERROR_NOT_FOUND: return "Interface not found";
//         case NIC_ERROR_INSUFFICIENT_BUFFER: return "Insufficient buffer";
//         case NIC_ERROR_SYSTEM_ERROR: return "System error";
//         case NIC_ERROR_NOT_SUPPORTED: return "Operation not supported";
//         case NIC_ERROR_TIMEOUT: return "Operation timeout";
//         case NIC_ERROR_INVALID_STATE: return "Invalid interface state";
//         case NIC_ERROR_INSUFFICIENT_MEMORY: return "Insufficient memory";
//         default: return "Unknown error";
//     }
// }

// /* ----------------------------- Interface Enumeration ------------------------------------ */

// /**
//  * Get number of network interfaces
//  * @param count [out] Number of interfaces
//  * @return NIC_SUCCESS on success, error code on failure
//  */
// static inline nic_error_t nic_get_interface_count(uint32_t* count) {
//     if (!count) return NIC_ERROR_INVALID_PARAMETER;
    
// #if defined(NIC_PLATFORM_WINDOWS)
//     ULONG size = 0;
//     GetAdaptersInfo(NULL, &size);
//     if (size == 0) {
//         *count = 0;
//         return NIC_SUCCESS;
//     }
    
//     PIP_ADAPTER_INFO adapters = (PIP_ADAPTER_INFO)malloc(size);
//     if (!adapters) return NIC_ERROR_INSUFFICIENT_MEMORY;
    
//     DWORD result = GetAdaptersInfo(adapters, &size);
//     if (result != ERROR_SUCCESS) {
//         free(adapters);
//         return NIC_ERROR_SYSTEM_ERROR;
//     }
    
//     uint32_t interface_count = 0;
//     PIP_ADAPTER_INFO adapter = adapters;
//     while (adapter) {
//         interface_count++;
//         adapter = adapter->Next;
//     }
    
//     free(adapters);
//     *count = interface_count;
//     return NIC_SUCCESS;
    
// #elif defined(NIC_PLATFORM_LINUX) || defined(NIC_PLATFORM_MACOS)
//     struct ifaddrs* ifaddrs_ptr;
//     if (getifaddrs(&ifaddrs_ptr) == -1) {
//         return NIC_ERROR_SYSTEM_ERROR;
//     }
    
//     uint32_t interface_count = 0;
//     struct ifaddrs* ifa = ifaddrs_ptr;
//     while (ifa) {
//         if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_LINK) {
//             interface_count++;
//         }
//         ifa = ifa->ifa_next;
//     }
    
//     freeifaddrs(ifaddrs_ptr);
//     *count = interface_count;
//     return NIC_SUCCESS;
// #else
//     *count = 0;
//     return NIC_ERROR_NOT_SUPPORTED;
// #endif
// }

// /**
//  * Enumerate all network interfaces
//  * @param interfaces [out] Array to store interface information
//  * @param max_interfaces Maximum number of interfaces to retrieve
//  * @param actual_count [out] Actual number of interfaces retrieved
//  * @return NIC_SUCCESS on success, error code on failure
//  */
// static inline nic_error_t nic_enumerate_interfaces(nic_info_t* interfaces, 
//                                                    uint32_t max_interfaces, 
//                                                    uint32_t* actual_count) {
//     if (!interfaces || !actual_count || max_interfaces == 0) {
//         return NIC_ERROR_INVALID_PARAMETER;
//     }
    
//     *actual_count = 0;
    
// #if defined(NIC_PLATFORM_WINDOWS)
//     ULONG size = 0;
//     GetAdaptersInfo(NULL, &size);
//     if (size == 0) return NIC_SUCCESS;
    
//     PIP_ADAPTER_INFO adapters = (PIP_ADAPTER_INFO)malloc(size);
//     if (!adapters) return NIC_ERROR_INSUFFICIENT_MEMORY;
    
//     DWORD result = GetAdaptersInfo(adapters, &size);
//     if (result != ERROR_SUCCESS) {
//         free(adapters);
//         return NIC_ERROR_SYSTEM_ERROR;
//     }
    
//     PIP_ADAPTER_INFO adapter = adapters;
//     uint32_t count = 0;
    
//     while (adapter && count < max_interfaces) {
//         nic_info_t* nic = &interfaces[count];
//         memset(nic, 0, sizeof(nic_info_t));
        
//         strncpy_s(nic->name, sizeof(nic->name), adapter->AdapterName, _TRUNCATE);
//         strncpy_s(nic->description, sizeof(nic->description), adapter->Description, _TRUNCATE);
        
//         // Copy MAC address
//         if (adapter->AddressLength == NIC_MAC_ADDRESS_LENGTH) {
//             memcpy(nic->mac_address.bytes, adapter->Address, NIC_MAC_ADDRESS_LENGTH);
//         }
        
//         nic->index = adapter->Index;
//         nic->type = (adapter->Type == MIB_IF_TYPE_ETHERNET) ? NIC_TYPE_ETHERNET : NIC_TYPE_UNKNOWN;
//         nic->dhcp_enabled = (adapter->DhcpEnabled != 0);
        
//         // Get IP address
//         if (adapter->IpAddressList.IpAddress.String[0] != '0') {
//             strncpy_s(nic->ip_address.str, sizeof(nic->ip_address.str), 
//                      adapter->IpAddressList.IpAddress.String, _TRUNCATE);
//             nic->ip_address.ipv4.addr = inet_addr(adapter->IpAddressList.IpAddress.String);
//         }
        
//         // Get subnet mask
//         if (adapter->IpAddressList.IpMask.String[0] != '0') {
//             strncpy_s(nic->subnet_mask.str, sizeof(nic->subnet_mask.str),
//                      adapter->IpAddressList.IpMask.String, _TRUNCATE);
//             nic->subnet_mask.ipv4.addr = inet_addr(adapter->IpAddressList.IpMask.String);
//         }
        
//         // Get gateway
//         if (adapter->GatewayList.IpAddress.String[0] != '0') {
//             strncpy_s(nic->gateway.str, sizeof(nic->gateway.str),
//                      adapter->GatewayList.IpAddress.String, _TRUNCATE);
//             nic->gateway.ipv4.addr = inet_addr(adapter->GatewayList.IpAddress.String);
//         }
        
//         count++;
//         adapter = adapter->Next;
//     }
    
//     free(adapters);
//     *actual_count = count;
//     return NIC_SUCCESS;
    
// #elif defined(NIC_PLATFORM_LINUX) || defined(NIC_PLATFORM_MACOS)
//     struct ifaddrs* ifaddrs_ptr;
//     if (getifaddrs(&ifaddrs_ptr) == -1) {
//         return NIC_ERROR_SYSTEM_ERROR;
//     }
    
//     uint32_t count = 0;
//     struct ifaddrs* ifa = ifaddrs_ptr;
    
//     while (ifa && count < max_interfaces) {
//         if (ifa->ifa_addr) {
//             nic_info_t* nic = &interfaces[count];
//             memset(nic, 0, sizeof(nic_info_t));
            
//             strncpy(nic->name, ifa->ifa_name, sizeof(nic->name) - 1);
//             nic->index = if_nametoindex(ifa->ifa_name);
            
//             // Determine interface type
//             if (strncmp(ifa->ifa_name, "lo", 2) == 0) {
//                 nic->type = NIC_TYPE_LOOPBACK;
//             } else if (strncmp(ifa->ifa_name, "eth", 3) == 0 || 
//                       strncmp(ifa->ifa_name, "en", 2) == 0) {
//                 nic->type = NIC_TYPE_ETHERNET;
//             } else if (strncmp(ifa->ifa_name, "wlan", 4) == 0 || 
//                       strncmp(ifa->ifa_name, "wifi", 4) == 0) {
//                 nic->type = NIC_TYPE_WIRELESS;
//                 nic->is_wireless = true;
//             } else {
//                 nic->type = NIC_TYPE_UNKNOWN;
//             }
            
//             // Check interface status
//             nic->status = (ifa->ifa_flags & IFF_UP) ? NIC_STATUS_UP : NIC_STATUS_DOWN;
            
//             // Get IP address
//             if (ifa->ifa_addr->sa_family == AF_INET) {
//                 struct sockaddr_in* sin = (struct sockaddr_in*)ifa->ifa_addr;
//                 inet_ntop(AF_INET, &sin->sin_addr, nic->ip_address.str, sizeof(nic->ip_address.str));
//                 nic->ip_address.ipv4.addr = sin->sin_addr.s_addr;
                
//                 // Get netmask
//                 if (ifa->ifa_netmask) {
//                     struct sockaddr_in* netmask = (struct sockaddr_in*)ifa->ifa_netmask;
//                     inet_ntop(AF_INET, &netmask->sin_addr, nic->subnet_mask.str, sizeof(nic->subnet_mask.str));
//                     nic->subnet_mask.ipv4.addr = netmask->sin_addr.s_addr;
//                 }
//             } else if (ifa->ifa_addr->sa_family == AF_INET6) {
//                 struct sockaddr_in6* sin6 = (struct sockaddr_in6*)ifa->ifa_addr;
//                 inet_ntop(AF_INET6, &sin6->sin6_addr, nic->ipv6_address.str, sizeof(nic->ipv6_address.str));
//                 memcpy(nic->ipv6_address.bytes, &sin6->sin6_addr, 16);
//             }
            
//             count++;
//         }
//         ifa = ifa->ifa_next;
//     }
    
//     freeifaddrs(ifaddrs_ptr);
//     *actual_count = count;
//     return NIC_SUCCESS;
// #else
//     return NIC_ERROR_NOT_SUPPORTED;
// #endif
// }

// /**
//  * Get interface information by name
//  * @param interface_name Interface name
//  * @param info [out] Interface information
//  * @return NIC_SUCCESS on success, error code on failure
//  */
// static inline nic_error_t nic_get_interface_info(const char* interface_name, nic_info_t* info) {
//     if (!interface_name || !info) {
//         return NIC_ERROR_INVALID_PARAMETER;
//     }
    
//     nic_info_t interfaces[NIC_MAX_INTERFACES];
//     uint32_t count;
    
//     nic_error_t result = nic_enumerate_interfaces(interfaces, NIC_MAX_INTERFACES, &count);
//     if (result != NIC_SUCCESS) {
//         return result;
//     }
    
//     for (uint32_t i = 0; i < count; i++) {
//         if (strcmp(interfaces[i].name, interface_name) == 0) {
//             *info = interfaces[i];
//             return NIC_SUCCESS;
//         }
//     }
    
//     return NIC_ERROR_NOT_FOUND;
// }

// /* ----------------------------- Interface Statistics ------------------------------------ */

// /**
//  * Get interface statistics
//  * @param interface_name Interface name
//  * @param stats [out] Interface statistics
//  * @return NIC_SUCCESS on success, error code on failure
//  */
// static inline nic_error_t nic_get_statistics(const char* interface_name, nic_statistics_t* stats) {
//     if (!interface_name || !stats) {
//         return NIC_ERROR_INVALID_PARAMETER;
//     }
    
//     memset(stats, 0, sizeof(nic_statistics_t));
    
// #if defined(NIC_PLATFORM_WINDOWS)
//     ULONG size = 0;
//     GetIfTable(NULL, &size, FALSE);
//     if (size == 0) return NIC_ERROR_SYSTEM_ERROR;
    
//     PMIB_IFTABLE ifTable = (PMIB_IFTABLE)malloc(size);
//     if (!ifTable) return NIC_ERROR_INSUFFICIENT_MEMORY;
    
//     DWORD result = GetIfTable(ifTable, &size, FALSE);
//     if (result != NO_ERROR) {
//         free(ifTable);
//         return NIC_ERROR_SYSTEM_ERROR;
//     }
    
//     // Find interface by name (simplified)
//     for (DWORD i = 0; i < ifTable->dwNumEntries; i++) {
//         MIB_IFROW* row = &ifTable->table[i];
//         stats->bytes_sent = row->dwOutOctets;
//         stats->bytes_received = row->dwInOctets;
//         stats->packets_sent = row->dwOutUcastPkts + row->dwOutNUcastPkts;
//         stats->packets_received = row->dwInUcastPkts + row->dwInNUcastPkts;
//         stats->errors_sent = row->dwOutErrors;
//         stats->errors_received = row->dwInErrors;
//         stats->drops_sent = row->dwOutDiscards;
//         stats->drops_received = row->dwInDiscards;
//         break; // Take first interface for simplicity
//     }
    
//     free(ifTable);
//     return NIC_SUCCESS;
    
// #elif defined(NIC_PLATFORM_LINUX)
//     FILE* file = fopen("/proc/net/dev", "r");
//     if (!file) return NIC_ERROR_SYSTEM_ERROR;
    
//     char line[512];
//     bool found = false;
    
//     // Skip header lines
//     fgets(line, sizeof(line), file);
//     fgets(line, sizeof(line), file);
    
//     while (fgets(line, sizeof(line), file)) {
//         char name[64];
//         unsigned long rx_bytes, rx_packets, rx_errs, rx_drop;
//         unsigned long tx_bytes, tx_packets, tx_errs, tx_drop;
        
//         if (sscanf(line, "%63[^:]: %lu %lu %lu %lu %*u %*u %*u %*u %lu %lu %lu %lu",
//                    name, &rx_bytes, &rx_packets, &rx_errs, &rx_drop,
//                    &tx_bytes, &tx_packets, &tx_errs, &tx_drop) == 9) {
            
//             // Remove leading/trailing whitespace from name
//             char* trimmed_name = name;
//             while (*trimmed_name == ' ') trimmed_name++;
            
//             if (strcmp(trimmed_name, interface_name) == 0) {
//                 stats->bytes_received = rx_bytes;
//                 stats->packets_received = rx_packets;
//                 stats->errors_received = rx_errs;
//                 stats->drops_received = rx_drop;
//                 stats->bytes_sent = tx_bytes;
//                 stats->packets_sent = tx_packets;
//                 stats->errors_sent = tx_errs;
//                 stats->drops_sent = tx_drop;
//                 found = true;
//                 break;
//             }
//         }
//     }
    
//     fclose(file);
//     return found ? NIC_SUCCESS : NIC_ERROR_NOT_FOUND;
    
// #else
//     return NIC_ERROR_NOT_SUPPORTED;
// #endif
// }

// /* ----------------------------- Interface Control ------------------------------------ */

// /**
//  * Enable network interface
//  * @param interface_name Interface name
//  * @return NIC_SUCCESS on success, error code on failure
//  */
// static inline nic_error_t nic_enable_interface(const char* interface_name) {
//     if (!interface_name) return NIC_ERROR_INVALID_PARAMETER;
    
// #if defined(NIC_PLATFORM_LINUX)
//     char command[256];
//     snprintf(command, sizeof(command), "ip link set %s up", interface_name);
//     int result = system(command);
//     return (result == 0) ? NIC_SUCCESS : NIC_ERROR_SYSTEM_ERROR;
// #elif defined(NIC_PLATFORM_WINDOWS)
//     // Windows requires WMI or netsh command
//     char command[256];
//     snprintf(command, sizeof(command), "netsh interface set interface \"%s\" enable", interface_name);
//     int result = system(command);
//     return (result == 0) ? NIC_SUCCESS : NIC_ERROR_SYSTEM_ERROR;
// #else
//     return NIC_ERROR_NOT_SUPPORTED;
// #endif
// }

// /**
//  * Disable network interface
//  * @param interface_name Interface name
//  * @return NIC_SUCCESS on success, error code on failure
//  */
// static inline nic_error_t nic_disable_interface(const char* interface_name) {
//     if (!interface_name) return NIC_ERROR_INVALID_PARAMETER;
    
// #if defined(NIC_PLATFORM_LINUX)
//     char command[256];
//     snprintf(command, sizeof(command), "ip link set %s down", interface_name);
//     int result = system(command);
//     return (result == 0) ? NIC_SUCCESS : NIC_ERROR_SYSTEM_ERROR;
// #elif defined(NIC_PLATFORM_WINDOWS)
//     char command[256];
//     snprintf(command, sizeof(command), "netsh interface set interface \"%s\" disable", interface_name);
//     int result = system(command);
//     return (result == 0) ? NIC_SUCCESS : NIC_ERROR_SYSTEM_ERROR;
// #else
//     return NIC_ERROR_NOT_SUPPORTED;
// #endif
// }

// /* ----------------------------- Utility Functions ------------------------------------ */

// /**
//  * Convert NIC type to string
//  * @param type NIC type
//  * @return String representation of NIC type
//  */
// static inline const char* nic_type_to_string(nic_type_t type) {
//     switch (type) {
//         case NIC_TYPE_ETHERNET: return "Ethernet";
//         case NIC_TYPE_WIRELESS: return "Wireless";
//         case NIC_TYPE_LOOPBACK: return "Loopback";
//         case NIC_TYPE_PPP: return "PPP";
//         case NIC_TYPE_TUNNEL: return "Tunnel";
//         case NIC_TYPE_VIRTUAL: return "Virtual";
//         default: return "Unknown";
//     }
// }

// /**
//  * Convert NIC status to string
//  * @param status NIC status
//  * @return String representation of NIC status
//  */
// static inline const char* nic_status_to_string(nic_status_t status) {
//     switch (status) {
//         case NIC_STATUS_UP: return "Up";
//         case NIC_STATUS_DOWN: return "Down";
//         case NIC_STATUS_TESTING: return "Testing";
//         case NIC_STATUS_DORMANT: return "Dormant";
//         case NIC_STATUS_NOT_PRESENT: return "Not Present";
//         case NIC_STATUS_LOWER_LAYER_DOWN: return "Lower Layer Down";
//         default: return "Unknown";
//     }
// }

// /**
//  * Format MAC address to string
//  * @param mac_addr MAC address
//  * @param buffer [out] Buffer to store formatted string
//  * @param buffer_size Size of buffer
//  * @return NIC_SUCCESS on success, error code on failure
//  */
// static inline nic_error_t nic_format_mac_address(const nic_mac_address_t* mac_addr, 
//                                                  char* buffer, size_t buffer_size) {
//     if (!mac_addr || !buffer || buffer_size < 18) {
//         return NIC_ERROR_INVALID_PARAMETER;
//     }
    
//     snprintf(buffer, buffer_size, "%02X:%02X:%02X:%02X:%02X:%02X",
//              mac_addr->bytes[0], mac_addr->bytes[1], mac_addr->bytes[2],
//              mac_addr->bytes[3], mac_addr->bytes[4], mac_addr->bytes[5]);
    
//     return NIC_SUCCESS;
// }

// /**
//  * Parse MAC address from string
//  * @param mac_str MAC address string (format: "XX:XX:XX:XX:XX:XX")
//  * @param mac_addr [out] MAC address structure
//  * @return NIC_SUCCESS on success, error code on failure
//  */
// static inline nic_error_t nic_parse_mac_address(const char* mac_str, nic_mac_address_t* mac_addr) {
//     if (!mac_str || !mac_addr) {
//         return NIC_ERROR_INVALID_PARAMETER;
//     }
    
//     unsigned int bytes[6];
//     if (sscanf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x",
//                &bytes[0], &bytes[1], &bytes[2], &bytes[3], &bytes[4], &bytes[5]) != 6) {
//         return NIC_ERROR_INVALID_PARAMETER;
//     }
    
//     for (int i = 0; i < 6; i++) {
//         mac_addr->bytes[i] = (uint8_t)bytes[i];
//     }
    
//     return NIC_SUCCESS;
// }

// /**
//  * Check if interface is wireless
//  * @param interface_name Interface name
//  * @param is_wireless [out] True if wireless interface
//  * @return NIC_SUCCESS on success, error code on failure
//  */
// static inline nic_error_t nic_is_wireless(const char* interface_name, bool* is_wireless) {
//     if (!interface_name || !is_wireless) {
//         return NIC_ERROR_INVALID_PARAMETER;
//     }
    
//     *is_wireless = false;
    
// #if defined(NIC_PLATFORM_LINUX)
//     char path[256];
//     snprintf(path, sizeof(path), "/sys/class/net/%s/wireless", interface_name);
    
//     // Check if wireless directory exists
//     struct stat st;
//     *is_wireless = (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
//     return NIC_SUCCESS;
// #else
//     // Simple heuristic based on interface name
//     if (strstr(interface_name, "wlan") || strstr(interface_name, "wifi") || 
//         strstr(interface_name, "wl") || strstr(interface_name, "ath")) {
//         *is_wireless = true;
//     }
//     return NIC_SUCCESS;
// #endif
// }

// /**
//  * Get wireless information (for wireless interfaces only)
//  * @param interface_name Interface name
//  * @param wireless_info [out] Wireless information
//  * @return NIC_SUCCESS on success, error code on failure
//  */
// static inline nic_error_t nic_get_wireless_info(const char* interface_name, 
//                                                 nic_wireless_info_t* wireless_info) {
//     if (!interface_name || !wireless_info) {
//         return NIC_ERROR_INVALID_PARAMETER;
//     }
    
//     memset(wireless_info, 0, sizeof(nic_wireless_info_t));
    
//     bool is_wireless = false;
//     nic_error_t result = nic_is_wireless(interface_name, &is_wireless);
//     if (result != NIC_SUCCESS || !is_wireless) {
//         return NIC_ERROR_NOT_SUPPORTED;
//     }
    
// #if defined(NIC_PLATFORM_LINUX)
//     // This is a simplified implementation
//     // In practice, you would use nl80211 or wireless extensions
//     char command[256];
//     snprintf(command, sizeof(command), "iwconfig %s 2>/dev/null", interface_name);
    
//     FILE* pipe = popen(command, "r");
//     if (!pipe) return NIC_ERROR_SYSTEM_ERROR;
    
//     char line[512];
//     while (fgets(line, sizeof(line), pipe)) {
//         if (strstr(line, "ESSID:")) {
//             char* start = strstr(line, "ESSID:\"");
//             if (start) {
//                 start += 7; // Skip 'ESSID:"'
//                 char* end = strchr(start, '"');
//                 if (end) {
//                     size_t len = end - start;
//                     if (len < sizeof(wireless_info->ssid)) {
//                         strncpy(wireless_info->ssid, start, len);
//                         wireless_info->ssid[len] = '\0';
//                         wireless_info->is_connected = true;
//                     }
//                 }
//             }
//         }
//         // Parse other wireless information as needed
//     }
    
//     pclose(pipe);
//     return NIC_SUCCESS;
// #else
//     return NIC_ERROR_NOT_SUPPORTED;
// #endif
// }

// void nic_mac_addr(char* buf, const uint8_t* mac)
// {
//    TODO: implement this
// }

// #ifdef __cplusplus
// }
// #endif

#endif // NIC_H