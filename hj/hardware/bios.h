#ifndef BIOS_H
#define BIOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <stdio.h>

#pragma pack(push, 1)
typedef struct {
    uint8_t type;
    uint8_t length;
    uint16_t handle;
} SMBIOSHeader;
#pragma pack(pop)

static int bios_vendor(unsigned char* vendor, size_t* length)
{
    UINT size = GetSystemFirmwareTable('RSMB', 0, NULL, 0);
    if (size == 0) 
        return -1;

    uint8_t* buffer = (uint8_t*)malloc(size);
    if (!buffer) 
        return -1;

    if (GetSystemFirmwareTable('RSMB', 0, buffer, size) != size) 
    {
        free(buffer);
        return -1;
    }
    uint8_t* p = buffer + 8;
    uint8_t* end = buffer + size;
    while (p + sizeof(SMBIOSHeader) <= end) 
    {
        SMBIOSHeader* h = (SMBIOSHeader*)p;
        if (h->length < 4) 
            break;

        if (h->type == 0) 
        {
            uint8_t vendorIndex = *(p + 4);
            char* str = (char*)(p + h->length);
            for (uint8_t i = 1; i < vendorIndex && *str; i++) str += strlen(str) + 1;
            if (!(*str)) 
                break;

            size_t len = strlen(str);
            if (len < *length) 
            {
                memcpy(vendor, str, len + 1);
                *length = len;
            } 
            else 
            {
                *length = 0;
            }
            break;
        }

        uint8_t* next = p + h->length;
        while (next < end - 1 && (next[0] != 0 || next[1] != 0)) 
            next++;

        next += 2;
        p = next;
    }
    free(buffer);
    return 0;
}

static uint16_t bios_starting_segment()
{
    UINT size = GetSystemFirmwareTable('RSMB', 0, NULL, 0);
    if (size == 0) 
        return 0;

    uint8_t* buffer = (uint8_t*)malloc(size);
    if (!buffer) 
        return 0;

    if (GetSystemFirmwareTable('RSMB', 0, buffer, size) != size) 
    {
        free(buffer);
        return 0;
    }

    uint8_t* p = buffer + 8;
    uint8_t* end = buffer + size;
    uint16_t segment = 0;
    while (p + sizeof(SMBIOSHeader) <= end) 
    {
        SMBIOSHeader* h = (SMBIOSHeader*)p;
        if (h->length < 4) 
            break;

        if (h->type == 0) 
        {
            segment = *(uint16_t*)(p + 6);
            break;
        }

        uint8_t* next = p + h->length;
        while (next < end - 1 && (next[0] != 0 || next[1] != 0)) 
            next++;

        next += 2;
        p = next;
    }
    free(buffer);
    return segment;
}

static int bios_release_date(uint8_t* year, uint8_t* month, uint8_t* day)
{
    UINT size = GetSystemFirmwareTable('RSMB', 0, NULL, 0);
    if (size == 0) 
        return -1;

    uint8_t* buffer = (uint8_t*)malloc(size);
    if (!buffer) 
        return -1;

    if (GetSystemFirmwareTable('RSMB', 0, buffer, size) != size) 
    {
        free(buffer);
        return -1;
    }

    uint8_t* p = buffer + 8;
    uint8_t* end = buffer + size;
    while (p + sizeof(SMBIOSHeader) <= end) 
    {
        SMBIOSHeader* h = (SMBIOSHeader*)p;
        if (h->length < 4) 
            break;

        if (h->type == 0) 
        {
            uint8_t dateIndex = *(p + 8);
            char* str = (char*)(p + h->length);
            for (uint8_t i = 1; i < dateIndex && *str; i++) str += strlen(str) + 1;
            if (!(*str))
                break;

            int y, m, d;
            if (sscanf(str, "%d/%d/%d", &m, &d, &y) == 3) 
            {
                *year = (uint8_t)(y % 100);
                *month = (uint8_t)m;
                *day = (uint8_t)d;
            }
            break;
        }
        uint8_t* next = p + h->length;
        while (next < end - 1 && (next[0] != 0 || next[1] != 0)) 
            next++;

        next += 2;
        p = next;
    }
    free(buffer);
    return 0;
}

static size_t bios_rom_size()
{
    UINT size = GetSystemFirmwareTable('RSMB', 0, NULL, 0);
    if (size == 0) 
        return 0;

    uint8_t* buffer = (uint8_t*)malloc(size);
    if (!buffer) 
        return 0;

    if (GetSystemFirmwareTable('RSMB', 0, buffer, size) != size) 
    {
        free(buffer);
        return 0;
    }

    uint8_t* p = buffer + 8;
    uint8_t* end = buffer + size;
    size_t rom_size = 0;
    while (p + sizeof(SMBIOSHeader) <= end) 
    {
        SMBIOSHeader* h = (SMBIOSHeader*)p;
        if (h->length < 4) 
            break;

        if (h->type == 0) 
        {
            uint8_t size_kb = *(p + 9);
            rom_size = (size_t)size_kb * 64 * 1024; // SMBIOS: size in 64KB units
            break;
        }

        uint8_t* next = p + h->length;
        while (next < end - 1 && (next[0] != 0 || next[1] != 0)) 
            next++;

        next += 2;
        p = next;
    }

    free(buffer);
    return rom_size;
}

static int bios_version(uint8_t* major, uint8_t* minor, uint8_t* patch)
{
    UINT size = GetSystemFirmwareTable('RSMB', 0, NULL, 0);
    if (size == 0) 
        return -1;

    uint8_t* buffer = (uint8_t*)malloc(size);
    if (!buffer) 
        return -1;

    if (GetSystemFirmwareTable('RSMB', 0, buffer, size) != size) 
    {
        free(buffer);
        return -1;
    }

    uint8_t* p = buffer + 8;
    uint8_t* end = buffer + size;
    while (p + sizeof(SMBIOSHeader) <= end) 
    {
        SMBIOSHeader* h = (SMBIOSHeader*)p;
        if (h->length < 4) 
            break;

        if (h->type == 0) 
        {
            uint8_t versionIndex = *(p + 5);
            char* str = (char*)(p + h->length);
            for (uint8_t i = 1; i < versionIndex && *str; i++)
                str += strlen(str) + 1;

            if (*str) 
                sscanf(str, "%hhu.%hhu.%hhu", major, minor, patch);

            break;
        }

        uint8_t* next = p + h->length;
        while (next < end - 1 && (next[0] != 0 || next[1] != 0)) 
            next++;

        next += 2;
        p = next;
    }

    free(buffer);
    return 0;
}

static int bios_serial_num(uint8_t* serial_num, size_t* length)
{
    UINT size = GetSystemFirmwareTable('RSMB', 0, NULL, 0);
    if (size == 0) 
        return -1;

    uint8_t* buffer = (uint8_t*)malloc(size);
    if (!buffer) 
        return -1;

    if (GetSystemFirmwareTable('RSMB', 0, buffer, size) != size) 
    {
        free(buffer);
        return -1;
    }

    uint8_t* p = buffer + 8;
    uint8_t* end = buffer + size;
    while (p + sizeof(SMBIOSHeader) <= end) 
    {
        SMBIOSHeader* h = (SMBIOSHeader*)p;
        if (h->length < 4) 
            break;

        if (h->type == 1) 
        {
            uint8_t serialIndex = *(p + 7);
            char* str = (char*)(p + h->length);
            for (uint8_t i = 1; i < serialIndex && *str; i++)
                str += strlen(str) + 1;

            if (*str) 
            {
                size_t len = strlen(str);
                if (len < *length) 
                {
                    memcpy(serial_num, str, len + 1);
                    *length = len;
                } 
                else 
                {
                    *length = 0;
                }
            }
            break;
        }

        uint8_t* next = p + h->length;
        while (next < end - 1 && (next[0] != 0 || next[1] != 0)) 
            next++;
            
        next += 2;
        p = next;
    }

    free(buffer);
    return 0;
}

#elif defined(__linux__)
#include <stdio.h>

static int bios_vendor(unsigned char* vendor, size_t* length)
{
    FILE* fp = fopen("/sys/class/dmi/id/bios_vendor", "r");
    if (!fp) 
        return -1;

    char buf[64] = {0};
    if (!fgets(buf, sizeof(buf), fp)) 
    { 
        fclose(fp); 
        return -1; 
    }

    fclose(fp);
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') 
        buf[--len] = 0;

    if (*length < len) 
        return -1;

    memcpy(vendor, buf, len);
    *length = len;
    return 0;
}

static uint16_t bios_starting_segment()
{
    // TODO
    return 0;
}

static int bios_release_date(uint8_t* year, uint8_t* month, uint8_t* day)
{
    FILE* fp = fopen("/sys/class/dmi/id/bios_date", "r");
    if (!fp) 
        return -1;

    char buf[32] = {0};
    if (!fgets(buf, sizeof(buf), fp)) 
    { 
        fclose(fp); 
        return -1; 
    }

    fclose(fp);
    int y, m, d;
    if (sscanf(buf, "%d/%d/%d", &m, &d, &y) == 3) 
    {
        *year = (uint8_t)(y % 100);
        *month = (uint8_t)m;
        *day = (uint8_t)d;
        return 0;
    }

    return -1;
}

static size_t bios_rom_size()
{
    // TODO
    return 0;
}

static int bios_version(uint8_t* major, uint8_t* minor, uint8_t* patch)
{
    FILE* fp = fopen("/sys/class/dmi/id/bios_version", "r");
    if (!fp) 
        return -1;

    char buf[32] = {0};
    if (!fgets(buf, sizeof(buf), fp)) 
    { 
        fclose(fp); 
        return -1; 
    }

    fclose(fp);
    sscanf(buf, "%hhu.%hhu.%hhu", major, minor, patch);
    return 0;
}

static int bios_serial_num(uint8_t* serial_num, size_t* length)
{
    FILE* fp = fopen("/sys/class/dmi/id/product_serial", "r");
    if (!fp) 
        return -1;

    char buf[64] = {0};
    if (!fgets(buf, sizeof(buf), fp)) 
    { 
        fclose(fp); 
        return -1; 
    }
    fclose(fp);
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') 
        buf[--len] = 0;

    if (*length < len) 
        return -1;

    memcpy(serial_num, buf, len);
    *length = len;
    return 0;
}

#elif defined(__APPLE__)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int bios_vendor(unsigned char* vendor, size_t* length)
{
    FILE* fp = popen("system_profiler SPHardwareDataType | grep 'Manufacturer'", "r");
    if (!fp) 
        return -1;

    char buf[128] = {0};
    if (!fgets(buf, sizeof(buf), fp)) 
    { 
        pclose(fp); 
        return -1; 
    }

    pclose(fp);
    char* mfg = strchr(buf, ':');
    if (!mfg) 
        return -1;

    mfg++;
    while (*mfg == ' ') 
        ++mfg;

    size_t len = strlen(mfg);
    if (len > 0 && mfg[len-1] == '\n') 
        mfg[--len] = 0;

    if (*length < len) 
        return -1;

    memcpy(vendor, mfg, len);
    *length = len;
    return 0;
}

static uint16_t bios_starting_segment()
{
    // TODO
    return 0;
}

static int bios_release_date(uint8_t* year, uint8_t* month, uint8_t* day)
{
    FILE* fp = popen("system_profiler SPHardwareDataType | grep 'Boot ROM Version'", "r");
    if (!fp) 
        return -1;

    char buf[128] = {0};
    if (!fgets(buf, sizeof(buf), fp)) 
    { 
        pclose(fp); 
        return -1; 
    }
    
    pclose(fp);
    *year = *month = *day = 0;
    return 0;
}

static size_t bios_rom_size()
{
    // TODO
    return 0;
}

static int bios_version(uint8_t* major, uint8_t* minor, uint8_t* patch)
{
    if (!major || !minor || !patch) 
        return -1;

    FILE* fp = popen("system_profiler SPHardwareDataType | grep 'Boot ROM Version'", "r");
    if (!fp) 
        return -1;

    char buf[128] = {0};
    if (!fgets(buf, sizeof(buf), fp)) 
    { 
        pclose(fp); 
        return -1; 
    }

    pclose(fp);
    // "Boot ROM Version: 123.0.0.0.0"
    char* ver = strchr(buf, ':');
    if (!ver) 
        return -1;

    ver++;
    while (*ver == ' ') 
        ++ver;

    int m=0, n=0, p=0;
    sscanf(ver, "%hhu.%hhu.%hhu", major, minor, patch);
    return 0;
}

static int bios_serial_num(uint8_t* serial_num, size_t* length)
{
    if (!serial_num || !length || *length < 32) 
        return -1;

    FILE* fp = popen("system_profiler SPHardwareDataType | grep 'Serial Number'", "r");
    if (!fp) 
        return -1;

    char buf[128] = {0};
    if (!fgets(buf, sizeof(buf), fp)) 
    { 
        pclose(fp); 
        return -1; 
    }

    pclose(fp);
    char* sn = strchr(buf, ':');
    if (!sn) 
        return -1;

    sn++;
    while (*sn == ' ') 
        ++sn;

    size_t len = strlen(sn);
    if (len > 0 && sn[len-1] == '\n') 
        sn[--len] = 0;

    if (*length < len) 
        return -1;

    memcpy(serial_num, sn, len);
    *length = len;
    return 0;
}

#else
static int bios_version(uint8_t* major, uint8_t* minor, uint8_t* patch)
{
    // TODO
    return -1;
}

static int bios_serial_num(uint8_t* serial_num, size_t* length)
{
    // TODO
    return -1;
}
#endif


#ifdef __cplusplus
}
#endif

#endif // BIOS_H
