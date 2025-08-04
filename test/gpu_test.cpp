#include <iostream>

#include <gtest/gtest.h>
#include <libcpp/hardware/gpu.h>

bool device_foreach_cb (const gpu_device_info_t *info, void *user_data)
{
    std::cout << "GPU name=" << info->name << ", vendor=" << info->vendor
              << ", version=" << info->version
              << ", global_mem_sz=" << info->global_mem_sz
              << ", compute_units=" << info->compute_units
              << ", work_group_sz=" << info->work_group_sz << std::endl;
    return true;
}

TEST (gpu, gpu_count)
{
    uint32_t count = gpu_count ();
    ASSERT_GE (count, 0) << "No GPU devices found";
}

TEST (gpu, gpu_device_get)
{
    if (gpu_count () == 0)
        return;

    gpu_device_info_t *infos = NULL;
    int count = 0;
    gpu_device_get (&infos, &count);
    ASSERT_NE (infos, nullptr) << "Failed to retrieve GPU device information";
    ASSERT_GT (count, 0);
}

TEST (gpu, gpu_device_foreach)
{
    gpu_device_foreach (device_foreach_cb, nullptr);
}

TEST (gpu, gpu_device_free)
{
    gpu_device_info_t *infos = NULL;
    int count = 0;
    ASSERT_TRUE (gpu_device_free (infos, count) != 0);

    gpu_device_get (&infos, &count);
    if (infos != nullptr && count > 0)
        ASSERT_TRUE (gpu_device_free (infos, count) == 0);
}

TEST (gpu, gpu_buffer_free)
{
    gpu_buffer_t buffer;
    memset (&buffer, 0, sizeof (gpu_buffer_t));
    ASSERT_TRUE (gpu_buffer_free (&buffer) != 0);
}

TEST (gpu, gpu_context_free)
{
}

TEST (gpu, gpu_program_free)
{
}

TEST (gpu, gpu_context_create)
{
    if (gpu_count () == 0)
        return;

    gpu_context_t ctx;
    ASSERT_TRUE (gpu_context_create (&ctx, NULL) != 0);

    gpu_device_info_t *devices = NULL;
    int device_count = 0;
    if (gpu_device_get (&devices, &device_count) == 0 && device_count > 0) {
        gpu_context_t ctx;
        if (gpu_context_create (&ctx, &devices[0]))
            gpu_context_free (&ctx);

        gpu_device_free (devices, device_count);
    }
}

TEST (gpu, gpu_program_create_from_source)
{
    if (gpu_count () == 0) {
        GTEST_SKIP () << "No GPU devices available, skipping test";
        return;
    }

    // OpenCL kernel
    const char *opencl_source = R"CLC(
__kernel void vector_add(__global const float* a, __global const float* b, __global float* c) {
    int id = get_global_id(0);
    c[id] = a[id] + b[id];
}
)CLC";

    // CUDA PTX
    const char *cuda_ptx_source = R"PTX(
.version 6.4
.target sm_50
.address_size 64

.visible .entry vector_add(
    .param .u64 vector_add_param_0,
    .param .u64 vector_add_param_1,
    .param .u64 vector_add_param_2
)
{
    .reg .u32   %r<4>;
    .reg .u64   %rd<10>;
    .reg .f32   %f<4>;

    ld.param.u64    %rd1, [vector_add_param_0];
    ld.param.u64    %rd2, [vector_add_param_1];
    ld.param.u64    %rd3, [vector_add_param_2];
    
    mov.u32         %r1, %tid.x;
    mul.wide.u32    %rd4, %r1, 4;
    add.u64         %rd5, %rd1, %rd4;
    add.u64         %rd6, %rd2, %rd4;
    add.u64         %rd7, %rd3, %rd4;
    
    ld.global.f32   %f1, [%rd5];
    ld.global.f32   %f2, [%rd6];
    add.f32         %f3, %f1, %f2;
    st.global.f32   [%rd7], %f3;
    
    ret;
}
)PTX";

    gpu_context_t ctx;
    ASSERT_TRUE (gpu_context_create (&ctx, nullptr))
      << "Failed to create GPU context";

    char log[2048];
    unsigned long log_sz = 2048;
    gpu_program_t program;
    ASSERT_TRUE (gpu_program_create_from_source (&program, &ctx, log, &log_sz,
                                                 opencl_source));
    if (log_sz > 0)
        std::cout << "GPU log: " << std::string (log, log_sz) << std::endl;

    bool kernel_created = gpu_kernel_create (&program, "vector_add");
    if (kernel_created) {
        std::cout << "Successfully created kernel 'vector_add'" << std::endl;
    } else {
        std::cout << "Failed to create kernel 'vector_add'" << std::endl;
    }
    gpu_program_free (&program);

    char log1[2048];
    unsigned long log1_sz = 2048;
    ASSERT_FALSE (gpu_program_create_from_source (nullptr, &ctx, log1, &log1_sz,
                                                  opencl_source))
      << "Should fail with null program pointer";

    char log2[2048];
    unsigned long log2_sz = 2048;
    ASSERT_FALSE (gpu_program_create_from_source (&program, nullptr, log2,
                                                  &log2_sz, opencl_source))
      << "Should fail with null context pointer";

    unsigned long log3_sz = 2048;
    ASSERT_TRUE (gpu_program_create_from_source (&program, &ctx, nullptr,
                                                 &log3_sz, opencl_source))
      << "Should not fail with null log pointer";

    const char *invalid_source = "this is not valid GPU code";
    gpu_program_t invalid_program;
    char log4[2048];
    unsigned long log4_sz = 2048;
    ASSERT_FALSE (gpu_program_create_from_source (&invalid_program, &ctx, log4,
                                                  &log4_sz, invalid_source))
      << "Should fail with invalid source code";

    gpu_context_free (&ctx);
}

TEST (gpu, gpu_malloc)
{
    if (gpu_count () == 0) {
        GTEST_SKIP () << "No GPU devices available, skipping test";
        return;
    }

    gpu_context_t ctx;
    ASSERT_TRUE (gpu_context_create (&ctx, nullptr))
      << "Failed to create GPU context";

    gpu_buffer_t buffer;
    memset (&buffer, 0, sizeof (gpu_buffer_t));

    size_t size = 1024 * 1024; // 1 MB
    ASSERT_TRUE (gpu_malloc (&buffer, &ctx, size))
      << "Failed to allocate GPU memory";

    ASSERT_NE (buffer.ptr, nullptr) << "Buffer pointer should not be null";
    ASSERT_EQ (buffer.size, size) << "Buffer size should match requested size";

    gpu_buffer_free (&buffer);
    gpu_context_free (&ctx);
}

TEST (gpu, gpu_malloc_read_only)
{
    if (gpu_count () == 0) {
        GTEST_SKIP () << "No GPU devices available, skipping test";
        return;
    }

    gpu_context_t ctx;
    ASSERT_TRUE (gpu_context_create (&ctx, nullptr))
      << "Failed to create GPU context";

    gpu_buffer_t buffer;
    memset (&buffer, 0, sizeof (gpu_buffer_t));

    size_t size = 1024 * 1024; // 1 MB
    ASSERT_TRUE (gpu_malloc_read_only (&buffer, &ctx, size))
      << "Failed to allocate GPU memory";

    ASSERT_NE (buffer.ptr, nullptr) << "Buffer pointer should not be null";
    ASSERT_EQ (buffer.size, size) << "Buffer size should match requested size";

    gpu_buffer_free (&buffer);
    gpu_context_free (&ctx);
}

TEST (gpu, gpu_malloc_write_only)
{
    if (gpu_count () == 0) {
        GTEST_SKIP () << "No GPU devices available, skipping test";
        return;
    }

    gpu_context_t ctx;
    ASSERT_TRUE (gpu_context_create (&ctx, nullptr))
      << "Failed to create GPU context";

    gpu_buffer_t buffer;
    memset (&buffer, 0, sizeof (gpu_buffer_t));

    size_t size = 1024 * 1024; // 1 MB
    ASSERT_TRUE (gpu_malloc_write_only (&buffer, &ctx, size))
      << "Failed to allocate GPU memory";

    ASSERT_NE (buffer.ptr, nullptr) << "Buffer pointer should not be null";
    ASSERT_EQ (buffer.size, size) << "Buffer size should match requested size";

    gpu_buffer_free (&buffer);
    gpu_context_free (&ctx);
}

TEST (gpu, gpu_malloc_pinned)
{
    if (!is_cuda_available ())
        return;

    gpu_buffer_t buffer;
    memset (&buffer, 0, sizeof (gpu_buffer_t));

    size_t size = 1024 * 1024; // 1 MB
    ASSERT_TRUE (gpu_malloc_pinned (&buffer, size))
      << "Failed to allocate pinned GPU memory";

    ASSERT_NE (buffer.ptr, nullptr) << "Buffer pointer should not be null";
    ASSERT_EQ (buffer.size, size) << "Buffer size should match requested size";

    gpu_buffer_free (&buffer);
}

TEST (gpu, gpu_malloc_unified)
{
    if (!is_cuda_available ())
        return;

    gpu_buffer_t buffer;
    memset (&buffer, 0, sizeof (gpu_buffer_t));

    size_t size = 1024 * 1024; // 1 MB
    ASSERT_TRUE (gpu_malloc_unified (&buffer, size))
      << "Failed to allocate unified GPU memory";

    ASSERT_NE (buffer.ptr, nullptr) << "Buffer pointer should not be null";
    ASSERT_EQ (buffer.size, size) << "Buffer size should match requested size";

    gpu_buffer_free (&buffer);
}

TEST (gpu, gpu_memcpy_to_device)
{
    if (gpu_count () == 0) {
        GTEST_SKIP () << "No GPU devices available, skipping test";
        return;
    }

    gpu_context_t ctx;
    ASSERT_TRUE (gpu_context_create (&ctx, nullptr))
      << "Failed to create GPU context";

    gpu_buffer_t dst_buffer;
    ASSERT_TRUE (gpu_malloc (&dst_buffer, &ctx, 1024))
      << "Failed to allocate destination buffer";

    int data[256];
    for (int i = 0; i < 256; ++i)
        data[i] = i;

    ASSERT_TRUE (gpu_memcpy_to_device (&dst_buffer, data, sizeof (data), &ctx))
      << "Failed to copy data to device";

    gpu_buffer_free (&dst_buffer);
    gpu_context_free (&ctx);
}

TEST (gpu, gpu_memcpy_from_device)
{
    if (gpu_count () == 0) {
        GTEST_SKIP () << "No GPU devices available, skipping test";
        return;
    }

    gpu_context_t ctx;
    ASSERT_TRUE (gpu_context_create (&ctx, nullptr))
      << "Failed to create GPU context";

    gpu_buffer_t src_buffer;
    ASSERT_TRUE (gpu_malloc (&src_buffer, &ctx, 1024))
      << "Failed to allocate source buffer";

    int data[256];
    for (int i = 0; i < 256; ++i)
        data[i] = i;

    ASSERT_TRUE (gpu_memcpy_to_device (&src_buffer, data, sizeof (data), &ctx))
      << "Failed to copy data to device";

    int host_data[256] = {0};
    ASSERT_TRUE (
      gpu_memcpy_from_device (host_data, &src_buffer, sizeof (host_data), &ctx))
      << "Failed to copy data from device";

    for (int i = 0; i < 256; ++i) {
        ASSERT_EQ (host_data[i], data[i]) << "Data mismatch at index " << i;
    }

    gpu_buffer_free (&src_buffer);
    gpu_context_free (&ctx);
}

TEST (gpu, gpu_memcpy_device_to_device)
{
    if (gpu_count () == 0) {
        GTEST_SKIP () << "No GPU devices available, skipping test";
        return;
    }

    gpu_context_t ctx;
    ASSERT_TRUE (gpu_context_create (&ctx, nullptr))
      << "Failed to create GPU context";

    gpu_buffer_t src_buffer;
    ASSERT_TRUE (gpu_malloc (&src_buffer, &ctx, 1024))
      << "Failed to allocate source buffer";

    int data[256];
    for (int i = 0; i < 256; ++i)
        data[i] = i;

    ASSERT_TRUE (gpu_memcpy_to_device (&src_buffer, data, sizeof (data), &ctx))
      << "Failed to copy data to device";

    gpu_buffer_t dst_buffer;
    ASSERT_TRUE (gpu_malloc (&dst_buffer, &ctx, 1024))
      << "Failed to allocate destination buffer";

    ASSERT_TRUE (gpu_memcpy_device_to_device (&dst_buffer, &src_buffer,
                                              sizeof (data), &ctx))
      << "Failed to copy data from one device buffer to another";

    int host_data[256] = {0};
    ASSERT_TRUE (
      gpu_memcpy_from_device (host_data, &dst_buffer, sizeof (host_data), &ctx))
      << "Failed to copy data from destination device buffer";

    for (int i = 0; i < 256; ++i) {
        ASSERT_EQ (host_data[i], data[i]) << "Data mismatch at index " << i;
    }

    gpu_buffer_free (&src_buffer);
    gpu_buffer_free (&dst_buffer);
    gpu_context_free (&ctx);
}

TEST (gpu, gpu_memcpy_to_device_async)
{
    if (gpu_count () == 0) {
        GTEST_SKIP () << "No GPU devices available, skipping test";
        return;
    }

    gpu_context_t ctx;
    ASSERT_TRUE (gpu_context_create (&ctx, nullptr))
      << "Failed to create GPU context";

    gpu_buffer_t dst_buffer;
    ASSERT_TRUE (gpu_malloc (&dst_buffer, &ctx, 1024))
      << "Failed to allocate destination buffer";

    int data[256];
    for (int i = 0; i < 256; ++i)
        data[i] = i;

    ASSERT_TRUE (
      gpu_memcpy_to_device_async (&dst_buffer, data, sizeof (data), &ctx))
      << "Failed to copy data to device";

    gpu_buffer_free (&dst_buffer);
    gpu_context_free (&ctx);
}

TEST (gpu, gpu_memcpy_from_device_async)
{
    if (gpu_count () == 0) {
        GTEST_SKIP () << "No GPU devices available, skipping test";
        return;
    }

    gpu_context_t ctx;
    ASSERT_TRUE (gpu_context_create (&ctx, nullptr))
      << "Failed to create GPU context";

    gpu_buffer_t src_buffer;
    ASSERT_TRUE (gpu_malloc (&src_buffer, &ctx, 1024))
      << "Failed to allocate source buffer";

    int data[256];
    for (int i = 0; i < 256; ++i)
        data[i] = i;

    ASSERT_TRUE (gpu_memcpy_to_device (&src_buffer, data, sizeof (data), &ctx))
      << "Failed to copy data to device";

    int host_data[256] = {0};
    ASSERT_TRUE (gpu_memcpy_from_device_async (host_data, &src_buffer,
                                               sizeof (host_data), &ctx))
      << "Failed to copy data from device";

    for (int i = 0; i < 256; ++i) {
        ASSERT_EQ (host_data[i], data[i]) << "Data mismatch at index " << i;
    }

    gpu_buffer_free (&src_buffer);
    gpu_context_free (&ctx);
}

TEST (gpu, gpu_memcpy_device_to_device_async)
{
    if (gpu_count () == 0) {
        GTEST_SKIP () << "No GPU devices available, skipping test";
        return;
    }

    gpu_context_t ctx;
    ASSERT_TRUE (gpu_context_create (&ctx, nullptr))
      << "Failed to create GPU context";

    gpu_buffer_t src_buffer;
    ASSERT_TRUE (gpu_malloc (&src_buffer, &ctx, 1024))
      << "Failed to allocate source buffer";

    int data[256];
    for (int i = 0; i < 256; ++i)
        data[i] = i;

    ASSERT_TRUE (gpu_memcpy_to_device (&src_buffer, data, sizeof (data), &ctx))
      << "Failed to copy data to device";

    gpu_buffer_t dst_buffer;
    ASSERT_TRUE (gpu_malloc (&dst_buffer, &ctx, 1024))
      << "Failed to allocate destination buffer";

    ASSERT_TRUE (gpu_memcpy_device_to_device_async (&dst_buffer, &src_buffer,
                                                    sizeof (data), &ctx))
      << "Failed to copy data from one device buffer to another";

    int host_data[256] = {0};
    ASSERT_TRUE (gpu_memcpy_from_device_async (host_data, &dst_buffer,
                                               sizeof (host_data), &ctx))
      << "Failed to copy data from destination device buffer";

    for (int i = 0; i < 256; ++i) {
        ASSERT_EQ (host_data[i], data[i]) << "Data mismatch at index " << i;
    }

    gpu_buffer_free (&src_buffer);
    gpu_buffer_free (&dst_buffer);
    gpu_context_free (&ctx);
}

TEST (gpu, gpu_set_kernel_arg)
{
    if (gpu_count () == 0) {
        GTEST_SKIP () << "No GPU devices available";
        return;
    }

    const char *source = "__kernel void test(const int value) { }";

    gpu_context_t ctx;
    if (!gpu_context_create (&ctx, nullptr)) {
        GTEST_SKIP () << "Failed to create GPU context";
        return;
    }

    gpu_program_t program;
    char log[1024];
    unsigned long log_size = sizeof (log);

    if (gpu_program_create_from_source (&program, &ctx, log, &log_size, source)
        && gpu_kernel_create (&program, "test")) {
        int value = 42;
        EXPECT_TRUE (gpu_set_kernel_arg (&program, 0, sizeof (int), &value));

        EXPECT_FALSE (gpu_set_kernel_arg (nullptr, 0, sizeof (int), &value));
        EXPECT_FALSE (gpu_set_kernel_arg (&program, -1, sizeof (int), &value));

        gpu_program_free (&program);
    }

    gpu_context_free (&ctx);
}

TEST (gpu, gpu_set_kernel_arg_buffer)
{
    if (gpu_count () == 0) {
        GTEST_SKIP () << "No GPU devices available";
        return;
    }

    const char *source = "__kernel void test(__global float* data) { }";

    gpu_context_t ctx;
    if (!gpu_context_create (&ctx, nullptr)) {
        GTEST_SKIP () << "Failed to create GPU context";
        return;
    }

    gpu_program_t program;
    char log[1024];
    unsigned long log_size = sizeof (log);

    if (gpu_program_create_from_source (&program, &ctx, log, &log_size, source)
        && gpu_kernel_create (&program, "test")) {
        gpu_buffer_t buffer;
        if (gpu_malloc (&buffer, &ctx, 1024)) {
            EXPECT_TRUE (gpu_set_kernel_arg_buffer (&program, 0, &buffer));

            EXPECT_FALSE (gpu_set_kernel_arg_buffer (nullptr, 0, &buffer));
            EXPECT_FALSE (gpu_set_kernel_arg_buffer (&program, -1, &buffer));
            EXPECT_FALSE (gpu_set_kernel_arg_buffer (&program, 0, nullptr));

            gpu_buffer_free (&buffer);
        }

        gpu_program_free (&program);
    }

    gpu_context_free (&ctx);
}

TEST (gpu, gpu_execute_kernel_1d)
{
    if (gpu_count () == 0) {
        GTEST_SKIP () << "No GPU devices available";
        return;
    }

    const char *source = "__kernel void test(const int value) { }";

    gpu_context_t ctx;
    if (!gpu_context_create (&ctx, nullptr)) {
        GTEST_SKIP () << "Failed to create GPU context";
        return;
    }

    gpu_program_t program;
    char log[1024];
    unsigned long log_size = sizeof (log);

    if (gpu_program_create_from_source (&program, &ctx, log, &log_size, source)
        && gpu_kernel_create (&program, "test")) {
        int value = 42;
        EXPECT_TRUE (gpu_set_kernel_arg (&program, 0, sizeof (int), &value));

        size_t global_work_size = 1;
        size_t local_work_size = 1;
        EXPECT_TRUE (gpu_execute_kernel_1d (&program, &ctx, global_work_size,
                                            local_work_size));

        gpu_program_free (&program);
    }

    gpu_context_free (&ctx);
}

TEST (gpu, gpu_execute_kernel_2d)
{
    if (gpu_count () == 0) {
        GTEST_SKIP () << "No GPU devices available";
        return;
    }

    const char *source = "__kernel void test(const int value) { }";

    gpu_context_t ctx;
    if (!gpu_context_create (&ctx, nullptr)) {
        GTEST_SKIP () << "Failed to create GPU context";
        return;
    }

    gpu_program_t program;
    char log[1024];
    unsigned long log_size = sizeof (log);

    if (gpu_program_create_from_source (&program, &ctx, log, &log_size, source)
        && gpu_kernel_create (&program, "test")) {
        int value = 42;
        EXPECT_TRUE (gpu_set_kernel_arg (&program, 0, sizeof (int), &value));

        size_t global_work_size[2] = {1, 1};
        size_t local_work_size[2] = {1, 1};
        EXPECT_TRUE (gpu_execute_kernel_2d (&program, &ctx, global_work_size,
                                            local_work_size));

        gpu_program_free (&program);
    }

    gpu_context_free (&ctx);
}

TEST (gpu, gpu_execute_kernel_3d)
{
    if (gpu_count () == 0) {
        GTEST_SKIP () << "No GPU devices available";
        return;
    }

    const char *source = "__kernel void test(const int value) { }";

    gpu_context_t ctx;
    if (!gpu_context_create (&ctx, nullptr)) {
        GTEST_SKIP () << "Failed to create GPU context";
        return;
    }

    gpu_program_t program;
    char log[1024];
    unsigned long log_size = sizeof (log);

    if (gpu_program_create_from_source (&program, &ctx, log, &log_size, source)
        && gpu_kernel_create (&program, "test")) {
        int value = 42;
        EXPECT_TRUE (gpu_set_kernel_arg (&program, 0, sizeof (int), &value));

        size_t global_work_size[3] = {1, 1, 1};
        size_t local_work_size[3] = {1, 1, 1};
        EXPECT_TRUE (gpu_execute_kernel_3d (&program, &ctx, global_work_size,
                                            local_work_size));

        gpu_program_free (&program);
    }

    gpu_context_free (&ctx);
}

// TEST(gpu, gpu_opencl_example)
// {
//     const char* kernelSource = R"CLC(
// __kernel void vec_add(__global const float* A, __global const float* B,
// __global float* C) {
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
//     clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 0, nullptr,
//     &numDevices); std::vector<cl_device_id> devices(numDevices);
//     clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, numDevices,
//     devices.data(), nullptr);

//     // 3. 创建上下文
//     cl_context context = clCreateContext(nullptr, 1, &devices[0], nullptr,
//     nullptr, nullptr);

//     // 4. 创建命令队列
//     cl_command_queue queue = clCreateCommandQueueWithProperties(context,
//     devices[0], 0, nullptr);

//     // 5. 创建和编译程序
//     cl_program program = clCreateProgramWithSource(context, 1, &kernelSource,
//     nullptr, nullptr); clBuildProgram(program, 1, &devices[0], nullptr,
//     nullptr, nullptr);

//     // 6. 创建内核
//     cl_kernel kernel = clCreateKernel(program, "vec_add", nullptr);

//     // 7. 创建输入输出数据
//     const int N = 1024;
//     std::vector<float> A(N, 1.0f), B(N, 2.0f), C(N);

//     // 8. 创建缓冲区
//     cl_mem bufA = clCreateBuffer(context, CL_MEM_READ_ONLY |
//     CL_MEM_COPY_HOST_PTR, sizeof(float)*N, A.data(), nullptr); cl_mem bufB =
//     clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
//     sizeof(float)*N, B.data(), nullptr); cl_mem bufC =
//     clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float)*N, nullptr,
//     nullptr);

//     // 9. 设置内核参数
//     clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
//     clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufB);
//     clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufC);

//     // 10. 执行内核
//     size_t globalSize = N;
//     clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &globalSize, nullptr,
//     0, nullptr, nullptr);

//     // 11. 读取结果
//     clEnqueueReadBuffer(queue, bufC, CL_TRUE, 0, sizeof(float)*N, C.data(),
//     0, nullptr, nullptr);

//     // 12. 检查结果
//     bool correct = true;
//     for (int i = 0; i < N; ++i) {
//         if (C[i] != 3.0f) {
//             correct = false;
//             break;
//         }
//     }
//     std::cout << "Result: " << (correct ? "Success" : "Failure") <<
//     std::endl;

//     // 13. 释放资源
//     clReleaseMemObject(bufA);
//     clReleaseMemObject(bufB);
//     clReleaseMemObject(bufC);
//     clReleaseKernel(kernel);
//     clReleaseProgram(program);
//     clReleaseCommandQueue(queue);
//     clReleaseContext(context);
// }