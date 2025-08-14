#ifndef DISK_H
#define DISK_H

// ------------------ Platform Detection ---------------------
#if defined(_WIN32) || defined(_WIN64)
#define DISK_PLATFORM_WINDOWS 1
#include <windows.h>
#include <setupapi.h>
#include <winioctl.h>

#elif defined(__linux__)
#define DISK_PLATFORM_LINUX 1
#include <fcntl.h>
#include <linux/fs.h>
#include <mntent.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
#elif defined(__APPLE__)
#define DISK_PLATFORM_MACOS 1
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOBlockStorageDriver.h>
#include <sys/disk.h>
#include <sys/mount.h>
#else
#define DISK_PLATFORM_UNKNOWN 1
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ------------------- Constants and Limits ----------------------
#define DISK_MAX_PATH_LENGTH 4096
#define DISK_MAX_DEVICE_NAME 256
#define DISK_MAX_FILESYSTEM_NAME 64
#define DISK_MAX_VOLUME_LABEL 256
#define DISK_SECTOR_SIZE_DEFAULT 512
#define DISK_MAX_DISKS 64
#define DISK_MAX_PARTITIONS 128

#ifdef __cplusplus
extern "C"
{
#endif

// ------------------------ Error Codes -------------------------
typedef enum
{
    DISK_SUCCESS = 0,
    DISK_ERROR_INVALID_PARAMETER = -1,
    DISK_ERROR_ACCESS_DENIED = -2,
    DISK_ERROR_NOT_FOUND = -3,
    DISK_ERROR_INSUFFICIENT_BUFFER = -4,
    DISK_ERROR_IO_ERROR = -5,
    DISK_ERROR_NOT_SUPPORTED = -6,
    DISK_ERROR_INSUFFICIENT_MEMORY = -7,
    DISK_ERROR_SYSTEM_ERROR = -8,
    DISK_ERROR_INVALID_FILESYSTEM = -9,
    DISK_ERROR_DEVICE_BUSY = -10
} disk_err_t;

// ------------------ Disk Types -------------------------
typedef enum
{
    DISK_TYPE_UNKNOWN = 0,
    DISK_TYPE_HDD = 1,
    DISK_TYPE_SSD = 2,
    DISK_TYPE_OPTICAL = 3,
    DISK_TYPE_REMOVABLE = 4,
    DISK_TYPE_NETWORK = 5,
    DISK_TYPE_RAM = 6
} disk_type_t;

// -------------------- Filesystem Types --------------------
typedef enum
{
    FILESYSTEM_UNKNOWN = 0,
    FILESYSTEM_NTFS = 1,
    FILESYSTEM_FAT32 = 2,
    FILESYSTEM_EXFAT = 3,
    FILESYSTEM_EXT2 = 4,
    FILESYSTEM_EXT3 = 5,
    FILESYSTEM_EXT4 = 6,
    FILESYSTEM_XFS = 7,
    FILESYSTEM_BTRFS = 8,
    FILESYSTEM_ZFS = 9,
    FILESYSTEM_HFS_PLUS = 10,
    FILESYSTEM_APFS = 11,
    FILESYSTEM_UFS = 12,
    FILESYSTEM_ISO9660 = 13
} filesystem_type_t;

// Disk information structure
typedef struct
{
    // Device name (e.g., "/dev/sda", "C:")
    char device_name[DISK_MAX_DEVICE_NAME];
    // Disk model name
    char model[DISK_MAX_DEVICE_NAME];
    // Serial number
    char serial[DISK_MAX_DEVICE_NAME];
    // Disk type
    disk_type_t type;
    // Total size in bytes
    uint64_t total_size;
    // Sector size in bytes
    uint64_t sector_size;
    // Total number of sectors
    uint64_t sector_count;
    // Is removable disk
    bool removable;
    // Is read-only
    bool read_only;
    // Rotation speed (0 for SSD)
    uint32_t rpm;
    // Temperature in Celsius (-1 if unknown)
    double temperature;
} disk_info_t;

// Partition information structure
typedef struct
{
    // Partition device
    char device_name[DISK_MAX_DEVICE_NAME];
    // Mount point
    char mount_point[DISK_MAX_PATH_LENGTH];
    // Volume label
    char volume_label[DISK_MAX_VOLUME_LABEL];
    // Filesystem type
    filesystem_type_t filesystem;
    // Starting sector
    uint64_t start_sector;
    // Number of sectors
    uint64_t sector_count;
    // Total size in bytes
    uint64_t total_size;
    // Used size in bytes
    uint64_t used_size;
    // Available size in bytes
    uint64_t available_size;
    // Is bootable partition
    bool bootable;
    // Is read-only
    bool read_only;
} partition_info_t;

// Disk usage statistics
typedef struct
{
    // Total bytes read
    uint64_t bytes_read;
    // Total bytes written
    uint64_t bytes_written;
    // Number of read operations
    uint64_t read_operations;
    // Number of write operations
    uint64_t write_operations;
    // Total read time in ms
    double read_time_ms;
    // Total write time in ms
    double write_time_ms;
    // Current queue depth
    uint32_t queue_depth;
} disk_stats_t;

// -------------------------- disk API -----------------------------
static disk_err_t disk_format_size(uint64_t bytes, char *buffer, size_t buffer_size)
{
    if (!buffer || buffer_size == 0)
        return DISK_ERROR_INVALID_PARAMETER;

    const char *units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    const size_t num_units = sizeof(units) / sizeof(units[0]);
    double size = (double)bytes;
    size_t unit_index = 0;
    while (size >= 1024.0 && unit_index < num_units - 1)
    {
        size /= 1024.0;
        unit_index++;
    }

    int result;
    if (unit_index == 0)
        result = snprintf(buffer, buffer_size, "%.0f %s", size,
                            units[unit_index]);
    else
        result = snprintf(buffer, buffer_size, "%.2f %s", size,
                            units[unit_index]);

    if (result < 0 || (size_t)result >= buffer_size)
        return DISK_ERROR_INSUFFICIENT_BUFFER;

    return DISK_SUCCESS;
}

static disk_err_t disk_init(void)
{
#if defined(DISK_PLATFORM_WINDOWS)
    return DISK_SUCCESS;
#elif defined(DISK_PLATFORM_LINUX)
    return DISK_SUCCESS;
#elif defined(DISK_PLATFORM_MACOS)
    return DISK_SUCCESS;
#else
    return DISK_ERROR_NOT_SUPPORTED;
#endif
}

static uint32_t disk_count(void)
{
    uint32_t disk_count = 0;
#if defined(DISK_PLATFORM_WINDOWS)
    DWORD drives = GetLogicalDrives();
    for (int i = 0; i < 26; i++)
    {
        if (!(drives & (1 << i)))
            continue;

        char drive_path[4] = {(char)('A' + i), ':', '\\', '\0'};
        UINT drive_type = GetDriveTypeA(drive_path);
        if (drive_type == DRIVE_FIXED || drive_type == DRIVE_REMOVABLE)
            disk_count++;
    }

    return disk_count;

#elif defined(DISK_PLATFORM_LINUX)
    FILE *fp = fopen("/proc/partitions", "r");
    if (!fp)
        return 0;

    char line[256];
    uint32_t disk_count = 0;
    if (!fgets(line, sizeof(line), fp) || !fgets(line, sizeof(line), fp))
    {
        fclose(fp);
        return 0;
    }

    while (fgets(line, sizeof(line), fp))
    {
        char device_name[64];
        unsigned long major, minor, blocks;
        if (sscanf(line, "%lu %lu %lu %63s", &major, &minor, &blocks,
                device_name) != 4)
            continue;

        size_t name_len = strlen(device_name);
        // SCSI/SATA: sda, sdb, sdc, ...
        if (strncmp(device_name, "sd", 2) == 0 && name_len == 3)
            disk_count++;
        // NVMe: nvme0n1, nvme1n1, ...
        else if (strncmp(device_name, "nvme", 4) == 0 &&
                strstr(device_name, "p") == NULL)
            disk_count++;
        // IDE: hda, hdb, hdc, hdd
        else if (strncmp(device_name, "hd", 2) == 0 && name_len == 3)
            disk_count++;
        // MMC/eMMC: mmcblk0, mmcblk1, ...
        else if (strncmp(device_name, "mmcblk", 6) == 0 &&
                strstr(device_name, "p") == NULL)
            disk_count++;
        // Virtual Disk: vda, vdb, ...
        else if (strncmp(device_name, "vd", 2) == 0 && name_len == 3)
            disk_count++;

        if (blocks < 1024) // pass the disk size less than 512 KB
            continue;
    }

    fclose(fp);
    return disk_count;

#elif defined(DISK_PLATFORM_MACOS)
    io_iterator_t disk_list;
    CFMutableDictionaryRef matching;
    matching = IOServiceMatching("IOMedia");
    if (!matching)
        return 0;

    CFDictionarySetValue(matching, CFSTR(kIOMediaWholeKey), kCFBooleanTrue);
    kern_return_t result = IOServiceGetMatchingServices(kIOMasterPortDefault,
                                                        matching, &disk_list);
    if (result != KERN_SUCCESS)
        return 0;

    uint32_t disk_count = 0;
    io_object_t disk_obj;
    while ((disk_obj = IOIteratorNext(disk_list)) != 0)
    {
        CFTypeRef size_ref = IORegistryEntryCreateCFProperty(
            disk_obj, CFSTR(kIOMediaSizeKey), kCFAllocatorDefault, 0);
        if (size_ref)
        {
            uint64_t size = 0;
            CFNumberGetValue((CFNumberRef)size_ref, kCFNumberSInt64Type, &size);
            CFRelease(size_ref);

            // > 1MB
            if (size > 1024 * 1024)
                disk_count++;
        }

        IOObjectRelease(disk_obj);
    }

    IOObjectRelease(disk_list);
    return disk_count;

#else
    return 0;

#endif
}

static filesystem_type_t disk_filesystem_type_from_string(const char* fs_name)
{
    if (!fs_name)
        return FILESYSTEM_UNKNOWN;
    if (strcmp(fs_name, "ntfs") == 0 || strcmp(fs_name, "NTFS") == 0)
        return FILESYSTEM_NTFS;
    if (strcmp(fs_name, "fat32") == 0 || strcmp(fs_name, "FAT32") == 0)
        return FILESYSTEM_FAT32;
    if (strcmp(fs_name, "exfat") == 0 || strcmp(fs_name, "exFAT") == 0)
        return FILESYSTEM_EXFAT;
    if (strcmp(fs_name, "ext2") == 0)
        return FILESYSTEM_EXT2;
    if (strcmp(fs_name, "ext3") == 0)
        return FILESYSTEM_EXT3;
    if (strcmp(fs_name, "ext4") == 0)
        return FILESYSTEM_EXT4;
    if (strcmp(fs_name, "xfs") == 0 || strcmp(fs_name, "XFS") == 0)
        return FILESYSTEM_XFS;
    if (strcmp(fs_name, "btrfs") == 0)
        return FILESYSTEM_BTRFS;
    if (strcmp(fs_name, "zfs") == 0 || strcmp(fs_name, "ZFS") == 0)
        return FILESYSTEM_ZFS;
    if (strcmp(fs_name, "hfs+") == 0 || strcmp(fs_name, "HFS+") == 0)
        return FILESYSTEM_HFS_PLUS;
    if (strcmp(fs_name, "apfs") == 0 || strcmp(fs_name, "APFS") == 0)
        return FILESYSTEM_APFS;
    if (strcmp(fs_name, "ufs") == 0 || strcmp(fs_name, "UFS") == 0)
        return FILESYSTEM_UFS;
    if (strcmp(fs_name, "iso9660") == 0 || strcmp(fs_name, "ISO9660") == 0)
        return FILESYSTEM_ISO9660;

    return FILESYSTEM_UNKNOWN;
}

static disk_err_t disk_info(const char *device_name, disk_info_t *info)
{
    if (!device_name || !info)
        return DISK_ERROR_INVALID_PARAMETER;

    memset(info, 0, sizeof(disk_info_t));
    strncpy(info->device_name, device_name, sizeof(info->device_name) - 1);
    info->temperature = -1.0;

#if defined(DISK_PLATFORM_WINDOWS)
    char full_path[MAX_PATH];
    snprintf(full_path, sizeof(full_path), "\\\\.\\%s", device_name);
    HANDLE hDevice =
        CreateFileA(full_path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                    OPEN_EXISTING, 0, NULL);

    if (hDevice == INVALID_HANDLE_VALUE)
        return DISK_ERROR_ACCESS_DENIED;

    DISK_GEOMETRY_EX geometry;
    DWORD bytes_returned;
    if (DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0,
                        &geometry, sizeof(geometry), &bytes_returned, NULL))
    {
        info->total_size = geometry.DiskSize.QuadPart;
        info->sector_size = geometry.Geometry.BytesPerSector;
        info->sector_count = info->total_size / info->sector_size;
    }

    UINT drive_type = GetDriveTypeA(device_name);
    switch (drive_type)
    {
    case DRIVE_FIXED:
        info->type =
            DISK_TYPE_HDD; // Could be SSD, but can't determine easily
        break;
    case DRIVE_REMOVABLE:
        info->type = DISK_TYPE_REMOVABLE;
        info->removable = true;
        break;
    case DRIVE_CDROM:
        info->type = DISK_TYPE_OPTICAL;
        info->removable = true;
        break;
    case DRIVE_REMOTE:
        info->type = DISK_TYPE_NETWORK;
        break;
    case DRIVE_RAMDISK:
        info->type = DISK_TYPE_RAM;
        break;
    default:
        info->type = DISK_TYPE_UNKNOWN;
        break;
    }
    CloseHandle(hDevice);

#elif defined(DISK_PLATFORM_LINUX)
    char full_path[256];
    if (strncmp(device_name, "/dev/", 5) == 0)
        strncpy(full_path, device_name, sizeof(full_path) - 1);
    else
        snprintf(full_path, sizeof(full_path), "/dev/%s", device_name);

    int fd = open(full_path, O_RDONLY);
    if (fd < 0)
        return DISK_ERROR_ACCESS_DENIED;

    uint64_t size_in_bytes;
    if (ioctl(fd, BLKGETSIZE64, &size_in_bytes) == 0)
        info->total_size = size_in_bytes;

    // Get sector size
    int sector_size;
    if (ioctl(fd, BLKSSZGET, &sector_size) == 0)
    {
        info->sector_size = sector_size;
        info->sector_count = info->total_size / info->sector_size;
    }
    else
    {
        info->sector_size = DISK_SECTOR_SIZE_DEFAULT;
        info->sector_count = info->total_size / DISK_SECTOR_SIZE_DEFAULT;
    }

    // Try to determine if it's SSD
    char sys_path[512];
    char rotational[16];
    snprintf(sys_path, sizeof(sys_path), "/sys/block/%s/queue/rotational",
            strrchr(device_name, '/') ? strrchr(device_name, '/') + 1
                                    : device_name);

    FILE *fp = fopen(sys_path, "r");
    if (fp && fgets(rotational, sizeof(rotational), fp))
    {
        if (rotational[0] == '0')
        {
            info->type = DISK_TYPE_SSD;
            info->rpm = 0;
        }
        else
        {
            info->type = DISK_TYPE_HDD;
            info->rpm = 7200; /* Default assumption */
        }
        fclose(fp);
    }
    else
    {
        info->type = DISK_TYPE_UNKNOWN;
    }
    close(fd);

#elif defined(DISK_PLATFORM_MACOS)
    info->type = DISK_TYPE_UNKNOWN;
    info->sector_size = DISK_SECTOR_SIZE_DEFAULT;

#else
    return DISK_ERROR_NOT_SUPPORTED;

#endif
    if (info->sector_size == 0)
        info->sector_size = DISK_SECTOR_SIZE_DEFAULT;

    strncpy(info->model, "Generic Disk", sizeof(info->model) - 1);
    strncpy(info->serial, "Unknown", sizeof(info->serial) - 1);
    return DISK_SUCCESS;
}

static disk_err_t disk_get_partition_by_mount(const char* mount_point, partition_info_t* info)
{
    if (!mount_point || !info)
        return DISK_ERROR_INVALID_PARAMETER;

    memset(info, 0, sizeof(partition_info_t));
    strncpy(info->mount_point, mount_point, sizeof(info->mount_point) - 1);
#if defined(DISK_PLATFORM_WINDOWS)
    ULARGE_INTEGER free_bytes, total_bytes, total_free_bytes;
    if (GetDiskFreeSpaceExA(mount_point, &free_bytes, &total_bytes, &total_free_bytes))
    {
        info->total_size = total_bytes.QuadPart;
        info->available_size = free_bytes.QuadPart;
        info->used_size = total_bytes.QuadPart - free_bytes.QuadPart;
    }

    char volume_name[MAX_PATH];
    char filesystem_name[MAX_PATH];
    DWORD serial_number, max_component_length, filesystem_flags;
    if (GetVolumeInformationA(mount_point, 
                              volume_name, 
                              sizeof(volume_name),
                              &serial_number, 
                              &max_component_length,
                              &filesystem_flags, 
                              filesystem_name,
                              sizeof(filesystem_name)))
    {
        strncpy(info->volume_label, volume_name, sizeof(info->volume_label) - 1);
        info->filesystem = disk_filesystem_type_from_string(filesystem_name);
        info->read_only = (filesystem_flags & FILE_READ_ONLY_VOLUME) != 0;
    }

    char device_path[MAX_PATH];
    if (QueryDosDeviceA(&mount_point[0], device_path, sizeof(device_path)))
        strncpy(info->device_name, device_path, sizeof(info->device_name) - 1);

    return DISK_SUCCESS;

#elif defined(DISK_PLATFORM_LINUX)
    struct statvfs vfs;
    if (statvfs(mount_point, &vfs) == 0)
    {
        info->total_size = (uint64_t) vfs.f_blocks * vfs.f_frsize;
        info->available_size = (uint64_t) vfs.f_bavail * vfs.f_frsize;
        info->used_size = info->total_size - ((uint64_t) vfs.f_bfree * vfs.f_frsize);
        info->read_only = (vfs.f_flag & ST_RDONLY) != 0;
    }

    FILE* fp = fopen("/proc/mounts", "r");
    if (!fp)
        return DISK_ERROR_NOT_FOUND;

    char line[1024];
    while (fgets(line, sizeof(line), fp))
    {
        char device[256], mountpoint[256], fstype[64];
        if (!sscanf(line, "%s %s %s", device, mountpoint, fstype) == 3)
            continue;

        if (strcmp(mountpoint, mount_point) == 0)
        {
            strncpy(info->device_name, device, sizeof(info->device_name) - 1);
            info->filesystem = filesystem_type_from_string(fstype);
            break;
        }
    }
    fclose(fp);

#elif defined(DISK_PLATFORM_MACOS)
    struct statfs fs;
    if (statfs(mount_point, &fs) == 0)
    {
        info->total_size = (uint64_t) fs.f_blocks * fs.f_bsize;
        info->available_size = (uint64_t) fs.f_bavail * fs.f_bsize;
        info->used_size = info->total_size - ((uint64_t) fs.f_bfree * fs.f_bsize);
        info->read_only = (fs.f_flags & MNT_RDONLY) != 0;

        strncpy(info->device_name, fs.f_mntfromname, sizeof(info->device_name) - 1);
        info->filesystem = filesystem_type_from_string(fs.f_fstypename);
    }

#else
    return DISK_ERROR_NOT_SUPPORTED;

#endif

    return DISK_SUCCESS;
}

static bool disk_is_ready(const char* device_name)
{
    if (!device_name || strlen(device_name) == 0)
        return false;

    bool ready = false;

#if defined(DISK_PLATFORM_WINDOWS)
    char full_path[MAX_PATH];
    snprintf(full_path, sizeof(full_path), "\\\\.\\%s", device_name);
    HANDLE hDevice = CreateFileA(full_path, 
                                 0, 
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING, 
                                 0, 
                                 NULL);
    if (hDevice != INVALID_HANDLE_VALUE)
    {
        DISK_GEOMETRY geometry;
        DWORD bytes_returned;
        if (DeviceIoControl(hDevice, 
                           IOCTL_DISK_GET_DRIVE_GEOMETRY, 
                           NULL, 0,
                           &geometry, 
                           sizeof(geometry), 
                           &bytes_returned, 
                           NULL))
        {
            ready = true;
        }
        else
        {
            DWORD error = GetLastError();
            if (error != ERROR_NOT_READY && error != ERROR_ACCESS_DENIED)
                ready = true;
        }
        CloseHandle(hDevice);
    }

#elif defined(DISK_PLATFORM_LINUX)
    char full_path[256];
    if (strncmp(device_name, "/dev/", 5) == 0)
        strncpy(full_path, device_name, sizeof(full_path) - 1);
    else
        snprintf(full_path, sizeof(full_path), "/dev/%s", device_name);
    
    full_path[sizeof(full_path) - 1] = '\0';
    struct stat st;
    if (stat(full_path, &st) != 0)
        return false;

    if (!S_ISBLK(st.st_mode))
        return false;
    
    int fd = open(full_path, O_RDONLY | O_NONBLOCK);
    if (fd >= 0)
    {
        uint64_t size;
        if (ioctl(fd, BLKGETSIZE64, &size) == 0 && size > 0)
            ready = true;
        else
            ready = true;

        close(fd);
    }
    else
    {
        if (errno == EACCES)
            ready = true;
    }

#elif defined(DISK_PLATFORM_MACOS)
    io_iterator_t disk_list;
    CFMutableDictionaryRef matching;
    matching = IOServiceMatching("IOMedia");
    if (!matching)
        return false;
    
    CFDictionarySetValue(matching, CFSTR(kIOMediaWholeKey), kCFBooleanTrue);
    kern_return_t result = IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &disk_list);
    if (result != KERN_SUCCESS)
        return false;
    
    io_object_t disk_obj;
    while ((disk_obj = IOIteratorNext(disk_list)) != 0)
    {
        CFTypeRef path_ref = IORegistryEntryCreateCFProperty(
            disk_obj, CFSTR(kIOBSDNameKey), kCFAllocatorDefault, 0);
        
        if (path_ref)
        {
            char bsd_name[256];
            CFStringGetCString((CFStringRef)path_ref, bsd_name, sizeof(bsd_name), kCFStringEncodingUTF8);
            CFRelease(path_ref);
            char full_device_path[256];
            snprintf(full_device_path, sizeof(full_device_path), "/dev/%s", bsd_name);
            if (strcmp(device_name, bsd_name) != 0 && strcmp(device_name, full_device_path) != 0)
                continue;

            CFTypeRef size_ref = IORegistryEntryCreateCFProperty(
                disk_obj, CFSTR(kIOMediaSizeKey), kCFAllocatorDefault, 0);
            
            if (size_ref)
            {
                uint64_t size = 0;
                CFNumberGetValue((CFNumberRef)size_ref, kCFNumberSInt64Type, &size);
                CFRelease(size_ref);
                
                if (size > 0)
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

static disk_err_t disk_enumerate(disk_info_t* disks, uint32_t max_disks, uint32_t* actual_count)
{
    if (!disks || !actual_count || max_disks == 0)
        return DISK_ERROR_INVALID_PARAMETER;

    *actual_count = 0;
#if defined(DISK_PLATFORM_WINDOWS)
    DWORD drives = GetLogicalDrives();
    for (int i = 0; i < 26 && *actual_count < max_disks; i++)
    {
        if (!(drives & (1 << i)))
            continue;

        char drive_letter[4] = {(char) ('A' + i), ':', '\0', '\0'}; 
        UINT drive_type = GetDriveTypeA(drive_letter);

        if (drive_type != DRIVE_FIXED && drive_type != DRIVE_REMOVABLE)
            continue;

        disk_err_t result = disk_info(drive_letter, &disks[*actual_count]);
        if (result == DISK_SUCCESS)
            (*actual_count)++;
    }

#elif defined(DISK_PLATFORM_LINUX)
    const char* disk_patterns[] = {
        "/dev/sda",     "/dev/sdb",     "/dev/sdc",     "/dev/sdd",
        "/dev/nvme0n1", "/dev/nvme1n1", "/dev/nvme2n1", "/dev/hda",
        "/dev/hdb",     "/dev/hdc",     "/dev/hdd"};
    for (size_t i = 0; i < sizeof(disk_patterns) / sizeof(disk_patterns[0]) &&  *actual_count < max_disks; i++)
    {
        if (access(disk_patterns[i], F_OK) != 0)
            continue;

        disk_err_t result = disk_get_info(disk_patterns[i], &disks[*actual_count]);
        if (result == DISK_SUCCESS)
            (*actual_count)++;
    }

#elif defined(DISK_PLATFORM_MACOS)
    if (max_disks > 0)
    {
        disk_err_t result = disk_get_info("/dev/disk0", &disks[0]);
        if (result == DISK_SUCCESS)
            *actual_count = 1;
    }

#else
    return DISK_ERROR_NOT_SUPPORTED;

#endif

    return DISK_SUCCESS;
}

static disk_err_t disk_read_speed_test(const char* device_name, uint32_t test_size_mb, double* read_speed_mbps)
{
    if (!device_name || !read_speed_mbps || test_size_mb == 0)
        return DISK_ERROR_INVALID_PARAMETER;

    if (test_size_mb > 1024)
        test_size_mb = 1024;

    *read_speed_mbps = 0.0;

    const size_t buffer_size = 1024 * 1024;
    const uint64_t total_bytes = (uint64_t)test_size_mb * 1024 * 1024;
    const uint32_t iterations = (uint32_t)(total_bytes / buffer_size);

    if (iterations == 0)
        return DISK_ERROR_INVALID_PARAMETER;

    void* buffer = NULL;

#if defined(DISK_PLATFORM_WINDOWS)
    char full_path[MAX_PATH];
    snprintf(full_path, sizeof(full_path), "\\\\.\\%s", device_name);

    HANDLE hDevice = CreateFileA(full_path,
                                 GENERIC_READ,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_FLAG_NO_BUFFERING | FILE_FLAG_SEQUENTIAL_SCAN,
                                 NULL);

    if (hDevice == INVALID_HANDLE_VALUE)
        return DISK_ERROR_ACCESS_DENIED;

    buffer = VirtualAlloc(NULL, buffer_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!buffer)
    {
        CloseHandle(hDevice);
        return DISK_ERROR_INSUFFICIENT_MEMORY;
    }

    DISK_GEOMETRY_EX geometry;
    DWORD bytes_returned;
    uint64_t device_size = 0;
    if (DeviceIoControl(hDevice, 
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

    if (device_size > 0 && total_bytes > device_size)
    {
        VirtualFree(buffer, 0, MEM_RELEASE);
        CloseHandle(hDevice);
        return DISK_ERROR_INVALID_PARAMETER;
    }

    LARGE_INTEGER frequency, start_time, end_time;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start_time);

    uint64_t total_read = 0;
    for (uint32_t i = 0; i < iterations; i++)
    {
        DWORD bytes_read;
        if (!ReadFile(hDevice, buffer, buffer_size, &bytes_read, NULL))
        {
            DWORD error = GetLastError();
            if (error == ERROR_HANDLE_EOF)
                break;
            
            VirtualFree(buffer, 0, MEM_RELEASE);
            CloseHandle(hDevice);
            return DISK_ERROR_IO_ERROR;
        }
        total_read += bytes_read;

        if (bytes_read < buffer_size)
            break;
    }

    QueryPerformanceCounter(&end_time);
    VirtualFree(buffer, 0, MEM_RELEASE);
    CloseHandle(hDevice);
    double elapsed_seconds = (double)(end_time.QuadPart - start_time.QuadPart) / frequency.QuadPart;
    if (elapsed_seconds > 0.0)
    {
        double bytes_per_second = total_read / elapsed_seconds;
        *read_speed_mbps = bytes_per_second / (1024.0 * 1024.0);
    }

#elif defined(DISK_PLATFORM_LINUX)
    char full_path[256];
    if (strncmp(device_name, "/dev/", 5) == 0)
        strncpy(full_path, device_name, sizeof(full_path) - 1);
    else
        snprintf(full_path, sizeof(full_path), "/dev/%s", device_name);

    full_path[sizeof(full_path) - 1] = '\0';
    int fd = open(full_path, O_RDONLY | O_DIRECT);
    if (fd < 0)
    {
        fd = open(full_path, O_RDONLY);
        if (fd < 0)
            return DISK_ERROR_ACCESS_DENIED;
    }

    if (posix_memalign(&buffer, 4096, buffer_size) != 0)
    {
        close(fd);
        return DISK_ERROR_INSUFFICIENT_MEMORY;
    }

    uint64_t device_size = 0;
    if (ioctl(fd, BLKGETSIZE64, &device_size) == 0)
    {
        if (total_bytes > device_size)
        {
            free(buffer);
            close(fd);
            return DISK_ERROR_INVALID_PARAMETER;
        }
    }

    sync();
    int cache_fd = open("/proc/sys/vm/drop_caches", O_WRONLY);
    if (cache_fd >= 0)
    {
        write(cache_fd, "3", 1);
        close(cache_fd);
    }

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    uint64_t total_read = 0;
    for (uint32_t i = 0; i < iterations; i++)
    {
        ssize_t bytes_read = read(fd, buffer, buffer_size);
        if (bytes_read < 0)
        {
            free(buffer);
            close(fd);
            return DISK_ERROR_IO_ERROR;
        }
        
        total_read += bytes_read;
        
        if ((size_t)bytes_read < buffer_size)
            break;
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    free(buffer);
    close(fd);
    double elapsed_seconds = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    if (elapsed_seconds > 0.0)
    {
        double bytes_per_second = total_read / elapsed_seconds;
        *read_speed_mbps = bytes_per_second / (1024.0 * 1024.0);
    }

#elif defined(DISK_PLATFORM_MACOS)
    char full_path[256];
    if (strncmp(device_name, "/dev/", 5) == 0)
        strncpy(full_path, device_name, sizeof(full_path) - 1);
    else
        snprintf(full_path, sizeof(full_path), "/dev/%s", device_name);

    full_path[sizeof(full_path) - 1] = '\0';
    int fd = open(full_path, O_RDONLY);
    if (fd < 0)
        return DISK_ERROR_ACCESS_DENIED;

    if (posix_memalign(&buffer, 4096, buffer_size) != 0)
    {
        close(fd);
        return DISK_ERROR_INSUFFICIENT_MEMORY;
    }

    uint64_t device_size = 0;
    if (ioctl(fd, DKIOCGETBLOCKCOUNT, &device_size) == 0)
    {
        uint32_t block_size = 0;
        if (ioctl(fd, DKIOCGETBLOCKSIZE, &block_size) == 0)
        {
            device_size *= block_size;
            if (total_bytes > device_size)
            {
                free(buffer);
                close(fd);
                return DISK_ERROR_INVALID_PARAMETER;
            }
        }
    }

    sync();
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    uint64_t total_read = 0;
    for (uint32_t i = 0; i < iterations; i++)
    {
        ssize_t bytes_read = read(fd, buffer, buffer_size);
        if (bytes_read < 0)
        {
            free(buffer);
            close(fd);
            return DISK_ERROR_IO_ERROR;
        }
        
        total_read += bytes_read;
        
        if ((size_t)bytes_read < buffer_size)
            break;
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    free(buffer);
    close(fd);
    double elapsed_seconds = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    if (elapsed_seconds > 0.0)
    {
        double bytes_per_second = total_read / elapsed_seconds;
        *read_speed_mbps = bytes_per_second / (1024.0 * 1024.0);
    }

#else
    return DISK_ERROR_NOT_SUPPORTED;

#endif

    return DISK_SUCCESS;
}

#ifdef __cplusplus
}
#endif

#endif // DISK_H