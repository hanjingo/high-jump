#ifndef DISK_H
#define DISK_H

// #include <stdbool.h>
// #include <stddef.h>
// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// #ifdef __cplusplus
// extern "C"
// {
// #endif

// /* ----------------------------- Platform Detection
//  * ------------------------------------ */
// #if defined(_WIN32) || defined(_WIN64)
// #define DISK_PLATFORM_WINDOWS 1
// #include <setupapi.h>
// #include <windows.h>
// #include <winioctl.h>
// #elif defined(__linux__)
// #define DISK_PLATFORM_LINUX 1
// #include <fcntl.h>
// #include <mntent.h>
// #include <unistd.h>

// #include <linux/fs.h>
// #include <sys/ioctl.h>
// #include <sys/stat.h>
// #include <sys/statvfs.h>
// #elif defined(__APPLE__)
// #define DISK_PLATFORM_MACOS 1
// #include <IOKit/IOKitLib.h>
// #include <IOKit/storage/IOBlockStorageDriver.h>
// #include <sys/disk.h>
// #include <sys/mount.h>
// #else
// #define DISK_PLATFORM_UNKNOWN 1
// #endif

// /* ----------------------------- Constants and Limits
//  * ------------------------------------ */
// #define DISK_MAX_PATH_LENGTH 4096
// #define DISK_MAX_DEVICE_NAME 256
// #define DISK_MAX_FILESYSTEM_NAME 64
// #define DISK_MAX_VOLUME_LABEL 256
// #define DISK_SECTOR_SIZE_DEFAULT 512
// #define DISK_MAX_DISKS 64
// #define DISK_MAX_PARTITIONS 128

//     /* ----------------------------- Error Codes
//      * ------------------------------------ */
//     typedef enum
//     {
//         DISK_SUCCESS = 0,
//         DISK_ERROR_INVALID_PARAMETER = -1,
//         DISK_ERROR_ACCESS_DENIED = -2,
//         DISK_ERROR_NOT_FOUND = -3,
//         DISK_ERROR_INSUFFICIENT_BUFFER = -4,
//         DISK_ERROR_IO_ERROR = -5,
//         DISK_ERROR_NOT_SUPPORTED = -6,
//         DISK_ERROR_INSUFFICIENT_MEMORY = -7,
//         DISK_ERROR_SYSTEM_ERROR = -8,
//         DISK_ERROR_INVALID_FILESYSTEM = -9,
//         DISK_ERROR_DEVICE_BUSY = -10
//     } disk_error_t;

//     /* ----------------------------- Disk Types
//      * ------------------------------------ */
//     typedef enum
//     {
//         DISK_TYPE_UNKNOWN = 0,
//         DISK_TYPE_HDD = 1,
//         DISK_TYPE_SSD = 2,
//         DISK_TYPE_OPTICAL = 3,
//         DISK_TYPE_REMOVABLE = 4,
//         DISK_TYPE_NETWORK = 5,
//         DISK_TYPE_RAM = 6
//     } disk_type_t;

//     /* ----------------------------- Filesystem Types
//      * ------------------------------------ */
//     typedef enum
//     {
//         FILESYSTEM_UNKNOWN = 0,
//         FILESYSTEM_NTFS = 1,
//         FILESYSTEM_FAT32 = 2,
//         FILESYSTEM_EXFAT = 3,
//         FILESYSTEM_EXT2 = 4,
//         FILESYSTEM_EXT3 = 5,
//         FILESYSTEM_EXT4 = 6,
//         FILESYSTEM_XFS = 7,
//         FILESYSTEM_BTRFS = 8,
//         FILESYSTEM_ZFS = 9,
//         FILESYSTEM_HFS_PLUS = 10,
//         FILESYSTEM_APFS = 11,
//         FILESYSTEM_UFS = 12,
//         FILESYSTEM_ISO9660 = 13
//     } filesystem_type_t;

//     /* ----------------------------- Structures
//      * ------------------------------------ */

//     /* Disk information structure */
//     typedef struct
//     {
//         char device_name[DISK_MAX_DEVICE_NAME]; /* Device name (e.g.,
//                                                    "/dev/sda", "C:") */
//         char model[DISK_MAX_DEVICE_NAME];       /* Disk model name */
//         char serial[DISK_MAX_DEVICE_NAME];      /* Serial number */
//         disk_type_t type;                       /* Disk type */
//         uint64_t total_size;                    /* Total size in bytes */
//         uint64_t sector_size;                   /* Sector size in bytes */
//         uint64_t sector_count;                  /* Total number of sectors */
//         bool removable;                         /* Is removable disk */
//         bool read_only;                         /* Is read-only */
//         uint32_t rpm;                           /* Rotation speed (0 for SSD)
//         */ double temperature; /* Temperature in Celsius (-1 if unknown) */
//     } disk_info_t;

//     /* Partition information structure */
//     typedef struct
//     {
//         char device_name[DISK_MAX_DEVICE_NAME];   /* Partition device name */
//         char mount_point[DISK_MAX_PATH_LENGTH];   /* Mount point */
//         char volume_label[DISK_MAX_VOLUME_LABEL]; /* Volume label */
//         filesystem_type_t filesystem;             /* Filesystem type */
//         uint64_t start_sector;                    /* Starting sector */
//         uint64_t sector_count;                    /* Number of sectors */
//         uint64_t total_size;                      /* Total size in bytes */
//         uint64_t used_size;                       /* Used size in bytes */
//         uint64_t available_size;                  /* Available size in bytes
//         */ bool bootable;                            /* Is bootable partition
//         */ bool read_only;                           /* Is read-only */
//     } partition_info_t;

//     /* Disk usage statistics */
//     typedef struct
//     {
//         uint64_t bytes_read;       /* Total bytes read */
//         uint64_t bytes_written;    /* Total bytes written */
//         uint64_t read_operations;  /* Number of read operations */
//         uint64_t write_operations; /* Number of write operations */
//         double read_time_ms;       /* Total read time in milliseconds */
//         double write_time_ms;      /* Total write time in milliseconds */
//         uint32_t queue_depth;      /* Current queue depth */
//     } disk_stats_t;

//     /* ----------------------------- Function Implementations
//      * ------------------------------------ */

//     /* Get error string from error code */
//     static inline const char* disk_get_error_string(disk_error_t error)
//     {
//         switch (error)
//         {
//             case DISK_SUCCESS:
//                 return "Success";
//             case DISK_ERROR_INVALID_PARAMETER:
//                 return "Invalid parameter";
//             case DISK_ERROR_ACCESS_DENIED:
//                 return "Access denied";
//             case DISK_ERROR_NOT_FOUND:
//                 return "Not found";
//             case DISK_ERROR_INSUFFICIENT_BUFFER:
//                 return "Insufficient buffer";
//             case DISK_ERROR_IO_ERROR:
//                 return "I/O error";
//             case DISK_ERROR_NOT_SUPPORTED:
//                 return "Not supported";
//             case DISK_ERROR_INSUFFICIENT_MEMORY:
//                 return "Insufficient memory";
//             case DISK_ERROR_SYSTEM_ERROR:
//                 return "System error";
//             case DISK_ERROR_INVALID_FILESYSTEM:
//                 return "Invalid filesystem";
//             case DISK_ERROR_DEVICE_BUSY:
//                 return "Device busy";
//             default:
//                 return "Unknown error";
//         }
//     }

//     /* Convert disk type to string */
//     static inline const char* disk_type_to_string(disk_type_t type)
//     {
//         switch (type)
//         {
//             case DISK_TYPE_UNKNOWN:
//                 return "Unknown";
//             case DISK_TYPE_HDD:
//                 return "Hard Disk Drive";
//             case DISK_TYPE_SSD:
//                 return "Solid State Drive";
//             case DISK_TYPE_OPTICAL:
//                 return "Optical Drive";
//             case DISK_TYPE_REMOVABLE:
//                 return "Removable Drive";
//             case DISK_TYPE_NETWORK:
//                 return "Network Drive";
//             case DISK_TYPE_RAM:
//                 return "RAM Drive";
//             default:
//                 return "Unknown";
//         }
//     }

//     /* Convert filesystem type to string */
//     static inline const char* filesystem_type_to_string(
//         filesystem_type_t fs_type)
//     {
//         switch (fs_type)
//         {
//             case FILESYSTEM_UNKNOWN:
//                 return "Unknown";
//             case FILESYSTEM_NTFS:
//                 return "NTFS";
//             case FILESYSTEM_FAT32:
//                 return "FAT32";
//             case FILESYSTEM_EXFAT:
//                 return "exFAT";
//             case FILESYSTEM_EXT2:
//                 return "ext2";
//             case FILESYSTEM_EXT3:
//                 return "ext3";
//             case FILESYSTEM_EXT4:
//                 return "ext4";
//             case FILESYSTEM_XFS:
//                 return "XFS";
//             case FILESYSTEM_BTRFS:
//                 return "Btrfs";
//             case FILESYSTEM_ZFS:
//                 return "ZFS";
//             case FILESYSTEM_HFS_PLUS:
//                 return "HFS+";
//             case FILESYSTEM_APFS:
//                 return "APFS";
//             case FILESYSTEM_UFS:
//                 return "UFS";
//             case FILESYSTEM_ISO9660:
//                 return "ISO9660";
//             default:
//                 return "Unknown";
//         }
//     }

//     /* Get filesystem type from string */
//     static inline filesystem_type_t filesystem_type_from_string(
//         const char* fs_name)
//     {
//         if (!fs_name)
//             return FILESYSTEM_UNKNOWN;

//         if (strcmp(fs_name, "ntfs") == 0 || strcmp(fs_name, "NTFS") == 0)
//             return FILESYSTEM_NTFS;
//         if (strcmp(fs_name, "fat32") == 0 || strcmp(fs_name, "FAT32") == 0)
//             return FILESYSTEM_FAT32;
//         if (strcmp(fs_name, "exfat") == 0 || strcmp(fs_name, "exFAT") == 0)
//             return FILESYSTEM_EXFAT;
//         if (strcmp(fs_name, "ext2") == 0)
//             return FILESYSTEM_EXT2;
//         if (strcmp(fs_name, "ext3") == 0)
//             return FILESYSTEM_EXT3;
//         if (strcmp(fs_name, "ext4") == 0)
//             return FILESYSTEM_EXT4;
//         if (strcmp(fs_name, "xfs") == 0 || strcmp(fs_name, "XFS") == 0)
//             return FILESYSTEM_XFS;
//         if (strcmp(fs_name, "btrfs") == 0)
//             return FILESYSTEM_BTRFS;
//         if (strcmp(fs_name, "zfs") == 0 || strcmp(fs_name, "ZFS") == 0)
//             return FILESYSTEM_ZFS;
//         if (strcmp(fs_name, "hfs+") == 0 || strcmp(fs_name, "HFS+") == 0)
//             return FILESYSTEM_HFS_PLUS;
//         if (strcmp(fs_name, "apfs") == 0 || strcmp(fs_name, "APFS") == 0)
//             return FILESYSTEM_APFS;
//         if (strcmp(fs_name, "ufs") == 0 || strcmp(fs_name, "UFS") == 0)
//             return FILESYSTEM_UFS;
//         if (strcmp(fs_name, "iso9660") == 0 || strcmp(fs_name, "ISO9660") ==
//         0)
//             return FILESYSTEM_ISO9660;

//         return FILESYSTEM_UNKNOWN;
//     }

//     /* Format size in bytes to human-readable string */
//     static inline disk_error_t disk_format_size(uint64_t bytes,
//                                                 char* buffer,
//                                                 size_t buffer_size)
//     {
//         if (!buffer || buffer_size == 0)
//         {
//             return DISK_ERROR_INVALID_PARAMETER;
//         }

//         const char* units[] = { "B", "KB", "MB", "GB", "TB", "PB" };
//         const size_t num_units = sizeof(units) / sizeof(units[0]);

//         double size = (double)bytes;
//         size_t unit_index = 0;

//         while (size >= 1024.0 && unit_index < num_units - 1)
//         {
//             size /= 1024.0;
//             unit_index++;
//         }

//         int result;
//         if (unit_index == 0)
//         {
//             result = snprintf(buffer,
//                               buffer_size,
//                               "%.0f %s",
//                               size,
//                               units[unit_index]);
//         }
//         else
//         {
//             result = snprintf(buffer,
//                               buffer_size,
//                               "%.2f %s",
//                               size,
//                               units[unit_index]);
//         }

//         if (result < 0 || (size_t)result >= buffer_size)
//         {
//             return DISK_ERROR_INSUFFICIENT_BUFFER;
//         }

//         return DISK_SUCCESS;
//     }

//     /* Initialize disk subsystem */
//     static inline disk_error_t disk_init(void)
//     {
//         /* Platform-specific initialization can be added here */
// #if defined(DISK_PLATFORM_WINDOWS)
//         /* Windows initialization */
//         return DISK_SUCCESS;
// #elif defined(DISK_PLATFORM_LINUX)
//     /* Linux initialization */
//     return DISK_SUCCESS;
// #elif defined(DISK_PLATFORM_MACOS)
//     /* macOS initialization */
//     return DISK_SUCCESS;
// #else
//     return DISK_ERROR_NOT_SUPPORTED;
// #endif
//     }

//     /* Cleanup disk subsystem */
//     static inline void disk_cleanup(void)
//     {
//         /* Platform-specific cleanup can be added here */
//     }

//     /* Get number of available disks */
//     static inline disk_error_t disk_get_count(uint32_t* count)
//     {
//         if (!count)
//         {
//             return DISK_ERROR_INVALID_PARAMETER;
//         }

// #if defined(DISK_PLATFORM_WINDOWS)
//         /* Windows implementation */
//         DWORD drives = GetLogicalDrives();
//         uint32_t disk_count = 0;

//         for (int i = 0; i < 26; i++)
//         {
//             if (drives & (1 << i))
//             {
//                 char drive_path[4] = { (char)('A' + i), ':', '\\', '\0' };
//                 UINT drive_type = GetDriveTypeA(drive_path);
//                 if (drive_type == DRIVE_FIXED || drive_type ==
//                 DRIVE_REMOVABLE)
//                 {
//                     disk_count++;
//                 }
//             }
//         }

//         *count = disk_count;
//         return DISK_SUCCESS;

// #elif defined(DISK_PLATFORM_LINUX)
//     /* Linux implementation - scan /proc/partitions or /sys/block */
//     FILE* fp = fopen("/proc/partitions", "r");
//     if (!fp)
//     {
//         return DISK_ERROR_IO_ERROR;
//     }

//     char line[256];
//     uint32_t disk_count = 0;

//     /* Skip header lines */
//     fgets(line, sizeof(line), fp);
//     fgets(line, sizeof(line), fp);

//     while (fgets(line, sizeof(line), fp))
//     {
//         char device_name[64];
//         unsigned long major, minor, blocks;

//         if (sscanf(line,
//                    "%lu %lu %lu %s",
//                    &major,
//                    &minor,
//                    &blocks,
//                    device_name) == 4)
//         {
//             /* Check if it's a whole disk (not a partition) */
//             if (strstr(device_name, "sd") == device_name &&
//                 strlen(device_name) == 3)
//             {
//                 disk_count++;
//             }
//             else if (strstr(device_name, "nvme") == device_name &&
//                      strstr(device_name, "p") == NULL)
//             {
//                 disk_count++;
//             }
//         }
//     }

//     fclose(fp);
//     *count = disk_count;
//     return DISK_SUCCESS;

// #elif defined(DISK_PLATFORM_MACOS)
//     /* macOS implementation using IOKit */
//     *count = 1; /* Simplified implementation */
//     return DISK_SUCCESS;

// #else
//     *count = 0;
//     return DISK_ERROR_NOT_SUPPORTED;
// #endif
//     }

//     /* Get disk information by device name */
//     static inline disk_error_t disk_get_info(const char* device_name,
//                                              disk_info_t* info)
//     {
//         if (!device_name || !info)
//         {
//             return DISK_ERROR_INVALID_PARAMETER;
//         }

//         /* Initialize structure */
//         memset(info, 0, sizeof(disk_info_t));
//         strncpy(info->device_name, device_name, sizeof(info->device_name) -
//         1); info->temperature = -1.0; /* Unknown temperature */

// #if defined(DISK_PLATFORM_WINDOWS)
//         /* Windows implementation */
//         char full_path[MAX_PATH];
//         snprintf(full_path, sizeof(full_path), "\\\\.\\%s", device_name);

//         HANDLE hDevice = CreateFileA(full_path,
//                                      0,
//                                      FILE_SHARE_READ | FILE_SHARE_WRITE,
//                                      NULL,
//                                      OPEN_EXISTING,
//                                      0,
//                                      NULL);

//         if (hDevice == INVALID_HANDLE_VALUE)
//         {
//             return DISK_ERROR_ACCESS_DENIED;
//         }

//         /* Get disk geometry */
//         DISK_GEOMETRY_EX geometry;
//         DWORD bytes_returned;

//         if (DeviceIoControl(hDevice,
//                             IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
//                             NULL,
//                             0,
//                             &geometry,
//                             sizeof(geometry),
//                             &bytes_returned,
//                             NULL))
//         {
//             info->total_size = geometry.DiskSize.QuadPart;
//             info->sector_size = geometry.Geometry.BytesPerSector;
//             info->sector_count = info->total_size / info->sector_size;
//         }

//         /* Get drive type */
//         UINT drive_type = GetDriveTypeA(device_name);
//         switch (drive_type)
//         {
//             case DRIVE_FIXED:
//                 info->type = DISK_TYPE_HDD; /* Could be SSD, but can't
//                 determine
//                                                easily */
//                 break;
//             case DRIVE_REMOVABLE:
//                 info->type = DISK_TYPE_REMOVABLE;
//                 info->removable = true;
//                 break;
//             case DRIVE_CDROM:
//                 info->type = DISK_TYPE_OPTICAL;
//                 info->removable = true;
//                 break;
//             case DRIVE_REMOTE:
//                 info->type = DISK_TYPE_NETWORK;
//                 break;
//             case DRIVE_RAMDISK:
//                 info->type = DISK_TYPE_RAM;
//                 break;
//             default:
//                 info->type = DISK_TYPE_UNKNOWN;
//                 break;
//         }

//         CloseHandle(hDevice);

// #elif defined(DISK_PLATFORM_LINUX)
//     /* Linux implementation */
//     char full_path[256];

//     /* Try to open the device */
//     if (strncmp(device_name, "/dev/", 5) == 0)
//     {
//         strncpy(full_path, device_name, sizeof(full_path) - 1);
//     }
//     else
//     {
//         snprintf(full_path, sizeof(full_path), "/dev/%s", device_name);
//     }

//     int fd = open(full_path, O_RDONLY);
//     if (fd < 0)
//     {
//         return DISK_ERROR_ACCESS_DENIED;
//     }

//     /* Get device size */
//     uint64_t size_in_bytes;
//     if (ioctl(fd, BLKGETSIZE64, &size_in_bytes) == 0)
//     {
//         info->total_size = size_in_bytes;
//     }

//     /* Get sector size */
//     int sector_size;
//     if (ioctl(fd, BLKSSZGET, &sector_size) == 0)
//     {
//         info->sector_size = sector_size;
//         info->sector_count = info->total_size / info->sector_size;
//     }
//     else
//     {
//         info->sector_size = DISK_SECTOR_SIZE_DEFAULT;
//         info->sector_count = info->total_size / DISK_SECTOR_SIZE_DEFAULT;
//     }

//     /* Try to determine if it's SSD */
//     char sys_path[512];
//     char rotational[16];
//     snprintf(sys_path,
//              sizeof(sys_path),
//              "/sys/block/%s/queue/rotational",
//              strrchr(device_name, '/') ? strrchr(device_name, '/') + 1
//                                        : device_name);

//     FILE* fp = fopen(sys_path, "r");
//     if (fp && fgets(rotational, sizeof(rotational), fp))
//     {
//         if (rotational[0] == '0')
//         {
//             info->type = DISK_TYPE_SSD;
//             info->rpm = 0;
//         }
//         else
//         {
//             info->type = DISK_TYPE_HDD;
//             info->rpm = 7200; /* Default assumption */
//         }
//         fclose(fp);
//     }
//     else
//     {
//         info->type = DISK_TYPE_UNKNOWN;
//     }

//     close(fd);

// #elif defined(DISK_PLATFORM_MACOS)
//     /* macOS implementation (simplified) */
//     info->type = DISK_TYPE_UNKNOWN;
//     info->sector_size = DISK_SECTOR_SIZE_DEFAULT;

// #else
//     return DISK_ERROR_NOT_SUPPORTED;
// #endif

//         /* Set default values if not determined */
//         if (info->sector_size == 0)
//         {
//             info->sector_size = DISK_SECTOR_SIZE_DEFAULT;
//         }

//         strncpy(info->model, "Generic Disk", sizeof(info->model) - 1);
//         strncpy(info->serial, "Unknown", sizeof(info->serial) - 1);

//         return DISK_SUCCESS;
//     }

//     /* Get partition information by mount point */
//     static inline disk_error_t disk_get_partition_by_mount(
//         const char* mount_point,
//         partition_info_t* info)
//     {
//         if (!mount_point || !info)
//         {
//             return DISK_ERROR_INVALID_PARAMETER;
//         }

//         memset(info, 0, sizeof(partition_info_t));
//         strncpy(info->mount_point, mount_point, sizeof(info->mount_point) -
//         1);

// #if defined(DISK_PLATFORM_WINDOWS)
//         /* Windows implementation */
//         ULARGE_INTEGER free_bytes, total_bytes, total_free_bytes;

//         if (GetDiskFreeSpaceExA(mount_point,
//                                 &free_bytes,
//                                 &total_bytes,
//                                 &total_free_bytes))
//         {
//             info->total_size = total_bytes.QuadPart;
//             info->available_size = free_bytes.QuadPart;
//             info->used_size = total_bytes.QuadPart - free_bytes.QuadPart;
//         }

//         /* Get volume information */
//         char volume_name[MAX_PATH];
//         char filesystem_name[MAX_PATH];
//         DWORD serial_number, max_component_length, filesystem_flags;

//         if (GetVolumeInformationA(mount_point,
//                                   volume_name,
//                                   sizeof(volume_name),
//                                   &serial_number,
//                                   &max_component_length,
//                                   &filesystem_flags,
//                                   filesystem_name,
//                                   sizeof(filesystem_name)))
//         {
//             strncpy(info->volume_label,
//                     volume_name,
//                     sizeof(info->volume_label) - 1);
//             info->filesystem = filesystem_type_from_string(filesystem_name);
//             info->read_only = (filesystem_flags & FILE_READ_ONLY_VOLUME) !=
//             0;
//         }

//         /* Get device name from mount point */
//         char device_path[MAX_PATH];
//         if (QueryDosDeviceA(&mount_point[0], device_path,
//         sizeof(device_path)))
//         {
//             strncpy(info->device_name,
//                     device_path,
//                     sizeof(info->device_name) - 1);
//         }

// #elif defined(DISK_PLATFORM_LINUX)
//     /* Linux implementation */
//     struct statvfs vfs;
//     if (statvfs(mount_point, &vfs) == 0)
//     {
//         info->total_size = (uint64_t)vfs.f_blocks * vfs.f_frsize;
//         info->available_size = (uint64_t)vfs.f_bavail * vfs.f_frsize;
//         info->used_size =
//             info->total_size - ((uint64_t)vfs.f_bfree * vfs.f_frsize);
//         info->read_only = (vfs.f_flag & ST_RDONLY) != 0;
//     }

//     /* Read /proc/mounts to get device and filesystem info */
//     FILE* fp = fopen("/proc/mounts", "r");
//     if (fp)
//     {
//         char line[1024];
//         while (fgets(line, sizeof(line), fp))
//         {
//             char device[256], mountpoint[256], fstype[64];
//             if (sscanf(line, "%s %s %s", device, mountpoint, fstype) == 3)
//             {
//                 if (strcmp(mountpoint, mount_point) == 0)
//                 {
//                     strncpy(info->device_name,
//                             device,
//                             sizeof(info->device_name) - 1);
//                     info->filesystem = filesystem_type_from_string(fstype);
//                     break;
//                 }
//             }
//         }
//         fclose(fp);
//     }

// #elif defined(DISK_PLATFORM_MACOS)
//     /* macOS implementation */
//     struct statfs fs;
//     if (statfs(mount_point, &fs) == 0)
//     {
//         info->total_size = (uint64_t)fs.f_blocks * fs.f_bsize;
//         info->available_size = (uint64_t)fs.f_bavail * fs.f_bsize;
//         info->used_size =
//             info->total_size - ((uint64_t)fs.f_bfree * fs.f_bsize);
//         info->read_only = (fs.f_flags & MNT_RDONLY) != 0;

//         strncpy(info->device_name,
//                 fs.f_mntfromname,
//                 sizeof(info->device_name) - 1);
//         info->filesystem = filesystem_type_from_string(fs.f_fstypename);
//     }

// #else
//     return DISK_ERROR_NOT_SUPPORTED;
// #endif

//         return DISK_SUCCESS;
//     }

//     /* Check if disk is ready */
//     static inline disk_error_t disk_is_ready(const char* device_name,
//                                              bool* ready)
//     {
//         if (!device_name || !ready)
//         {
//             return DISK_ERROR_INVALID_PARAMETER;
//         }

//         *ready = false;

// #if defined(DISK_PLATFORM_WINDOWS)
//         char full_path[MAX_PATH];
//         snprintf(full_path, sizeof(full_path), "\\\\.\\%s", device_name);

//         HANDLE hDevice = CreateFileA(full_path,
//                                      0,
//                                      FILE_SHARE_READ | FILE_SHARE_WRITE,
//                                      NULL,
//                                      OPEN_EXISTING,
//                                      0,
//                                      NULL);

//         if (hDevice != INVALID_HANDLE_VALUE)
//         {
//             *ready = true;
//             CloseHandle(hDevice);
//         }

// #elif defined(DISK_PLATFORM_LINUX)
//     char full_path[256];
//     if (strncmp(device_name, "/dev/", 5) == 0)
//     {
//         strncpy(full_path, device_name, sizeof(full_path) - 1);
//     }
//     else
//     {
//         snprintf(full_path, sizeof(full_path), "/dev/%s", device_name);
//     }

//     int fd = open(full_path, O_RDONLY | O_NONBLOCK);
//     if (fd >= 0)
//     {
//         *ready = true;
//         close(fd);
//     }

// #elif defined(DISK_PLATFORM_MACOS)
//     /* macOS implementation (simplified) */
//     *ready = true; /* Assume ready for now */

// #else
//     return DISK_ERROR_NOT_SUPPORTED;
// #endif

//         return DISK_SUCCESS;
//     }

//     /* Simple disk enumeration */
//     static inline disk_error_t disk_enumerate(disk_info_t* disks,
//                                               uint32_t max_disks,
//                                               uint32_t* actual_count)
//     {
//         if (!disks || !actual_count || max_disks == 0)
//         {
//             return DISK_ERROR_INVALID_PARAMETER;
//         }

//         *actual_count = 0;

// #if defined(DISK_PLATFORM_WINDOWS)
//         DWORD drives = GetLogicalDrives();

//         for (int i = 0; i < 26 && *actual_count < max_disks; i++)
//         {
//             if (drives & (1 << i))
//             {
//                 char drive_letter[4] = { (char)('A' + i), ':', '\0', '\0' };
//                 UINT drive_type = GetDriveTypeA(drive_letter);

//                 if (drive_type == DRIVE_FIXED || drive_type ==
//                 DRIVE_REMOVABLE)
//                 {
//                     disk_error_t result =
//                         disk_get_info(drive_letter, &disks[*actual_count]);
//                     if (result == DISK_SUCCESS)
//                     {
//                         (*actual_count)++;
//                     }
//                 }
//             }
//         }

// #elif defined(DISK_PLATFORM_LINUX)
//     /* Scan common disk device patterns */
//     const char* disk_patterns[] = {
//         "/dev/sda",     "/dev/sdb",     "/dev/sdc",     "/dev/sdd",
//         "/dev/nvme0n1", "/dev/nvme1n1", "/dev/nvme2n1", "/dev/hda",
//         "/dev/hdb",     "/dev/hdc",     "/dev/hdd"
//     };

//     for (size_t i = 0; i < sizeof(disk_patterns) / sizeof(disk_patterns[0])
//     &&
//                        *actual_count < max_disks;
//          i++)
//     {
//         if (access(disk_patterns[i], F_OK) == 0)
//         {
//             disk_error_t result =
//                 disk_get_info(disk_patterns[i], &disks[*actual_count]);
//             if (result == DISK_SUCCESS)
//             {
//                 (*actual_count)++;
//             }
//         }
//     }

// #elif defined(DISK_PLATFORM_MACOS)
//     /* macOS implementation (simplified) */
//     if (max_disks > 0)
//     {
//         disk_error_t result = disk_get_info("/dev/disk0", &disks[0]);
//         if (result == DISK_SUCCESS)
//         {
//             *actual_count = 1;
//         }
//     }

// #else
//     return DISK_ERROR_NOT_SUPPORTED;
// #endif

//         return DISK_SUCCESS;
//     }

//     /* Test disk read speed (simplified implementation) */
//     static inline disk_error_t disk_test_read_speed(const char* device_name,
//                                                     uint32_t test_size_mb,
//                                                     double* read_speed_mbps)
//     {
//         if (!device_name || !read_speed_mbps || test_size_mb == 0)
//         {
//             return DISK_ERROR_INVALID_PARAMETER;
//         }

//         /* This is a simplified implementation that would need
//         platform-specific
//          * optimizations */
//         *read_speed_mbps = 100.0; /* Placeholder value */

//         /* In a real implementation, this would:
//          * 1. Open the device for reading
//          * 2. Allocate test buffer
//          * 3. Perform timed sequential reads
//          * 4. Calculate and return actual speed
//          */

//         return DISK_SUCCESS;
//     }

// #ifdef __cplusplus
// }
// #endif

#endif  // DISK_H