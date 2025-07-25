/*
*  This file is part of libcpp.
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

#ifndef GPU_H
#define GPU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef OPENCL_ENABLE
    #define CL_TARGET_OPENCL_VERSION 300
    #include <CL/cl.h>
#elif CUDA_ENABLE
    #include <cuda_runtime.h>
    #include <cuda.h>
#else
    #pragma warning unknown GPU backend, some function will be disabled
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void* id;// （OpenCL: cl_device_id, CUDA: int)
    char name[256];
    char vendor[128];
    char version[128];
    unsigned long long global_mem_sz;
    unsigned int compute_units;
    unsigned int work_group_sz;
} gpu_device_info_t;

typedef struct {
    void* context; // for OpenCL: 
    void* queue;
    void* device;
} gpu_context_t;

typedef struct {
    void* program;
    void* kernel;

#ifdef CUDA_ENABLE
    void** kernel_params;
    int param_count;
    int param_capacity;
#endif
} gpu_program_t;

typedef struct {
    void* ptr;
    size_t size;
} gpu_buffer_t;

typedef bool (*gpu_device_callback_t)(const gpu_device_info_t* info, void* user_data);
typedef void (*gpu_error_callback_t)(const char* error_msg, void* user_data);

// ----------------------------- gpu API define ------------------------------------
// platform info
bool is_opencl_available(void);
bool is_cuda_available(void);

// device info
int gpu_count(void);
int gpu_device_get(gpu_device_info_t** infos, int* count);
void gpu_device_foreach(gpu_device_callback_t callback, void* user_data);

// free gpu resources
int gpu_buffer_free(gpu_buffer_t* buffer);
int gpu_device_free(gpu_device_info_t* infos, int count);
int gpu_context_free(gpu_context_t* ctx);
int gpu_program_free(gpu_program_t* program);

// init context
bool gpu_context_create(gpu_context_t* ctx, const gpu_device_info_t* device_info);

// build program
bool gpu_program_create_from_source(gpu_program_t* program, gpu_context_t* ctx, char* log, unsigned long* log_sz, const char* source);
// bool gpu_create_program_from_file(gpu_program_t* program, gpu_context_t* ctx, const char* filename);

// kernel
bool gpu_kernel_create(gpu_program_t* program, const char* kernel_name);

// malloc gpu memory
bool gpu_malloc(gpu_buffer_t* buffer, gpu_context_t* ctx, size_t size);
bool gpu_malloc_read_only(gpu_buffer_t* buffer, gpu_context_t* ctx, size_t size);
bool gpu_malloc_write_only(gpu_buffer_t* buffer, gpu_context_t* ctx, size_t size);
bool gpu_malloc_pinned(gpu_buffer_t* buffer, size_t size);
bool gpu_malloc_unified(gpu_buffer_t* buffer, size_t size);

// copy gpu memory
bool gpu_memcpy_to_device(gpu_buffer_t* dst, const void* src, size_t size, gpu_context_t* ctx);
bool gpu_memcpy_from_device(void* dst, const gpu_buffer_t* src, size_t size, gpu_context_t* ctx);
bool gpu_memcpy_device_to_device(gpu_buffer_t* dst, const gpu_buffer_t* src, size_t size, gpu_context_t* ctx);
bool gpu_memcpy_to_device_async(gpu_buffer_t* dst, const void* src, size_t size, gpu_context_t* ctx);
bool gpu_memcpy_from_device_async(void* dst, const gpu_buffer_t* src, size_t size, gpu_context_t* ctx);
bool gpu_memcpy_device_to_device_async(gpu_buffer_t* dst, const gpu_buffer_t* src, size_t size, gpu_context_t* ctx);

// set kernel arg
bool gpu_set_kernel_arg(gpu_program_t* program, int arg_index, size_t arg_size, const void* arg_value);
bool gpu_set_kernel_arg_buffer(gpu_program_t* program, int arg_index, const gpu_buffer_t* buffer);

// exec kernel
bool gpu_execute_kernel_1d(gpu_program_t* program, gpu_context_t* ctx, size_t global_work_size, size_t local_work_size);
bool gpu_execute_kernel_2d(gpu_program_t* program, gpu_context_t* ctx, size_t global_work_size[2], size_t local_work_size[2]);
bool gpu_execute_kernel_3d(gpu_program_t* program, gpu_context_t* ctx, size_t global_work_size[3], size_t local_work_size[3]);

// sync run
bool gpu_sync(gpu_context_t* ctx);


// ----------------------------- gpu API implement ------------------------------------

static int _gpu_get_devices_by_opencl(gpu_device_info_t** infos, int* count) 
{
#ifdef OPENCL_ENABLE
    cl_uint num_platforms = 0;
    if (clGetPlatformIDs(0, NULL, &num_platforms) != CL_SUCCESS || num_platforms == 0) 
    {
        *infos = NULL;
        *count = 0;
        return 0;
    }

    cl_platform_id* platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * num_platforms);
    if (!platforms) 
        return -1;
    
    clGetPlatformIDs(num_platforms, platforms, NULL);
    cl_uint total_devices = 0;
    for (cl_uint i = 0; i < num_platforms; ++i) 
    {
        cl_uint num_devices_in_platform = 0;
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices_in_platform);
        total_devices += num_devices_in_platform;
    }

    if (total_devices == 0) 
    {
        free(platforms);
        *infos = NULL;
        *count = 0;
        return 0;
    }

    *infos = (gpu_device_info_t*)calloc(total_devices, sizeof(gpu_device_info_t));
    if (!*infos) 
    {
        free(platforms);
        return -1;
    }

    cl_uint device_index = 0;
    for (cl_uint i = 0; i < num_platforms; ++i) 
    {
        cl_uint num_devices_in_platform = 0;
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices_in_platform);
        if (num_devices_in_platform <= 0) 
            break;

        cl_device_id* devices = (cl_device_id*)malloc(sizeof(cl_device_id) * num_devices_in_platform);
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, num_devices_in_platform, devices, NULL);
        for (cl_uint j = 0; j < num_devices_in_platform; ++j) 
        {
            gpu_device_info_t* info = &(*infos)[device_index];
            info->id = malloc(sizeof(cl_device_id));
            memcpy(info->id, &devices[j], sizeof(cl_device_id));

            clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(info->name), info->name, NULL);
            clGetDeviceInfo(devices[j], CL_DEVICE_VENDOR, sizeof(info->vendor), info->vendor, NULL);
            clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, sizeof(info->version), info->version, NULL);
            
            cl_ulong global_mem_size;
            clGetDeviceInfo(devices[j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(global_mem_size), &global_mem_size, NULL);
            info->global_mem_sz = global_mem_size;
            
            cl_uint compute_units;
            clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
            info->compute_units = compute_units;
            
            size_t work_group_size;
            clGetDeviceInfo(devices[j], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(work_group_size), &work_group_size, NULL);
            info->work_group_sz = (unsigned int)work_group_size;
            
            device_index++;
        }

        free(devices);
    }

    free(platforms);
    *count = (int)total_devices;
#endif
    return 0;
}

static int _gpu_get_devices_by_cuda(gpu_device_info_t** infos, int* count) 
{
#ifdef CUDA_ENABLE
    int device_count = 0;
    if (cudaGetDeviceCount(&device_count) != cudaSuccess || device_count == 0) 
    {
        *infos = NULL;
        *count = 0;
        return 0;
    }

    *infos = (gpu_device_info_t*)calloc(device_count, sizeof(gpu_device_info_t));
    if (!*infos) 
        return -1;

    for (int i = 0; i < device_count; ++i) 
    {
        gpu_device_info_t* info = &(*infos)[i];
        cudaDeviceProp prop;
        
        if (cudaGetDeviceProperties(&prop, i) != cudaSuccess)
            continue;

        info->id = malloc(sizeof(int));
        *((int*)info->id) = i;

        strncpy(info->name, prop.name, sizeof(info->name) - 1);
        strncpy(info->vendor, "NVIDIA", sizeof(info->vendor) - 1);
        snprintf(info->version, sizeof(info->version), "Compute Capability %d.%d", prop.major, prop.minor);

        info->global_mem_sz = prop.totalGlobalMem;
        info->compute_units = prop.multiProcessorCount;
        info->work_group_sz = prop.maxThreadsPerBlock;
    }

    *count = device_count;
#endif
    return 0;
}

bool is_opencl_available(void)
{
#ifdef OPENCL_ENABLE
    cl_uint num_platforms = 0;
    if (clGetPlatformIDs(0, NULL, &num_platforms) == CL_SUCCESS && num_platforms > 0)
        return true;
#endif

    return false;
}

bool is_cuda_available(void)
{
#ifdef CUDA_ENABLE
    int device_count = 0;
    if (cudaGetDeviceCount(&device_count) == cudaSuccess && device_count > 0)
        return true;
#endif

    return false;
}

int gpu_count(void) 
{
    int total_count = 0;
    
#ifdef OPENCL_ENABLE
    cl_uint num_platforms = 0;
    if (clGetPlatformIDs(0, NULL, &num_platforms) == CL_SUCCESS && num_platforms > 0)
    {
        cl_platform_id* platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * num_platforms);
        if (platforms)
        {
            clGetPlatformIDs(num_platforms, platforms, NULL);

            cl_uint total_devices = 0;
            for (cl_uint i = 0; i < num_platforms; ++i) 
            {
                cl_uint num_devices_in_platform = 0;
                clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices_in_platform);
                total_devices += num_devices_in_platform;
            }
            free(platforms);
            total_count += (int)total_devices;
        }
    }
#endif
    
#ifdef CUDA_ENABLE
    int device_count = 0;
    if (cudaGetDeviceCount(&device_count) == cudaSuccess)
        total_count += device_count;
#endif

    return total_count;
}

int gpu_device_get(gpu_device_info_t** infos, int* count) 
{
    if (!infos || !count) 
        return -1;

    int total_count = gpu_count();
    if (total_count == 0) 
    {
        *infos = NULL;
        *count = 0;
        return 0;
    }

    *infos = (gpu_device_info_t*)calloc(total_count, sizeof(gpu_device_info_t));
    if (!(*infos)) 
        return -1;

    int current_index = 0;
#ifdef OPENCL_ENABLE
    gpu_device_info_t* opencl_infos = NULL;
    int opencl_count = 0;
    if (_gpu_get_devices_by_opencl(&opencl_infos, &opencl_count) == 0) 
    {
        memcpy(*infos + current_index, opencl_infos, opencl_count * sizeof(gpu_device_info_t));
        current_index += opencl_count;
        free(opencl_infos);
    }
#endif

#ifdef CUDA_ENABLE
    gpu_device_info_t* cuda_infos = NULL;
    int cuda_count = 0;
    if (_gpu_get_devices_by_cuda(&cuda_infos, &cuda_count) == 0) 
    {
        memcpy(*infos + current_index, cuda_infos, cuda_count * sizeof(gpu_device_info_t));
        current_index += cuda_count;
        free(cuda_infos);
    }
#endif

    *count = current_index;
    return 0;
}

void gpu_device_foreach(gpu_device_callback_t callback, void* user_data) 
{
    if (!callback) 
        return;

    gpu_device_info_t* infos = NULL;
    int count = 0;
    if (gpu_device_get(&infos, &count) != 0 || infos == NULL || count == 0) 
        return;

    for (int i = 0; i < count; ++i) 
    {
        if (!callback(&infos[i], user_data))
            break;
    }

    gpu_device_free(infos, count);
}

bool gpu_context_create(gpu_context_t* ctx, const gpu_device_info_t* device_info)
{
    if (!ctx) 
        return false;

    memset(ctx, 0, sizeof(gpu_context_t));

#ifdef OPENCL_ENABLE
    cl_int err;
    cl_device_id device;
    
    if (device_info && device_info->id) 
    {
        device = *((cl_device_id*)device_info->id);
    } 
    else 
    {
        gpu_device_info_t* infos = NULL;
        int count = 0;
        if (gpu_device_get(&infos, &count) != 0 || count == 0) 
            return false;

        device = *((cl_device_id*)infos[0].id);
        gpu_device_free(infos, count);
    }
    
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (err != CL_SUCCESS) 
        return false;
    
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);
    if (err != CL_SUCCESS) 
    {
        clReleaseContext(context);
        return false;
    }
    
    ctx->context = context;
    ctx->queue = queue;
    ctx->device = malloc(sizeof(cl_device_id));
    memcpy(ctx->device, &device, sizeof(cl_device_id));
    return true;
#endif
    
#ifdef CUDA_ENABLE
    int device_id;
    
    if (device_info && device_info->id) 
        device_id = *((int*)device_info->id);
    else
        device_id = 0;
    
    cudaError_t err = cudaSetDevice(device_id);
    if (err != cudaSuccess)
        return false;
    
    cudaStream_t stream;
    err = cudaStreamCreate(&stream);
    if (err != cudaSuccess)
        return false;
    
    ctx->context = NULL;
    ctx->queue = stream;
    ctx->device = malloc(sizeof(int));
    *((int*)ctx->device) = device_id;
    return true;
#endif

    return false;
}

int gpu_device_free(gpu_device_info_t* infos, int count) 
{
    if (!infos) 
        return -1;
    
    for (int i = 0; i < count; i++) 
    {
        if (infos[i].id) 
            free(infos[i].id);
    }
    
    free(infos);
    return 0;
}

int gpu_buffer_free(gpu_buffer_t* buffer)
{
    if (!buffer || !buffer->ptr) 
        return -1;

#ifdef OPENCL_ENABLE
    cl_mem mem_obj = (cl_mem)buffer->ptr;
    cl_int err = clReleaseMemObject(mem_obj);
    if (err != CL_SUCCESS)
        return -1;
#endif

#ifdef CUDA_ENABLE
    cudaError_t err = cudaFree(buffer->ptr);
    if (err != cudaSuccess)
        return -1;
#endif

    buffer->ptr = NULL;
    buffer->size = 0;
    return 0;
}

int gpu_context_free(gpu_context_t* ctx)
{
    if (!ctx) 
        return -1;

#ifdef OPENCL_ENABLE
    if (ctx->queue)
        clReleaseCommandQueue((cl_command_queue)ctx->queue);

    if (ctx->context) 
        clReleaseContext((cl_context)ctx->context);

#elif CUDA_ENABLE
    if (ctx->queue) 
        cudaStreamDestroy((cudaStream_t)ctx->queue);

#endif

    if (ctx->device) 
        free(ctx->device);
    
    memset(ctx, 0, sizeof(gpu_context_t));
    return 0;
}

int gpu_program_free(gpu_program_t* program)
{
    if (!program)
        return -1;

#ifdef OPENCL_ENABLE
    if (program->kernel) 
        clReleaseKernel((cl_kernel)program->kernel);
    
    if (program->program) 
        clReleaseProgram((cl_program)program->program);
    
#elif CUDA_ENABLE
    if (program->program) 
        cuModuleUnload((CUmodule)program->program);
    
#endif

    memset(program, 0, sizeof(gpu_program_t));
    return 0;
}

bool gpu_program_create_from_source(gpu_program_t* program, gpu_context_t* ctx, char* log, unsigned long* log_sz, const char* source)
{
    if (!program || !ctx || !source) 
        return false;

    memset(program, 0, sizeof(gpu_program_t));

#ifdef OPENCL_ENABLE
    cl_int err;
    cl_context context = (cl_context)ctx->context;
    cl_device_id device = *((cl_device_id*)ctx->device);
    cl_program cl_prog = clCreateProgramWithSource(context, 1, &source, NULL, &err);
    if (err != CL_SUCCESS) 
        return false;
    
    err = clBuildProgram(cl_prog, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) 
    {
        size_t sz = 0;
        clGetProgramBuildInfo(cl_prog, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &sz);
        if (sz > 0 && log && log_sz) 
        {
            clGetProgramBuildInfo(cl_prog, device, CL_PROGRAM_BUILD_LOG, sz, log, NULL);
            *log_sz = (unsigned long)sz;
        }
        else if (log_sz) 
        {
            *log_sz = 0;
        }
        clReleaseProgram(cl_prog);
        return false;
    }
    
    *log_sz = 0;
    program->program = cl_prog;
    program->kernel = NULL;
    return true;
    
#elif CUDA_ENABLE
    CUmodule module;
    CUresult result = cuModuleLoadData(&module, source);
    if (result != CUDA_SUCCESS) 
    {
        const char* error_string = NULL;
        cuGetErrorString(result, &error_string);
        
        if (error_string && log && log_sz) 
        {
            size_t error_len = strlen(error_string);
            size_t max_copy = (*log_sz > 0) ? *log_sz - 1 : 0;
            size_t copy_len = (error_len < max_copy) ? error_len : max_copy;
            
            if (copy_len > 0) {
                strncpy(log, error_string, copy_len);
                log[copy_len] = '\0';
            }
            *log_sz = (unsigned long)copy_len;
        }
        else if (log_sz) 
        {
            *log_sz = 0;
        }
        
        return false;
    }
    
    *log_sz = 0;
    program->program = (void*)module;
    program->kernel = NULL;
    return true;
    
#endif
    
    return false;
}

bool gpu_kernel_create(gpu_program_t* program, const char* kernel_name)
{
    if (!program || !program->program || !kernel_name) 
        return false;

#ifdef GPU_ENABLE_OPENCL
    cl_int err;
    cl_kernel kernel = clCreateKernel((cl_program)program->program, kernel_name, &err);
    if (err != CL_SUCCESS) 
        return false;
    
    if (program->kernel)
        clReleaseKernel((cl_kernel)program->kernel);
    
    program->kernel = kernel;
    return true;
    
#elif GPU_ENABLE_CUDA
    CUfunction function;
    CUresult result = cuModuleGetFunction(&function, (CUmodule)program->program, kernel_name);
    if (result != CUDA_SUCCESS)
        return false;
    
    program->kernel = (void*)function;
    return true;
    
#endif
    
    return false;
}

bool gpu_malloc(gpu_buffer_t* buffer, gpu_context_t* ctx, size_t size)
{
    if (!buffer || !ctx || size == 0) 
        return false;

    memset(buffer, 0, sizeof(gpu_buffer_t));

#ifdef OPENCL_ENABLE
    cl_int err;
    cl_context context = (cl_context)ctx->context;
    
    cl_mem mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, size, NULL, &err);
    if (err != CL_SUCCESS) 
        return false;
    
    buffer->ptr = mem_obj;
    buffer->size = size;
    return true;
    
#elif CUDA_ENABLE
    void* dev_ptr = NULL;
    cudaError_t err = cudaMalloc(&dev_ptr, size);
    if (err != cudaSuccess) 
        return false;
    
    buffer->ptr = dev_ptr;
    buffer->size = size;
    return true;
    
#endif
    
    return false;
}

bool gpu_malloc_read_only(gpu_buffer_t* buffer, gpu_context_t* ctx, size_t size)
{
    if (!buffer || !ctx || size == 0) 
        return false;

    memset(buffer, 0, sizeof(gpu_buffer_t));

#ifdef OPENCL_ENABLE
    cl_int err;
    cl_context context = (cl_context)ctx->context;
    
    // 分配只读内存
    cl_mem mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, size, NULL, &err);
    if (err != CL_SUCCESS) 
        return false;
    
    buffer->ptr = mem_obj;
    buffer->size = size;
    return true;
    
#elif CUDA_ENABLE
    void* dev_ptr = NULL;
    cudaError_t err = cudaMalloc(&dev_ptr, size);
    if (err != cudaSuccess) 
        return false;
    
    buffer->ptr = dev_ptr;
    buffer->size = size;
    return true;
    
#endif
    
    return false;
}

bool gpu_malloc_write_only(gpu_buffer_t* buffer, gpu_context_t* ctx, size_t size)
{
    if (!buffer || !ctx || size == 0) 
        return false;

    memset(buffer, 0, sizeof(gpu_buffer_t));

#ifdef OPENCL_ENABLE
    cl_int err;
    cl_context context = (cl_context)ctx->context;
    
    cl_mem mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, size, NULL, &err);
    if (err != CL_SUCCESS) 
        return false;
    
    buffer->ptr = mem_obj;
    buffer->size = size;
    return true;
    
#elif CUDA_ENABLE
    void* dev_ptr = NULL;
    cudaError_t err = cudaMalloc(&dev_ptr, size);
    if (err != cudaSuccess) 
        return false;
    
    buffer->ptr = dev_ptr;
    buffer->size = size;
    return true;
    
#endif
    
    return false;
}

bool gpu_malloc_pinned(gpu_buffer_t* buffer, size_t size)
{
    if (!buffer || size == 0) 
        return false;

    memset(buffer, 0, sizeof(gpu_buffer_t));

#ifdef CUDA_ENABLE
    void* host_ptr = NULL;
    cudaError_t err = cudaMallocHost(&host_ptr, size);
    if (err != cudaSuccess) 
        return false;
    
    buffer->ptr = host_ptr;
    buffer->size = size;
    return true;
#endif

    return true;
}

bool gpu_malloc_unified(gpu_buffer_t* buffer, size_t size)
{
    if (!buffer || size == 0) 
        return false;

    memset(buffer, 0, sizeof(gpu_buffer_t));
    
#ifdef CUDA_ENABLE
    void* unified_ptr = NULL;
    cudaError_t err = cudaMallocManaged(&unified_ptr, size);
    if (err != cudaSuccess) 
        return false;
    
    buffer->ptr = unified_ptr;
    buffer->size = size;
    return true;
#endif

    return true;
}

bool gpu_memcpy_to_device(gpu_buffer_t* dst, const void* src, size_t size, gpu_context_t* ctx)
{
    if (!dst || !dst->ptr || !src || !ctx || size == 0) 
        return false;

    if (size > dst->size) 
        return false;

#ifdef OPENCL_ENABLE
    cl_command_queue queue = (cl_command_queue)ctx->queue;
    cl_mem mem_obj = (cl_mem)dst->ptr;
    
    cl_int err = clEnqueueWriteBuffer(queue, mem_obj, CL_TRUE, 0, size, src, 0, NULL, NULL);
    if (err != CL_SUCCESS) 
        return false;
    
    return true;
    
#elif CUDA_ENABLE
    cudaStream_t stream = (cudaStream_t)ctx->queue;
    
    cudaError_t err = cudaMemcpyAsync(dst->ptr, src, size, cudaMemcpyHostToDevice, stream);
    if (err != cudaSuccess) 
        return false;
    
    err = cudaStreamSynchronize(stream);
    if (err != cudaSuccess) 
        return false;
    
    return true;
    
#endif

    return false;
}

bool gpu_memcpy_from_device(void* dst, const gpu_buffer_t* src, size_t size, gpu_context_t* ctx)
{
    if (!dst || !src || !src->ptr || !ctx || size == 0) 
        return false;

    if (size > src->size) 
        return false;

#ifdef OPENCL_ENABLE
    cl_command_queue queue = (cl_command_queue)ctx->queue;
    cl_mem mem_obj = (cl_mem)src->ptr;
    
    cl_int err = clEnqueueReadBuffer(queue, mem_obj, CL_TRUE, 0, size, dst, 0, NULL, NULL);
    if (err != CL_SUCCESS) 
        return false;
    
    return true;
    
#elif CUDA_ENABLE
    cudaStream_t stream = (cudaStream_t)ctx->queue;
    
    cudaError_t err = cudaMemcpyAsync(dst, src->ptr, size, cudaMemcpyDeviceToHost, stream);
    if (err != cudaSuccess) 
        return false;
    
    err = cudaStreamSynchronize(stream);
    if (err != cudaSuccess) 
        return false;
    
    return true;
    
#endif
    
    return false;
}

bool gpu_memcpy_device_to_device(gpu_buffer_t* dst, const gpu_buffer_t* src, size_t size, gpu_context_t* ctx)
{
    if (!dst || !dst->ptr || !src || !src->ptr || !ctx || size == 0) 
        return false;

    if (size > dst->size || size > src->size) 
        return false;

#ifdef OPENCL_ENABLE
    cl_command_queue queue = (cl_command_queue)ctx->queue;
    cl_mem src_mem = (cl_mem)src->ptr;
    cl_mem dst_mem = (cl_mem)dst->ptr;
    
    cl_int err = clEnqueueCopyBuffer(queue, src_mem, dst_mem, 0, 0, size, 0, NULL, NULL);
    if (err != CL_SUCCESS) 
        return false;
    
    err = clFinish(queue);
    if (err != CL_SUCCESS) 
        return false;
    
    return true;
    
#elif CUDA_ENABLE
    cudaStream_t stream = (cudaStream_t)ctx->queue;
    
    cudaError_t err = cudaMemcpyAsync(dst->ptr, src->ptr, size, cudaMemcpyDeviceToDevice, stream);
    if (err != cudaSuccess) 
        return false;
    
    err = cudaStreamSynchronize(stream);
    if (err != cudaSuccess) 
        return false;
    
    return true;
    
#endif
    
    return false;
}

bool gpu_memcpy_to_device_async(gpu_buffer_t* dst, const void* src, size_t size, gpu_context_t* ctx)
{
    if (!dst || !dst->ptr || !src || !ctx || size == 0) 
        return false;

    if (size > dst->size) 
        return false;

#ifdef OPENCL_ENABLE
    cl_command_queue queue = (cl_command_queue)ctx->queue;
    cl_mem mem_obj = (cl_mem)dst->ptr;
    
    cl_int err = clEnqueueWriteBuffer(queue, mem_obj, CL_FALSE, 0, size, src, 0, NULL, NULL);
    return (err == CL_SUCCESS);
    
#elif CUDA_ENABLE
    cudaStream_t stream = (cudaStream_t)ctx->queue;
    
    cudaError_t err = cudaMemcpyAsync(dst->ptr, src, size, cudaMemcpyHostToDevice, stream);
    return (err == cudaSuccess);
    
#endif
    
    return false;
}

bool gpu_memcpy_from_device_async(void* dst, const gpu_buffer_t* src, size_t size, gpu_context_t* ctx)
{
    if (!dst || !src || !src->ptr || !ctx || size == 0) 
        return false;

    if (size > src->size) 
        return false;

#ifdef OPENCL_ENABLE
    cl_command_queue queue = (cl_command_queue)ctx->queue;
    cl_mem mem_obj = (cl_mem)src->ptr;
    
    cl_int err = clEnqueueReadBuffer(queue, mem_obj, CL_FALSE, 0, size, dst, 0, NULL, NULL);
    return (err == CL_SUCCESS);
    
#elif CUDA_ENABLE
    cudaStream_t stream = (cudaStream_t)ctx->queue;
    
    cudaError_t err = cudaMemcpyAsync(dst, src->ptr, size, cudaMemcpyDeviceToHost, stream);
    return (err == cudaSuccess);
    
#endif
    
    return false;
}

bool gpu_memcpy_device_to_device_async(gpu_buffer_t* dst, const gpu_buffer_t* src, size_t size, gpu_context_t* ctx)
{
    if (!dst || !dst->ptr || !src || !src->ptr || !ctx || size == 0) 
        return false;

    if (size > dst->size || size > src->size) 
        return false;

#ifdef OPENCL_ENABLE
    cl_command_queue queue = (cl_command_queue)ctx->queue;
    cl_mem src_mem = (cl_mem)src->ptr;
    cl_mem dst_mem = (cl_mem)dst->ptr;
    cl_int err = clEnqueueCopyBuffer(queue, src_mem, dst_mem, 0, 0, size, 0, NULL, NULL);
    return (err == CL_SUCCESS);
    
#elif CUDA_ENABLE
    cudaStream_t stream = (cudaStream_t)ctx->queue;
    cudaError_t err = cudaMemcpyAsync(dst->ptr, src->ptr, size, cudaMemcpyDeviceToDevice, stream);
    return (err == cudaSuccess);
    
#endif
    
    return false;
}

bool gpu_set_kernel_arg(gpu_program_t* program, int arg_index, size_t arg_size, const void* arg_value)
{
    if (!program || !program->kernel || arg_index < 0) 
        return false;

#ifdef OPENCL_ENABLE
    cl_kernel kernel = (cl_kernel)program->kernel;
    cl_int err = clSetKernelArg(kernel, (cl_uint)arg_index, arg_size, arg_value);
    return (err == CL_SUCCESS);
    
#elif CUDA_ENABLE
    if (arg_index >= program->param_capacity) 
    {
        int new_capacity = arg_index + 8;
        void** new_params = (void**)realloc(program->kernel_params, 
                                           new_capacity * sizeof(void*));
        if (!new_params) 
            return false;
        
        for (int i = program->param_capacity; i < new_capacity; i++)
            new_params[i] = NULL;
        
        program->kernel_params = new_params;
        program->param_capacity = new_capacity;
    }
    
    if (program->kernel_params[arg_index])
        free(program->kernel_params[arg_index]);
    
    program->kernel_params[arg_index] = malloc(arg_size);
    if (!program->kernel_params[arg_index]) 
        return false;
    
    memcpy(program->kernel_params[arg_index], arg_value, arg_size);
    
    if (arg_index >= program->param_count)
        program->param_count = arg_index + 1;
    
    return true;
    
#endif
    
    return false;
}

bool gpu_set_kernel_arg_buffer(gpu_program_t* program, int arg_index, const gpu_buffer_t* buffer)
{
    if (!program || !program->kernel || !buffer || !buffer->ptr || arg_index < 0) 
        return false;

#ifdef OPENCL_ENABLE
    cl_kernel kernel = (cl_kernel)program->kernel;
    cl_mem mem_obj = (cl_mem)buffer->ptr;
    
    cl_int err = clSetKernelArg(kernel, (cl_uint)arg_index, sizeof(cl_mem), &mem_obj);
    return (err == CL_SUCCESS);
    
#elif CUDA_ENABLE
    return gpu_set_kernel_arg(program, arg_index, sizeof(void*), &buffer->ptr);
    
#endif
    
    return false;
}

bool gpu_execute_kernel_1d(gpu_program_t* program, gpu_context_t* ctx, size_t global_work_size, size_t local_work_size)
{
    if (!program || !program->kernel || !ctx || global_work_size == 0) 
        return false;

#ifdef OPENCL_ENABLE
    cl_command_queue queue = (cl_command_queue)ctx->queue;
    cl_kernel kernel = (cl_kernel)program->kernel;
    
    size_t* local_ptr = (local_work_size > 0) ? &local_work_size : NULL;
    
    cl_int err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_work_size, local_ptr, 0, NULL, NULL);
    if (err != CL_SUCCESS) 
        return false;
    
    return true;
    
#elif CUDA_ENABLE
    CUfunction function = (CUfunction)program->kernel;
    cudaStream_t stream = (cudaStream_t)ctx->queue;
    
    unsigned int block_size = (local_work_size > 0) ? (unsigned int)local_work_size : 256;
    unsigned int grid_size = (unsigned int)((global_work_size + block_size - 1) / block_size);
    CUresult result = cuLaunchKernel(function,
                                     grid_size, 1, 1,        // grid dimensions
                                     block_size, 1, 1,       // block dimensions
                                     0,                      // shared memory size
                                     stream,                 // stream
                                     program->kernel_params, // parameters
                                     NULL);                  // extra options
    
    return (result == CUDA_SUCCESS);
    
#endif
    
    return false;
}

bool gpu_execute_kernel_2d(gpu_program_t* program, gpu_context_t* ctx, size_t global_work_size[2], size_t local_work_size[2])
{
    if (!program || !program->kernel || !ctx || !global_work_size || 
        global_work_size[0] == 0 || global_work_size[1] == 0) 
        return false;

#ifdef OPENCL_ENABLE
    cl_command_queue queue = (cl_command_queue)ctx->queue;
    cl_kernel kernel = (cl_kernel)program->kernel;
    
    size_t* local_ptr = NULL;
    if (local_work_size && local_work_size[0] > 0 && local_work_size[1] > 0) 
        local_ptr = local_work_size;
    
    cl_int err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_work_size, local_ptr, 0, NULL, NULL);
    if (err != CL_SUCCESS) 
        return false;
    
    return true;
    
#elif CUDA_ENABLE
    CUfunction function = (CUfunction)program->kernel;
    cudaStream_t stream = (cudaStream_t)ctx->queue;
    
    unsigned int block_x = (local_work_size && local_work_size[0] > 0) ? (unsigned int)local_work_size[0] : 16;
    unsigned int block_y = (local_work_size && local_work_size[1] > 0) ? (unsigned int)local_work_size[1] : 16;
    
    unsigned int grid_x = (unsigned int)((global_work_size[0] + block_x - 1) / block_x);
    unsigned int grid_y = (unsigned int)((global_work_size[1] + block_y - 1) / block_y);
    
    CUresult result = cuLaunchKernel(function,
                                     grid_x, grid_y, 1,      // grid dimensions
                                     block_x, block_y, 1,    // block dimensions
                                     0,                      // shared memory size
                                     stream,                 // stream
                                     program->kernel_params, // parameters
                                     NULL);                  // extra options
    
    return (result == CUDA_SUCCESS);
    
#endif
    
    return false;
}

bool gpu_execute_kernel_3d(gpu_program_t* program, gpu_context_t* ctx, size_t global_work_size[3], size_t local_work_size[3])
{
    if (!program || !program->kernel || !ctx || !global_work_size || 
        global_work_size[0] == 0 || global_work_size[1] == 0 || global_work_size[2] == 0) 
        return false;

#ifdef OPENCL_ENABLE
    cl_command_queue queue = (cl_command_queue)ctx->queue;
    cl_kernel kernel = (cl_kernel)program->kernel;
    
    size_t* local_ptr = NULL;
    if (local_work_size && local_work_size[0] > 0 && local_work_size[1] > 0 && local_work_size[2] > 0) 
        local_ptr = local_work_size;
    
    cl_int err = clEnqueueNDRangeKernel(queue, kernel, 3, NULL, global_work_size, local_ptr, 0, NULL, NULL);
    if (err != CL_SUCCESS) 
        return false;
    
    return true;
    
#elif CUDA_ENABLE
    CUfunction function = (CUfunction)program->kernel;
    cudaStream_t stream = (cudaStream_t)ctx->queue;
    
    unsigned int block_x = (local_work_size && local_work_size[0] > 0) ? (unsigned int)local_work_size[0] : 8;
    unsigned int block_y = (local_work_size && local_work_size[1] > 0) ? (unsigned int)local_work_size[1] : 8;
    unsigned int block_z = (local_work_size && local_work_size[2] > 0) ? (unsigned int)local_work_size[2] : 8;
    
    unsigned int grid_x = (unsigned int)((global_work_size[0] + block_x - 1) / block_x);
    unsigned int grid_y = (unsigned int)((global_work_size[1] + block_y - 1) / block_y);
    unsigned int grid_z = (unsigned int)((global_work_size[2] + block_z - 1) / block_z);
    
    CUresult result = cuLaunchKernel(function,
                                     grid_x, grid_y, grid_z,    // grid dimensions
                                     block_x, block_y, block_z, // block dimensions
                                     0,                         // shared memory size
                                     stream,                    // stream
                                     program->kernel_params,    // parameters
                                     NULL);                     // extra options
    
    return (result == CUDA_SUCCESS);
    
#endif
    
    return false;
}

bool gpu_sync(gpu_context_t* ctx)
{
    if (!ctx) 
        return false;

#ifdef OPENCL_ENABLE
    cl_command_queue queue = (cl_command_queue)ctx->queue;
    cl_int err = clFinish(queue);
    return (err == CL_SUCCESS);
    
#elif CUDA_ENABLE
    cudaStream_t stream = (cudaStream_t)ctx->queue;
    cudaError_t err = cudaStreamSynchronize(stream);
    return (err == cudaSuccess);
    
#endif
    
    return false;
}

#ifdef __cplusplus
}
#endif

#endif /* GPU_H */