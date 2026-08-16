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
#ifndef DISK_H
#define DISK_H

// ------------------- API Macro Definition ----------------------
#ifndef HJ_DISK_API
#if defined(HJ_DISK_STATIC)
#define HJ_DISK_API static inline
#else
#define HJ_DISK_API extern
#endif
#endif

// ------------------ Platform Detection ---------------------
#if defined(_WIN32) || defined(_WIN64)
#define HJ_DISK_PLATFORM_WINDOWS 1
#include <windows.h>
#include <setupapi.h>
#include <winioctl.h>

#elif defined(__linux__)
#define HJ_DISK_PLATFORM_LINUX 1
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <mntent.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <time.h>
#include <unistd.h>
#elif defined(__APPLE__)
#define HJ_DISK_PLATFORM_MACOS 1
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOBSD.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOBlockStorageDriver.h>
#include <IOKit/storage/IOMedia.h>
#include <errno.h>
#include <sys/disk.h>
#include <sys/mount.h>
#include <time.h>
#include <unistd.h>
#else
#define HJ_DISK_PLATFORM_UNKNOWN 1
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ------------------- Constants and Limits ----------------------
#define HJ_DISK_MAX_PATH_LENGTH 4096
#define HJ_DISK_MAX_DEVICE_NAME 256
#define HJ_DISK_MAX_FILESYSTEM_NAME 64
#define HJ_DISK_MAX_VOLUME_LABEL 256
#define HJ_DISK_SECTOR_SIZE_DEFAULT 512
#define HJ_DISK_MAX_DISKS 64
#define HJ_DISK_MAX_PARTITIONS 128

#ifdef __cplusplus
extern "C" {
#endif

// ------------------------ disk defines -------------------------
typedef enum
{
    HJ_DISK_OK                       = 0,
    HJ_DISK_ERROR_NULL_POINTER       = -1,
    HJ_DISK_ERROR_INVALID_PARAMETER  = -2,
    HJ_DISK_ERROR_BUFFER_TOO_SMALL   = -3,
    HJ_DISK_ERROR_NOT_FOUND          = -4,
    HJ_DISK_ERROR_ACCESS_DENIED      = -5,
    HJ_DISK_ERROR_DEVICE_BUSY        = -6,
    HJ_DISK_ERROR_IO_ERROR           = -7,
    HJ_DISK_ERROR_NOT_SUPPORTED      = -8,
    HJ_DISK_ERROR_MEMORY_ALLOCATION  = -9,
    HJ_DISK_ERROR_TIMEOUT            = -10,
    HJ_DISK_ERROR_INVALID_FILESYSTEM = -11,
    HJ_DISK_ERROR_DEVICE_OFFLINE     = -12,
    HJ_DISK_ERROR_QUOTA_EXCEEDED     = -13,
    HJ_DISK_ERROR_READ_ONLY          = -14,
    HJ_DISK_ERROR_CORRUPTED_DATA     = -15
} hj_disk_err_t;

typedef enum
{
    HJ_DISK_TYPE_UNKNOWN   = 0,
    HJ_DISK_TYPE_HDD       = 1,
    HJ_DISK_TYPE_SSD       = 2,
    HJ_DISK_TYPE_OPTICAL   = 3,
    HJ_DISK_TYPE_REMOVABLE = 4,
    HJ_DISK_TYPE_NETWORK   = 5,
    HJ_DISK_TYPE_RAM       = 6
} hj_disk_type_t;

typedef enum
{
    HJ_FILESYSTEM_UNKNOWN  = 0,
    HJ_FILESYSTEM_NTFS     = 1,
    HJ_FILESYSTEM_FAT16    = 2,
    HJ_FILESYSTEM_FAT32    = 3,
    HJ_FILESYSTEM_EXFAT    = 4,
    HJ_FILESYSTEM_EXT2     = 5,
    HJ_FILESYSTEM_EXT3     = 6,
    HJ_FILESYSTEM_EXT4     = 7,
    HJ_FILESYSTEM_XFS      = 8,
    HJ_FILESYSTEM_BTRFS    = 9,
    HJ_FILESYSTEM_ZFS      = 10,
    HJ_FILESYSTEM_F2FS     = 11,
    HJ_FILESYSTEM_HFS_PLUS = 12,
    HJ_FILESYSTEM_APFS     = 13,
    HJ_FILESYSTEM_UFS      = 14,
    HJ_FILESYSTEM_JFS      = 15,
    HJ_FILESYSTEM_REISERFS = 16,
    HJ_FILESYSTEM_ISO9660  = 17,
    HJ_FILESYSTEM_UDF      = 18,
    HJ_FILESYSTEM_SWAP     = 19,
    HJ_FILESYSTEM_TMPFS    = 20,
    HJ_FILESYSTEM_PROCFS   = 21,
    HJ_FILESYSTEM_SYSFS    = 22
} hj_filesystem_type_t;

typedef struct
{
    char           device_name[HJ_DISK_MAX_DEVICE_NAME];
    char           model[HJ_DISK_MAX_DEVICE_NAME];
    char           serial[HJ_DISK_MAX_DEVICE_NAME];
    hj_disk_type_t type;
    uint64_t       total_size;
    uint64_t       sector_size;
    uint64_t       sector_count;
    bool           removable;
    bool           read_only;
    uint32_t       rpm;
    double         temperature;
} hj_disk_info_t;

typedef struct
{
    char                 device_name[HJ_DISK_MAX_DEVICE_NAME];
    char                 mount_point[HJ_DISK_MAX_PATH_LENGTH];
    char                 volume_label[HJ_DISK_MAX_VOLUME_LABEL];
    hj_filesystem_type_t filesystem;
    uint64_t             start_sector;
    uint64_t             sector_count;
    uint64_t             total_size;
    uint64_t             used_size;
    uint64_t             available_size;
    bool                 bootable;
    bool                 read_only;
} hj_partition_info_t;

typedef struct
{
    uint64_t bytes_read;
    uint64_t bytes_written;
    uint64_t read_operations;
    uint64_t write_operations;
    double   read_time_ms;
    double   write_time_ms;
    uint32_t queue_depth;
} hj_disk_stats_t;

static const struct
{
    hj_filesystem_type_t type;
    const char          *name;
} hj_filesystem_names[] = {
    {HJ_FILESYSTEM_NTFS, "NTFS"},       {HJ_FILESYSTEM_FAT16, "FAT16"},
    {HJ_FILESYSTEM_FAT32, "FAT32"},     {HJ_FILESYSTEM_EXFAT, "exFAT"},
    {HJ_FILESYSTEM_EXT2, "ext2"},       {HJ_FILESYSTEM_EXT3, "ext3"},
    {HJ_FILESYSTEM_EXT4, "ext4"},       {HJ_FILESYSTEM_XFS, "XFS"},
    {HJ_FILESYSTEM_BTRFS, "Btrfs"},     {HJ_FILESYSTEM_ZFS, "ZFS"},
    {HJ_FILESYSTEM_F2FS, "F2FS"},       {HJ_FILESYSTEM_HFS_PLUS, "HFS+"},
    {HJ_FILESYSTEM_APFS, "APFS"},       {HJ_FILESYSTEM_UFS, "UFS"},
    {HJ_FILESYSTEM_JFS, "JFS"},         {HJ_FILESYSTEM_REISERFS, "ReiserFS"},
    {HJ_FILESYSTEM_ISO9660, "ISO9660"}, {HJ_FILESYSTEM_UDF, "UDF"},
    {HJ_FILESYSTEM_SWAP, "Linux swap"}, {HJ_FILESYSTEM_TMPFS, "tmpfs"},
    {HJ_FILESYSTEM_PROCFS, "proc"},     {HJ_FILESYSTEM_SYSFS, "sysfs"},
    {HJ_FILESYSTEM_UNKNOWN, "Unknown"}};

HJ_DISK_API hj_disk_err_t hj_disk_init(void);
HJ_DISK_API hj_disk_err_t hj_disk_format_size(uint64_t bytes,
                                              char    *buffer,
                                              size_t   buffer_size);
HJ_DISK_API int32_t       hj_disk_count(void);
HJ_DISK_API hj_filesystem_type_t
hj_disk_filesystem_type_from_string(const char *fs_name);
HJ_DISK_API const char *
hj_disk_filesystem_type_to_string(hj_filesystem_type_t fs_type);
HJ_DISK_API hj_disk_err_t hj_disk_info(const char     *device_name,
                                       hj_disk_info_t *info);
HJ_DISK_API hj_disk_err_t hj_disk_enumerate(hj_disk_info_t *disks,
                                            uint32_t        max_disks,
                                            uint32_t       *actual_count);
HJ_DISK_API bool          hj_disk_is_ready(const char *device_name);
HJ_DISK_API hj_disk_err_t hj_disk_get_partition_by_mount(
    const char *mount_point, hj_partition_info_t *info);
HJ_DISK_API hj_disk_err_t hj_disk_read_speed_test(const char *device_name,
                                                  uint32_t    test_size_mb,
                                                  double     *read_speed_mbps);

#ifdef __cplusplus
}
#endif

#endif // DISK_H

// --------------------- Implementation -------------------------
#if (defined(HJ_DISK_IMPL) || defined(HJ_DISK_STATIC))                         \
    && !defined(HJ_DISK_IMPL_DONE)
#define HJ_DISK_IMPL_DONE

static inline int _hj_stricmp(const char *s1, const char *s2)
{
#if defined(HJ_DISK_PLATFORM_WINDOWS)
    return _stricmp(s1, s2);
#else
    return strcasecmp(s1, s2);
#endif
}

#if defined(HJ_DISK_PLATFORM_LINUX)
static inline bool _hj_is_valid_disk(const char *device_name)
{
    size_t name_len = strlen(device_name);
    if((strncmp(device_name, "sd", 2) == 0 && name_len == 3)
       || (strncmp(device_name, "nvme", 4) == 0
           && strstr(device_name, "p") == NULL)
       || (strncmp(device_name, "hd", 2) == 0 && name_len == 3)
       || (strncmp(device_name, "mmcblk", 6) == 0
           && strstr(device_name, "p") == NULL)
       || (strncmp(device_name, "vd", 2) == 0 && name_len == 3))
    {
        char path[512];
        snprintf(path, sizeof(path), "/sys/block/%s/size", device_name);
        FILE *fp = fopen(path, "r");
        if(fp)
        {
            unsigned long long sectors = 0;
            if(fscanf(fp, "%llu", &sectors) == 1)
            {
                fclose(fp);
                if(sectors >= 2048) // 至少 1MB (2048 * 512 bytes)
                {
                    return true;
                }
            } else
            {
                fclose(fp);
            }
        }
    }
    return false;
}
#endif

HJ_DISK_API hj_disk_err_t hj_disk_format_size(uint64_t bytes,
                                              char    *buffer,
                                              size_t   buffer_size)
{
    if(!buffer || buffer_size == 0)
        return HJ_DISK_ERROR_INVALID_PARAMETER;

    const char  *units[]    = {"B", "KB", "MB", "GB", "TB", "PB"};
    const size_t num_units  = sizeof(units) / sizeof(units[0]);
    double       size       = (double) bytes;
    size_t       unit_index = 0;
    while(size >= 1024.0 && unit_index < num_units - 1)
    {
        size /= 1024.0;
        unit_index++;
    }

    int result;
    if(unit_index == 0)
        result =
            snprintf(buffer, buffer_size, "%.0f %s", size, units[unit_index]);
    else
        result =
            snprintf(buffer, buffer_size, "%.2f %s", size, units[unit_index]);

    if(result < 0 || (size_t) result >= buffer_size)
        return HJ_DISK_ERROR_BUFFER_TOO_SMALL;

    return HJ_DISK_OK;
}

HJ_DISK_API hj_disk_err_t hj_disk_init(void)
{
#if defined(HJ_DISK_PLATFORM_WINDOWS)
    return HJ_DISK_OK;
#elif defined(HJ_DISK_PLATFORM_LINUX)
    return HJ_DISK_OK;
#elif defined(HJ_DISK_PLATFORM_MACOS)
    return HJ_DISK_OK;
#else
    return HJ_DISK_ERROR_NOT_SUPPORTED;
#endif
}

HJ_DISK_API int32_t hj_disk_count(void)
{
#if defined(HJ_DISK_PLATFORM_WINDOWS)
    uint32_t disk_cnt = 0;
    DWORD    drives   = GetLogicalDrives();
    for(int i = 0; i < 26; i++)
    {
        if(!(drives & (1 << i)))
            continue;

        char drive_path[4] = {(char) ('A' + i), ':', '\\', '\0'};
        UINT drive_type    = GetDriveTypeA(drive_path);
        if(drive_type == DRIVE_FIXED || drive_type == DRIVE_REMOVABLE)
            disk_cnt++;
    }
    return (int32_t) disk_cnt;

#elif defined(HJ_DISK_PLATFORM_LINUX)
    DIR *dir = opendir("/sys/block");
    if(!dir)
        return 0;

    uint32_t       disk_cnt = 0;
    struct dirent *entry;
    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_name[0] == '.')
            continue;
        if(_hj_is_valid_disk(entry->d_name))
        {
            disk_cnt++;
        }
    }
    closedir(dir);
    return (int32_t) disk_cnt;

#elif defined(HJ_DISK_PLATFORM_MACOS)
    io_iterator_t          disk_list;
    CFMutableDictionaryRef matching = IOServiceMatching("IOMedia");
    if(!matching)
        return 0;

    CFDictionarySetValue(matching, CFSTR(kIOMediaWholeKey), kCFBooleanTrue);
    kern_return_t result = IOServiceGetMatchingServices(kIOMasterPortDefault,
                                                        matching,
                                                        &disk_list);
    if(result != KERN_SUCCESS)
        return 0;

    uint32_t    disk_cnt = 0;
    io_object_t disk_obj;
    while((disk_obj = IOIteratorNext(disk_list)) != 0)
    {
        CFTypeRef size_ref =
            IORegistryEntryCreateCFProperty(disk_obj,
                                            CFSTR(kIOMediaSizeKey),
                                            kCFAllocatorDefault,
                                            0);
        if(size_ref)
        {
            uint64_t size = 0;
            CFNumberGetValue((CFNumberRef) size_ref,
                             kCFNumberSInt64Type,
                             &size);
            CFRelease(size_ref);
            if(size > 1024 * 1024)
                disk_cnt++;
        }
        IOObjectRelease(disk_obj);
    }

    IOObjectRelease(disk_list);
    return (int32_t) disk_cnt;

#else
    return HJ_DISK_ERROR_NOT_SUPPORTED;

#endif
}

HJ_DISK_API hj_filesystem_type_t
hj_disk_filesystem_type_from_string(const char *fs_name)
{
    if(!fs_name)
        return HJ_FILESYSTEM_UNKNOWN;
    if(_hj_stricmp(fs_name, "ntfs") == 0)
        return HJ_FILESYSTEM_NTFS;
    if(_hj_stricmp(fs_name, "fat16") == 0)
        return HJ_FILESYSTEM_FAT16;
    if(_hj_stricmp(fs_name, "fat32") == 0)
        return HJ_FILESYSTEM_FAT32;
    if(_hj_stricmp(fs_name, "exfat") == 0)
        return HJ_FILESYSTEM_EXFAT;
    if(strcmp(fs_name, "ext2") == 0)
        return HJ_FILESYSTEM_EXT2;
    if(strcmp(fs_name, "ext3") == 0)
        return HJ_FILESYSTEM_EXT3;
    if(strcmp(fs_name, "ext4") == 0)
        return HJ_FILESYSTEM_EXT4;
    if(_hj_stricmp(fs_name, "xfs") == 0)
        return HJ_FILESYSTEM_XFS;
    if(strcmp(fs_name, "btrfs") == 0)
        return HJ_FILESYSTEM_BTRFS;
    if(_hj_stricmp(fs_name, "zfs") == 0)
        return HJ_FILESYSTEM_ZFS;
    if(_hj_stricmp(fs_name, "f2fs") == 0)
        return HJ_FILESYSTEM_F2FS;
    if(_hj_stricmp(fs_name, "hfs+") == 0 || _hj_stricmp(fs_name, "hfs") == 0)
        return HJ_FILESYSTEM_HFS_PLUS;
    if(_hj_stricmp(fs_name, "apfs") == 0)
        return HJ_FILESYSTEM_APFS;
    if(_hj_stricmp(fs_name, "ufs") == 0)
        return HJ_FILESYSTEM_UFS;
    if(_hj_stricmp(fs_name, "jfs") == 0)
        return HJ_FILESYSTEM_JFS;
    if(_hj_stricmp(fs_name, "reiserfs") == 0)
        return HJ_FILESYSTEM_REISERFS;
    if(_hj_stricmp(fs_name, "iso9660") == 0)
        return HJ_FILESYSTEM_ISO9660;
    if(_hj_stricmp(fs_name, "udf") == 0)
        return HJ_FILESYSTEM_UDF;
    if(_hj_stricmp(fs_name, "swap") == 0)
        return HJ_FILESYSTEM_SWAP;
    if(_hj_stricmp(fs_name, "tmpfs") == 0)
        return HJ_FILESYSTEM_TMPFS;
    if(_hj_stricmp(fs_name, "proc") == 0 || _hj_stricmp(fs_name, "procfs") == 0)
        return HJ_FILESYSTEM_PROCFS;
    if(_hj_stricmp(fs_name, "sysfs") == 0)
        return HJ_FILESYSTEM_SYSFS;

    return HJ_FILESYSTEM_UNKNOWN;
}

HJ_DISK_API const char *
hj_disk_filesystem_type_to_string(hj_filesystem_type_t fs_type)
{
    for(size_t i = 0;
        i < sizeof(hj_filesystem_names) / sizeof(hj_filesystem_names[0]);
        i++)
    {
        if(hj_filesystem_names[i].type == fs_type)
            return hj_filesystem_names[i].name;
    }
    return "Unknown";
}

HJ_DISK_API hj_disk_err_t hj_disk_info(const char     *device_name,
                                       hj_disk_info_t *info)
{
    if(!device_name || !info)
        return HJ_DISK_ERROR_INVALID_PARAMETER;

    memset(info, 0, sizeof(hj_disk_info_t));
    strncpy(info->device_name, device_name, sizeof(info->device_name) - 1);
    info->temperature = -1.0;

#if defined(HJ_DISK_PLATFORM_WINDOWS)
    char full_path[MAX_PATH];
    snprintf(full_path, sizeof(full_path), "\\\\.\\%s", device_name);
    HANDLE hDevice = CreateFileA(full_path,
                                 0,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 0,
                                 NULL);

    if(hDevice == INVALID_HANDLE_VALUE)
        return HJ_DISK_ERROR_ACCESS_DENIED;

    DISK_GEOMETRY_EX geometry;
    DWORD            bytes_returned;
    if(DeviceIoControl(hDevice,
                       IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
                       NULL,
                       0,
                       &geometry,
                       sizeof(geometry),
                       &bytes_returned,
                       NULL))
    {
        info->total_size   = geometry.DiskSize.QuadPart;
        info->sector_size  = geometry.Geometry.BytesPerSector;
        info->sector_count = info->total_size / info->sector_size;
    }

    UINT drive_type = GetDriveTypeA(device_name);
    switch(drive_type)
    {
        case DRIVE_FIXED:
            info->type = HJ_DISK_TYPE_HDD;
            break;
        case DRIVE_REMOVABLE:
            info->type      = HJ_DISK_TYPE_REMOVABLE;
            info->removable = true;
            break;
        case DRIVE_CDROM:
            info->type      = HJ_DISK_TYPE_OPTICAL;
            info->removable = true;
            break;
        case DRIVE_REMOTE:
            info->type = HJ_DISK_TYPE_NETWORK;
            break;
        case DRIVE_RAMDISK:
            info->type = HJ_DISK_TYPE_RAM;
            break;
        default:
            info->type = HJ_DISK_TYPE_UNKNOWN;
            break;
    }
    CloseHandle(hDevice);

#elif defined(HJ_DISK_PLATFORM_LINUX)
    char full_path[256];
    if(strncmp(device_name, "/dev/", 5) == 0)
        strncpy(full_path, device_name, sizeof(full_path) - 1);
    else
        snprintf(full_path, sizeof(full_path), "/dev/%s", device_name);

    int fd = open(full_path, O_RDONLY);
    if(fd < 0)
        return HJ_DISK_ERROR_ACCESS_DENIED;

    uint64_t size_in_bytes;
    if(ioctl(fd, BLKGETSIZE64, &size_in_bytes) == 0)
        info->total_size = size_in_bytes;

    int sector_size;
    if(ioctl(fd, BLKSSZGET, &sector_size) == 0)
    {
        info->sector_size  = sector_size;
        info->sector_count = info->total_size / info->sector_size;
    } else
    {
        info->sector_size  = HJ_DISK_SECTOR_SIZE_DEFAULT;
        info->sector_count = info->total_size / HJ_DISK_SECTOR_SIZE_DEFAULT;
    }

    char        sys_path[512];
    char        rotational[16];
    const char *basename_dev = strrchr(device_name, '/');
    basename_dev             = basename_dev ? basename_dev + 1 : device_name;
    snprintf(sys_path,
             sizeof(sys_path),
             "/sys/block/%s/queue/rotational",
             basename_dev);

    FILE *fp = fopen(sys_path, "r");
    if(fp && fgets(rotational, sizeof(rotational), fp))
    {
        if(rotational[0] == '0')
        {
            info->type = HJ_DISK_TYPE_SSD;
            info->rpm  = 0;
        } else
        {
            info->type = HJ_DISK_TYPE_HDD;
            info->rpm  = 7200;
        }
        fclose(fp);
    } else
    {
        info->type = HJ_DISK_TYPE_UNKNOWN;
    }
    close(fd);

#elif defined(HJ_DISK_PLATFORM_MACOS)
    info->type        = HJ_DISK_TYPE_UNKNOWN;
    info->sector_size = HJ_DISK_SECTOR_SIZE_DEFAULT;

#else
    return HJ_DISK_ERROR_NOT_SUPPORTED;
#endif

    if(info->sector_size == 0)
        info->sector_size = HJ_DISK_SECTOR_SIZE_DEFAULT;

    if(info->sector_count == 0 && info->total_size > 0)
        info->sector_count = info->total_size / info->sector_size;

    strncpy(info->model, "Generic Disk", sizeof(info->model) - 1);
    strncpy(info->serial, "Unknown", sizeof(info->serial) - 1);
    return HJ_DISK_OK;
}

HJ_DISK_API hj_disk_err_t hj_disk_get_partition_by_mount(
    const char *mount_point, hj_partition_info_t *info)
{
    if(!mount_point || !info)
        return HJ_DISK_ERROR_INVALID_PARAMETER;

    memset(info, 0, sizeof(hj_partition_info_t));
    strncpy(info->mount_point, mount_point, sizeof(info->mount_point) - 1);

#if defined(HJ_DISK_PLATFORM_WINDOWS)
    ULARGE_INTEGER free_bytes, total_bytes, total_free_bytes;
    if(GetDiskFreeSpaceExA(mount_point,
                           &free_bytes,
                           &total_bytes,
                           &total_free_bytes))
    {
        info->total_size     = total_bytes.QuadPart;
        info->available_size = free_bytes.QuadPart;
        info->used_size      = total_bytes.QuadPart - free_bytes.QuadPart;
    }

    char  volume_name[MAX_PATH];
    char  filesystem_name[MAX_PATH];
    DWORD serial_number, max_component_length, filesystem_flags;
    if(GetVolumeInformationA(mount_point,
                             volume_name,
                             sizeof(volume_name),
                             &serial_number,
                             &max_component_length,
                             &filesystem_flags,
                             filesystem_name,
                             sizeof(filesystem_name)))
    {
        strncpy(info->volume_label,
                volume_name,
                sizeof(info->volume_label) - 1);
        info->filesystem = hj_disk_filesystem_type_from_string(filesystem_name);
        info->read_only  = (filesystem_flags & FILE_READ_ONLY_VOLUME) != 0;
    }

    char device_path[MAX_PATH];
    if(QueryDosDeviceA(&mount_point[0], device_path, sizeof(device_path)))
        strncpy(info->device_name, device_path, sizeof(info->device_name) - 1);

    return HJ_DISK_OK;

#elif defined(HJ_DISK_PLATFORM_LINUX)
    struct statvfs vfs;
    if(statvfs(mount_point, &vfs) == 0)
    {
        info->total_size     = (uint64_t) vfs.f_blocks * vfs.f_frsize;
        info->available_size = (uint64_t) vfs.f_bavail * vfs.f_frsize;
        info->used_size =
            info->total_size - ((uint64_t) vfs.f_bfree * vfs.f_frsize);
        info->read_only = (vfs.f_flag & ST_RDONLY) != 0;
    }

    FILE *fp = fopen("/proc/mounts", "r");
    if(!fp)
        return HJ_DISK_ERROR_NOT_FOUND;

    char line[1024];
    while(fgets(line, sizeof(line), fp))
    {
        char device[256], mountpoint[256], fstype[64];
        if(sscanf(line, "%255s %255s %63s", device, mountpoint, fstype) != 3)
            continue;

        if(strcmp(mountpoint, mount_point) == 0)
        {
            strncpy(info->device_name, device, sizeof(info->device_name) - 1);
            info->filesystem = hj_disk_filesystem_type_from_string(fstype);
            break;
        }
    }
    fclose(fp);
    return HJ_DISK_OK;

#elif defined(HJ_DISK_PLATFORM_MACOS)
    struct statfs fs;
    if(statfs(mount_point, &fs) == 0)
    {
        info->total_size     = (uint64_t) fs.f_blocks * fs.f_bsize;
        info->available_size = (uint64_t) fs.f_bavail * fs.f_bsize;
        info->used_size =
            info->total_size - ((uint64_t) fs.f_bfree * fs.f_bsize);
        info->read_only = (fs.f_flags & MNT_RDONLY) != 0;

        strncpy(info->device_name,
                fs.f_mntfromname,
                sizeof(info->device_name) - 1);
        info->filesystem = hj_disk_filesystem_type_from_string(fs.f_fstypename);
    }
    return HJ_DISK_OK;

#else
    return HJ_DISK_ERROR_NOT_SUPPORTED;
#endif
}

HJ_DISK_API bool hj_disk_is_ready(const char *device_name)
{
    if(!device_name || strlen(device_name) == 0)
        return false;

    bool ready = false;

#if defined(HJ_DISK_PLATFORM_WINDOWS)
    char full_path[MAX_PATH];
    snprintf(full_path, sizeof(full_path), "\\\\.\\%s", device_name);
    HANDLE hDevice = CreateFileA(full_path,
                                 0,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 0,
                                 NULL);
    if(hDevice != INVALID_HANDLE_VALUE)
    {
        DISK_GEOMETRY geometry;
        DWORD         bytes_returned;
        if(DeviceIoControl(hDevice,
                           IOCTL_DISK_GET_DRIVE_GEOMETRY,
                           NULL,
                           0,
                           &geometry,
                           sizeof(geometry),
                           &bytes_returned,
                           NULL))
        {
            ready = true;
        } else
        {
            DWORD error = GetLastError();
            if(error != ERROR_NOT_READY && error != ERROR_ACCESS_DENIED)
                ready = true;
        }
        CloseHandle(hDevice);
    }

#elif defined(HJ_DISK_PLATFORM_LINUX)
    char full_path[256];
    if(strncmp(device_name, "/dev/", 5) == 0)
        strncpy(full_path, device_name, sizeof(full_path) - 1);
    else
        snprintf(full_path, sizeof(full_path), "/dev/%s", device_name);

    full_path[sizeof(full_path) - 1] = '\0';
    struct stat st;
    if(stat(full_path, &st) != 0)
        return false;

    if(!S_ISBLK(st.st_mode))
        return false;

    int fd = open(full_path, O_RDONLY | O_NONBLOCK);
    if(fd >= 0)
    {
        uint64_t size = 0;
        if(ioctl(fd, BLKGETSIZE64, &size) == 0 && size > 0)
            ready = true;

        close(fd);
    } else
    {
        if(errno == EACCES)
            ready = true;
    }

#elif defined(HJ_DISK_PLATFORM_MACOS)
    io_iterator_t          disk_list;
    CFMutableDictionaryRef matching = IOServiceMatching("IOMedia");
    if(!matching)
        return false;

    CFDictionarySetValue(matching, CFSTR(kIOMediaWholeKey), kCFBooleanTrue);
    kern_return_t result = IOServiceGetMatchingServices(kIOMasterPortDefault,
                                                        matching,
                                                        &disk_list);
    if(result != KERN_SUCCESS)
        return false;

    io_object_t disk_obj;
    while((disk_obj = IOIteratorNext(disk_list)) != 0)
    {
        CFTypeRef path_ref =
            IORegistryEntryCreateCFProperty(disk_obj,
                                            CFSTR("BSD Name"),
                                            kCFAllocatorDefault,
                                            0);
        if(path_ref)
        {
            char bsd_name[256];
            CFStringGetCString((CFStringRef) path_ref,
                               bsd_name,
                               sizeof(bsd_name),
                               kCFStringEncodingUTF8);
            CFRelease(path_ref);

            char full_device_path[256];
            snprintf(full_device_path,
                     sizeof(full_device_path),
                     "/dev/%s",
                     bsd_name);
            if(strcmp(device_name, bsd_name) != 0
               && strcmp(device_name, full_device_path) != 0)
            {
                IOObjectRelease(disk_obj);
                continue;
            }

            CFTypeRef size_ref =
                IORegistryEntryCreateCFProperty(disk_obj,
                                                CFSTR(kIOMediaSizeKey),
                                                kCFAllocatorDefault,
                                                0);
            if(size_ref)
            {
                uint64_t size = 0;
                CFNumberGetValue((CFNumberRef) size_ref,
                                 kCFNumberSInt64Type,
                                 &size);
                CFRelease(size_ref);
                if(size > 0)
                    ready = true;
            }
            IOObjectRelease(disk_obj);
            break;
        }
        IOObjectRelease(disk_obj);
    }
    IOObjectRelease(disk_list);

#else
    ready = false;
#endif

    return ready;
}

HJ_DISK_API hj_disk_err_t hj_disk_enumerate(hj_disk_info_t *disks,
                                            uint32_t        max_disks,
                                            uint32_t       *actual_count)
{
    if(!disks || !actual_count || max_disks == 0)
        return HJ_DISK_ERROR_INVALID_PARAMETER;

    *actual_count = 0;

#if defined(HJ_DISK_PLATFORM_WINDOWS)
    DWORD drives = GetLogicalDrives();
    for(int i = 0; i < 26 && *actual_count < max_disks; i++)
    {
        if(!(drives & (1 << i)))
            continue;

        char drive_letter[4] = {(char) ('A' + i), ':', '\0', '\0'};
        UINT drive_type      = GetDriveTypeA(drive_letter);

        if(drive_type != DRIVE_FIXED && drive_type != DRIVE_REMOVABLE)
            continue;

        hj_disk_err_t result =
            hj_disk_info(drive_letter, &disks[*actual_count]);
        if(result == HJ_DISK_OK)
            (*actual_count)++;
    }

#elif defined(HJ_DISK_PLATFORM_LINUX)
    DIR *dir = opendir("/sys/block");
    if(!dir)
        return HJ_DISK_ERROR_NOT_FOUND;

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL && *actual_count < max_disks)
    {
        if(entry->d_name[0] == '.')
            continue;

        if(_hj_is_valid_disk(entry->d_name))
        {
            char dev_path[256];
            snprintf(dev_path, sizeof(dev_path), "/dev/%s", entry->d_name);

            hj_disk_err_t result =
                hj_disk_info(dev_path, &disks[*actual_count]);
            if(result == HJ_DISK_OK)
            {
                (*actual_count)++;
            }
        }
    }
    closedir(dir);

#elif defined(HJ_DISK_PLATFORM_MACOS)
    if(max_disks > 0)
    {
        hj_disk_err_t result = hj_disk_info("/dev/disk0", &disks[0]);
        if(result == HJ_DISK_OK)
            *actual_count = 1;
    }

#else
    return HJ_DISK_ERROR_NOT_SUPPORTED;
#endif

    return HJ_DISK_OK;
}

HJ_DISK_API hj_disk_err_t hj_disk_read_speed_test(const char *device_name,
                                                  uint32_t    test_size_mb,
                                                  double     *read_speed_mbps)
{
    if(!device_name || !read_speed_mbps || test_size_mb == 0)
        return HJ_DISK_ERROR_INVALID_PARAMETER;

    if(test_size_mb > 1024)
        test_size_mb = 1024;

    *read_speed_mbps = 0.0;

    const size_t   buffer_size = 1024 * 1024;
    const uint64_t total_bytes = (uint64_t) test_size_mb * 1024 * 1024;
    const uint32_t iterations  = (uint32_t) (total_bytes / buffer_size);

    if(iterations == 0)
        return HJ_DISK_ERROR_INVALID_PARAMETER;

    void *buffer = NULL;

#if defined(HJ_DISK_PLATFORM_WINDOWS)
    char full_path[MAX_PATH];
    snprintf(full_path, sizeof(full_path), "\\\\.\\%s", device_name);

    HANDLE hDevice =
        CreateFileA(full_path,
                    GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_NO_BUFFERING | FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL);

    if(hDevice == INVALID_HANDLE_VALUE)
        return HJ_DISK_ERROR_ACCESS_DENIED;

    buffer = VirtualAlloc(NULL,
                          buffer_size,
                          MEM_COMMIT | MEM_RESERVE,
                          PAGE_READWRITE);
    if(!buffer)
    {
        CloseHandle(hDevice);
        return HJ_DISK_ERROR_BUFFER_TOO_SMALL;
    }

    DISK_GEOMETRY_EX geometry;
    DWORD            bytes_returned;
    uint64_t         device_size = 0;
    if(DeviceIoControl(hDevice,
                       IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
                       NULL,
                       0,
                       &geometry,
                       sizeof(geometry),
                       &bytes_returned,
                       NULL))
    {
        device_size = geometry.DiskSize.QuadPart;
    }

    if(device_size > 0 && total_bytes > device_size)
    {
        VirtualFree(buffer, 0, MEM_RELEASE);
        CloseHandle(hDevice);
        return HJ_DISK_ERROR_INVALID_PARAMETER;
    }

    LARGE_INTEGER frequency, start_time, end_time;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start_time);

    uint64_t total_read = 0;
    for(uint32_t i = 0; i < iterations; i++)
    {
        DWORD bytes_read;
        if(!ReadFile(hDevice, buffer, buffer_size, &bytes_read, NULL))
        {
            DWORD error = GetLastError();
            if(error == ERROR_HANDLE_EOF)
                break;

            VirtualFree(buffer, 0, MEM_RELEASE);
            CloseHandle(hDevice);
            return HJ_DISK_ERROR_IO_ERROR;
        }
        total_read += bytes_read;
        if(bytes_read < buffer_size)
            break;
    }

    QueryPerformanceCounter(&end_time);
    VirtualFree(buffer, 0, MEM_RELEASE);
    CloseHandle(hDevice);

    double elapsed_seconds =
        (double) (end_time.QuadPart - start_time.QuadPart) / frequency.QuadPart;
    if(elapsed_seconds > 0.0)
    {
        double bytes_per_second = total_read / elapsed_seconds;
        *read_speed_mbps        = bytes_per_second / (1024.0 * 1024.0);
    }

#elif defined(HJ_DISK_PLATFORM_LINUX)
    char full_path[256];
    if(strncmp(device_name, "/dev/", 5) == 0)
        strncpy(full_path, device_name, sizeof(full_path) - 1);
    else
        snprintf(full_path, sizeof(full_path), "/dev/%s", device_name);

    full_path[sizeof(full_path) - 1] = '\0';
    int fd                           = open(full_path, O_RDONLY | O_DIRECT);
    if(fd < 0)
    {
        fd = open(full_path, O_RDONLY);
        if(fd < 0)
            return HJ_DISK_ERROR_ACCESS_DENIED;
    }

    if(posix_memalign(&buffer, 4096, buffer_size) != 0)
    {
        close(fd);
        return HJ_DISK_ERROR_BUFFER_TOO_SMALL;
    }

    uint64_t device_size = 0;
    if(ioctl(fd, BLKGETSIZE64, &device_size) == 0)
    {
        if(total_bytes > device_size)
        {
            free(buffer);
            close(fd);
            return HJ_DISK_ERROR_INVALID_PARAMETER;
        }
    }

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    uint64_t total_read = 0;
    for(uint32_t i = 0; i < iterations; i++)
    {
        ssize_t bytes_read = read(fd, buffer, buffer_size);
        if(bytes_read < 0)
        {
            free(buffer);
            close(fd);
            return HJ_DISK_ERROR_IO_ERROR;
        }
        total_read += bytes_read;
        if((size_t) bytes_read < buffer_size)
            break;
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    free(buffer);
    close(fd);

    double elapsed_seconds = (end_time.tv_sec - start_time.tv_sec)
                             + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    if(elapsed_seconds > 0.0)
    {
        double bytes_per_second = total_read / elapsed_seconds;
        *read_speed_mbps        = bytes_per_second / (1024.0 * 1024.0);
    }

#elif defined(HJ_DISK_PLATFORM_MACOS)
    char full_path[256];
    if(strncmp(device_name, "/dev/", 5) == 0)
        strncpy(full_path, device_name, sizeof(full_path) - 1);
    else
        snprintf(full_path, sizeof(full_path), "/dev/%s", device_name);

    full_path[sizeof(full_path) - 1] = '\0';
    int fd                           = open(full_path, O_RDONLY);
    if(fd < 0)
        return HJ_DISK_ERROR_ACCESS_DENIED;

    if(posix_memalign(&buffer, 4096, buffer_size) != 0)
    {
        close(fd);
        return HJ_DISK_ERROR_BUFFER_TOO_SMALL;
    }

    uint64_t device_size = 0;
    if(ioctl(fd, DKIOCGETBLOCKCOUNT, &device_size) == 0)
    {
        uint32_t block_size = 0;
        if(ioctl(fd, DKIOCGETBLOCKSIZE, &block_size) == 0)
        {
            device_size *= block_size;
            if(total_bytes > device_size)
            {
                free(buffer);
                close(fd);
                return HJ_DISK_ERROR_INVALID_PARAMETER;
            }
        }
    }

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    uint64_t total_read = 0;
    for(uint32_t i = 0; i < iterations; i++)
    {
        ssize_t bytes_read = read(fd, buffer, buffer_size);
        if(bytes_read < 0)
        {
            free(buffer);
            close(fd);
            return HJ_DISK_ERROR_IO_ERROR;
        }
        total_read += bytes_read;
        if((size_t) bytes_read < buffer_size)
            break;
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    free(buffer);
    close(fd);

    double elapsed_seconds = (end_time.tv_sec - start_time.tv_sec)
                             + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    if(elapsed_seconds > 0.0)
    {
        double bytes_per_second = total_read / elapsed_seconds;
        *read_speed_mbps        = bytes_per_second / (1024.0 * 1024.0);
    }

#else
    return HJ_DISK_ERROR_NOT_SUPPORTED;
#endif

    return HJ_DISK_OK;
}

#endif // HJ_DISK_IMPL && !HJ_DISK_IMPL_DONE