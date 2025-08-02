#ifndef RAM_H
#define RAM_H

// #include <stdint.h>
// #include <stdbool.h>
// #include <stddef.h>

// #ifdef __cplusplus
// extern "C" {
// #endif

// /* ----------------------------- Platform Detection ------------------------------------ */
// #if defined(_WIN32) || defined(_WIN64)
//     #define RAM_PLATFORM_WINDOWS 1
//     #include <windows.h>
//     #include <psapi.h>
//     #include <memoryapi.h>
//     #pragma comment(lib, "psapi.lib")
// #elif defined(__linux__)
//     #define RAM_PLATFORM_LINUX 1
//     #include <sys/sysinfo.h>
//     #include <sys/mman.h>
//     #include <unistd.h>
//     #include <stdio.h>
//     #include <string.h>
//     #include <stdlib.h>
// #elif defined(__APPLE__)
//     #define RAM_PLATFORM_MACOS 1
//     #include <sys/types.h>
//     #include <sys/sysctl.h>
//     #include <sys/mman.h>
//     #include <mach/mach.h>
//     #include <mach/vm_statistics.h>
//     #include <mach/mach_types.h>
//     #include <mach/mach_init.h>
//     #include <mach/mach_host.h>
//     #include <unistd.h>
// #else
//     #define RAM_PLATFORM_UNKNOWN 1
// #endif

// /* ----------------------------- Constants and Limits ------------------------------------ */
// #define RAM_MAX_MODULES                 8
// #define RAM_MAX_MANUFACTURER_LENGTH     64
// #define RAM_MAX_PART_NUMBER_LENGTH      32
// #define RAM_MAX_SERIAL_LENGTH           32
// #define RAM_BYTES_PER_KB               1024ULL
// #define RAM_BYTES_PER_MB               (1024ULL * 1024ULL)
// #define RAM_BYTES_PER_GB               (1024ULL * 1024ULL * 1024ULL)

// /* ----------------------------- Error Codes ------------------------------------ */
// typedef enum {
//     RAM_SUCCESS = 0,
//     RAM_ERROR_INVALID_PARAMETER = -1,
//     RAM_ERROR_ACCESS_DENIED = -2,
//     RAM_ERROR_NOT_FOUND = -3,
//     RAM_ERROR_INSUFFICIENT_BUFFER = -4,
//     RAM_ERROR_SYSTEM_ERROR = -5,
//     RAM_ERROR_NOT_SUPPORTED = -6,
//     RAM_ERROR_INSUFFICIENT_MEMORY = -7,
//     RAM_ERROR_ALLOCATION_FAILED = -8,
//     RAM_ERROR_INVALID_ADDRESS = -9,
//     RAM_ERROR_PROTECTION_VIOLATION = -10
// } ram_error_t;

// /* ----------------------------- RAM Types ------------------------------------ */
// typedef enum {
//     RAM_TYPE_UNKNOWN = 0,
//     RAM_TYPE_DDR = 1,
//     RAM_TYPE_DDR2 = 2,
//     RAM_TYPE_DDR3 = 3,
//     RAM_TYPE_DDR4 = 4,
//     RAM_TYPE_DDR5 = 5,
//     RAM_TYPE_SDRAM = 6,
//     RAM_TYPE_SRAM = 7,
//     RAM_TYPE_RDRAM = 8
// } ram_type_t;

// /* ----------------------------- Memory Protection ------------------------------------ */
// typedef enum {
//     RAM_PROTECTION_NONE = 0,
//     RAM_PROTECTION_READ = 1,
//     RAM_PROTECTION_WRITE = 2,
//     RAM_PROTECTION_EXECUTE = 4,
//     RAM_PROTECTION_READ_WRITE = 3,
//     RAM_PROTECTION_READ_EXECUTE = 5,
//     RAM_PROTECTION_WRITE_EXECUTE = 6,
//     RAM_PROTECTION_READ_WRITE_EXECUTE = 7
// } ram_protection_t;

// /* ----------------------------- Structures ------------------------------------ */

// /* RAM module information */
// typedef struct {
//     char manufacturer[RAM_MAX_MANUFACTURER_LENGTH];     /* Manufacturer name */
//     char part_number[RAM_MAX_PART_NUMBER_LENGTH];       /* Part number */
//     char serial_number[RAM_MAX_SERIAL_LENGTH];          /* Serial number */
//     ram_type_t type;                                    /* RAM type (DDR, DDR2, etc.) */
//     uint64_t capacity;                                  /* Capacity in bytes */
//     uint32_t speed;                                     /* Speed in MHz */
//     uint32_t voltage;                                   /* Voltage in millivolts */
//     uint32_t slot_number;                               /* Physical slot number */
//     bool is_ecc;                                        /* ECC support */
//     bool is_registered;                                 /* Registered memory */
// } ram_module_info_t;

// /* System memory information */
// typedef struct {
//     uint64_t total_physical;                            /* Total physical RAM in bytes */
//     uint64_t available_physical;                        /* Available physical RAM in bytes */
//     uint64_t used_physical;                             /* Used physical RAM in bytes */
//     uint64_t total_virtual;                             /* Total virtual memory in bytes */
//     uint64_t available_virtual;                         /* Available virtual memory in bytes */
//     uint64_t used_virtual;                              /* Used virtual memory in bytes */
//     uint64_t total_swap;                                /* Total swap space in bytes */
//     uint64_t available_swap;                            /* Available swap space in bytes */
//     uint64_t used_swap;                                 /* Used swap space in bytes */
//     uint32_t page_size;                                 /* Memory page size in bytes */
//     uint32_t large_page_size;                           /* Large page size in bytes */
//     double memory_load;                                 /* Memory load percentage (0-100) */
//     uint32_t module_count;                              /* Number of RAM modules */
// } ram_system_info_t;

// /* Memory statistics */
// typedef struct {
//     uint64_t allocations_count;                         /* Number of allocations */
//     uint64_t deallocations_count;                       /* Number of deallocations */
//     uint64_t bytes_allocated;                           /* Total bytes allocated */
//     uint64_t bytes_deallocated;                         /* Total bytes deallocated */
//     uint64_t peak_usage;                                /* Peak memory usage */
//     uint64_t current_usage;                             /* Current memory usage */
//     uint32_t page_faults;                               /* Page fault count */
//     uint32_t page_file_usage;                           /* Page file usage count */
// } ram_statistics_t;

// /* Memory region information */
// typedef struct {
//     void* base_address;                                 /* Base address of region */
//     uint64_t size;                                      /* Size of region in bytes */
//     ram_protection_t protection;                        /* Memory protection flags */
//     bool is_committed;                                  /* Is memory committed */
//     bool is_mapped;                                     /* Is memory mapped */
//     bool is_shared;                                     /* Is shared memory */
// } ram_region_info_t;

// /* ----------------------------- Core Functions ------------------------------------ */

// /**
//  * Initialize RAM subsystem
//  * @return RAM_SUCCESS on success, error code on failure
//  */
// static inline ram_error_t ram_init(void) {
//     // Most platforms don't need explicit initialization for RAM operations
//     return RAM_SUCCESS;
// }

// /**
//  * Cleanup RAM subsystem
//  */
// static inline void ram_cleanup(void) {
//     // Most platforms don't need explicit cleanup for RAM operations
// }

// /**
//  * Get error message string
//  * @param error Error code
//  * @return Pointer to error message string
//  */
// static inline const char* ram_get_error_string(ram_error_t error) {
//     switch (error) {
//         case RAM_SUCCESS: return "Success";
//         case RAM_ERROR_INVALID_PARAMETER: return "Invalid parameter";
//         case RAM_ERROR_ACCESS_DENIED: return "Access denied";
//         case RAM_ERROR_NOT_FOUND: return "Memory region not found";
//         case RAM_ERROR_INSUFFICIENT_BUFFER: return "Insufficient buffer";
//         case RAM_ERROR_SYSTEM_ERROR: return "System error";
//         case RAM_ERROR_NOT_SUPPORTED: return "Operation not supported";
//         case RAM_ERROR_INSUFFICIENT_MEMORY: return "Insufficient memory";
//         case RAM_ERROR_ALLOCATION_FAILED: return "Memory allocation failed";
//         case RAM_ERROR_INVALID_ADDRESS: return "Invalid memory address";
//         case RAM_ERROR_PROTECTION_VIOLATION: return "Memory protection violation";
//         default: return "Unknown error";
//     }
// }

// /* ----------------------------- System Memory Information ------------------------------------ */

// /**
//  * Get system memory information
//  * @param info [out] System memory information
//  * @return RAM_SUCCESS on success, error code on failure
//  */
// static inline ram_error_t ram_get_system_info(ram_system_info_t* info) {
//     if (!info) return RAM_ERROR_INVALID_PARAMETER;
    
//     memset(info, 0, sizeof(ram_system_info_t));
    
// #if defined(RAM_PLATFORM_WINDOWS)
//     MEMORYSTATUSEX memStatus;
//     memStatus.dwLength = sizeof(memStatus);
    
//     if (!GlobalMemoryStatusEx(&memStatus)) {
//         return RAM_ERROR_SYSTEM_ERROR;
//     }
    
//     info->total_physical = memStatus.ullTotalPhys;
//     info->available_physical = memStatus.ullAvailPhys;
//     info->used_physical = info->total_physical - info->available_physical;
//     info->total_virtual = memStatus.ullTotalVirtual;
//     info->available_virtual = memStatus.ullAvailVirtual;
//     info->used_virtual = info->total_virtual - info->available_virtual;
//     info->total_swap = memStatus.ullTotalPageFile - memStatus.ullTotalPhys;
//     info->available_swap = memStatus.ullAvailPageFile - memStatus.ullAvailPhys;
//     info->used_swap = info->total_swap - info->available_swap;
//     info->memory_load = (double)memStatus.dwMemoryLoad;
    
//     SYSTEM_INFO sysInfo;
//     GetSystemInfo(&sysInfo);
//     info->page_size = sysInfo.dwPageSize;
    
//     // Get large page size
//     SIZE_T largePageSize = GetLargePageMinimum();
//     info->large_page_size = (uint32_t)largePageSize;
    
//     return RAM_SUCCESS;
    
// #elif defined(RAM_PLATFORM_LINUX)
//     struct sysinfo si;
//     if (sysinfo(&si) != 0) {
//         return RAM_ERROR_SYSTEM_ERROR;
//     }
    
//     info->total_physical = si.totalram * si.mem_unit;
//     info->available_physical = si.freeram * si.mem_unit;
//     info->used_physical = info->total_physical - info->available_physical;
//     info->total_swap = si.totalswap * si.mem_unit;
//     info->available_swap = si.freeswap * si.mem_unit;
//     info->used_swap = info->total_swap - info->available_swap;
    
//     // Calculate memory load percentage
//     if (info->total_physical > 0) {
//         info->memory_load = ((double)info->used_physical / info->total_physical) * 100.0;
//     }
    
//     // Get page size
//     long page_size = sysconf(_SC_PAGESIZE);
//     if (page_size > 0) {
//         info->page_size = (uint32_t)page_size;
//     }
    
//     // Try to get virtual memory info from /proc/meminfo
//     FILE* meminfo = fopen("/proc/meminfo", "r");
//     if (meminfo) {
//         char line[256];
//         while (fgets(line, sizeof(line), meminfo)) {
//             unsigned long value;
//             if (sscanf(line, "VmallocTotal: %lu kB", &value) == 1) {
//                 info->total_virtual = value * 1024;
//             } else if (sscanf(line, "VmallocUsed: %lu kB", &value) == 1) {
//                 info->used_virtual = value * 1024;
//             }
//         }
//         fclose(meminfo);
//         info->available_virtual = info->total_virtual - info->used_virtual;
//     }
    
//     return RAM_SUCCESS;
    
// #elif defined(RAM_PLATFORM_MACOS)
//     int mib[2];
//     size_t length;
    
//     // Get total physical memory
//     mib[0] = CTL_HW;
//     mib[1] = HW_MEMSIZE;
//     length = sizeof(info->total_physical);
//     if (sysctl(mib, 2, &info->total_physical, &length, NULL, 0) != 0) {
//         return RAM_ERROR_SYSTEM_ERROR;
//     }
    
//     // Get page size
//     mib[1] = HW_PAGESIZE;
//     length = sizeof(info->page_size);
//     if (sysctl(mib, 2, &info->page_size, &length, NULL, 0) != 0) {
//         info->page_size = 4096; // Default page size
//     }
    
//     // Get VM statistics
//     vm_size_t page_size_vm;
//     vm_statistics64_data_t vm_stat;
//     mach_msg_type_number_t count = sizeof(vm_stat) / sizeof(natural_t);
    
//     if (host_page_size(mach_host_self(), &page_size_vm) == KERN_SUCCESS &&
//         host_statistics64(mach_host_self(), HOST_VM_INFO, 
//                          (host_info64_t)&vm_stat, &count) == KERN_SUCCESS) {
        
//         info->available_physical = (uint64_t)vm_stat.free_count * page_size_vm;
//         info->used_physical = info->total_physical - info->available_physical;
        
//         if (info->total_physical > 0) {
//             info->memory_load = ((double)info->used_physical / info->total_physical) * 100.0;
//         }
//     }
    
//     return RAM_SUCCESS;
    
// #else
//     return RAM_ERROR_NOT_SUPPORTED;
// #endif
// }

// /**
//  * Get RAM module information
//  * @param modules [out] Array to store module information
//  * @param max_modules Maximum number of modules to retrieve
//  * @param actual_count [out] Actual number of modules retrieved
//  * @return RAM_SUCCESS on success, error code on failure
//  */
// static inline ram_error_t ram_get_modules(ram_module_info_t* modules, 
//                                           uint32_t max_modules, 
//                                           uint32_t* actual_count) {
//     if (!modules || !actual_count || max_modules == 0) {
//         return RAM_ERROR_INVALID_PARAMETER;
//     }
    
//     *actual_count = 0;
    
// #if defined(RAM_PLATFORM_WINDOWS)
//     // On Windows, we would need WMI to get detailed RAM module information
//     // This is a simplified implementation
//     ram_system_info_t sys_info;
//     ram_error_t result = ram_get_system_info(&sys_info);
//     if (result != RAM_SUCCESS) {
//         return result;
//     }
    
//     // Create a single module entry representing total system RAM
//     if (max_modules >= 1) {
//         memset(&modules[0], 0, sizeof(ram_module_info_t));
//         strcpy_s(modules[0].manufacturer, sizeof(modules[0].manufacturer), "Unknown");
//         strcpy_s(modules[0].part_number, sizeof(modules[0].part_number), "System RAM");
//         modules[0].type = RAM_TYPE_UNKNOWN;
//         modules[0].capacity = sys_info.total_physical;
//         modules[0].slot_number = 1;
//         *actual_count = 1;
//     }
    
//     return RAM_SUCCESS;
    
// #elif defined(RAM_PLATFORM_LINUX)
//     // On Linux, we can try to read from /proc/meminfo and /sys/devices/system/memory
//     // This is a simplified implementation
//     ram_system_info_t sys_info;
//     ram_error_t result = ram_get_system_info(&sys_info);
//     if (result != RAM_SUCCESS) {
//         return result;
//     }
    
//     if (max_modules >= 1) {
//         memset(&modules[0], 0, sizeof(ram_module_info_t));
//         strncpy(modules[0].manufacturer, "Unknown", sizeof(modules[0].manufacturer) - 1);
//         strncpy(modules[0].part_number, "System RAM", sizeof(modules[0].part_number) - 1);
//         modules[0].type = RAM_TYPE_UNKNOWN;
//         modules[0].capacity = sys_info.total_physical;
//         modules[0].slot_number = 1;
//         *actual_count = 1;
//     }
    
//     return RAM_SUCCESS;
    
// #else
//     return RAM_ERROR_NOT_SUPPORTED;
// #endif
// }

// /* ----------------------------- Memory Allocation ------------------------------------ */

// /**
//  * Allocate aligned memory
//  * @param size Size in bytes to allocate
//  * @param alignment Alignment requirement in bytes (must be power of 2)
//  * @param ptr [out] Pointer to allocated memory
//  * @return RAM_SUCCESS on success, error code on failure
//  */
// static inline ram_error_t ram_allocate_aligned(size_t size, size_t alignment, void** ptr) {
//     if (!ptr || size == 0 || alignment == 0 || (alignment & (alignment - 1)) != 0) {
//         return RAM_ERROR_INVALID_PARAMETER;
//     }
    
// #if defined(RAM_PLATFORM_WINDOWS)
//     *ptr = _aligned_malloc(size, alignment);
//     return (*ptr != NULL) ? RAM_SUCCESS : RAM_ERROR_ALLOCATION_FAILED;
    
// #elif defined(RAM_PLATFORM_LINUX) || defined(RAM_PLATFORM_MACOS)
//     int result = posix_memalign(ptr, alignment, size);
//     if (result == 0) {
//         return RAM_SUCCESS;
//     } else if (result == ENOMEM) {
//         return RAM_ERROR_INSUFFICIENT_MEMORY;
//     } else {
//         return RAM_ERROR_INVALID_PARAMETER;
//     }
    
// #else
//     // Fallback: use malloc with manual alignment
//     void* raw_ptr = malloc(size + alignment - 1 + sizeof(void*));
//     if (!raw_ptr) {
//         return RAM_ERROR_ALLOCATION_FAILED;
//     }
    
//     char* aligned_ptr = (char*)raw_ptr + sizeof(void*);
//     aligned_ptr += alignment - ((uintptr_t)aligned_ptr % alignment);
//     ((void**)aligned_ptr)[-1] = raw_ptr;
//     *ptr = aligned_ptr;
//     return RAM_SUCCESS;
// #endif
// }

// /**
//  * Free aligned memory
//  * @param ptr Pointer to memory allocated with ram_allocate_aligned
//  * @return RAM_SUCCESS on success, error code on failure
//  */
// static inline ram_error_t ram_free_aligned(void* ptr) {
//     if (!ptr) return RAM_ERROR_INVALID_PARAMETER;
    
// #if defined(RAM_PLATFORM_WINDOWS)
//     _aligned_free(ptr);
//     return RAM_SUCCESS;
    
// #elif defined(RAM_PLATFORM_LINUX) || defined(RAM_PLATFORM_MACOS)
//     free(ptr);
//     return RAM_SUCCESS;
    
// #else
//     // Fallback: retrieve original pointer and free
//     void* raw_ptr = ((void**)ptr)[-1];
//     free(raw_ptr);
//     return RAM_SUCCESS;
// #endif
// }

// /**
//  * Allocate large pages (huge pages)
//  * @param size Size in bytes to allocate
//  * @param ptr [out] Pointer to allocated memory
//  * @return RAM_SUCCESS on success, error code on failure
//  */
// static inline ram_error_t ram_allocate_large_pages(size_t size, void** ptr) {
//     if (!ptr || size == 0) {
//         return RAM_ERROR_INVALID_PARAMETER;
//     }
    
// #if defined(RAM_PLATFORM_WINDOWS)
//     // Enable SeLockMemoryPrivilege first (requires admin rights)
//     *ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE | MEM_LARGE_PAGES, PAGE_READWRITE);
//     return (*ptr != NULL) ? RAM_SUCCESS : RAM_ERROR_ALLOCATION_FAILED;
    
// #elif defined(RAM_PLATFORM_LINUX)
//     *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
//     if (*ptr == MAP_FAILED) {
//         *ptr = NULL;
//         return RAM_ERROR_ALLOCATION_FAILED;
//     }
//     return RAM_SUCCESS;
    
// #elif defined(RAM_PLATFORM_MACOS)
//     // macOS doesn't have huge pages, use regular allocation
//     *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
//     if (*ptr == MAP_FAILED) {
//         *ptr = NULL;
//         return RAM_ERROR_ALLOCATION_FAILED;
//     }
//     return RAM_SUCCESS;
    
// #else
//     return RAM_ERROR_NOT_SUPPORTED;
// #endif
// }

// /**
//  * Free large pages
//  * @param ptr Pointer to memory allocated with ram_allocate_large_pages
//  * @param size Size of the allocated memory
//  * @return RAM_SUCCESS on success, error code on failure
//  */
// static inline ram_error_t ram_free_large_pages(void* ptr, size_t size) {
//     if (!ptr || size == 0) {
//         return RAM_ERROR_INVALID_PARAMETER;
//     }
    
// #if defined(RAM_PLATFORM_WINDOWS)
//     BOOL result = VirtualFree(ptr, 0, MEM_RELEASE);
//     return result ? RAM_SUCCESS : RAM_ERROR_SYSTEM_ERROR;
    
// #elif defined(RAM_PLATFORM_LINUX) || defined(RAM_PLATFORM_MACOS)
//     int result = munmap(ptr, size);
//     return (result == 0) ? RAM_SUCCESS : RAM_ERROR_SYSTEM_ERROR;
    
// #else
//     return RAM_ERROR_NOT_SUPPORTED;
// #endif
// }

// /* ----------------------------- Memory Protection ------------------------------------ */

// /**
//  * Change memory protection
//  * @param ptr Pointer to memory region
//  * @param size Size of memory region
//  * @param protection New protection flags
//  * @return RAM_SUCCESS on success, error code on failure
//  */
// static inline ram_error_t ram_protect_memory(void* ptr, size_t size, ram_protection_t protection) {
//     if (!ptr || size == 0) {
//         return RAM_ERROR_INVALID_PARAMETER;
//     }
    
// #if defined(RAM_PLATFORM_WINDOWS)
//     DWORD win_protection = 0;
//     switch (protection) {
//         case RAM_PROTECTION_NONE: win_protection = PAGE_NOACCESS; break;
//         case RAM_PROTECTION_READ: win_protection = PAGE_READONLY; break;
//         case RAM_PROTECTION_READ_WRITE: win_protection = PAGE_READWRITE; break;
//         case RAM_PROTECTION_READ_EXECUTE: win_protection = PAGE_EXECUTE_READ; break;
//         case RAM_PROTECTION_READ_WRITE_EXECUTE: win_protection = PAGE_EXECUTE_READWRITE; break;
//         default: return RAM_ERROR_INVALID_PARAMETER;
//     }
    
//     DWORD old_protection;
//     BOOL result = VirtualProtect(ptr, size, win_protection, &old_protection);
//     return result ? RAM_SUCCESS : RAM_ERROR_PROTECTION_VIOLATION;
    
// #elif defined(RAM_PLATFORM_LINUX) || defined(RAM_PLATFORM_MACOS)
//     int posix_protection = PROT_NONE;
//     if (protection & RAM_PROTECTION_READ) posix_protection |= PROT_READ;
//     if (protection & RAM_PROTECTION_WRITE) posix_protection |= PROT_WRITE;
//     if (protection & RAM_PROTECTION_EXECUTE) posix_protection |= PROT_EXEC;
    
//     int result = mprotect(ptr, size, posix_protection);
//     return (result == 0) ? RAM_SUCCESS : RAM_ERROR_PROTECTION_VIOLATION;
    
// #else
//     return RAM_ERROR_NOT_SUPPORTED;
// #endif
// }

// /* ----------------------------- Memory Operations ------------------------------------ */

// /**
//  * Lock memory in physical RAM (prevent swapping)
//  * @param ptr Pointer to memory region
//  * @param size Size of memory region
//  * @return RAM_SUCCESS on success, error code on failure
//  */
// static inline ram_error_t ram_lock_memory(void* ptr, size_t size) {
//     if (!ptr || size == 0) {
//         return RAM_ERROR_INVALID_PARAMETER;
//     }
    
// #if defined(RAM_PLATFORM_WINDOWS)
//     BOOL result = VirtualLock(ptr, size);
//     return result ? RAM_SUCCESS : RAM_ERROR_SYSTEM_ERROR;
    
// #elif defined(RAM_PLATFORM_LINUX) || defined(RAM_PLATFORM_MACOS)
//     int result = mlock(ptr, size);
//     return (result == 0) ? RAM_SUCCESS : RAM_ERROR_SYSTEM_ERROR;
    
// #else
//     return RAM_ERROR_NOT_SUPPORTED;
// #endif
// }

// /**
//  * Unlock memory (allow swapping)
//  * @param ptr Pointer to memory region
//  * @param size Size of memory region
//  * @return RAM_SUCCESS on success, error code on failure
//  */
// static inline ram_error_t ram_unlock_memory(void* ptr, size_t size) {
//     if (!ptr || size == 0) {
//         return RAM_ERROR_INVALID_PARAMETER;
//     }
    
// #if defined(RAM_PLATFORM_WINDOWS)
//     BOOL result = VirtualUnlock(ptr, size);
//     return result ? RAM_SUCCESS : RAM_ERROR_SYSTEM_ERROR;
    
// #elif defined(RAM_PLATFORM_LINUX) || defined(RAM_PLATFORM_MACOS)
//     int result = munlock(ptr, size);
//     return (result == 0) ? RAM_SUCCESS : RAM_ERROR_SYSTEM_ERROR;
    
// #else
//     return RAM_ERROR_NOT_SUPPORTED;
// #endif
// }

// /**
//  * Prefetch memory into cache
//  * @param ptr Pointer to memory region
//  * @param size Size of memory region
//  * @return RAM_SUCCESS on success, error code on failure
//  */
// static inline ram_error_t ram_prefetch_memory(const void* ptr, size_t size) {
//     if (!ptr || size == 0) {
//         return RAM_ERROR_INVALID_PARAMETER;
//     }
    
// #if defined(RAM_PLATFORM_WINDOWS)
//     WIN32_MEMORY_RANGE_ENTRY range;
//     range.VirtualAddress = (PVOID)ptr;
//     range.NumberOfBytes = size;
    
//     BOOL result = PrefetchVirtualMemory(GetCurrentProcess(), 1, &range, 0);
//     return result ? RAM_SUCCESS : RAM_ERROR_SYSTEM_ERROR;
    
// #elif defined(RAM_PLATFORM_LINUX) || defined(RAM_PLATFORM_MACOS)
//     // Use madvise with MADV_WILLNEED
//     int result = madvise((void*)ptr, size, MADV_WILLNEED);
//     return (result == 0) ? RAM_SUCCESS : RAM_ERROR_SYSTEM_ERROR;
    
// #else
//     // Manual prefetch by touching memory
//     const char* byte_ptr = (const char*)ptr;
//     volatile char dummy;
//     for (size_t i = 0; i < size; i += 64) { // Assume 64-byte cache lines
//         dummy = byte_ptr[i];
//     }
//     (void)dummy; // Suppress unused variable warning
//     return RAM_SUCCESS;
// #endif
// }

// /* ----------------------------- Utility Functions ------------------------------------ */

// /**
//  * Convert RAM type to string
//  * @param type RAM type
//  * @return String representation of RAM type
//  */
// static inline const char* ram_type_to_string(ram_type_t type) {
//     switch (type) {
//         case RAM_TYPE_DDR: return "DDR";
//         case RAM_TYPE_DDR2: return "DDR2";
//         case RAM_TYPE_DDR3: return "DDR3";
//         case RAM_TYPE_DDR4: return "DDR4";
//         case RAM_TYPE_DDR5: return "DDR5";
//         case RAM_TYPE_SDRAM: return "SDRAM";
//         case RAM_TYPE_SRAM: return "SRAM";
//         case RAM_TYPE_RDRAM: return "RDRAM";
//         default: return "Unknown";
//     }
// }

// /**
//  * Format memory size to human-readable string
//  * @param bytes Size in bytes
//  * @param buffer [out] Buffer to store formatted string
//  * @param buffer_size Size of buffer
//  * @return RAM_SUCCESS on success, error code on failure
//  */
// static inline ram_error_t ram_format_size(uint64_t bytes, char* buffer, size_t buffer_size) {
//     if (!buffer || buffer_size == 0) {
//         return RAM_ERROR_INVALID_PARAMETER;
//     }
    
//     if (bytes >= RAM_BYTES_PER_GB) {
//         double gb = (double)bytes / RAM_BYTES_PER_GB;
//         snprintf(buffer, buffer_size, "%.2f GB", gb);
//     } else if (bytes >= RAM_BYTES_PER_MB) {
//         double mb = (double)bytes / RAM_BYTES_PER_MB;
//         snprintf(buffer, buffer_size, "%.2f MB", mb);
//     } else if (bytes >= RAM_BYTES_PER_KB) {
//         double kb = (double)bytes / RAM_BYTES_PER_KB;
//         snprintf(buffer, buffer_size, "%.2f KB", kb);
//     } else {
//         snprintf(buffer, buffer_size, "%llu bytes", (unsigned long long)bytes);
//     }
    
//     return RAM_SUCCESS;
// }

// /**
//  * Get memory page size
//  * @param page_size [out] Page size in bytes
//  * @return RAM_SUCCESS on success, error code on failure
//  */
// static inline ram_error_t ram_get_page_size(uint32_t* page_size) {
//     if (!page_size) {
//         return RAM_ERROR_INVALID_PARAMETER;
//     }
    
// #if defined(RAM_PLATFORM_WINDOWS)
//     SYSTEM_INFO sys_info;
//     GetSystemInfo(&sys_info);
//     *page_size = sys_info.dwPageSize;
//     return RAM_SUCCESS;
    
// #elif defined(RAM_PLATFORM_LINUX) || defined(RAM_PLATFORM_MACOS)
//     long size = sysconf(_SC_PAGESIZE);
//     if (size > 0) {
//         *page_size = (uint32_t)size;
//         return RAM_SUCCESS;
//     }
//     return RAM_ERROR_SYSTEM_ERROR;
    
// #else
//     *page_size = 4096; // Default assumption
//     return RAM_SUCCESS;
// #endif
// }

// /**
//  * Check if address is valid and accessible
//  * @param ptr Pointer to check
//  * @param size Size of memory region to check
//  * @param for_write True to check write access, false for read access
//  * @param is_valid [out] True if memory is valid and accessible
//  * @return RAM_SUCCESS on success, error code on failure
//  */
// static inline ram_error_t ram_is_valid_address(const void* ptr, size_t size, 
//                                                bool for_write, bool* is_valid) {
//     if (!ptr || !is_valid || size == 0) {
//         return RAM_ERROR_INVALID_PARAMETER;
//     }
    
//     *is_valid = false;
    
// #if defined(RAM_PLATFORM_WINDOWS)
//     MEMORY_BASIC_INFORMATION mbi;
//     SIZE_T result = VirtualQuery(ptr, &mbi, sizeof(mbi));
    
//     if (result == sizeof(mbi) && mbi.State == MEM_COMMIT) {
//         if (for_write) {
//             *is_valid = (mbi.Protect & (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE)) != 0;
//         } else {
//             *is_valid = (mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | 
//                                        PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)) != 0;
//         }
//     }
    
//     return RAM_SUCCESS;
    
// #elif defined(RAM_PLATFORM_LINUX) || defined(RAM_PLATFORM_MACOS)
//     // Try to access the memory using system calls
//     // This is a simplified check
//     if (for_write) {
//         // Check write access by trying to mprotect
//         int result = mprotect((void*)ptr, size, PROT_READ | PROT_WRITE);
//         *is_valid = (result == 0);
//     } else {
//         // Check read access by trying mincore or similar
//         *is_valid = true; // Simplified - assume readable if not null
//     }
    
//     return RAM_SUCCESS;
    
// #else
//     // Basic null pointer check
//     *is_valid = (ptr != NULL);
//     return RAM_SUCCESS;
// #endif
// }

// /**
//  * Get current process memory usage
//  * @param stats [out] Memory usage statistics
//  * @return RAM_SUCCESS on success, error code on failure
//  */
// static inline ram_error_t ram_get_process_usage(ram_statistics_t* stats) {
//     if (!stats) {
//         return RAM_ERROR_INVALID_PARAMETER;
//     }
    
//     memset(stats, 0, sizeof(ram_statistics_t));
    
// #if defined(RAM_PLATFORM_WINDOWS)
//     PROCESS_MEMORY_COUNTERS_EX pmc;
//     if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
//         stats->current_usage = pmc.WorkingSetSize;
//         stats->peak_usage = pmc.PeakWorkingSetSize;
//         stats->page_faults = pmc.PageFaultCount;
//         stats->page_file_usage = pmc.PagefileUsage;
//         return RAM_SUCCESS;
//     }
//     return RAM_ERROR_SYSTEM_ERROR;
    
// #elif defined(RAM_PLATFORM_LINUX)
//     FILE* status = fopen("/proc/self/status", "r");
//     if (!status) {
//         return RAM_ERROR_SYSTEM_ERROR;
//     }
    
//     char line[256];
//     while (fgets(line, sizeof(line), status)) {
//         unsigned long value;
//         if (sscanf(line, "VmRSS: %lu kB", &value) == 1) {
//             stats->current_usage = value * 1024;
//         } else if (sscanf(line, "VmHWM: %lu kB", &value) == 1) {
//             stats->peak_usage = value * 1024;
//         }
//     }
    
//     fclose(status);
//     return RAM_SUCCESS;
    
// #elif defined(RAM_PLATFORM_MACOS)
//     struct mach_task_basic_info info;
//     mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    
//     if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, 
//                   (task_info_t)&info, &count) == KERN_SUCCESS) {
//         stats->current_usage = info.resident_size;
//         return RAM_SUCCESS;
//     }
//     return RAM_ERROR_SYSTEM_ERROR;
    
// #else
//     return RAM_ERROR_NOT_SUPPORTED;
// #endif
// }

// #ifdef __cplusplus
// }
// #endif

#endif // RAM_H