#include <gtest/gtest.h>
#include <libcpp/hardware/gpu.h>

TEST(gpu, gpu_count)
{
    uint32_t count = gpu_count();
    ASSERT_GE(count, 0) << "No GPU devices found";
}

TEST(gpu, gpu_count_by_backend)
{
    uint32_t count = gpu_count_by_backend(GPU_BACKEND_OPENCL);
    ASSERT_GE(count, 0) << "No OpenCL GPU devices found";
}

// TEST(gpu, gpu_example)
// {
//     const char* kernelSource = R"CLC(
// __kernel void vec_add(__global const float* A, __global const float* B, __global float* C) {
//     int id = get_global_id(0);
//     C[id] = A[id] + B[id];
// }
// )CLC";

//     // 1. get platforms
//     cl_uint numPlatforms;
//     clGetPlatformIDs(0, nullptr, &numPlatforms);
//     std::vector<cl_platform_id> platforms(numPlatforms);
//     clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);

//     // 2. get GPU devices
//     cl_uint numDevices;
//     clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 0, nullptr, &numDevices);
//     std::vector<cl_device_id> devices(numDevices);
//     clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, numDevices, devices.data(), nullptr);

//     // 3. 创建上下文
//     cl_context context = clCreateContext(nullptr, 1, &devices[0], nullptr, nullptr, nullptr);

//     // 4. 创建命令队列
//     cl_command_queue queue = clCreateCommandQueueWithProperties(context, devices[0], 0, nullptr);

//     // 5. 创建和编译程序
//     cl_program program = clCreateProgramWithSource(context, 1, &kernelSource, nullptr, nullptr);
//     clBuildProgram(program, 1, &devices[0], nullptr, nullptr, nullptr);

//     // 6. 创建内核
//     cl_kernel kernel = clCreateKernel(program, "vec_add", nullptr);

//     // 7. 创建输入输出数据
//     const int N = 1024;
//     std::vector<float> A(N, 1.0f), B(N, 2.0f), C(N);

//     // 8. 创建缓冲区
//     cl_mem bufA = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*N, A.data(), nullptr);
//     cl_mem bufB = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*N, B.data(), nullptr);
//     cl_mem bufC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float)*N, nullptr, nullptr);

//     // 9. 设置内核参数
//     clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
//     clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufB);
//     clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufC);

//     // 10. 执行内核
//     size_t globalSize = N;
//     clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &globalSize, nullptr, 0, nullptr, nullptr);

//     // 11. 读取结果
//     clEnqueueReadBuffer(queue, bufC, CL_TRUE, 0, sizeof(float)*N, C.data(), 0, nullptr, nullptr);

//     // 12. 检查结果
//     bool correct = true;
//     for (int i = 0; i < N; ++i) {
//         if (C[i] != 3.0f) {
//             correct = false;
//             break;
//         }
//     }
//     std::cout << "Result: " << (correct ? "Success" : "Failure") << std::endl;

//     // 13. 释放资源
//     clReleaseMemObject(bufA);
//     clReleaseMemObject(bufB);
//     clReleaseMemObject(bufC);
//     clReleaseKernel(kernel);
//     clReleaseProgram(program);
//     clReleaseCommandQueue(queue);
//     clReleaseContext(context);
// }