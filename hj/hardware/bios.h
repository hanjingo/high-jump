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

#ifndef BIOS_H
#define BIOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define HJ_SMBIOS_TYPE_BIOS 0
#define HJ_SMBIOS_TYPE_SYSTEM 1
#define HJ_SMBIOS_SIGNATURE 'RSMB'
#define HJ_BIOS_ROM_SIZE_UNIT (64 * 1024) // 64KB units

#define HJ_MAX_VENDOR_LENGTH 127
#define HJ_MAX_VERSION_LENGTH 63
#define HJ_MAX_DATE_LENGTH 31
#define HJ_MAX_SERIAL_LENGTH 127

#ifndef HJ_BIOS_API
#if defined(HJ_BIOS_STATIC)
#define HJ_BIOS_API static inline
#else
#define HJ_BIOS_API extern
#endif
#endif

typedef enum
{
    HJ_BIOS_OK                       = 0,
    HJ_BIOS_FAILED                   = -1,
    HJ_BIOS_ERROR_NULL_POINTER       = -2,
    HJ_BIOS_ERROR_BUFFER_TOO_SMALL   = -3,
    HJ_BIOS_ERROR_NOT_FOUND          = -4,
    HJ_BIOS_ERROR_SYSTEM_CALL_FAILED = -5,
    HJ_BIOS_ERROR_PARSE_FAILED       = -6,
    HJ_BIOS_ERROR_NOT_SUPPORTED      = -7,
    HJ_BIOS_ERROR_MEMORY_ALLOCATION  = -8
} hj_bios_err_t;

typedef struct
{
    char     vendor[128];
    char     version[64];
    char     release_date[32];
    char     serial_number[128];
    uint16_t starting_segment;
    size_t   rom_size;
} hj_bios_info_t;

HJ_BIOS_API hj_bios_err_t hj_bios_safe_string_copy(char       *dst,
                                                   size_t      dst_size,
                                                   const char *src);
HJ_BIOS_API hj_bios_err_t hj_bios_vendor(char *vendor, size_t *length);
HJ_BIOS_API hj_bios_err_t hj_bios_version(char *version, size_t *length);
HJ_BIOS_API hj_bios_err_t hj_bios_release_date(char *date_str, size_t *length);
HJ_BIOS_API hj_bios_err_t hj_bios_starting_segment(uint16_t *segment);
HJ_BIOS_API hj_bios_err_t hj_bios_serial_num(char *serial_num, size_t *length);
HJ_BIOS_API hj_bios_err_t hj_bios_rom_size(size_t *rom_size);
HJ_BIOS_API hj_bios_err_t hj_bios_info(hj_bios_info_t *info);

#ifdef __cplusplus
}
#endif

#endif // BIOS_H


// --------------------- Implementation -------------------------
// This section is using STB-style implementation. To include the
// implementation, define HJ_BIOS_IMPL before including
// this header in one source file.
#if (defined(HJ_BIOS_IMPL) || defined(HJ_BIOS_STATIC))                         \
    && !defined(HJ_BIOS_IMPL_DONE)
#define HJ_BIOS_IMPL_DONE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

static inline hj_bios_err_t
hj_bios_safe_string_copy(char *dst, size_t dst_size, const char *src)
{
    if(!dst || !src || dst_size == 0)
        return HJ_BIOS_ERROR_NULL_POINTER;

    size_t src_len = strlen(src);
    if(src_len >= dst_size)
        return HJ_BIOS_ERROR_BUFFER_TOO_SMALL;

    memcpy(dst, src, src_len + 1);
    return HJ_BIOS_OK;
}

#if defined(_WIN32) || defined(_WIN64)

#pragma pack(push, 1)
typedef struct
{
    uint8_t  type;
    uint8_t  length;
    uint16_t handle;
} hj_smbios_header_t;
#pragma pack(pop)

static inline hj_bios_err_t smbios_get_data(uint8_t **data, size_t *size)
{
    if(!data || !size)
        return HJ_BIOS_ERROR_NULL_POINTER;

    UINT required_size =
        GetSystemFirmwareTable(HJ_SMBIOS_SIGNATURE, 0, NULL, 0);
    if(required_size == 0)
        return HJ_BIOS_ERROR_SYSTEM_CALL_FAILED;

    uint8_t *buffer = (uint8_t *) malloc(required_size);
    if(!buffer)
        return HJ_BIOS_ERROR_MEMORY_ALLOCATION;

    if(GetSystemFirmwareTable(HJ_SMBIOS_SIGNATURE, 0, buffer, required_size)
       != required_size)
    {
        free(buffer);
        return HJ_BIOS_ERROR_SYSTEM_CALL_FAILED;
    }

    *data = buffer;
    *size = required_size;
    return HJ_BIOS_OK;
}

static inline const char *hj_smbios_find_string(const uint8_t *entry_data,
                                                size_t         entry_length,
                                                const uint8_t *buffer_end,
                                                uint8_t        string_index)
{
    if(!entry_data || !buffer_end || string_index == 0)
        return NULL;

    if((size_t) (buffer_end - entry_data) < entry_length)
        return NULL;

    const uint8_t *str_ptr = entry_data + entry_length;
    if(str_ptr >= buffer_end)
        return NULL;

    for(uint8_t i = 1; i < string_index && str_ptr < buffer_end; i++)
    {
        while(str_ptr < buffer_end && *str_ptr != 0)
            str_ptr++;

        if(str_ptr < buffer_end && *str_ptr == 0)
            str_ptr++;
    }

    if(str_ptr >= buffer_end || *str_ptr == 0)
        return NULL;

    const uint8_t *s = str_ptr;
    while(s < buffer_end && *s != 0)
        s++;
    if(s >= buffer_end)
        return NULL;

    return (const char *) str_ptr;
}

static inline hj_bios_err_t hj_smbios_parse_entry(uint8_t type,
                                                  uint8_t field_offset,
                                                  uint8_t string_index_offset,
                                                  char   *output,
                                                  size_t  output_size)
{
    if(!output || output_size == 0)
        return HJ_BIOS_ERROR_NULL_POINTER;

    uint8_t      *buffer      = NULL;
    size_t        buffer_size = 0;
    hj_bios_err_t result      = smbios_get_data(&buffer, &buffer_size);
    if(result != HJ_BIOS_OK)
        return result;

    result             = HJ_BIOS_ERROR_NOT_FOUND;
    const uint8_t *p   = buffer + 8;
    const uint8_t *end = buffer + buffer_size;

    while(p + sizeof(hj_smbios_header_t) <= end)
    {
        const hj_smbios_header_t *header = (const hj_smbios_header_t *) p;
        if(header->length < sizeof(hj_smbios_header_t)
           || (size_t) (end - p) < header->length)
        {
            break;
        }

        if(header->type == type)
        {
            if(string_index_offset >= header->length)
            {
                result = HJ_BIOS_ERROR_PARSE_FAILED;
                break;
            }

            uint8_t     string_index = *(p + string_index_offset);
            const char *str =
                hj_smbios_find_string(p, header->length, end, string_index);
            if(str)
            {
                size_t src_len = strlen(str);
                if(src_len >= output_size)
                    result = HJ_BIOS_ERROR_BUFFER_TOO_SMALL;
                else
                {
                    memcpy(output, str, src_len + 1);
                    result = HJ_BIOS_OK;
                }
            }
            break;
        }

        const uint8_t *next = p + header->length;
        while(next + 1 < end && (next[0] != 0 || next[1] != 0))
            next++;

        if(next + 1 >= end)
            break;

        next += 2;
        p = next;
    }

    free(buffer);
    return result;
}

#elif defined(__linux__)

static inline hj_bios_err_t
hj_bios_sys_file_read(const char *path, char *buffer, size_t buffer_size)
{
    if(!path || !buffer || buffer_size == 0)
        return HJ_BIOS_ERROR_NULL_POINTER;

    FILE *fp = fopen(path, "r");
    if(!fp)
        return HJ_BIOS_ERROR_NOT_FOUND;

    char temp[512] = {0};
    if(!fgets(temp, sizeof(temp), fp))
    {
        fclose(fp);
        return HJ_BIOS_ERROR_SYSTEM_CALL_FAILED;
    }

    fclose(fp);

    size_t len = strlen(temp);
    while(len > 0 && (temp[len - 1] == '\n' || temp[len - 1] == '\r'))
    {
        temp[--len] = '\0';
    }

    if(len >= buffer_size)
        return HJ_BIOS_ERROR_BUFFER_TOO_SMALL;

    memcpy(buffer, temp, len + 1);
    return HJ_BIOS_OK;
}

#elif defined(__APPLE__)

static inline hj_bios_err_t hj_bios_read_apple_iokit_prop(const char *prop_name,
                                                          char       *output,
                                                          size_t output_size)
{
    if(!prop_name || !output || output_size == 0)
        return HJ_BIOS_ERROR_NULL_POINTER;

    io_service_t service = IOServiceGetMatchingService(
        kIOMainPortDefault,
        IOServiceMatching("IOPlatformExpertDevice"));
    if(!service)
        return HJ_BIOS_ERROR_NOT_FOUND;

    CFStringRef cf_key = CFStringCreateWithCString(kCFAllocatorDefault,
                                                   prop_name,
                                                   kCFStringEncodingUTF8);
    if(!cf_key)
    {
        IOObjectRelease(service);
        return HJ_BIOS_ERROR_MEMORY_ALLOCATION;
    }

    CFTypeRef prop = IORegistryEntryCreateCFProperty(service,
                                                     cf_key,
                                                     kCFAllocatorDefault,
                                                     0);
    CFRelease(cf_key);

    hj_bios_err_t result = HJ_BIOS_ERROR_NOT_FOUND;
    if(prop && CFGetTypeID(prop) == CFStringGetTypeID())
    {
        if(CFStringGetCString((CFStringRef) prop,
                              output,
                              output_size,
                              kCFStringEncodingUTF8))
            result = HJ_BIOS_OK;
        else
            result = HJ_BIOS_ERROR_BUFFER_TOO_SMALL;
    }

    if(prop)
        CFRelease(prop);
    IOObjectRelease(service);

    return result;
}

#endif

static inline hj_bios_err_t hj_bios_vendor(char *vendor, size_t *length)
{
    if(!vendor || !length || *length == 0)
        return HJ_BIOS_ERROR_NULL_POINTER;

#if defined(_WIN32) || defined(_WIN64)
    hj_bios_err_t result =
        hj_smbios_parse_entry(HJ_SMBIOS_TYPE_BIOS, 4, 4, vendor, *length);
    if(result == HJ_BIOS_OK)
        *length = strlen(vendor);
    return result;

#elif defined(__linux__)
    hj_bios_err_t result =
        hj_bios_sys_file_read("/sys/class/dmi/id/bios_vendor", vendor, *length);
    if(result == HJ_BIOS_OK)
        *length = strlen(vendor);
    return result;

#elif defined(__APPLE__)
    hj_bios_err_t result =
        hj_bios_read_apple_iokit_prop("manufacturer", vendor, *length);
    if(result == HJ_BIOS_OK)
        *length = strlen(vendor);
    return result;

#else
    return HJ_BIOS_ERROR_NOT_SUPPORTED;
#endif
}

static inline hj_bios_err_t hj_bios_version(char *version, size_t *length)
{
    if(!version || !length || *length == 0)
        return HJ_BIOS_ERROR_NULL_POINTER;

#if defined(_WIN32) || defined(_WIN64)
    hj_bios_err_t result =
        hj_smbios_parse_entry(HJ_SMBIOS_TYPE_BIOS, 5, 5, version, *length);
    if(result == HJ_BIOS_OK)
        *length = strlen(version);
    return result;

#elif defined(__linux__)
    hj_bios_err_t result =
        hj_bios_sys_file_read("/sys/class/dmi/id/bios_version",
                              version,
                              *length);
    if(result == HJ_BIOS_OK)
        *length = strlen(version);
    return result;

#elif defined(__APPLE__)
    hj_bios_err_t result =
        hj_bios_read_apple_iokit_prop("rom-version", version, *length);
    if(result == HJ_BIOS_OK)
        *length = strlen(version);
    return result;

#else
    return HJ_BIOS_ERROR_NOT_SUPPORTED;
#endif
}

static inline hj_bios_err_t hj_bios_release_date(char *date_str, size_t *length)
{
    if(!date_str || !length || *length == 0)
        return HJ_BIOS_ERROR_NULL_POINTER;

#if defined(_WIN32) || defined(_WIN64)
    hj_bios_err_t result =
        hj_smbios_parse_entry(HJ_SMBIOS_TYPE_BIOS, 8, 8, date_str, *length);
    if(result == HJ_BIOS_OK)
        *length = strlen(date_str);
    return result;

#elif defined(__linux__)
    hj_bios_err_t result =
        hj_bios_sys_file_read("/sys/class/dmi/id/bios_date", date_str, *length);
    if(result == HJ_BIOS_OK)
        *length = strlen(date_str);
    return result;

#elif defined(__APPLE__)
    return HJ_BIOS_ERROR_NOT_SUPPORTED;

#else
    return HJ_BIOS_ERROR_NOT_SUPPORTED;
#endif
}

static inline hj_bios_err_t hj_bios_starting_segment(uint16_t *segment)
{
    if(!segment)
        return HJ_BIOS_ERROR_NULL_POINTER;

#if defined(_WIN32) || defined(_WIN64)
    uint8_t      *buffer      = NULL;
    size_t        buffer_size = 0;
    hj_bios_err_t ec          = smbios_get_data(&buffer, &buffer_size);
    if(ec != HJ_BIOS_OK || !buffer || buffer_size <= 8)
        return HJ_BIOS_ERROR_SYSTEM_CALL_FAILED;

    hj_bios_err_t  result = HJ_BIOS_ERROR_NOT_FOUND;
    const uint8_t *p      = buffer + 8;
    const uint8_t *end    = buffer + buffer_size;
    while(p + sizeof(hj_smbios_header_t) <= end)
    {
        const hj_smbios_header_t *header = (const hj_smbios_header_t *) p;
        if(header->length < sizeof(hj_smbios_header_t)
           || (size_t) (end - p) < header->length)
        {
            break;
        }

        if(header->type == HJ_SMBIOS_TYPE_BIOS)
        {
            if(header->length >= 8)
            {
                *segment = (uint16_t) (p[6] | (p[7] << 8));
                result   = HJ_BIOS_OK;
            }
            break;
        }

        const uint8_t *next = p + header->length;
        while(next + 1 < end && (next[0] != 0 || next[1] != 0))
            next++;

        if(next + 1 >= end)
            break;

        next += 2;
        p = next;
    }
    free(buffer);
    return result;

#else
    return HJ_BIOS_ERROR_NOT_SUPPORTED;
#endif
}

static inline hj_bios_err_t hj_bios_serial_num(char *serial_num, size_t *length)
{
    if(!serial_num || !length || *length == 0)
        return HJ_BIOS_ERROR_NULL_POINTER;

#if defined(_WIN32) || defined(_WIN64)
    hj_bios_err_t result =
        hj_smbios_parse_entry(HJ_SMBIOS_TYPE_BIOS, 1, 1, serial_num, *length);
    if(result == HJ_BIOS_OK)
        *length = strlen(serial_num);
    return result;

#elif defined(__linux__)
    hj_bios_err_t result =
        hj_bios_sys_file_read("/sys/class/dmi/id/bios_serial",
                              serial_num,
                              *length);
    if(result == HJ_BIOS_OK)
        *length = strlen(serial_num);
    return result;

#elif defined(__APPLE__)
    hj_bios_err_t result =
        hj_bios_read_apple_iokit_prop("IOPlatformSerialNumber",
                                      serial_num,
                                      *length);
    if(result == HJ_BIOS_OK)
        *length = strlen(serial_num);
    return result;

#else
    return HJ_BIOS_ERROR_NOT_SUPPORTED;
#endif
}

static inline hj_bios_err_t hj_bios_rom_size(size_t *rom_size)
{
    if(!rom_size)
        return HJ_BIOS_ERROR_NULL_POINTER;

#if defined(_WIN32) || defined(_WIN64)
    uint8_t      *buffer      = NULL;
    size_t        buffer_size = 0;
    hj_bios_err_t ec          = smbios_get_data(&buffer, &buffer_size);
    if(ec != HJ_BIOS_OK || !buffer || buffer_size <= 8)
        return HJ_BIOS_ERROR_SYSTEM_CALL_FAILED;

    hj_bios_err_t  result = HJ_BIOS_ERROR_NOT_FOUND;
    const uint8_t *p      = buffer + 8;
    const uint8_t *end    = buffer + buffer_size;
    while(p + sizeof(hj_smbios_header_t) <= end)
    {
        const hj_smbios_header_t *header = (const hj_smbios_header_t *) p;
        if(header->length < sizeof(hj_smbios_header_t)
           || (size_t) (end - p) < header->length)
        {
            break;
        }

        if(header->type == HJ_SMBIOS_TYPE_BIOS)
        {
            if(header->length >= 9)
            {
                uint8_t raw_size = p[8];
                if(raw_size != 0xFF)
                {
                    // SMBIOS 规范: (n + 1) * 64KB
                    *rom_size = (size_t) (raw_size + 1) * HJ_BIOS_ROM_SIZE_UNIT;
                    result    = HJ_BIOS_OK;
                } else if(header->length
                          >= 0x1A) // 26 字节及以上包含 Extended BIOS ROM Size
                {
                    uint16_t ext_size = (uint16_t) (p[0x18] | (p[0x19] << 8));
                    uint16_t unit     = (ext_size >> 14) & 0x03;
                    uint16_t val      = ext_size & 0x3FFF;

                    if(unit == 0) // Megabytes
                    {
                        *rom_size = (size_t) val * 1024 * 1024;
                        result    = HJ_BIOS_OK;
                    } else if(unit == 1) // Gigabytes
                    {
                        *rom_size = (size_t) val * 1024 * 1024 * 1024;
                        result    = HJ_BIOS_OK;
                    } else
                    {
                        result = HJ_BIOS_ERROR_PARSE_FAILED;
                    }
                } else
                {
                    // 兼容旧规范：0xFF 且无扩展字段时表示 16MB (256 * 64KB)
                    *rom_size = (size_t) 256 * HJ_BIOS_ROM_SIZE_UNIT;
                    result    = HJ_BIOS_OK;
                }
            }
            break;
        }

        const uint8_t *next = p + header->length;
        while(next + 1 < end && (next[0] != 0 || next[1] != 0))
            next++;

        if(next + 1 >= end)
            break;

        next += 2;
        p = next;
    }

    free(buffer);
    return result;

#else
    return HJ_BIOS_ERROR_NOT_SUPPORTED;
#endif
}

static inline hj_bios_err_t hj_bios_info(hj_bios_info_t *info)
{
    if(!info)
        return HJ_BIOS_ERROR_NULL_POINTER;

    memset(info, 0, sizeof(hj_bios_info_t));

    hj_bios_err_t first_err = HJ_BIOS_OK;
    hj_bios_err_t err;

    size_t vendor_len = sizeof(info->vendor);
    err               = hj_bios_vendor(info->vendor, &vendor_len);
    if(err != HJ_BIOS_OK && first_err == HJ_BIOS_OK)
        first_err = err;

    size_t version_len = sizeof(info->version);
    err                = hj_bios_version(info->version, &version_len);
    if(err != HJ_BIOS_OK && first_err == HJ_BIOS_OK)
        first_err = err;

    size_t date_len = sizeof(info->release_date);
    err             = hj_bios_release_date(info->release_date, &date_len);
    if(err != HJ_BIOS_OK && first_err == HJ_BIOS_OK)
        first_err = err;

    size_t sn_len = sizeof(info->serial_number);
    err           = hj_bios_serial_num(info->serial_number, &sn_len);
    if(err != HJ_BIOS_OK && first_err == HJ_BIOS_OK)
        first_err = err;

    err = hj_bios_starting_segment(&info->starting_segment);
    if(err != HJ_BIOS_OK && first_err == HJ_BIOS_OK)
        first_err = err;

    err = hj_bios_rom_size(&info->rom_size);
    if(err != HJ_BIOS_OK && first_err == HJ_BIOS_OK)
        first_err = err;

    return first_err;
}

#ifdef __cplusplus
}
#endif

#endif // HJ_BIOS_IMPL && !HJ_BIOS_IMPL_DONE