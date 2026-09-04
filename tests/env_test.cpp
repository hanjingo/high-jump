#include <gtest/gtest.h>
#include <chrono>
#include <cstring>
#include <iostream>

#define HJ_ENV_IMPL
#include <hj/os/env.h>

class env : public ::testing::Test
{
  protected:
    void SetUp() override {}
    void TearDown() override {}

    bool IsValidValue(int64_t value) { return value != -1; }
};

TEST_F(env, compile_time_macros)
{
    EXPECT_GE(HJ_COMPILE_YEAR, 2020);
    EXPECT_LE(HJ_COMPILE_YEAR, 2030);

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
    EXPECT_GT(strlen(compile_time), 0);
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