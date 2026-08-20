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
#ifndef RAM_H
#define RAM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HJ_RAM_API
#if defined(HJ_RAM_STATIC)
#define HJ_RAM_API static inline
#else
#define HJ_RAM_API extern
#endif
#endif

#define HJ_RAM_MAX_MODULES 8
#define HJ_RAM_MAX_MANUFACTURER_LENGTH 64
#define HJ_RAM_MAX_PART_NUMBER_LENGTH 32
#define HJ_RAM_MAX_SERIAL_LENGTH 32
#define HJ_RAM_BYTES_PER_KB 1024ULL
#define HJ_RAM_BYTES_PER_MB (1024ULL * 1024ULL)
#define HJ_RAM_BYTES_PER_GB (1024ULL * 1024ULL * 1024ULL)

typedef enum
{
    HJ_RAM_SUCCESS                  = 0,
    HJ_RAM_ERR_INVALID_PARAMETER    = -1,
    HJ_RAM_ERR_ACCESS_DENIED        = -2,
    HJ_RAM_ERR_NOT_FOUND            = -3,
    HJ_RAM_ERR_INSUFFICIENT_BUFFER  = -4,
    HJ_RAM_ERR_SYSTEM_ERROR         = -5,
    HJ_RAM_ERR_NOT_SUPPORTED        = -6,
    HJ_RAM_ERR_INSUFFICIENT_MEMORY  = -7,
    HJ_RAM_ERR_ALLOCATION_FAILED    = -8,
    HJ_RAM_ERR_INVALID_ADDRESS      = -9,
    HJ_RAM_ERR_PROTECTION_VIOLATION = -10
} hj_ram_err_t;

typedef enum
{
    HJ_RAM_TYPE_UNKNOWN = 0,
    HJ_RAM_TYPE_DDR     = 1,
    HJ_RAM_TYPE_DDR2    = 2,
    HJ_RAM_TYPE_DDR3    = 3,
    HJ_RAM_TYPE_DDR4    = 4,
    HJ_RAM_TYPE_DDR5    = 5,
    HJ_RAM_TYPE_SDRAM   = 6,
    HJ_RAM_TYPE_SRAM    = 7,
    HJ_RAM_TYPE_RDRAM   = 8
} hj_ram_type_t;

typedef enum
{
    HJ_RAM_PROTECTION_NONE               = 0,
    HJ_RAM_PROTECTION_READ               = 1,
    HJ_RAM_PROTECTION_WRITE              = 2,
    HJ_RAM_PROTECTION_EXECUTE            = 4,
    HJ_RAM_PROTECTION_READ_WRITE         = 3,
    HJ_RAM_PROTECTION_READ_EXECUTE       = 5,
    HJ_RAM_PROTECTION_WRITE_EXECUTE      = 6,
    HJ_RAM_PROTECTION_READ_WRITE_EXECUTE = 7
} hj_ram_protection_t;

typedef struct
{
    char manufacturer[HJ_RAM_MAX_MANUFACTURER_LENGTH]; /* Manufacturer name */
    char part_number[HJ_RAM_MAX_PART_NUMBER_LENGTH];   /* Part number */
    char serial_number[HJ_RAM_MAX_SERIAL_LENGTH];      /* Serial number */
    hj_ram_type_t type;          /* RAM type (DDR, DDR2, etc.) */
    uint64_t      capacity;      /* Capacity in bytes */
    uint32_t      speed;         /* Speed in MHz */
    uint32_t      voltage;       /* Voltage in millivolts */
    uint32_t      slot_number;   /* Physical slot number */
    bool          is_ecc;        /* ECC support */
    bool          is_registered; /* Registered memory */
} hj_ram_module_info_t;

typedef struct
{
    uint64_t total_physical;     /* Total physical RAM in bytes */
    uint64_t available_physical; /* Available physical RAM in bytes */
    uint64_t used_physical;      /* Used physical RAM in bytes */
    uint64_t total_virtual;      /* Total virtual memory in bytes */
    uint64_t available_virtual;  /* Available virtual memory in bytes */
    uint64_t used_virtual;       /* Used virtual memory in bytes */
    uint64_t total_swap;         /* Total swap space in bytes */
    uint64_t available_swap;     /* Available swap space in bytes */
    uint64_t used_swap;          /* Used swap space in bytes */
    uint32_t page_size;          /* Memory page size in bytes */
    uint32_t large_page_size;    /* Large page size in bytes */
    double   memory_load;        /* Memory load percentage (0-100) */
    uint32_t module_count;       /* Number of RAM modules */
} hj_ram_system_info_t;

typedef struct
{
    uint64_t allocations_count;   /* Number of allocations */
    uint64_t deallocations_count; /* Number of deallocations */
    uint64_t bytes_allocated;     /* Total bytes allocated */
    uint64_t bytes_deallocated;   /* Total bytes deallocated */
    uint64_t peak_usage;          /* Peak memory usage */
    uint64_t current_usage;       /* Current memory usage */
    uint32_t page_faults;         /* Page fault count */
    uint32_t page_file_usage;     /* Page file usage count */
} hj_ram_statistics_t;

typedef struct
{
    void               *base_address; /* Base address of region */
    uint64_t            size;         /* Size of region in bytes */
    hj_ram_protection_t protection;   /* Memory protection flags */
    bool                is_committed; /* Is memory committed */
    bool                is_mapped;    /* Is memory mapped */
    bool                is_shared;    /* Is shared memory */
} hj_ram_region_info_t;


// ------------------------ RAM API Declarations ------------------------

HJ_RAM_API hj_ram_err_t hj_ram_init(void);
HJ_RAM_API void         hj_ram_cleanup(void);
HJ_RAM_API hj_ram_err_t hj_ram_get_system_info(hj_ram_system_info_t *info);
HJ_RAM_API hj_ram_err_t hj_ram_get_modules(hj_ram_module_info_t *modules,
                                           uint32_t              max_modules,
                                           uint32_t             *actual_count);
HJ_RAM_API hj_ram_err_t hj_ram_allocate_aligned(size_t size,
                                                size_t alignment,
                                                void **ptr);
HJ_RAM_API hj_ram_err_t hj_ram_free_aligned(void *ptr);
HJ_RAM_API hj_ram_err_t hj_ram_allocate_large_pages(size_t size, void **ptr);
HJ_RAM_API hj_ram_err_t hj_ram_free_large_pages(void *ptr, size_t size);
HJ_RAM_API hj_ram_err_t hj_ram_protect_memory(void               *ptr,
                                              size_t              size,
                                              hj_ram_protection_t protection);
HJ_RAM_API hj_ram_err_t hj_ram_lock_memory(void *ptr, size_t size);
HJ_RAM_API hj_ram_err_t hj_ram_unlock_memory(void *ptr, size_t size);
HJ_RAM_API hj_ram_err_t hj_ram_prefetch_memory(const void *ptr, size_t size);
HJ_RAM_API const char  *hj_ram_type_to_string(hj_ram_type_t type);
HJ_RAM_API hj_ram_err_t hj_ram_format_size(uint64_t bytes,
                                           char    *buffer,
                                           size_t   buffer_size);
HJ_RAM_API hj_ram_err_t hj_ram_get_page_size(uint32_t *page_size);
HJ_RAM_API hj_ram_err_t hj_ram_is_valid_address(const void *ptr,
                                                size_t      size,
                                                bool        for_write,
                                                bool       *is_valid);
HJ_RAM_API hj_ram_err_t hj_ram_get_process_usage(hj_ram_statistics_t *stats);

#ifdef __cplusplus
}
#endif

#endif // RAM_H

// --------------------- Implementation -------------------------
#if (defined(HJ_RAM_IMPL) || defined(HJ_RAM_STATIC))                           \
    && !defined(HJ_RAM_IMPL_DONE)
#define HJ_RAM_IMPL_DONE

#if defined(_WIN32) || defined(_WIN64)
#define RAM_PLATFORM_WINDOWS 1
#include <windows.h>
#include <psapi.h>
#include <memoryapi.h>
#pragma comment(lib, "psapi.lib")
#elif defined(__linux__)
#define RAM_PLATFORM_LINUX 1
#include <sys/sysinfo.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#elif defined(__APPLE__)
#define RAM_PLATFORM_MACOS 1
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/mman.h>
#include <mach/mach.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#include <unistd.h>
#else
#define RAM_PLATFORM_UNKNOWN 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

// --------------- RAM API Implementation -----------------
HJ_RAM_API hj_ram_err_t hj_ram_init(void)
{
    return HJ_RAM_SUCCESS;
}

HJ_RAM_API void hj_ram_cleanup(void)
{
}

HJ_RAM_API hj_ram_err_t hj_ram_get_system_info(hj_ram_system_info_t *info)
{
    if(!info)
        return HJ_RAM_ERR_INVALID_PARAMETER;

    memset(info, 0, sizeof(hj_ram_system_info_t));

#if defined(RAM_PLATFORM_WINDOWS)
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if(!GlobalMemoryStatusEx(&memStatus))
    {
        if(GetLastError() == ERROR_ACCESS_DENIED)
            return HJ_RAM_ERR_ACCESS_DENIED;
        return HJ_RAM_ERR_SYSTEM_ERROR;
    }

    info->total_physical     = memStatus.ullTotalPhys;
    info->available_physical = memStatus.ullAvailPhys;
    info->used_physical =
        (info->total_physical >= info->available_physical)
            ? (info->total_physical - info->available_physical)
            : 0;

    info->total_virtual     = memStatus.ullTotalVirtual;
    info->available_virtual = memStatus.ullAvailVirtual;
    info->used_virtual = (info->total_virtual >= info->available_virtual)
                             ? (info->total_virtual - info->available_virtual)
                             : 0;

    uint64_t total_page  = memStatus.ullTotalPageFile;
    uint64_t avail_page  = memStatus.ullAvailPageFile;
    info->total_swap     = (total_page >= info->total_physical)
                               ? (total_page - info->total_physical)
                               : 0;
    info->available_swap = (avail_page >= info->available_physical)
                               ? (avail_page - info->available_physical)
                               : 0;
    info->used_swap      = (info->total_swap >= info->available_swap)
                               ? (info->total_swap - info->available_swap)
                               : 0;

    info->memory_load = (double) memStatus.dwMemoryLoad;

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    info->page_size = sysInfo.dwPageSize;

    SIZE_T largePageSize  = GetLargePageMinimum();
    info->large_page_size = (uint32_t) largePageSize;

    return HJ_RAM_SUCCESS;

#elif defined(RAM_PLATFORM_LINUX)
    struct sysinfo si;
    if(sysinfo(&si) != 0)
    {
        if(errno == EACCES || errno == EPERM)
            return HJ_RAM_ERR_ACCESS_DENIED;
        return HJ_RAM_ERR_SYSTEM_ERROR;
    }

    info->total_physical     = si.totalram * si.mem_unit;
    info->available_physical = si.freeram * si.mem_unit;
    info->used_physical =
        (info->total_physical >= info->available_physical)
            ? (info->total_physical - info->available_physical)
            : 0;

    info->total_swap     = si.totalswap * si.mem_unit;
    info->available_swap = si.freeswap * si.mem_unit;
    info->used_swap      = (info->total_swap >= info->available_swap)
                               ? (info->total_swap - info->available_swap)
                               : 0;

    if(info->total_physical > 0)
        info->memory_load =
            ((double) info->used_physical / info->total_physical) * 100.0;

    long page_size = sysconf(_SC_PAGESIZE);
    if(page_size > 0)
        info->page_size = (uint32_t) page_size;

    FILE *meminfo = fopen("/proc/meminfo", "r");
    if(!meminfo)
    {
        if(errno == ENOENT)
            return HJ_RAM_ERR_NOT_FOUND;
        if(errno == EACCES || errno == EPERM)
            return HJ_RAM_ERR_ACCESS_DENIED;
        return HJ_RAM_ERR_SYSTEM_ERROR;
    }

    char line[256];
    while(fgets(line, sizeof(line), meminfo))
    {
        unsigned long value;
        if(sscanf(line, "VmallocTotal: %lu kB", &value) == 1)
            info->total_virtual = value * 1024;
        else if(sscanf(line, "VmallocUsed: %lu kB", &value) == 1)
            info->used_virtual = value * 1024;
    }
    fclose(meminfo);

    info->available_virtual = (info->total_virtual >= info->used_virtual)
                                  ? (info->total_virtual - info->used_virtual)
                                  : 0;

    return HJ_RAM_SUCCESS;

#elif defined(RAM_PLATFORM_MACOS)
    int    mib[2];
    size_t length;

    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    length = sizeof(info->total_physical);
    if(sysctl(mib, 2, &info->total_physical, &length, NULL, 0) != 0)
        return HJ_RAM_ERR_SYSTEM_ERROR;

    mib[1] = HW_PAGESIZE;
    length = sizeof(info->page_size);
    if(sysctl(mib, 2, &info->page_size, &length, NULL, 0) != 0)
        info->page_size = 4096;

    vm_size_t              page_size_vm;
    vm_statistics64_data_t vm_stat;
    mach_msg_type_number_t count = sizeof(vm_stat) / sizeof(natural_t);
    if(host_page_size(mach_host_self(), &page_size_vm) == KERN_SUCCESS
       && host_statistics64(mach_host_self(),
                            HOST_VM_INFO,
                            (host_info64_t) &vm_stat,
                            &count)
              == KERN_SUCCESS)
    {
        info->available_physical = (uint64_t) vm_stat.free_count * page_size_vm;
        info->used_physical =
            (info->total_physical >= info->available_physical)
                ? (info->total_physical - info->available_physical)
                : 0;
        if(info->total_physical > 0)
            info->memory_load =
                ((double) info->used_physical / info->total_physical) * 100.0;
    }
    return HJ_RAM_SUCCESS;

#else
    return HJ_RAM_ERR_NOT_SUPPORTED;

#endif
}

static hj_ram_type_t smbios_parse_memory_type(uint8_t smbios_type)
{
    switch(smbios_type)
    {
        case 0x14:
            return HJ_RAM_TYPE_DDR;
        case 0x15:
            return HJ_RAM_TYPE_DDR2;
        case 0x18:
            return HJ_RAM_TYPE_DDR3;
        case 0x19:
            return HJ_RAM_TYPE_DDR4;
        case 0x22:
            return HJ_RAM_TYPE_DDR5;
        case 0x03:
            return HJ_RAM_TYPE_SDRAM;
        case 0x13:
            return HJ_RAM_TYPE_RDRAM;
        default:
            return HJ_RAM_TYPE_UNKNOWN;
    }
}

static const char *smbios_get_string(const uint8_t *struct_ptr,
                                     uint8_t        length,
                                     const uint8_t *end,
                                     uint8_t        string_index)
{
    if(string_index == 0 || !struct_ptr || !end || struct_ptr >= end)
        return "Unknown";

    const uint8_t *p = struct_ptr + length;
    if(p >= end)
        return "Unknown";

    int current_index = 1;
    while(p < end && *p != '\0')
    {
        if(current_index == string_index)
        {
            const uint8_t *q = p;
            while(q < end && *q != '\0')
                q++;
            if(q < end)
                return (const char *) p;
            return "Unknown";
        }

        while(p < end && *p != '\0')
            p++;

        if(p < end)
            p++;

        current_index++;
    }
    return "Unknown";
}

HJ_RAM_API hj_ram_err_t hj_ram_get_modules(hj_ram_module_info_t *modules,
                                           uint32_t              max_modules,
                                           uint32_t             *actual_count)
{
    if(!modules || !actual_count || max_modules == 0)
        return HJ_RAM_ERR_INVALID_PARAMETER;

    *actual_count = 0;

#if defined(RAM_PLATFORM_WINDOWS)
    DWORD smbios_size = GetSystemFirmwareTable('RSMB', 0, NULL, 0);
    if(smbios_size > 0)
    {
        uint8_t *smbios_buffer = (uint8_t *) malloc(smbios_size);
        if(smbios_buffer)
        {
            if(GetSystemFirmwareTable('RSMB', 0, smbios_buffer, smbios_size)
               == smbios_size)
            {
                uint8_t *p   = smbios_buffer + 8;
                uint8_t *end = smbios_buffer + smbios_size;

                while(p + 2 <= end && *actual_count < max_modules)
                {
                    uint8_t type   = p[0];
                    uint8_t length = p[1];

                    if(type == 127)
                        break;

                    if(length < 4 || p + length > end)
                        break;

                    if(type == 17)
                    {
                        hj_ram_module_info_t *mod = &modules[*actual_count];
                        memset(mod, 0, sizeof(hj_ram_module_info_t));

                        // Capacity (Offset 0x0C)
                        if(length >= 0x0E)
                        {
                            uint16_t size_field = *((uint16_t *) (p + 0x0C));
                            if(size_field != 0 && size_field != 0xFFFF)
                            {
                                if(size_field & 0x8000)
                                    mod->capacity =
                                        (uint64_t) (size_field & 0x7FFF)
                                        * 1024ULL;
                                else
                                    mod->capacity = (uint64_t) size_field
                                                    * 1024ULL * 1024ULL;
                            }
                        }

                        // Extended Size (Offset 0x1C) for capacity >= 32GB
                        if(length >= 0x20)
                        {
                            uint32_t ext_size = *((uint32_t *) (p + 0x1C));
                            if(ext_size != 0 && ext_size != 0xFFFFFFFF)
                            {
                                if(ext_size & 0x80000000)
                                    mod->capacity =
                                        (uint64_t) (ext_size & 0x7FFFFFFF)
                                        * 1024ULL;
                                else
                                    mod->capacity =
                                        (uint64_t) ext_size * 1024ULL * 1024ULL;
                            }
                        }

                        if(length >= 0x0C)
                        {
                            uint16_t total_width = *((uint16_t *) (p + 0x08));
                            uint16_t data_width  = *((uint16_t *) (p + 0x0A));
                            if(total_width != 0xFFFF && data_width != 0xFFFF
                               && total_width > data_width)
                                mod->is_ecc = true;
                        }

                        if(length >= 0x13)
                            mod->type = smbios_parse_memory_type(p[0x12]);

                        if(length >= 0x15)
                        {
                            uint16_t type_detail = *((uint16_t *) (p + 0x13));
                            if(type_detail & 0x0008)
                                mod->is_registered = true;
                        }

                        const char *mfr =
                            (length >= 0x0F)
                                ? smbios_get_string(p, length, end, p[0x0E])
                                : "Unknown";
                        const char *sn =
                            (length >= 0x11)
                                ? smbios_get_string(p, length, end, p[0x10])
                                : "Unknown";
                        const char *part =
                            (length >= 0x12)
                                ? smbios_get_string(p, length, end, p[0x11])
                                : "Unknown";

                        strcpy_s(mod->manufacturer,
                                 sizeof(mod->manufacturer),
                                 mfr);
                        strcpy_s(mod->serial_number,
                                 sizeof(mod->serial_number),
                                 sn);
                        strcpy_s(mod->part_number,
                                 sizeof(mod->part_number),
                                 part);

                        // Speed (Offset 0x15)
                        if(length >= 0x17)
                        {
                            mod->speed = *((uint16_t *) (p + 0x15));
                        }

                        // Configured Clock Speed (Offset 0x20)
                        if(length >= 0x22)
                        {
                            uint16_t cfg_speed = *((uint16_t *) (p + 0x20));
                            if(cfg_speed != 0 && cfg_speed != 0xFFFF)
                                mod->speed = cfg_speed;
                        }

                        if(length >= 0x28)
                        {
                            uint16_t voltage = *((uint16_t *) (p + 0x26));
                            if(voltage != 0 && voltage != 0xFFFF)
                                mod->voltage = voltage;
                        }

                        mod->slot_number = *actual_count + 1;
                        (*actual_count)++;
                    }

                    uint8_t *next = p + length;
                    while(next < end - 1 && !(next[0] == 0 && next[1] == 0))
                    {
                        next++;
                    }
                    next += 2;
                    if(next > end)
                        break;
                    p = next;
                }
            }
            free(smbios_buffer);
        }
    }

    if(*actual_count == 0 && max_modules >= 1)
    {
        hj_ram_system_info_t sys_info;
        if(hj_ram_get_system_info(&sys_info) == HJ_RAM_SUCCESS)
        {
            memset(&modules[0], 0, sizeof(hj_ram_module_info_t));
            strcpy_s(modules[0].manufacturer,
                     sizeof(modules[0].manufacturer),
                     "Unknown");
            strcpy_s(modules[0].part_number,
                     sizeof(modules[0].part_number),
                     "System RAM");
            modules[0].type        = HJ_RAM_TYPE_UNKNOWN;
            modules[0].capacity    = sys_info.total_physical;
            modules[0].slot_number = 1;
            *actual_count          = 1;
        }
    }
    return HJ_RAM_SUCCESS;

#elif defined(RAM_PLATFORM_LINUX)
    FILE *f = fopen("/sys/firmware/dmi/tables/DMI", "rb");
    if(!f && (errno == EACCES || errno == EPERM))
        return HJ_RAM_ERR_ACCESS_DENIED;

    if(f)
    {
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        if(fsize <= 0)
        {
            fclose(f);
            return HJ_RAM_ERR_NOT_FOUND;
        }

        uint8_t *dmi_buffer = (uint8_t *) malloc(fsize);
        if(!dmi_buffer)
        {
            fclose(f);
            return HJ_RAM_ERR_INSUFFICIENT_MEMORY;
        }

        if(fread(dmi_buffer, 1, fsize, f) == (size_t) fsize)
        {
            uint8_t *p   = dmi_buffer;
            uint8_t *end = dmi_buffer + fsize;
            while(p + 2 <= end && *actual_count < max_modules)
            {
                uint8_t type   = p[0];
                uint8_t length = p[1];

                if(type == 127)
                    break;

                if(length < 4 || p + length > end)
                    break;

                if(type == 17)
                {
                    hj_ram_module_info_t *mod = &modules[*actual_count];
                    memset(mod, 0, sizeof(hj_ram_module_info_t));

                    // Capacity (Offset 0x0C)
                    if(length >= 0x0E)
                    {
                        uint16_t size_field = *((uint16_t *) (p + 0x0C));
                        if(size_field != 0 && size_field != 0xFFFF)
                        {
                            if(size_field & 0x8000)
                                mod->capacity =
                                    (uint64_t) (size_field & 0x7FFF) * 1024ULL;
                            else
                                mod->capacity =
                                    (uint64_t) size_field * 1024ULL * 1024ULL;
                        }
                    }

                    // Extended Size (Offset 0x1C)
                    if(length >= 0x20)
                    {
                        uint32_t ext_size = *((uint32_t *) (p + 0x1C));
                        if(ext_size != 0 && ext_size != 0xFFFFFFFF)
                        {
                            if(ext_size & 0x80000000)
                                mod->capacity =
                                    (uint64_t) (ext_size & 0x7FFFFFFF)
                                    * 1024ULL;
                            else
                                mod->capacity =
                                    (uint64_t) ext_size * 1024ULL * 1024ULL;
                        }
                    }

                    if(length >= 0x0C)
                    {
                        uint16_t total_width = *((uint16_t *) (p + 0x08));
                        uint16_t data_width  = *((uint16_t *) (p + 0x0A));
                        if(total_width != 0xFFFF && data_width != 0xFFFF
                           && total_width > data_width)
                        {
                            mod->is_ecc = true;
                        }
                    }

                    if(length >= 0x13)
                        mod->type = smbios_parse_memory_type(p[0x12]);

                    if(length >= 0x15)
                    {
                        uint16_t type_detail = *((uint16_t *) (p + 0x13));
                        if(type_detail & 0x0008)
                            mod->is_registered = true;
                    }

                    const char *mfr =
                        (length >= 0x0F)
                            ? smbios_get_string(p, length, end, p[0x0E])
                            : "Unknown";
                    const char *sn =
                        (length >= 0x11)
                            ? smbios_get_string(p, length, end, p[0x10])
                            : "Unknown";
                    const char *part =
                        (length >= 0x12)
                            ? smbios_get_string(p, length, end, p[0x11])
                            : "Unknown";

                    strncpy(mod->manufacturer,
                            mfr,
                            sizeof(mod->manufacturer) - 1);
                    strncpy(mod->serial_number,
                            sn,
                            sizeof(mod->serial_number) - 1);
                    strncpy(mod->part_number,
                            part,
                            sizeof(mod->part_number) - 1);

                    // Speed (Offset 0x15)
                    if(length >= 0x17)
                    {
                        mod->speed = *((uint16_t *) (p + 0x15));
                    }

                    // Configured Clock Speed (Offset 0x20)
                    if(length >= 0x22)
                    {
                        uint16_t cfg_speed = *((uint16_t *) (p + 0x20));
                        if(cfg_speed != 0 && cfg_speed != 0xFFFF)
                            mod->speed = cfg_speed;
                    }

                    if(length >= 0x28)
                    {
                        uint16_t voltage = *((uint16_t *) (p + 0x26));
                        if(voltage != 0 && voltage != 0xFFFF)
                            mod->voltage = voltage;
                    }

                    mod->slot_number = *actual_count + 1;
                    (*actual_count)++;
                }

                uint8_t *next = p + length;
                while(next < end - 1 && !(next[0] == 0 && next[1] == 0))
                    next++;

                next += 2;
                if(next > end)
                    break;

                p = next;
            }
        }

        free(dmi_buffer);
        fclose(f);
    }

    if(*actual_count == 0 && max_modules >= 1)
    {
        hj_ram_system_info_t sys_info;
        if(hj_ram_get_system_info(&sys_info) == HJ_RAM_SUCCESS)
        {
            memset(&modules[0], 0, sizeof(hj_ram_module_info_t));
            strncpy(modules[0].manufacturer,
                    "Unknown",
                    sizeof(modules[0].manufacturer) - 1);
            strncpy(modules[0].part_number,
                    "System RAM",
                    sizeof(modules[0].part_number) - 1);
            modules[0].type        = HJ_RAM_TYPE_UNKNOWN;
            modules[0].capacity    = sys_info.total_physical;
            modules[0].slot_number = 1;
            *actual_count          = 1;
        }
    }
    return HJ_RAM_SUCCESS;

#elif defined(RAM_PLATFORM_MACOS)
    hj_ram_system_info_t sys_info;
    if(hj_ram_get_system_info(&sys_info) == HJ_RAM_SUCCESS && max_modules >= 1)
    {
        memset(&modules[0], 0, sizeof(hj_ram_module_info_t));
        strncpy(modules[0].manufacturer,
                "Apple",
                sizeof(modules[0].manufacturer) - 1);
        strncpy(modules[0].part_number,
                "Unified Memory",
                sizeof(modules[0].part_number) - 1);
        modules[0].type        = HJ_RAM_TYPE_DDR5;
        modules[0].capacity    = sys_info.total_physical;
        modules[0].slot_number = 1;
        *actual_count          = 1;
        return HJ_RAM_SUCCESS;
    }
    return HJ_RAM_ERR_NOT_SUPPORTED;

#else
    return HJ_RAM_ERR_NOT_SUPPORTED;

#endif
}

HJ_RAM_API hj_ram_err_t hj_ram_allocate_aligned(size_t size,
                                                size_t alignment,
                                                void **ptr)
{
    if(!ptr || size == 0 || alignment == 0
       || (alignment & (alignment - 1)) != 0)
        return HJ_RAM_ERR_INVALID_PARAMETER;

#if defined(RAM_PLATFORM_WINDOWS)
    *ptr = _aligned_malloc(size, alignment);
    return (*ptr != NULL) ? HJ_RAM_SUCCESS : HJ_RAM_ERR_ALLOCATION_FAILED;

#elif defined(RAM_PLATFORM_LINUX) || defined(RAM_PLATFORM_MACOS)
    int result = posix_memalign(ptr, alignment, size);
    if(result == 0)
        return HJ_RAM_SUCCESS;
    else if(result == ENOMEM)
        return HJ_RAM_ERR_INSUFFICIENT_MEMORY;
    else
        return HJ_RAM_ERR_INVALID_PARAMETER;

#else
    size_t total_size = size + alignment - 1 + sizeof(void *);
    void  *raw_ptr    = malloc(total_size);
    if(!raw_ptr)
        return HJ_RAM_ERR_ALLOCATION_FAILED;

    uintptr_t ideal_addr   = (uintptr_t) raw_ptr + sizeof(void *);
    uintptr_t aligned_addr = (ideal_addr + alignment - 1) & ~(alignment - 1);
    *((void **) (aligned_addr - sizeof(void *))) = raw_ptr;
    *ptr                                         = (void *) aligned_addr;
    return HJ_RAM_SUCCESS;

#endif
}

HJ_RAM_API hj_ram_err_t hj_ram_free_aligned(void *ptr)
{
    if(!ptr)
        return HJ_RAM_ERR_INVALID_PARAMETER;

#if defined(RAM_PLATFORM_WINDOWS)
    _aligned_free(ptr);
    return HJ_RAM_SUCCESS;

#elif defined(RAM_PLATFORM_LINUX) || defined(RAM_PLATFORM_MACOS)
    free(ptr);
    return HJ_RAM_SUCCESS;

#else
    void *raw_ptr = ((void **) ptr)[-1];
    free(raw_ptr);
    return HJ_RAM_SUCCESS;

#endif
}

#if defined(RAM_PLATFORM_WINDOWS)
static bool enable_lock_memory_privilege(void)
{
    HANDLE hToken = NULL;
    if(!OpenProcessToken(GetCurrentProcess(),
                         TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                         &hToken))
    {
        return false;
    }

    TOKEN_PRIVILEGES tp;
    LUID             luid;
    if(!LookupPrivilegeValue(NULL, SE_LOCK_MEMORY_NAME, &luid))
    {
        CloseHandle(hToken);
        return false;
    }

    tp.PrivilegeCount           = 1;
    tp.Privileges[0].Luid       = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    BOOL  result = AdjustTokenPrivileges(hToken,
                                         FALSE,
                                         &tp,
                                         sizeof(TOKEN_PRIVILEGES),
                                         NULL,
                                         NULL);
    DWORD err    = GetLastError();
    CloseHandle(hToken);
    if(!result || err == ERROR_NOT_ALL_ASSIGNED)
        return false;

    return true;
}
#endif

HJ_RAM_API hj_ram_err_t hj_ram_allocate_large_pages(size_t size, void **ptr)
{
    if(!ptr || size == 0)
        return HJ_RAM_ERR_INVALID_PARAMETER;

#if defined(RAM_PLATFORM_WINDOWS)
    if(!enable_lock_memory_privilege())
    {
        *ptr = NULL;
        return HJ_RAM_ERR_ACCESS_DENIED;
    }

    *ptr = VirtualAlloc(NULL,
                        size,
                        MEM_COMMIT | MEM_RESERVE | MEM_LARGE_PAGES,
                        PAGE_READWRITE);
    if(!*ptr)
    {
        DWORD err = GetLastError();
        if(err == ERROR_ACCESS_DENIED)
            return HJ_RAM_ERR_ACCESS_DENIED;
        return HJ_RAM_ERR_ALLOCATION_FAILED;
    }
    return HJ_RAM_SUCCESS;

#elif defined(RAM_PLATFORM_LINUX)
    *ptr = mmap(NULL,
                size,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                -1,
                0);
    if(*ptr == MAP_FAILED)
    {
        *ptr = NULL;
        return HJ_RAM_ERR_ALLOCATION_FAILED;
    }

    return HJ_RAM_SUCCESS;

#elif defined(RAM_PLATFORM_MACOS)
    *ptr = mmap(NULL,
                size,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS,
                -1,
                0);
    if(*ptr == MAP_FAILED)
    {
        *ptr = NULL;
        return HJ_RAM_ERR_ALLOCATION_FAILED;
    }

    return HJ_RAM_SUCCESS;

#else
    return HJ_RAM_ERR_NOT_SUPPORTED;

#endif
}

HJ_RAM_API hj_ram_err_t hj_ram_free_large_pages(void *ptr, size_t size)
{
    if(!ptr || size == 0)
        return HJ_RAM_ERR_INVALID_PARAMETER;

#if defined(RAM_PLATFORM_WINDOWS)
    BOOL result = VirtualFree(ptr, 0, MEM_RELEASE);
    return result ? HJ_RAM_SUCCESS : HJ_RAM_ERR_SYSTEM_ERROR;

#elif defined(RAM_PLATFORM_LINUX) || defined(RAM_PLATFORM_MACOS)
    int result = munmap(ptr, size);
    return (result == 0) ? HJ_RAM_SUCCESS : HJ_RAM_ERR_SYSTEM_ERROR;

#else
    return HJ_RAM_ERR_NOT_SUPPORTED;

#endif
}

HJ_RAM_API hj_ram_err_t hj_ram_protect_memory(void               *ptr,
                                              size_t              size,
                                              hj_ram_protection_t protection)
{
    if(!ptr || size == 0)
        return HJ_RAM_ERR_INVALID_PARAMETER;

#if defined(RAM_PLATFORM_WINDOWS)
    DWORD win_protection = 0;
    bool  can_read       = (protection & HJ_RAM_PROTECTION_READ) != 0;
    bool  can_write      = (protection & HJ_RAM_PROTECTION_WRITE) != 0;
    bool  can_exec       = (protection & HJ_RAM_PROTECTION_EXECUTE) != 0;
    if(protection == HJ_RAM_PROTECTION_NONE)
    {
        win_protection = PAGE_NOACCESS;
    } else if(can_exec)
    {
        if(can_read && can_write)
        {
            win_protection = PAGE_EXECUTE_READWRITE;
        } else if(can_read)
        {
            win_protection = PAGE_EXECUTE_READ;
        } else if(can_write)
        {
            win_protection = PAGE_EXECUTE_READWRITE;
        } else
        {
            win_protection = PAGE_EXECUTE;
        }
    } else
    {
        if(can_read && can_write)
        {
            win_protection = PAGE_READWRITE;
        } else if(can_read)
        {
            win_protection = PAGE_READONLY;
        } else if(can_write)
        {
            win_protection = PAGE_READWRITE;
        } else
        {
            win_protection = PAGE_NOACCESS;
        }
    }

    DWORD old_protection;
    BOOL  result = VirtualProtect(ptr, size, win_protection, &old_protection);
    if(!result)
    {
        DWORD err = GetLastError();
        if(err == ERROR_ACCESS_DENIED)
            return HJ_RAM_ERR_ACCESS_DENIED;
        if(err == ERROR_INVALID_ADDRESS || err == ERROR_INVALID_PARAMETER)
            return HJ_RAM_ERR_INVALID_ADDRESS;
        return HJ_RAM_ERR_PROTECTION_VIOLATION;
    }
    return HJ_RAM_SUCCESS;

#elif defined(RAM_PLATFORM_LINUX) || defined(RAM_PLATFORM_MACOS)
    int posix_protection = PROT_NONE;
    if(protection & HJ_RAM_PROTECTION_READ)
        posix_protection |= PROT_READ;

    if(protection & HJ_RAM_PROTECTION_WRITE)
        posix_protection |= PROT_WRITE;

    if(protection & HJ_RAM_PROTECTION_EXECUTE)
        posix_protection |= PROT_EXEC;

    int result = mprotect(ptr, size, posix_protection);
    if(result != 0)
    {
        if(errno == EACCES || errno == EPERM)
            return HJ_RAM_ERR_ACCESS_DENIED;
        if(errno == EFAULT || errno == EINVAL)
            return HJ_RAM_ERR_INVALID_ADDRESS;
        return HJ_RAM_ERR_PROTECTION_VIOLATION;
    }
    return HJ_RAM_SUCCESS;

#else
    return HJ_RAM_ERR_NOT_SUPPORTED;

#endif
}

HJ_RAM_API hj_ram_err_t hj_ram_lock_memory(void *ptr, size_t size)
{
    if(!ptr || size == 0)
        return HJ_RAM_ERR_INVALID_PARAMETER;

#if defined(RAM_PLATFORM_WINDOWS)
    BOOL result = VirtualLock(ptr, size);
    if(!result)
    {
        DWORD err = GetLastError();
        if(err == ERROR_ACCESS_DENIED)
            return HJ_RAM_ERR_ACCESS_DENIED;
        if(err == ERROR_INVALID_ADDRESS)
            return HJ_RAM_ERR_INVALID_ADDRESS;
        return HJ_RAM_ERR_SYSTEM_ERROR;
    }
    return HJ_RAM_SUCCESS;

#elif defined(RAM_PLATFORM_LINUX) || defined(RAM_PLATFORM_MACOS)
    int result = mlock(ptr, size);
    if(result != 0)
    {
        if(errno == EACCES || errno == EPERM)
            return HJ_RAM_ERR_ACCESS_DENIED;
        if(errno == EFAULT || errno == EINVAL)
            return HJ_RAM_ERR_INVALID_ADDRESS;
        return HJ_RAM_ERR_SYSTEM_ERROR;
    }
    return HJ_RAM_SUCCESS;

#else
    return HJ_RAM_ERR_NOT_SUPPORTED;

#endif
}

HJ_RAM_API hj_ram_err_t hj_ram_unlock_memory(void *ptr, size_t size)
{
    if(!ptr || size == 0)
        return HJ_RAM_ERR_INVALID_PARAMETER;

#if defined(RAM_PLATFORM_WINDOWS)
    BOOL result = VirtualUnlock(ptr, size);
    if(!result)
    {
        DWORD err = GetLastError();
        if(err == ERROR_ACCESS_DENIED)
            return HJ_RAM_ERR_ACCESS_DENIED;
        if(err == ERROR_INVALID_ADDRESS)
            return HJ_RAM_ERR_INVALID_ADDRESS;
        return HJ_RAM_ERR_SYSTEM_ERROR;
    }
    return HJ_RAM_SUCCESS;

#elif defined(RAM_PLATFORM_LINUX) || defined(RAM_PLATFORM_MACOS)
    int result = munlock(ptr, size);
    if(result != 0)
    {
        if(errno == EACCES || errno == EPERM)
            return HJ_RAM_ERR_ACCESS_DENIED;
        if(errno == EFAULT || errno == EINVAL)
            return HJ_RAM_ERR_INVALID_ADDRESS;
        return HJ_RAM_ERR_SYSTEM_ERROR;
    }
    return HJ_RAM_SUCCESS;

#else
    return HJ_RAM_ERR_NOT_SUPPORTED;

#endif
}

HJ_RAM_API hj_ram_err_t hj_ram_prefetch_memory(const void *ptr, size_t size)
{
    if(!ptr || size == 0)
        return HJ_RAM_ERR_INVALID_PARAMETER;

#if defined(RAM_PLATFORM_WINDOWS)
    WIN32_MEMORY_RANGE_ENTRY range;
    range.VirtualAddress = (PVOID) ptr;
    range.NumberOfBytes  = size;
    BOOL result = PrefetchVirtualMemory(GetCurrentProcess(), 1, &range, 0);
    return result ? HJ_RAM_SUCCESS : HJ_RAM_ERR_SYSTEM_ERROR;

#elif defined(RAM_PLATFORM_LINUX) || defined(RAM_PLATFORM_MACOS)
    int result = madvise((void *) ptr, size, MADV_WILLNEED);
    if(result != 0 && errno == EFAULT)
        return HJ_RAM_ERR_INVALID_ADDRESS;
    return (result == 0) ? HJ_RAM_SUCCESS : HJ_RAM_ERR_SYSTEM_ERROR;

#else
    const char   *byte_ptr = (const char *) ptr;
    volatile char dummy;
    for(size_t i = 0; i < size; i += 64)
        dummy = byte_ptr[i];

    (void) dummy;
    return HJ_RAM_SUCCESS;
#endif
}

HJ_RAM_API const char *hj_ram_type_to_string(hj_ram_type_t type)
{
    switch(type)
    {
        case HJ_RAM_TYPE_DDR:
            return "DDR";
        case HJ_RAM_TYPE_DDR2:
            return "DDR2";
        case HJ_RAM_TYPE_DDR3:
            return "DDR3";
        case HJ_RAM_TYPE_DDR4:
            return "DDR4";
        case HJ_RAM_TYPE_DDR5:
            return "DDR5";
        case HJ_RAM_TYPE_SDRAM:
            return "SDRAM";
        case HJ_RAM_TYPE_SRAM:
            return "SRAM";
        case HJ_RAM_TYPE_RDRAM:
            return "RDRAM";
        default:
            return "Unknown";
    }
}

HJ_RAM_API hj_ram_err_t hj_ram_format_size(uint64_t bytes,
                                           char    *buffer,
                                           size_t   buffer_size)
{
    if(!buffer || buffer_size == 0)
        return HJ_RAM_ERR_INVALID_PARAMETER;

    int written = 0;
    if(bytes >= HJ_RAM_BYTES_PER_GB)
    {
        double gb = (double) bytes / HJ_RAM_BYTES_PER_GB;
        written   = snprintf(buffer, buffer_size, "%.2f GB", gb);
    } else if(bytes >= HJ_RAM_BYTES_PER_MB)
    {
        double mb = (double) bytes / HJ_RAM_BYTES_PER_MB;
        written   = snprintf(buffer, buffer_size, "%.2f MB", mb);
    } else if(bytes >= HJ_RAM_BYTES_PER_KB)
    {
        double kb = (double) bytes / HJ_RAM_BYTES_PER_KB;
        written   = snprintf(buffer, buffer_size, "%.2f KB", kb);
    } else
    {
        written = snprintf(buffer,
                           buffer_size,
                           "%llu bytes",
                           (unsigned long long) bytes);
    }

    if(written < 0 || (size_t) written >= buffer_size)
        return HJ_RAM_ERR_INSUFFICIENT_BUFFER;

    return HJ_RAM_SUCCESS;
}

HJ_RAM_API hj_ram_err_t hj_ram_get_page_size(uint32_t *page_size)
{
    if(!page_size)
        return HJ_RAM_ERR_INVALID_PARAMETER;

#if defined(RAM_PLATFORM_WINDOWS)
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    *page_size = sys_info.dwPageSize;
    return HJ_RAM_SUCCESS;

#elif defined(RAM_PLATFORM_LINUX) || defined(RAM_PLATFORM_MACOS)
    long size = sysconf(_SC_PAGESIZE);
    if(size > 0)
    {
        *page_size = (uint32_t) size;
        return HJ_RAM_SUCCESS;
    }

    return HJ_RAM_ERR_SYSTEM_ERROR;

#else
    *page_size = 4096;
    return HJ_RAM_SUCCESS;

#endif
}

HJ_RAM_API hj_ram_err_t hj_ram_is_valid_address(const void *ptr,
                                                size_t      size,
                                                bool        for_write,
                                                bool       *is_valid)
{
    if(!ptr || !is_valid || size == 0)
        return HJ_RAM_ERR_INVALID_PARAMETER;
    *is_valid = false;

#if defined(RAM_PLATFORM_WINDOWS)
    MEMORY_BASIC_INFORMATION mbi;
    if(VirtualQuery(ptr, &mbi, sizeof(mbi)) == sizeof(mbi)
       && mbi.State == MEM_COMMIT)
    {
        if(for_write)
            *is_valid =
                (mbi.Protect
                 & (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE))
                != 0;
        else
            *is_valid = (mbi.Protect
                         & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE
                            | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE))
                        != 0;
    }
    return HJ_RAM_SUCCESS;

#elif defined(RAM_PLATFORM_LINUX)
    FILE *maps = fopen("/proc/self/maps", "r");
    if(!maps)
    {
        if(errno == EACCES || errno == EPERM)
            return HJ_RAM_ERR_ACCESS_DENIED;
        return HJ_RAM_ERR_SYSTEM_ERROR;
    }

    char      line[2048];
    uintptr_t start, end;
    char      perms[5];
    while(fgets(line, sizeof(line), maps))
    {
        size_t len = strlen(line);
        if(len > 0 && line[len - 1] != '\n')
        {
            char discard[256];
            while(fgets(discard, sizeof(discard), maps))
            {
                size_t dlen = strlen(discard);
                if(dlen > 0 && discard[dlen - 1] == '\n')
                {
                    break;
                }
            }
            continue;
        }

        if(sscanf(line, "%lx-%lx %4s", &start, &end, perms) == 3)
        {
            if((uintptr_t) ptr >= start && (uintptr_t) ptr + size <= end)
            {
                if(for_write)
                    *is_valid = (perms[1] == 'w');
                else
                    *is_valid = (perms[0] == 'r');
                break;
            }
        }
    }
    fclose(maps);
    return HJ_RAM_SUCCESS;

#else
    *is_valid = (ptr != NULL);
    return HJ_RAM_SUCCESS;

#endif
}

HJ_RAM_API hj_ram_err_t hj_ram_get_process_usage(hj_ram_statistics_t *stats)
{
    if(!stats)
        return HJ_RAM_ERR_INVALID_PARAMETER;

    memset(stats, 0, sizeof(hj_ram_statistics_t));

#if defined(RAM_PLATFORM_WINDOWS)
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if(GetProcessMemoryInfo(GetCurrentProcess(),
                            (PROCESS_MEMORY_COUNTERS *) &pmc,
                            sizeof(pmc)))
    {
        stats->current_usage   = pmc.WorkingSetSize;
        stats->peak_usage      = pmc.PeakWorkingSetSize;
        stats->page_faults     = pmc.PageFaultCount;
        stats->page_file_usage = pmc.PagefileUsage;
        return HJ_RAM_SUCCESS;
    }

    return HJ_RAM_ERR_SYSTEM_ERROR;

#elif defined(RAM_PLATFORM_LINUX)
    FILE *status = fopen("/proc/self/status", "r");
    if(!status)
    {
        if(errno == EACCES || errno == EPERM)
            return HJ_RAM_ERR_ACCESS_DENIED;
        if(errno == ENOENT)
            return HJ_RAM_ERR_NOT_FOUND;
        return HJ_RAM_ERR_SYSTEM_ERROR;
    }

    char line[256];
    while(fgets(line, sizeof(line), status))
    {
        unsigned long value;
        if(sscanf(line, "VmRSS: %lu kB", &value) == 1)
            stats->current_usage = value * 1024;
        else if(sscanf(line, "VmHWM: %lu kB", &value) == 1)
            stats->peak_usage = value * 1024;
    }

    fclose(status);
    return HJ_RAM_SUCCESS;

#elif defined(RAM_PLATFORM_MACOS)
    struct mach_task_basic_info info;
    mach_msg_type_number_t      count = MACH_TASK_BASIC_INFO_COUNT;
    if(task_info(mach_task_self(),
                 MACH_TASK_BASIC_INFO,
                 (task_info_t) &info,
                 &count)
       == KERN_SUCCESS)
    {
        stats->current_usage = info.resident_size;
        return HJ_RAM_SUCCESS;
    }

    return HJ_RAM_ERR_SYSTEM_ERROR;

#else
    return HJ_RAM_ERR_NOT_SUPPORTED;

#endif
}

#ifdef __cplusplus
}
#endif

#endif // RAM_H