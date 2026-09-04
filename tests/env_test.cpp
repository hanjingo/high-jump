#include <gtest/gtest.h>
#include <chrono>
#include <cstring>
#include <iostream>

#define HJ_ENV_IMPL
#include <hj/os/env.h>

class env : public ::testing::Test
{
  protected:
    bool IsValidValue(int64_t value) const { return value != -1; }
};

TEST_F(env, compile_time_macros)
{
    EXPECT_GE(HJ_COMPILE_YEAR, 2020);
    EXPECT_LE(HJ_COMPILE_YEAR, 9999);

    EXPECT_GE(HJ_COMPILE_MONTH, 1);
    EXPECT_LE(HJ_COMPILE_MONTH, 12);

    EXPECT_GE(HJ_COMPILE_DAY, 1);
    EXPECT_LE(HJ_COMPILE_DAY, 31);

    EXPECT_GE(HJ_COMPILE_HOUR, 0);
    EXPECT_LE(HJ_COMPILE_HOUR, 23);

    EXPECT_GE(HJ_COMPILE_MINUTE, 0);
    EXPECT_LE(HJ_COMPILE_MINUTE, 59);

    EXPECT_GE(HJ_COMPILE_SECOND, 0);
    EXPECT_LE(HJ_COMPILE_SECOND, 59);

    const char *compile_time = HJ_COMPILE_TIME;
    EXPECT_NE(compile_time, nullptr);
    EXPECT_EQ(std::strlen(compile_time), 19);

    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
    int parsed = sscanf(compile_time,
                        "%04d-%02d-%02d %02d:%02d:%02d",
                        &year,
                        &month,
                        &day,
                        &hour,
                        &minute,
                        &second);

    EXPECT_EQ(parsed, 6);
    EXPECT_EQ(year, HJ_COMPILE_YEAR);
    EXPECT_EQ(month, HJ_COMPILE_MONTH);
    EXPECT_EQ(day, HJ_COMPILE_DAY);
    EXPECT_EQ(hour, HJ_COMPILE_HOUR);
    EXPECT_EQ(minute, HJ_COMPILE_MINUTE);
    EXPECT_EQ(second, HJ_COMPILE_SECOND);
}

TEST_F(env, cpu_configuration)
{
    int64_t cpu_count = hj_env_get(HJ_CONF_CPU_COUNT);
    EXPECT_GT(cpu_count, 0);
    EXPECT_LE(cpu_count, 256);
    EXPECT_EQ(cpu_count, HJ_ENV_CPU_COUNT);
}

TEST_F(env, memory_configuration)
{
    int64_t page_size = hj_env_get(HJ_CONF_MEMORY_PAGE_SIZE);
    EXPECT_GT(page_size, 0);
    EXPECT_TRUE(page_size == 4096 || page_size == 8192 || page_size == 16384
                || page_size == 65536);
    EXPECT_EQ(page_size, HJ_ENV_MEMORY_PAGE_SIZE);

    int64_t virtual_mem_max = hj_env_get(HJ_CONF_VIRTUAL_MEMORY_MAX);
    if(IsValidValue(virtual_mem_max))
    {
        EXPECT_GT(virtual_mem_max, 0);
    }
}

TEST_F(env, process_configuration)
{
    int64_t uid = hj_env_get(HJ_CONF_USER_ID);
    if(IsValidValue(uid))
    {
        EXPECT_GE(uid, 0);
    }
    EXPECT_EQ(uid, HJ_ENV_USER_ID);

    int64_t gid = hj_env_get(HJ_CONF_PRIMARY_GROUP_ID);
    if(IsValidValue(gid))
    {
        EXPECT_GE(gid, 0);
    }
    EXPECT_EQ(gid, HJ_ENV_PRIMARY_GROUP_ID);
}

TEST_F(env, system_limits)
{
    int64_t clk_tck = hj_env_get(HJ_CONF_CLK_TCK);
    EXPECT_GT(clk_tck, 0);

    int64_t arg_max = hj_env_get(HJ_CONF_ARG_MAX);
    EXPECT_GT(arg_max, 1024);

    int64_t name_max = hj_env_get(HJ_CONF_NAME_MAX);
    if(IsValidValue(name_max))
    {
        EXPECT_GE(name_max, 14);
        EXPECT_LE(name_max, 4096);
    }

    int64_t path_max = hj_env_get(HJ_CONF_PATH_MAX);
    EXPECT_GT(path_max, 256);

    int64_t open_max = hj_env_get(HJ_CONF_OPEN_MAX);
    EXPECT_GT(open_max, 16);

    int64_t pipe_buf = hj_env_get(HJ_CONF_PIPE_BUF);
    if(IsValidValue(pipe_buf))
    {
        EXPECT_GT(pipe_buf, 128);
    }
}

TEST_F(env, resource_limits)
{
    int64_t file_size_max = hj_env_get(HJ_CONF_FILE_SIZE_MAX);
    if(IsValidValue(file_size_max))
    {
        EXPECT_GT(file_size_max, 1024 * 1024);
    }

    int64_t stack_size_max = hj_env_get(HJ_CONF_STACK_SIZE_MAX);
    if(IsValidValue(stack_size_max))
    {
        EXPECT_GT(stack_size_max, 1024);
    }

    int64_t heap_size_max = hj_env_get(HJ_CONF_HEAP_SIZE_MAX);
    if(IsValidValue(heap_size_max))
    {
        EXPECT_GT(heap_size_max, 1024 * 1024);
    }
}

TEST_F(env, ipc_limits)
{
    int64_t aio_max = hj_env_get(HJ_CONF_AIO_MAX);
    if(IsValidValue(aio_max))
    {
        EXPECT_GT(aio_max, 0);
    }

    int64_t mq_size_max = hj_env_get(HJ_CONF_MSG_QUEUE_SIZE_MAX);
    if(IsValidValue(mq_size_max))
    {
        EXPECT_GT(mq_size_max, 0);
    }

    int64_t msg_max = hj_env_get(HJ_CONF_MSG_MAX);
    if(IsValidValue(msg_max))
    {
        EXPECT_GT(msg_max, 0);
    }

    int64_t sem_nsems_max = hj_env_get(HJ_CONF_SEM_NSEMS_MAX);
    if(IsValidValue(sem_nsems_max))
    {
        EXPECT_GT(sem_nsems_max, 0);
    }

    int64_t sem_value_max = hj_env_get(HJ_CONF_SEM_VALUE_MAX);
    if(IsValidValue(sem_value_max))
    {
        EXPECT_GT(sem_value_max, 0);
    }
}

TEST_F(env, name_limits)
{
    int64_t hostname_max = hj_env_get(HJ_CONF_HOST_NAME_MAX);
    EXPECT_GT(hostname_max, 0);

    int64_t login_name_max = hj_env_get(HJ_CONF_LOGIN_NAME_MAX);
    EXPECT_GT(login_name_max, 0);

    int64_t tty_name_max = hj_env_get(HJ_CONF_TTY_NAME_MAX);
    if(IsValidValue(tty_name_max))
    {
        EXPECT_GT(tty_name_max, 0);
    }

    int64_t ngroups_max = hj_env_get(HJ_CONF_NGROUPS_MAX);
    if(IsValidValue(ngroups_max))
    {
        EXPECT_GT(ngroups_max, 0);
    }
}

TEST_F(env, invalid_configuration)
{
    int64_t invalid_result = hj_env_get(static_cast<hj_conf_t>(9999));
    EXPECT_EQ(invalid_result, -1);
}

TEST_F(env, consistency_check)
{
    int64_t cpu_count1 = hj_env_get(HJ_CONF_CPU_COUNT);
    int64_t cpu_count2 = hj_env_get(HJ_CONF_CPU_COUNT);
    EXPECT_EQ(cpu_count1, cpu_count2);

    int64_t page_size1 = hj_env_get(HJ_CONF_MEMORY_PAGE_SIZE);
    int64_t page_size2 = hj_env_get(HJ_CONF_MEMORY_PAGE_SIZE);
    EXPECT_EQ(page_size1, page_size2);
}

TEST_F(env, system_information_display)
{
    std::cout << "\n=== SYSTEM ENVIRONMENT SUMMARY ===" << std::endl;
    std::cout << "OS: " << HJ_OS << std::endl;
    std::cout << "Arch: " << HJ_ARCH << std::endl;
    std::cout << "Compile Time: " << HJ_COMPILE_TIME << std::endl;
    std::cout << "CPU Count: " << HJ_ENV_CPU_COUNT << std::endl;
    std::cout << "Page Size: " << HJ_ENV_MEMORY_PAGE_SIZE << " bytes"
              << std::endl;
    std::cout << "User ID: " << HJ_ENV_USER_ID << std::endl;
    std::cout << "Primary Group ID: " << HJ_ENV_PRIMARY_GROUP_ID << std::endl;
    std::cout << "Path Max: " << HJ_ENV_PATH_LEN_MAX << std::endl;
    std::cout << "Open Files Max: " << HJ_ENV_OPEN_FILES_MAX << std::endl;
    std::cout << "===================================" << std::endl;
}

TEST_F(env, initialization)
{
    hj_env_init();

    EXPECT_GT(HJ_ENV_CPU_COUNT, 0);
    EXPECT_GT(HJ_ENV_MEMORY_PAGE_SIZE, 0);
}

TEST_F(env, initialization_idempotency)
{
    hj_env_init();
    int64_t cpu1  = HJ_ENV_CPU_COUNT;
    int64_t page1 = HJ_ENV_MEMORY_PAGE_SIZE;

    hj_env_init();
    hj_env_init();

    int64_t cpu2  = HJ_ENV_CPU_COUNT;
    int64_t page2 = HJ_ENV_MEMORY_PAGE_SIZE;

    EXPECT_EQ(cpu1, cpu2);
    EXPECT_EQ(page1, page2);
}

TEST_F(env, multithread_tls_initialization)
{
    hj_env_init();
    int64_t main_cpu  = HJ_ENV_CPU_COUNT;
    int64_t main_page = HJ_ENV_MEMORY_PAGE_SIZE;

    int64_t thread_cpu  = 0;
    int64_t thread_page = 0;

    std::thread t([&]() {
        hj_env_init();
        thread_cpu  = HJ_ENV_CPU_COUNT;
        thread_page = HJ_ENV_MEMORY_PAGE_SIZE;
    });

    t.join();

    EXPECT_EQ(main_cpu, thread_cpu);
    EXPECT_EQ(main_page, thread_page);
    EXPECT_GT(thread_cpu, 0);
    EXPECT_GT(thread_page, 0);
}

TEST_F(env, enum_completeness_and_validity)
{
    for(int i = 0; i < HJ_CONF_MAX_COUNT; ++i)
    {
        const auto conf  = static_cast<hj_conf_t>(i);
        const auto value = hj_env_get(conf);

        EXPECT_TRUE(value >= 0 || value == -1) << "Failed at enum index: " << i;
    }
}

TEST_F(env, enum_out_of_bounds)
{
    EXPECT_EQ(hj_env_get(static_cast<hj_conf_t>(-1)), -1);

    EXPECT_EQ(hj_env_get(HJ_CONF_MAX_COUNT), -1);

    EXPECT_EQ(hj_env_get(static_cast<hj_conf_t>(9999)), -1);
}

TEST_F(env, os_macro_verification)
{
    EXPECT_NE(std::strlen(HJ_OS), 0);
    EXPECT_NE(std::string(HJ_OS), "unknown");

#if defined(_WIN32) || defined(_WIN64)
    EXPECT_STREQ(HJ_OS, "windows");
#elif defined(__APPLE__)
#if TARGET_OS_IPHONE
    EXPECT_STREQ(HJ_OS, "ios");
#else
    EXPECT_STREQ(HJ_OS, "macos");
#endif
#elif defined(__ANDROID__)
    EXPECT_STREQ(HJ_OS, "android");
#elif defined(__linux__)
    EXPECT_STREQ(HJ_OS, "linux");
#endif
}

TEST_F(env, arch_macro_verification)
{
    EXPECT_NE(std::strlen(HJ_ARCH), 0);
    EXPECT_NE(std::string(HJ_ARCH), "unknown");

#if defined(_M_IX86) || defined(__i386__)
    EXPECT_STREQ(HJ_ARCH, "x86");
#elif defined(_M_X64) || defined(__x86_64__)
    EXPECT_STREQ(HJ_ARCH, "x64");
#elif defined(_M_ARM64) || defined(__aarch64__)
    EXPECT_STREQ(HJ_ARCH, "arm64");
#elif defined(_M_ARM) || defined(__arm__)
    EXPECT_STREQ(HJ_ARCH, "arm");
#elif defined(__loongarch__)
    EXPECT_STREQ(HJ_ARCH, "loong64");
#endif
}