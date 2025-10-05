/*
 *  This file is part of hj.
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

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define SMBIOS_TYPE_BIOS 0
#define SMBIOS_TYPE_SYSTEM 1
#define SMBIOS_SIGNATURE 'RSMB'
#define BIOS_ROM_SIZE_UNIT (64 * 1024) // 64KB units

#define MAX_VENDOR_LENGTH 127
#define MAX_VERSION_LENGTH 63
#define MAX_DATE_LENGTH 31
#define MAX_SERIAL_LENGTH 127

typedef enum
{
    BIOS_OK                       = 0,
    BIOS_FAILED                   = -1,
    BIOS_ERROR_NULL_POINTER       = -2,
    BIOS_ERROR_BUFFER_TOO_SMALL   = -3,
    BIOS_ERROR_NOT_FOUND          = -4,
    BIOS_ERROR_SYSTEM_CALL_FAILED = -5,
    BIOS_ERROR_PARSE_FAILED       = -6,
    BIOS_ERROR_NOT_SUPPORTED      = -7,
    BIOS_ERROR_MEMORY_ALLOCATION  = -8
} bios_error_t;

typedef struct
{
    char     vendor[128];
    char     version[64];
    char     release_date[32];
    char     serial_number[128];
    uint16_t starting_segment;
    size_t   rom_size;
} bios_info_t;

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct
{
    uint8_t  type;
    uint8_t  length;
    uint16_t handle;
} smbios_header_t;
#pragma pack(pop)

inline bios_error_t smbios_get_data(uint8_t **data, size_t *size)
{
    if(!data || !size)
        return BIOS_ERROR_NULL_POINTER;

    UINT required_size = GetSystemFirmwareTable(SMBIOS_SIGNATURE, 0, NULL, 0);
    if(required_size == 0)
        return BIOS_ERROR_SYSTEM_CALL_FAILED;

    uint8_t *buffer = (uint8_t *) malloc(required_size);
    if(!buffer)
        return BIOS_ERROR_MEMORY_ALLOCATION;

    if(GetSystemFirmwareTable(SMBIOS_SIGNATURE, 0, buffer, required_size)
       != required_size)
    {
        free(buffer);
        buffer = NULL;
        return BIOS_ERROR_SYSTEM_CALL_FAILED;
    }

    *data = buffer;
    *size = required_size;
    return BIOS_OK;
}

inline const char *smbios_find_string(const uint8_t *entry_data,
                                      size_t         entry_length,
                                      const uint8_t *buffer_end,
                                      uint8_t        string_index)
{
    if(!entry_data || !buffer_end || string_index == 0)
        return NULL;

    const uint8_t *str_ptr = entry_data + entry_length;
    if(str_ptr >= buffer_end)
        return NULL;

    for(uint8_t i = 1; i < string_index && str_ptr < buffer_end - 1; i++)
    {
        while(str_ptr < buffer_end && *str_ptr != 0)
            str_ptr++;

        if(str_ptr < buffer_end)
            str_ptr++;
    }

    if(str_ptr >= buffer_end || *str_ptr == 0)
        return NULL;

    return (const char *) str_ptr;
}

inline bios_error_t smbios_parse_entry(uint8_t type,
                                       uint8_t field_offset,
                                       uint8_t string_index_offset,
                                       char   *output,
                                       size_t  output_size)
{
    if(!output || output_size == 0)
        return BIOS_ERROR_NULL_POINTER;

    uint8_t     *buffer;
    size_t       buffer_size;
    bios_error_t result = smbios_get_data(&buffer, &buffer_size);
    if(result != BIOS_OK)
        return result;

    const uint8_t *p   = buffer + 8;
    const uint8_t *end = buffer + buffer_size;
    while(p + sizeof(smbios_header_t) <= end)
    {
        const smbios_header_t *header = (const smbios_header_t *) p;
        if(header->length < sizeof(smbios_header_t))
            break;

        if(header->type == type)
        {
            if(p + string_index_offset >= p + header->length)
                return BIOS_ERROR_PARSE_FAILED;

            uint8_t     string_index = *(p + string_index_offset);
            const char *str =
                smbios_find_string(p, header->length, end, string_index);
            if(str)
            {
                if(!output || !str || output_size == 0)
                    return BIOS_ERROR_NULL_POINTER;

                size_t src_len = strlen(str);
                if(src_len >= output_size)
                    return BIOS_ERROR_BUFFER_TOO_SMALL;

                memcpy(output, str, src_len + 1);
                return BIOS_OK;
            }
            break;
        }

        const uint8_t *next = p + header->length;
        while(next < end - 1 && (next[0] != 0 || next[1] != 0))
            next++;

        next += 2;
        p = next;
    }

    return BIOS_ERROR_NOT_FOUND;
}

#elif defined(__linux__)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

inline bios_error_t
bios_sys_file_read(const char *path, char *buffer, size_t buffer_size)
{
    if(!path || !buffer || buffer_size == 0)
        return BIOS_ERROR_NULL_POINTER;

    FILE *fp = fopen(path, "r");
    if(!fp)
        return BIOS_ERROR_NOT_FOUND;

    if(!fgets(buffer, buffer_size, fp))
        fclose(fp);
    return BIOS_ERROR_SYSTEM_CALL_FAILED;

    fclose(fp);
    size_t len = strlen(buffer);
    if(len > 0 && buffer[len - 1] == '\n')
        buffer[len - 1] = '\0';

    return BIOS_OK;
}

#elif defined(__APPLE__)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

inline bios_error_t bios_execute_command_and_parse(const char *command,
                                                   const char *search_pattern,
                                                   char       *output,
                                                   size_t      output_size)
{
    if(!command || !search_pattern || !output || output_size == 0)
        return BIOS_ERROR_NULL_POINTER;

    FILE *fp = popen(command, "r");
    if(!fp)
        return BIOS_ERROR_SYSTEM_CALL_FAILED;

    char        *found = NULL;
    char        *value = NULL;
    char         buffer[256];
    bios_error_t result = BIOS_ERROR_NOT_FOUND;
    while(fgets(buffer, sizeof(buffer), fp))
    {
        found = strstr(buffer, search_pattern);
        if(!found)
            continue;

        value = strchr(found, ':');
        if(!value)
            continue;

        value++;
        while(*value == ' ' || *value == '\t')
            value++;

        size_t len = strlen(value);
        if(len > 0 && value[len - 1] == '\n')
            value[len - 1] = '\0';

        result = safe_string_copy(output, output_size, value);
        break;
    }

    pclose(fp);
    return result;
}

#endif

// --------------------- API -------------------------
inline bios_error_t bios_vendor(char *vendor, size_t *length)
{
    if(!vendor || !length || *length == 0)
        return BIOS_ERROR_NULL_POINTER;

#if defined(_WIN32) || defined(_WIN64)
    return smbios_parse_entry(SMBIOS_TYPE_BIOS, 4, 4, vendor, *length);

#elif defined(__linux__)
    bios_error_t result =
        bios_sys_file_read("/sys/class/dmi/id/bios_vendor", vendor, *length);
    if(result == BIOS_OK)
        *length = strlen(vendor);

    return result;

#elif defined(__APPLE__)
    bios_error_t result = bios_execute_command_and_parse(
        "system_profiler SPHardwareDataType | grep 'Manufacturer'",
        "Manufacturer",
        vendor,
        *length);
    if(result == BIOS_OK)
        *length = strlen(vendor);

    return result;

#else
    return BIOS_ERROR_NOT_SUPPORTED;

#endif
}

inline bios_error_t bios_version(char *version, size_t *length)
{
    if(!version || !length || *length == 0)
        return BIOS_ERROR_NULL_POINTER;

#if defined(_WIN32) || defined(_WIN64)
    return smbios_parse_entry(SMBIOS_TYPE_BIOS, 5, 5, version, *length);

#elif defined(__linux__)
    bios_error_t result =
        bios_sys_file_read("/sys/class/dmi/id/bios_version", version, *length);
    if(result == BIOS_OK)
        *length = strlen(version);

    return result;

#elif defined(__APPLE__)
    bios_error_t result = bios_execute_command_and_parse(
        "system_profiler SPHardwareDataType | grep 'Boot ROM Version'",
        "Boot ROM Version",
        version,
        *length);
    if(result == BIOS_OK)
        *length = strlen(version);

    return result;

#else
    return BIOS_ERROR_NOT_SUPPORTED;

#endif
}

inline bios_error_t bios_release_date(char *date_str, size_t *length)
{
    if(!date_str || !length || *length == 0)
        return BIOS_ERROR_NULL_POINTER;

#if defined(_WIN32) || defined(_WIN64)
    return smbios_parse_entry(SMBIOS_TYPE_BIOS, 8, 8, date_str, *length);

#elif defined(__linux__)
    bios_error_t result =
        bios_sys_file_read("/sys/class/dmi/id/bios_date", date_str, *length);
    if(result == BIOS_OK)
        *length = strlen(date_str);

    return result;

#elif defined(__APPLE__)
    if(*length > 0)
    {
        date_str[0] = '\0';
        *length     = 0;
        return BIOS_OK;
    }

    return BIOS_ERROR_BUFFER_TOO_SMALL;

#else
    return BIOS_ERROR_NOT_SUPPORTED;

#endif
}

inline bios_error_t bios_starting_segment(uint16_t *segment)
{
    if(!segment)
        return BIOS_ERROR_NULL_POINTER;

#if defined(_WIN32) || defined(_WIN64)
    uint8_t     *buffer      = NULL;
    size_t       buffer_size = 0;
    bios_error_t ec          = smbios_get_data(&buffer, &buffer_size);
    if(ec != BIOS_OK || !buffer || buffer_size <= 8)
        return BIOS_ERROR_SYSTEM_CALL_FAILED;

    const uint8_t *p   = buffer + 8;
    const uint8_t *end = buffer + buffer_size;
    while(p + sizeof(smbios_header_t) <= end)
    {
        const smbios_header_t *header = (const smbios_header_t *) p;
        if(header->length < sizeof(smbios_header_t))
            break;

        if(header->type == SMBIOS_TYPE_BIOS)
        {
            if(header->length >= 8)
            {
                *segment = (uint16_t) (p[6] | (p[7] << 8));
                free(buffer);
                return BIOS_OK;
            }
            break;
        }

        const uint8_t *next = p + header->length;
        while(next < end - 1 && (next[0] != 0 || next[1] != 0))
            next++;

        next += 2;
        p = next;
    }
    free(buffer);
    return BIOS_ERROR_NOT_FOUND;

#elif defined(__linux__)
    return BIOS_ERROR_NOT_SUPPORTED;

#elif defined(__APPLE__)
    return BIOS_ERROR_NOT_SUPPORTED;

#else
    return BIOS_ERROR_NOT_SUPPORTED;

#endif
}

inline bios_error_t bios_serial_num(char *serial_num, size_t *length)
{
    if(!serial_num || !length || *length == 0)
        return BIOS_ERROR_NULL_POINTER;

#if defined(_WIN32) || defined(_WIN64)
    return smbios_parse_entry(SMBIOS_TYPE_BIOS, 1, 1, serial_num, *length);

#elif defined(__linux__)
    bios_error_t result = bios_sys_file_read("/sys/class/dmi/id/bios_serial",
                                             serial_num,
                                             *length);
    if(result == BIOS_OK)
        *length = strlen(serial_num);

    return result;

#elif defined(__APPLE__)
    bios_error_t result = bios_execute_command_and_parse(
        "system_profiler SPHardwareDataType | grep 'Serial Number'",
        "Serial Number",
        serial_num,
        *length);
    if(result == BIOS_OK)
        *length = strlen(serial_num);

    return result;

#else
    return BIOS_ERROR_NOT_SUPPORTED;

#endif
}

inline bios_error_t bios_rom_size(size_t *rom_size)
{
#if defined(_WIN32) || defined(_WIN64)
    uint8_t     *buffer      = NULL;
    size_t       buffer_size = 0;
    bios_error_t ec          = smbios_get_data(&buffer, &buffer_size);
    if(ec != BIOS_OK || !buffer || buffer_size <= 8)
        return BIOS_ERROR_SYSTEM_CALL_FAILED;

    const uint8_t *p   = buffer + 8;
    const uint8_t *end = buffer + buffer_size;
    while(p + sizeof(smbios_header_t) <= end)
    {
        const smbios_header_t *header = (const smbios_header_t *) p;
        if(header->length < sizeof(smbios_header_t))
            break;

        if(header->type == SMBIOS_TYPE_BIOS)
        {
            if(header->length >= 9)
            {
                *rom_size = (size_t) (p[8] * BIOS_ROM_SIZE_UNIT);
                free(buffer);
                return BIOS_OK;
            }
            break;
        }

        const uint8_t *next = p + header->length;
        while(next < end - 1 && (next[0] != 0 || next[1] != 0))
            next++;

        next += 2;
        p = next;
    }

    free(buffer);
    return BIOS_ERROR_NOT_FOUND;

#elif defined(__linux__)
    return BIOS_ERROR_NOT_SUPPORTED;

#elif defined(__APPLE__)
    return BIOS_ERROR_NOT_SUPPORTED;

#endif
    return BIOS_ERROR_NOT_SUPPORTED;
}

inline bios_error_t bios_info(bios_info_t *info)
{
    if(!info)
        return BIOS_ERROR_NULL_POINTER;

    bios_error_t ec = BIOS_OK;
    memset(info, 0, sizeof(bios_info_t));
    size_t vendor_len = sizeof(info->vendor);
    ec                = bios_vendor(info->vendor, &vendor_len);
    if(ec != BIOS_OK)
        return ec;

    size_t version_len = sizeof(info->version);
    ec                 = bios_version(info->version, &version_len);
    if(ec != BIOS_OK)
        return ec;

    size_t date_len = sizeof(info->release_date);
    ec              = bios_release_date(info->release_date, &date_len);
    if(ec != BIOS_OK)
        return ec;

    size_t sn_len = sizeof(info->serial_number);
    ec            = bios_serial_num(info->serial_number, &sn_len);
    if(ec != BIOS_OK)
        return ec;

    ec = bios_starting_segment(&info->starting_segment);
    // ignore if starting segment not found

    ec = bios_rom_size(&info->rom_size);
    // ignore if rom size not found

    return BIOS_OK;
}

#ifdef __cplusplus
}
#endif

#endif // BIOS_H