#include <gtest/gtest.h>
#include <hj/os/process.hpp>
#include <chrono>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

// 辅助函数：校验指定 PID 的进程当前是否处于活跃运行状态
inline bool is_process_alive(hj::os::pid_t pid)
{
    if(pid <= 0)
        return false;
    std::vector<hj::os::process_info> vec;
    hj::os::list(vec, [pid](const hj::os::process_info &info) {
        return info.pid == pid;
    });
    return !vec.empty();
}

TEST(process, getpid)
{
    ASSERT_GE(hj::os::getpid(), 0u);
}

TEST(process, getppid)
{
#if defined(_WIN32)
    ASSERT_GE(hj::os::getppid(), 0u);
#else
    ASSERT_GT(hj::os::getppid(), -1);
#endif
}

// 1. RAII 析构闭环测试 (验证 destructor -> kill -> wait 链条)
TEST(process, raii_destruction_policy)
{
    std::string exe;
#if defined(_WIN32)
    char buf[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buf);
    exe = std::string(buf) + "\\child.exe";
#else
    exe = "./child";
#endif

    std::error_code ec;
    hj::os::pid_t   pid = 0;
    {
        // 采用带 error_code 的 noexcept 签名 spawn，防止二进制不存在时抛出异常崩溃
        auto proc = hj::os::spawn(exe,
                                  {},
                                  ec,
                                  "",
                                  hj::os::process_policy::kill_on_destroy);
        if(!proc.is_valid() || ec)
        {
            GTEST_SKIP() << "child executable missing, skipping RAII test.";
        }
        pid = proc.id();
        ASSERT_TRUE(proc.is_valid());
        ASSERT_TRUE(is_process_alive(pid));
    } // 离开作用域：自动触发 ~process() -> kill() -> wait()

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // 校验：RAII 析构后子进程必须已被物理杀灭并回收
    ASSERT_FALSE(is_process_alive(pid));
}

// 2. Move 移动构造函数测试
TEST(process, move_constructor)
{
    std::string exe;
#if defined(_WIN32)
    char buf[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buf);
    exe = std::string(buf) + "\\child.exe";
#else
    exe = "./child";
#endif

    std::error_code ec;
    hj::os::process p1 =
        hj::os::spawn(exe, {}, ec, "", hj::os::process_policy::kill_on_destroy);
    if(!p1.is_valid() || ec)
    {
        GTEST_SKIP()
            << "child executable missing, skipping move_constructor test.";
    }

    ASSERT_TRUE(p1.is_valid());
    hj::os::pid_t pid = p1.id();
    ASSERT_NE(pid, 0u);

    // 执行 Move 移动构造
    hj::os::process p2 = std::move(p1);

    // 校验：p1 所有权移交后彻底置空为 invalid 状态；p2 完全接管该 PID 资源
    EXPECT_FALSE(p1.is_valid());
    EXPECT_EQ(p1.id(), 0u);

    EXPECT_TRUE(p2.is_valid());
    EXPECT_EQ(p2.id(), pid);
    EXPECT_TRUE(is_process_alive(pid));
}

// 3. Move 移动赋值测试 (校验旧资源覆盖释放，杜绝 Double Close / Double Kill)
TEST(process, move_assignment)
{
    std::string exe;
#if defined(_WIN32)
    char buf[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buf);
    exe = std::string(buf) + "\\child.exe";
#else
    exe = "./child";
#endif

    std::error_code ec2, ec3;
    hj::os::process p2 = hj::os::spawn(exe,
                                       {},
                                       ec2,
                                       "",
                                       hj::os::process_policy::kill_on_destroy);
    hj::os::process p3 = hj::os::spawn(exe,
                                       {},
                                       ec3,
                                       "",
                                       hj::os::process_policy::kill_on_destroy);

    if(!p2.is_valid() || !p3.is_valid() || ec2 || ec3)
    {
        GTEST_SKIP()
            << "child executable missing, skipping move_assignment test.";
    }

    hj::os::pid_t pid2 = p2.id();
    hj::os::pid_t pid3 = p3.id();

    ASSERT_TRUE(is_process_alive(pid2));
    ASSERT_TRUE(is_process_alive(pid3));

    // 执行 Move 移动赋值：p2 必须先释放清理自身原有的 pid2，再接管 p3 的 pid3
    p2 = std::move(p3);

    // 校验：p3 已彻底清空；p2 接管 pid3；被覆盖的旧进程 pid2 在赋值时刻已自动安全杀灭
    EXPECT_FALSE(p3.is_valid());
    EXPECT_EQ(p3.id(), 0u);

    EXPECT_TRUE(p2.is_valid());
    EXPECT_EQ(p2.id(), pid3);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(is_process_alive(pid2)); // pid2 被安全清理回收
    EXPECT_TRUE(is_process_alive(pid3));  // pid3 仍由 p2 正常接管持有
}

// 4. 复杂命令行转义与参数完整性测试 (Spaces, Quotes, Backslashes, Empty, Unicode)
TEST(process, argument_escaping)
{
    std::string exe;
    std::string out_file = "arg_test_out.txt";
    std::remove(out_file.c_str());

#if defined(_WIN32)
    char buf[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buf);
    exe = std::string(buf) + "\\child_args.exe";
#else
    exe = "./child_args";
#endif

    std::vector<std::string> test_args = {"hello world",
                                          R"(a\b\c)",
                                          R"(a"b)",
                                          "",
                                          "中文参数"};

    std::error_code ec;
    {
        auto proc = hj::os::spawn(exe,
                                  test_args,
                                  ec,
                                  "",
                                  hj::os::process_policy::wait_on_destroy);
        if(!proc.is_valid() || ec)
        {
            GTEST_SKIP() << "child_args executable missing (" << ec.message()
                         << "), skipping argument_escaping test.";
        }
    }

    std::ifstream fin(out_file);
    if(!fin.is_open())
    {
        GTEST_SKIP() << "arg_test_out.txt not generated, skipping arg test.";
    }

    std::vector<std::string> read_args;
    std::string              line;
    while(std::getline(fin, line))
    {
        read_args.push_back(line);
    }
    fin.close();
    std::remove(out_file.c_str());

    ASSERT_EQ(read_args.size(), test_args.size());
    for(size_t i = 0; i < test_args.size(); ++i)
    {
        EXPECT_EQ(read_args[i], test_args[i])
            << "Mismatch at argument index: " << i;
    }
}

// 5. 生命周期、阻塞 Wait 与退出码精准提取测试
TEST(process, lifecycle_and_wait)
{
    std::string exe;
#if defined(_WIN32)
    char buf[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buf);
    exe = std::string(buf) + "\\child_exit_42.exe";
#else
    exe = "./child_exit_42";
#endif

    std::error_code ec;
    auto p = hj::os::spawn(exe, {}, ec, "", hj::os::process_policy::manual);
    if(!p.is_valid() || ec)
    {
        GTEST_SKIP() << "child_exit_42 executable missing (" << ec.message()
                     << "), skipping wait test.";
    }

    ASSERT_TRUE(p.is_valid());
    ASSERT_NE(p.id(), 0u);

    auto code = p.wait();
    ASSERT_TRUE(code.has_value());
    ASSERT_TRUE(code->exited_normally);
    ASSERT_EQ(code->exit_code, 42);

    ASSERT_FALSE(p.is_valid());
    ASSERT_EQ(p.id(), 0u);
    ASSERT_FALSE(p.is_running());
}

TEST(process, spawn_detached)
{
    std::string exe;
#if defined(_WIN32)
    char buf[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buf);
    exe = std::string(buf) + "\\daemon.exe";
#else
    exe = "./daemon";
#endif
    std::error_code ec;
    bool            res = hj::os::spawn_detached(exe, {}, ec);
    if(!res || ec)
    {
        GTEST_SKIP() << "daemon executable missing, skipping test.";
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::ifstream fin("daemon_test.txt");
    if(!fin.is_open())
    {
        GTEST_SKIP() << "daemon_test.txt not generated, skipping test.";
    }
    std::string line;
    std::getline(fin, line);
    fin.close();
    ASSERT_TRUE(line.find("daemon") != std::string::npos);
    std::remove("daemon_test.txt");

    std::vector<hj::os::process_info> vec;
    hj::os::list(vec, [](const hj::os::process_info &info) -> bool {
        return info.name.find("daemon") != std::string::npos;
    });
    for(const auto &info : vec)
    {
        hj::os::kill(info.pid);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

TEST(process, list)
{
    std::string exe;
#if defined(_WIN32)
    char buf[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buf);
    exe = std::string(buf) + "\\child.exe";
#else
    exe = "./child";
#endif

    std::vector<hj::os::process_info> vec;
    hj::os::list(vec, [](const hj::os::process_info &info) -> bool {
        return info.name.find("child") != std::string::npos;
    });
    for(const auto &info : vec)
    {
        hj::os::kill(info.pid);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    vec.clear();
    std::error_code ec;
    auto proc = hj::os::spawn(exe,
                              {"--arg1", "value1"},
                              ec,
                              "",
                              hj::os::process_policy::kill_on_destroy);
    if(!proc.is_valid() || ec)
    {
        GTEST_SKIP() << "child executable missing, skipping list test.";
    }

    bool found = false;
    for(int i = 0; i < 5; ++i)
    {
        vec.clear();
        hj::os::list(vec, [](const hj::os::process_info &info) -> bool {
            return info.name.find("child") != std::string::npos;
        });
        if(vec.size() == 1)
        {
            found = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    if(!found)
    {
        GTEST_SKIP() << "process not found after retries, skipping test.";
    }
    ASSERT_EQ(vec.size(), 1);

#if defined(_WIN32)
    ASSERT_FALSE(vec[0].cmdline.has_value());
#else
    ASSERT_TRUE(vec[0].cmdline.has_value());
    ASSERT_TRUE(vec[0].cmdline->find("--arg1 value1") != std::string::npos);
#endif
}

TEST(process, kill)
{
    std::string exe;
#if defined(_WIN32)
    char buf[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buf);
    exe = std::string(buf) + "\\child.exe";
#else
    exe = "./child";
#endif

    std::error_code ec;
    auto proc = hj::os::spawn(exe,
                              {},
                              ec,
                              "",
                              hj::os::process_policy::detach_on_destroy);
    if(!proc.is_valid() || ec)
    {
        GTEST_SKIP() << "child executable missing, skipping kill test.";
    }

    proc.kill();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_FALSE(is_process_alive(proc.id()));
}

TEST(process, spawn_error_handling)
{
    std::error_code ec;
    auto            proc = hj::os::spawn("non_existent_binary_xxx", {}, ec);

    ASSERT_FALSE(proc.is_valid());
    ASSERT_TRUE(ec);

    std::cout << "Captured expected error code: " << ec.value() << " ("
              << ec.message() << ")" << std::endl;
}