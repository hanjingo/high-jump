// /*
// *  This file is part of libcpp.
// *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
// *
// *  This program is free software: you can redistribute it and/or modify
// *  it under the terms of the GNU General Public License as published by
// *  the Free Software Foundation, either version 3 of the License, or
// *  (at your option) any later version.
// *
// *  This program is distributed in the hope that it will be useful,
// *  but WITHOUT ANY WARRANTY; without even the implied warranty of
// *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// *  GNU General Public License for more details.
// *
// *  You should have received a copy of the GNU General Public License
// *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
// */

// #ifndef GPU_H
// #define GPU_H

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <stdbool.h>

// #ifdef __cplusplus
// extern "C" {
// #endif

// typedef enum {
//     GPU_BACKEND_NONE = 0,
//     GPU_BACKEND_OPENCL = 1,
//     GPU_BACKEND_CUDA = 2
// } gpu_backend_t;

// typedef enum {
//     GPU_DEVICE_TYPE_ALL = 0,
//     GPU_DEVICE_TYPE_GPU = 1,
//     GPU_DEVICE_TYPE_CPU = 2
// } gpu_device_type_t;

// typedef struct {
//     void* id;// （OpenCL: cl_device_id, CUDA: int
//     char name[256];
//     char vendor[128];
//     char version[128];
//     unsigned long long global_mem_sz;
//     unsigned int compute_units;
//     unsigned int work_group_sz;
//     gpu_backend_t backend;
//     gpu_device_type_t device_type;
// } gpu_device_info_t;

// typedef struct {
//     void* context; // for OpenCL: 
//     void* queue;
//     void* device;
//     gpu_backend_t backend;
// } gpu_context_t;

// typedef struct {
//     void* program;
//     void* kernel;
//     gpu_backend_t backend;
// } gpu_program_t;

// typedef struct {
//     void* ptr;
//     size_t size;
//     gpu_backend_t backend;
// } gpu_buffer_t;

// typedef bool (*gpu_device_callback_t)(const gpu_device_info_t* info, void* user_data);

// // ----------------------------- gpu API define ------------------------------------
// // check gpu is available
// bool gpu_is_opencl_available(void);
// bool gpu_is_cuda_available(void);
// gpu_backend_t gpu_get_preferred_backend(void);

// // get gpu device count
// int gpu_count(void);
// int gpu_count_by_backend(gpu_backend_t backend);
// int gpu_count_by_type(gpu_device_type_t type);

// // get gpu device info
// int gpu_get_devices(gpu_device_info_t** infos, int* count);
// int gpu_get_devices_by_backend(gpu_device_info_t** infos, int* count, gpu_backend_t backend);
// int gpu_get_devices_by_type(gpu_device_info_t** infos, int* count, gpu_device_type_t type);
// bool gpu_get_device_info_by_index(int index, gpu_device_info_t* info);
// bool gpu_get_best_device(gpu_device_info_t* info, gpu_backend_t preferred_backend);

// // range gpu device
// void gpu_foreach_device(gpu_device_callback_t callback, void* user_data);
// void gpu_foreach_device_by_backend(gpu_device_callback_t callback, void* user_data, gpu_backend_t backend);

// // free device info
// void gpu_free_devices(gpu_device_info_t* infos);

// // init context
// bool gpu_create_context(gpu_context_t* ctx, const gpu_device_info_t* device_info);
// bool gpu_create_context_by_index(gpu_context_t* ctx, int device_index);
// bool gpu_create_default_context(gpu_context_t* ctx);

// // free context
// void gpu_destroy_context(gpu_context_t* ctx);

// // build program
// bool gpu_create_program_from_source(gpu_program_t* program, gpu_context_t* ctx, const char* source);
// bool gpu_create_program_from_file(gpu_program_t* program, gpu_context_t* ctx, const char* filename);

// // kernel
// bool gpu_create_kernel(gpu_program_t* program, const char* kernel_name);

// // destroy program
// void gpu_destroy_program(gpu_program_t* program);

// // malloc gpu memory
// bool gpu_malloc(gpu_buffer_t* buffer, gpu_context_t* ctx, size_t size);
// bool gpu_malloc_read_only(gpu_buffer_t* buffer, gpu_context_t* ctx, size_t size);
// bool gpu_malloc_write_only(gpu_buffer_t* buffer, gpu_context_t* ctx, size_t size);

// // copy gpu memory
// bool gpu_memcpy_to_device(gpu_buffer_t* dst, const void* src, size_t size, gpu_context_t* ctx);
// bool gpu_memcpy_from_device(void* dst, const gpu_buffer_t* src, size_t size, gpu_context_t* ctx);
// bool gpu_memcpy_device_to_device(gpu_buffer_t* dst, const gpu_buffer_t* src, size_t size, gpu_context_t* ctx);

// // release gpu memory
// void gpu_free(gpu_buffer_t* buffer);

// // set kernel arg
// bool gpu_set_kernel_arg(gpu_program_t* program, int arg_index, size_t arg_size, const void* arg_value);
// bool gpu_set_kernel_arg_buffer(gpu_program_t* program, int arg_index, const gpu_buffer_t* buffer);

// // exec kernel
// bool gpu_execute_kernel_1d(gpu_program_t* program, gpu_context_t* ctx, size_t global_work_size, size_t local_work_size);
// bool gpu_execute_kernel_2d(gpu_program_t* program, gpu_context_t* ctx, size_t global_work_size[2], size_t local_work_size[2]);
// bool gpu_execute_kernel_3d(gpu_program_t* program, gpu_context_t* ctx, size_t global_work_size[3], size_t local_work_size[3]);

// // sync run
// bool gpu_sync(gpu_context_t* ctx);

// // get last error
// const char* gpu_get_last_error(void);
// void gpu_clear_last_error(void);

// // set error callback
// typedef void (*gpu_error_callback_t)(const char* error_msg, void* user_data);
// void gpu_set_error_callback(gpu_error_callback_t callback, void* user_data);

// // get gpu backend
// const char* gpu_backend_name(gpu_backend_t backend);
// const char* gpu_device_type_name(gpu_device_type_t type);

// // performance
// bool gpu_benchmark_memory_bandwidth(gpu_context_t* ctx, size_t buffer_size, double* bandwidth_gb_s);
// bool gpu_benchmark_compute_performance(gpu_context_t* ctx, double* gflops);

// #ifdef GPU_ENABLE_OPENCL
//     #define CL_TARGET_OPENCL_VERSION 300
//     #include <CL/cl.h>
// #endif

// #ifdef GPU_ENABLE_CUDA
//     #include <cuda_runtime.h>
//     #include <cuda.h>
// #endif

// static char gpu_last_error[512] = {0};
// static gpu_error_callback_t gpu_error_callback = NULL;
// static void* gpu_error_callback_user_data = NULL;

// static void gpu_set_error(const char* format, ...) 
// {
//     va_list args;
//     va_start(args, format);
//     vsnprintf(g_last_error, sizeof(g_last_error), format, args);
//     va_end(args);
    
//     if (g_error_callback)
//         g_error_callback(g_last_error, g_error_callback_user_data);
// }

// bool gpu_is_opencl_available(void) 
// {
// #ifdef GPU_ENABLE_OPENCL
//     cl_uint num_platforms = 0;
//     cl_int err = clGetPlatformIDs(0, NULL, &num_platforms);
//     return (err == CL_SUCCESS && num_platforms > 0);
// #else
//     return false;
// #endif
// }

// bool gpu_is_cuda_available(void) 
// {
// #ifdef GPU_ENABLE_CUDA
//     int device_count = 0;
//     cudaError_t err = cudaGetDeviceCount(&device_count);
//     return (err == cudaSuccess && device_count > 0);
// #else
//     return false;
// #endif
// }

// gpu_backend_t gpu_get_preferred_backend(void) 
// {
//     if (gpu_is_cuda_available())
//         return GPU_BACKEND_CUDA;
//     else if (gpu_is_opencl_available())
//         return GPU_BACKEND_OPENCL;

//     return GPU_BACKEND_NONE;
// }

// int gpu_count(void) 
// {
//     int opencl_count = 0;
//     int cuda_count = 0;
    
// #ifdef GPU_ENABLE_OPENCL
//     opencl_count = gpu_count_by_backend(GPU_BACKEND_OPENCL);
// #endif

// #ifdef GPU_ENABLE_CUDA
//     cuda_count = gpu_count_by_backend(GPU_BACKEND_CUDA);
// #endif

//     return opencl_count + cuda_count;
// }

// int gpu_count_by_backend(gpu_backend_t backend) 
// {
//     switch (backend) {
// #ifdef GPU_ENABLE_OPENCL
//         case GPU_BACKEND_OPENCL: {
//             cl_uint num_platforms = 0;
//             if (clGetPlatformIDs(0, NULL, &num_platforms) != CL_SUCCESS || num_platforms == 0) 
//                 return 0;

//             cl_platform_id* platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * num_platforms);
//             if (!platforms) return 0;
            
//             clGetPlatformIDs(num_platforms, platforms, NULL);

//             cl_uint total_devices = 0;
//             for (cl_uint i = 0; i < num_platforms; ++i) 
//             {
//                 cl_uint num_devices_in_platform = 0;
//                 clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices_in_platform);
//                 total_devices += num_devices_in_platform;
//             }
//             free(platforms);
//             return (int)total_devices;
//         }
// #endif

// #ifdef GPU_ENABLE_CUDA
//         case GPU_BACKEND_CUDA: {
//             int device_count = 0;
//             if (cudaGetDeviceCount(&device_count) != cudaSuccess)
//                 return 0;

//             return device_count;
//         }
// #endif
//         default:
//             return 0;
//     }
// }

// int gpu_get_devices(gpu_device_info_t** infos, int* count) 
// {
//     if (!infos || !count) 
//     {
//         gpu_set_error("Invalid parameters: infos or count is NULL");
//         return -1;
//     }

//     int total_count = gpu_count();
//     if (total_count == 0) 
//     {
//         *infos = NULL;
//         *count = 0;
//         return 0;
//     }

//     *infos = (gpu_device_info_t*)calloc(total_count, sizeof(gpu_device_info_t));
//     if (!*infos) 
//     {
//         gpu_set_error("Failed to allocate memory for device infos");
//         return -1;
//     }

//     int current_index = 0;
// #ifdef GPU_ENABLE_OPENCL
//     gpu_device_info_t* opencl_infos = NULL;
//     int opencl_count = 0;
//     if (gpu_get_devices_by_backend(&opencl_infos, &opencl_count, GPU_BACKEND_OPENCL) == 0) 
//     {
//         memcpy(*infos + current_index, opencl_infos, opencl_count * sizeof(gpu_device_info_t));
//         current_index += opencl_count;
//         free(opencl_infos);
//     }
// #endif

// #ifdef GPU_ENABLE_CUDA
//     gpu_device_info_t* cuda_infos = NULL;
//     int cuda_count = 0;
//     if (gpu_get_devices_by_backend(&cuda_infos, &cuda_count, GPU_BACKEND_CUDA) == 0) 
//     {
//         memcpy(*infos + current_index, cuda_infos, cuda_count * sizeof(gpu_device_info_t));
//         current_index += cuda_count;
//         free(cuda_infos);
//     }
// #endif

//     *count = current_index;
//     return 0;
// }

// #ifdef GPU_ENABLE_OPENCL
// int gpu_get_devices_by_backend_opencl(gpu_device_info_t** infos, int* count) 
// {
//     cl_uint num_platforms = 0;
//     if (clGetPlatformIDs(0, NULL, &num_platforms) != CL_SUCCESS || num_platforms == 0) 
//     {
//         *infos = NULL;
//         *count = 0;
//         return 0;
//     }

//     cl_platform_id* platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * num_platforms);
//     if (!platforms) 
//     {
//         gpu_set_error("Failed to allocate memory for platforms");
//         return -1;
//     }
    
//     clGetPlatformIDs(num_platforms, platforms, NULL);
//     cl_uint total_devices = 0;
//     for (cl_uint i = 0; i < num_platforms; ++i) 
//     {
//         cl_uint num_devices_in_platform = 0;
//         clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices_in_platform);
//         total_devices += num_devices_in_platform;
//     }

//     if (total_devices == 0) 
//     {
//         free(platforms);
//         *infos = NULL;
//         *count = 0;
//         return 0;
//     }

//     *infos = (gpu_device_info_t*)calloc(total_devices, sizeof(gpu_device_info_t));
//     if (!*infos) 
//     {
//         free(platforms);
//         gpu_set_error("Failed to allocate memory for device infos");
//         return -1;
//     }

//     cl_uint device_index = 0;
//     for (cl_uint i = 0; i < num_platforms; ++i) 
//     {
//         cl_uint num_devices_in_platform = 0;
//         clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices_in_platform);
//         if (num_devices_in_platform > 0) 
//         {
//             cl_device_id* devices = (cl_device_id*)malloc(sizeof(cl_device_id) * num_devices_in_platform);
//             clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, num_devices_in_platform, devices, NULL);
//             for (cl_uint j = 0; j < num_devices_in_platform; ++j) 
//             {
//                 gpu_device_info_t* info = &(*infos)[device_index];
//                 info->id = malloc(sizeof(cl_device_id));
//                 memcpy(info->id, &devices[j], sizeof(cl_device_id));

//                 clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(info->name), info->name, NULL);
//                 clGetDeviceInfo(devices[j], CL_DEVICE_VENDOR, sizeof(info->vendor), info->vendor, NULL);
//                 clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, sizeof(info->version), info->version, NULL);
                
//                 cl_ulong global_mem_size;
//                 clGetDeviceInfo(devices[j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(global_mem_size), &global_mem_size, NULL);
//                 info->global_mem_sz = global_mem_size;
                
//                 cl_uint compute_units;
//                 clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
//                 info->compute_units = compute_units;
                
//                 size_t work_group_size;
//                 clGetDeviceInfo(devices[j], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(work_group_size), &work_group_size, NULL);
//                 info->work_group_sz = (unsigned int)work_group_size;
                
//                 info->backend = GPU_BACKEND_OPENCL;
//                 info->device_type = GPU_DEVICE_TYPE_GPU;
                
//                 device_index++;
//             }
//             free(devices);
//         }
//     }

//     free(platforms);
//     *count = (int)total_devices;
//     return 0;
// }
// #endif

// #ifdef GPU_ENABLE_CUDA
// int gpu_get_devices_by_backend_cuda(gpu_device_info_t** infos, int* count) 
// {
//     int device_count = 0;
//     if (cudaGetDeviceCount(&device_count) != cudaSuccess || device_count == 0) 
//     {
//         *infos = NULL;
//         *count = 0;
//         return 0;
//     }

//     *infos = (gpu_device_info_t*)calloc(device_count, sizeof(gpu_device_info_t));
//     if (!*infos) 
//     {
//         gpu_set_error("Failed to allocate memory for CUDA device infos");
//         return -1;
//     }

//     for (int i = 0; i < device_count; ++i) 
//     {
//         gpu_device_info_t* info = &(*infos)[i];
//         cudaDeviceProp prop;
        
//         if (cudaGetDeviceProperties(&prop, i) != cudaSuccess)
//             continue;

//         info->id = malloc(sizeof(int));
//         *((int*)info->id) = i;

//         strncpy(info->name, prop.name, sizeof(info->name) - 1);
//         strncpy(info->vendor, "NVIDIA", sizeof(info->vendor) - 1);
//         snprintf(info->version, sizeof(info->version), "Compute Capability %d.%d", prop.major, prop.minor);

//         info->global_mem_sz = prop.totalGlobalMem;
//         info->compute_units = prop.multiProcessorCount;
//         info->work_group_sz = prop.maxThreadsPerBlock;
//         info->backend = GPU_BACKEND_CUDA;
//         info->device_type = GPU_DEVICE_TYPE_GPU;
//     }

//     *count = device_count;
//     return 0;
// }
// #endif

// int gpu_get_devices_by_backend(gpu_device_info_t** infos, int* count, gpu_backend_t backend) 
// {
//     switch (backend) {
// #ifdef GPU_ENABLE_OPENCL
//         case GPU_BACKEND_OPENCL:
//             return gpu_get_devices_by_backend_opencl(infos, count);
// #endif

// #ifdef GPU_ENABLE_CUDA
//         case GPU_BACKEND_CUDA:
//             return gpu_get_devices_by_backend_cuda(infos, count);
// #endif

//         default:
//             gpu_set_error("Unsupported backend: %d", backend);
//             *infos = NULL;
//             *count = 0;
//             return -1;
//     }
// }

// void gpu_free_devices(gpu_device_info_t* infos) 
// {
//     if (infos) 
//         free(infos);
// }

// void gpu_foreach_device(gpu_device_callback_t callback, void* user_data) 
// {
//     if (!callback) return;

//     gpu_device_info_t* infos = NULL;
//     int count = 0;
    
//     if (gpu_get_devices(&infos, &count) == 0 && infos) 
//     {
//         for (int i = 0; i < count; ++i) 
//         {
//             if (!callback(&infos[i], user_data))
//                 break;
//         }
//         gpu_free_devices(infos);
//     }
// }

// bool gpu_get_best_device(gpu_device_info_t* info, gpu_backend_t preferred_backend) 
// {
//     if (!info) 
//     {
//         gpu_set_error("Invalid parameter: info is NULL");
//         return false;
//     }

//     gpu_device_info_t* infos = NULL;
//     int count = 0;
//     if (gpu_get_devices(&infos, &count) != 0 || !infos || count == 0) 
//         return false;

//     int best_index = -1;
//     unsigned long long best_score = 0;

//     for (int i = 0; i < count; ++i) 
//     {
//         if (preferred_backend != GPU_BACKEND_NONE && infos[i].backend != preferred_backend)
//             continue;

//         unsigned long long score = infos[i].global_mem_sz / (1024 * 1024) + infos[i].compute_units * 1000;
//         if (score > best_score) 
//         {
//             best_score = score;
//             best_index = i;
//         }
//     }

//     if (best_index >= 0) 
//     {
//         memcpy(info, &infos[best_index], sizeof(gpu_device_info_t));
//         if (infos[best_index].backend == GPU_BACKEND_OPENCL) 
//         {
//             info->id = malloc(sizeof(cl_device_id));
//             memcpy(info->id, infos[best_index].id, sizeof(cl_device_id));
//         } 
//         else if (infos[best_index].backend == GPU_BACKEND_CUDA) 
//         {
//             info->id = malloc(sizeof(int));
//             memcpy(info->id, infos[best_index].id, sizeof(int));
//         }

//         gpu_free_devices(infos);
//         return true;
//     }

//     gpu_free_devices(infos);
//     return false;
// }

// const char* gpu_get_last_error(void) 
// {
//     return g_last_error;
// }

// void gpu_clear_last_error(void) 
// {
//     g_last_error[0] = '\0';
// }

// void gpu_set_error_callback(gpu_error_callback_t callback, void* user_data) 
// {
//     g_error_callback = callback;
//     g_error_callback_user_data = user_data;
// }

// const char* gpu_backend_name(gpu_backend_t backend) 
// {
//     switch (backend) {
//         case GPU_BACKEND_OPENCL: return "OpenCL";
//         case GPU_BACKEND_CUDA: return "CUDA";
//         case GPU_BACKEND_NONE: return "None";
//         default: return "Unknown";
//     }
// }

// const char* gpu_device_type_name(gpu_device_type_t type) 
// {
//     switch (type) {
//         case GPU_DEVICE_TYPE_GPU: return "GPU";
//         case GPU_DEVICE_TYPE_CPU: return "CPU";
//         case GPU_DEVICE_TYPE_ALL: return "All";
//         default: return "Unknown";
//     }
// }

// bool gpu_create_default_context(gpu_context_t* ctx) 
// {
//     if (!ctx) return false;
    
//     gpu_device_info_t best_device;
//     if (!gpu_get_best_device(&best_device, GPU_BACKEND_NONE)) 
//     {
//         gpu_set_error("No suitable GPU device found");
//         return false;
//     }
    
//     return gpu_create_context(ctx, &best_device);
// }

// #ifdef __cplusplus
// }
// #endif

// #endif /* GPU_H */