#ifndef GPU_H
#define GPU_H

#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>

typedef bool(*gpu_device_range_cb)(cl_device_id);

int gpu_count()
{
    cl_uint num_platforms = 0;
    clGetPlatformIDs(0, NULL, &num_platforms);
    if (num_platforms == 0)
        return 0;

    cl_platform_id* platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * num_platforms);
    clGetPlatformIDs(num_platforms, platforms, NULL);

    cl_uint total_devices = 0;
    for (cl_uint i = 0; i < num_platforms; ++i)
    {
        cl_uint num_devices_in_platform = 0;
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices_in_platform);
        total_devices += num_devices_in_platform;
    }
    free(platforms);
    return total_devices;
}

void gpu_devices(cl_device_id* devices, cl_uint num_devices)
{
    cl_uint num_platforms = 0;
    clGetPlatformIDs(0, NULL, &num_platforms);
    if (num_platforms == 0 || num_devices == 0)
        return;

    cl_platform_id* platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * num_platforms);
    clGetPlatformIDs(num_platforms, platforms, NULL);

    cl_uint total_devices = 0;
    for (cl_uint i = 0; i < num_platforms; ++i)
    {
        cl_uint num_devices_in_platform = 0;
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices_in_platform);
        if (total_devices + num_devices_in_platform > num_devices)
            break;

        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, num_devices_in_platform, &devices[total_devices], NULL);
        total_devices += num_devices_in_platform;
    }
    free(platforms);
}

void gpu_device_range(const gpu_device_range_cb fn)
{
    cl_uint num_platforms = 0;
    clGetPlatformIDs(0, NULL, &num_platforms);
    if (num_platforms == 0)
        return;

    cl_platform_id* platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * num_platforms);
    clGetPlatformIDs(num_platforms, platforms, NULL);

    for (cl_uint i = 0; i < num_platforms; ++i)
    {
        cl_uint num_devices = 0;
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
        if (num_devices == 0)
            continue;

        cl_device_id* devices = (cl_device_id*)malloc(sizeof(cl_device_id) * num_devices);
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, num_devices, devices, NULL);

        for (cl_uint j = 0; j < num_devices; ++j)
        {
            if (!fn(devices[j]))
            {
                free(devices);
                free(platforms);
                return;
            }
        }
        free(devices);
    }
    free(platforms);
}

#endif