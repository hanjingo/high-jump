#include <gtest/gtest.h>
#include <hj/os/process.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <string>
#include <thread>
#include <vector>

#if !defined(_WIN32)
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

inline std::string get_child_helper_path()
{
#if defined(_WIN32)
    char buf[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buf);
    return std::string(buf) + "\\child_helpers.exe";
#else
    return fs::absolute("./child_helpers").string();
#endif
}

inline bool is_process_alive(hj::os::pid_t pid)
{
    return hj::os::is_alive(pid);
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

TEST(process, terminate_graceful)
{
    std::string     exe = get_child_helper_path();
    std::error_code ec;

    hj::os::process::options opts;
    opts.command           = exe;
    opts.args              = {"--sleep"};
    opts.working_directory = "";
    opts.policy            = hj::os::process_policy::manual;
    auto proc              = hj::os::spawn(opts, ec);

    ASSERT_TRUE(proc.is_valid()) << "child_helpers missing: " << ec.message();
    ASSERT_FALSE(ec);

    hj::os::pid_t pid = proc.id();
    ASSERT_TRUE(is_process_alive(pid));

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    bool term_req = proc.terminate();
    EXPECT_TRUE(term_req);

    auto status = proc.wait();
    ASSERT_TRUE(status.has_value());

    EXPECT_TRUE(status->exited_normally || status->signaled);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(is_process_alive(pid));
}

TEST(process, kill_force)
{
    std::string              exe = get_child_helper_path();
    std::error_code          ec;
    hj::os::process::options opts;
    opts.command           = exe;
    opts.args              = {"--sleep"};
    opts.working_directory = "";
    opts.policy            = hj::os::process_policy::manual;
    auto proc              = hj::os::spawn(opts, ec);

    ASSERT_TRUE(proc.is_valid()) << "child_helpers missing: " << ec.message();
    ASSERT_FALSE(ec);

    hj::os::pid_t pid = proc.id();
    ASSERT_TRUE(is_process_alive(pid));

    proc.kill();
    auto status = proc.wait();
    ASSERT_TRUE(status.has_value());

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(is_process_alive(pid));
}

TEST(process, signal_exit_status)
{
    std::string              exe = get_child_helper_path();
    std::error_code          ec;
    hj::os::process::options opts;
    opts.command           = exe;
    opts.args              = {"--raise-sigterm"};
    opts.working_directory = "";
    opts.policy            = hj::os::process_policy::manual;
    auto proc              = hj::os::spawn(opts, ec);

    ASSERT_TRUE(proc.is_valid()) << "child_helpers missing: " << ec.message();

    auto status = proc.wait();
    ASSERT_TRUE(status.has_value());

#if !defined(_WIN32)
    EXPECT_TRUE(status->signaled);
    EXPECT_EQ(status->termsig, SIGTERM);
    EXPECT_EQ(status->code(), 128 + SIGTERM);
#else
    EXPECT_TRUE(status->exited_normally);
    EXPECT_EQ(status->exit_code, 128 + 15);
#endif
}

TEST(process, working_directory)
{
    std::string exe      = get_child_helper_path();
    std::string out_file = "cwd_out.txt";
    std::remove(out_file.c_str());

    fs::path    target_dir     = fs::temp_directory_path();
    std::string target_dir_str = target_dir.string();

    std::error_code          ec;
    hj::os::process::options opts;
    opts.command           = exe;
    opts.args              = {"--print-cwd"};
    opts.working_directory = target_dir_str;
    opts.policy            = hj::os::process_policy::wait_on_destroy;
    auto proc              = hj::os::spawn(opts, ec);

    ASSERT_TRUE(proc.is_valid()) << "child_helpers missing: " << ec.message();
    proc.wait();

    fs::path      expected_out_path = target_dir / out_file;
    std::ifstream fin(expected_out_path);
    ASSERT_TRUE(fin.is_open())
        << "Failed to open cwd_out.txt in target directory: "
        << expected_out_path;

    std::string child_cwd;
    std::getline(fin, child_cwd);
    fin.close();
    fs::remove(expected_out_path);

    EXPECT_TRUE(fs::equivalent(fs::path(child_cwd), target_dir));
}

TEST(process, stdin_redirect_null)
{
    std::string exe      = get_child_helper_path();
    std::string out_file = "stdin_out.txt";
    std::remove(out_file.c_str());

    hj::os::process::options opts;
    opts.command             = exe;
    opts.args                = {"--check-stdin"};
    opts.redirect_stdin_null = true;
    opts.policy              = hj::os::process_policy::wait_on_destroy;

    std::error_code ec;
    auto            proc = hj::os::spawn(opts, ec);
    ASSERT_TRUE(proc.is_valid()) << "child_helpers missing: " << ec.message();

    proc.wait();

    std::ifstream fin(out_file);
    ASSERT_TRUE(fin.is_open())
        << "stdin_out.txt was not generated by child process";
    std::string result;
    std::getline(fin, result);
    fin.close();
    std::remove(out_file.c_str());

    EXPECT_EQ(result, "EOF_REACHED");
}

#if !defined(_WIN32)
TEST(process, daemon_options_pid_file)
{
    std::string pid_path =
        (fs::temp_directory_path() / "test_daemon.pid").string();
    std::remove(pid_path.c_str());

    hj::os::daemon_options dopts;
    dopts.pid_file       = pid_path;
    dopts.redirect_stdio = true;
    dopts.auto_close_fds = true;

    pid_t pid = ::fork();
    ASSERT_GE(pid, 0);

    if(pid == 0)
    {
        std::error_code ec;
        if(!hj::os::daemonize(dopts, ec))
        {
            ::_exit(EXIT_FAILURE);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        std::exit(EXIT_SUCCESS);
    }

    int status = 0;
    ::waitpid(pid, &status, 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::ifstream pfile(pid_path);
    ASSERT_TRUE(pfile.is_open());
    pid_t written_pid = 0;
    pfile >> written_pid;
    pfile.close();

    EXPECT_GT(written_pid, 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    EXPECT_FALSE(fs::exists(pid_path));
}
#endif

TEST(process, repeated_wait_idempotency)
{
    std::string              exe = get_child_helper_path();
    std::error_code          ec;
    hj::os::process::options opts;
    opts.command           = exe;
    opts.args              = {};
    opts.working_directory = "";
    opts.policy            = hj::os::process_policy::manual;
    auto proc              = hj::os::spawn(opts, ec);
    ASSERT_TRUE(proc.is_valid()) << "child_helpers missing: " << ec.message();

    auto status1 = proc.wait();
    ASSERT_TRUE(status1.has_value());

    auto status2 = proc.wait();
    ASSERT_TRUE(status2.has_value());

    EXPECT_EQ(status1->exited_normally, status2->exited_normally);
    EXPECT_EQ(status1->exit_code, status2->exit_code);
    EXPECT_EQ(status1->signaled, status2->signaled);
    EXPECT_EQ(status1->termsig, status2->termsig);
    EXPECT_EQ(status1->code(), status2->code());
}

TEST(process, concurrent_spawn_100)
{
    constexpr size_t CONCURRENT_COUNT = 100;
    std::string      exe              = get_child_helper_path();

    std::vector<std::future<bool>> futures;
    futures.reserve(CONCURRENT_COUNT);

    for(size_t i = 0; i < CONCURRENT_COUNT; ++i)
    {
        futures.push_back(std::async(std::launch::async, [exe]() {
            std::error_code          ec;
            hj::os::process::options opts;
            opts.command           = exe;
            opts.args              = {};
            opts.working_directory = "";
            opts.policy            = hj::os::process_policy::wait_on_destroy;
            auto proc              = hj::os::spawn(opts, ec);
            if(!proc.is_valid() || ec)
            {
                return false;
            }
            auto st = proc.wait();
            return st.has_value() && st->success();
        }));
    }

    for(size_t i = 0; i < CONCURRENT_COUNT; ++i)
    {
        bool success = futures[i].get();
        EXPECT_TRUE(success) << "Concurrent process index " << i << " failed.";
    }
}

TEST(process, move_assignment_cleans_previous)
{
    std::string     exe = get_child_helper_path();
    std::error_code ec1, ec2;

    hj::os::process::options opts1;
    opts1.command           = exe;
    opts1.args              = {"--sleep"};
    opts1.working_directory = "";
    opts1.policy            = hj::os::process_policy::kill_on_destroy;
    auto p1                 = hj::os::spawn(opts1, ec1);

    hj::os::process::options opts2;
    opts2.command           = exe;
    opts2.args              = {"--sleep"};
    opts2.working_directory = "";
    opts2.policy            = hj::os::process_policy::kill_on_destroy;
    auto p2                 = hj::os::spawn(opts2, ec2);

    ASSERT_TRUE(p1.is_valid() && p2.is_valid());
    hj::os::pid_t pid1 = p1.id();
    hj::os::pid_t pid2 = p2.id();

    p1 = std::move(p2);

    EXPECT_FALSE(p2.is_valid());
    EXPECT_EQ(p1.id(), pid2);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(is_process_alive(pid1));
    EXPECT_TRUE(is_process_alive(pid2));
}

TEST(process, detach_lifecycle_no_zombie)
{
    std::string     exe = get_child_helper_path();
    std::error_code ec;

    hj::os::pid_t pid = 0;
    {
        hj::os::process::options opts;
        opts.command           = exe;
        opts.args              = {"--sleep"};
        opts.working_directory = "";
        opts.policy            = hj::os::process_policy::manual;
        auto proc              = hj::os::spawn(opts, ec);
        ASSERT_TRUE(proc.is_valid()) << ec.message();

        pid = proc.id();
        ASSERT_TRUE(is_process_alive(pid));

        proc.detach();

        EXPECT_FALSE(proc.is_valid());
        EXPECT_EQ(proc.id(), 0);
    }

    hj::os::terminate(pid);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_FALSE(is_process_alive(pid));
}

TEST(process, policy_matrix_detach_on_destroy)
{
    std::string   exe = get_child_helper_path();
    hj::os::pid_t pid = 0;
    {
        hj::os::process::options opts;
        opts.command           = exe;
        opts.args              = {"--sleep"};
        opts.working_directory = "";
        opts.policy            = hj::os::process_policy::detach_on_destroy;
        auto proc              = hj::os::spawn(opts);
        pid                    = proc.id();
        ASSERT_TRUE(is_process_alive(pid));
    }

    EXPECT_TRUE(is_process_alive(pid));

    hj::os::terminate(pid);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(is_process_alive(pid));
}

TEST(process, policy_matrix_wait_on_destroy)
{
    std::string exe        = get_child_helper_path();
    auto        start_time = std::chrono::steady_clock::now();
    {
        hj::os::process::options opts;
        opts.command           = exe;
        opts.args              = {"--sleep"};
        opts.working_directory = "";
        opts.policy            = hj::os::process_policy::wait_on_destroy;
        auto proc              = hj::os::spawn(opts);
        ASSERT_TRUE(proc.is_valid());
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        proc.terminate();
    }
    auto duration = std::chrono::steady_clock::now() - start_time;
    SUCCEED();
}

TEST(process, policy_matrix_terminate_on_destroy)
{
    std::string   exe = get_child_helper_path();
    hj::os::pid_t pid = 0;
    {
        hj::os::process::options opts;
        opts.command           = exe;
        opts.args              = {"--sleep"};
        opts.working_directory = "";
        opts.policy            = hj::os::process_policy::terminate_on_destroy;
        auto proc              = hj::os::spawn(opts);
        pid                    = proc.id();
        ASSERT_TRUE(is_process_alive(pid));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(is_process_alive(pid));
}

TEST(process, policy_matrix_kill_on_destroy)
{
    std::string   exe = get_child_helper_path();
    hj::os::pid_t pid = 0;
    {
        hj::os::process::options opts;
        opts.command           = exe;
        opts.args              = {"--sleep"};
        opts.working_directory = "";
        opts.policy            = hj::os::process_policy::kill_on_destroy;
        auto proc              = hj::os::spawn(opts);
        pid                    = proc.id();
        ASSERT_TRUE(is_process_alive(pid));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(is_process_alive(pid));
}

TEST(process, dynamic_set_policy_after_spawn)
{
    std::string   exe = get_child_helper_path();
    hj::os::pid_t pid = 0;
    {
        hj::os::process::options opts;
        opts.command           = exe;
        opts.args              = {"--sleep"};
        opts.working_directory = "";
        opts.policy            = hj::os::process_policy::manual;
        auto proc              = hj::os::spawn(opts);
        pid                    = proc.id();
        ASSERT_EQ(proc.get_policy(), hj::os::process_policy::manual);

        proc.set_policy(hj::os::process_policy::kill_on_destroy);
        EXPECT_EQ(proc.get_policy(), hj::os::process_policy::kill_on_destroy);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(is_process_alive(pid));
}

TEST(process, win_utf8_invalid_handling)
{
    std::error_code ec;
    std::string     invalid_utf8 = "cmd\xFF\xFE.exe";

    hj::os::process::options opts;
    opts.command = invalid_utf8;

    hj::os::process proc;
    bool            success = proc.start(opts, ec);

#if defined(_WIN32)
    EXPECT_FALSE(success);
    EXPECT_TRUE(ec);
#else
    EXPECT_FALSE(success);
#endif
}

TEST(process, win_daemonize_not_supported)
{
#if defined(_WIN32)
    hj::os::daemon_options opts;
    std::error_code        ec;

    bool res = hj::os::daemonize(opts, ec);

    EXPECT_FALSE(res);
    EXPECT_EQ(ec, std::make_error_code(std::errc::not_supported));
#else
    SUCCEED();
#endif
}

#if !defined(_WIN32)
TEST(process, daemon_options_pid_file_normal_exit)
{
    const fs::path pid_path =
        fs::temp_directory_path() / "test_daemon_normal.pid";
    std::error_code fs_ec;
    fs::remove(pid_path, fs_ec);
    hj::os::daemon_options dopts;
    dopts.pid_file       = pid_path.string();
    dopts.redirect_stdio = true;
    dopts.auto_close_fds = true;
    pid_t pid            = ::fork();
    ASSERT_GE(pid, 0);
    if(pid == 0)
    {
        std::error_code ec;
        if(!hj::os::daemonize(dopts, ec))
        {
            ::_exit(EXIT_FAILURE);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::exit(EXIT_SUCCESS);
    }
    int status = 0;
    ASSERT_EQ(::waitpid(pid, &status, 0), pid);
    ASSERT_TRUE(WIFEXITED(status));
    ASSERT_EQ(
        WEXITSTATUS(status),
        EXIT_SUCCESS); // The first child has exited, but the daemon (grandchild) may
    // still be running. Poll for PID-file cleanup for at most 1 second.
    constexpr auto poll_interval = std::chrono::milliseconds(10);
    constexpr auto timeout       = std::chrono::seconds(1);
    const auto     deadline      = std::chrono::steady_clock::now() + timeout;
    while(fs::exists(pid_path))
    {
        if(std::chrono::steady_clock::now() >= deadline)
            break;
        std::this_thread::sleep_for(poll_interval);
    }
    EXPECT_FALSE(fs::exists(pid_path))
        << "PID file was not removed within 1 second: " << pid_path;
    // Best-effort cleanup so a failed test does not affect subsequent runs.
    fs::remove(pid_path, fs_ec);
}

TEST(process, daemon_options_pid_file_sigterm_cleanup)
{
    std::string pid_path =
        (fs::temp_directory_path() / "test_daemon_sigterm.pid").string();
    std::remove(pid_path.c_str());

    hj::os::daemon_options dopts;
    dopts.pid_file       = pid_path;
    dopts.redirect_stdio = true;
    dopts.auto_close_fds = true;

    pid_t pid = ::fork();
    ASSERT_GE(pid, 0);

    if(pid == 0)
    {
        std::error_code ec;
        if(!hj::os::daemonize(dopts, ec))
        {
            ::_exit(EXIT_FAILURE);
        }

        while(true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_TRUE(fs::exists(pid_path));

    std::ifstream pfile(pid_path);
    pid_t         daemon_pid = 0;
    pfile >> daemon_pid;
    pfile.close();

    ASSERT_GT(daemon_pid, 0);
    ::kill(daemon_pid, SIGTERM);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_FALSE(fs::exists(pid_path));
}

TEST(process, daemon_options_pid_file_stale_lock_recovery)
{
    std::string pid_path =
        (fs::temp_directory_path() / "test_daemon_stale.pid").string();
    std::remove(pid_path.c_str());

    hj::os::daemon_options dopts;
    dopts.pid_file       = pid_path;
    dopts.redirect_stdio = true;

    pid_t pid = ::fork();
    ASSERT_GE(pid, 0);

    if(pid == 0)
    {
        std::error_code ec;
        hj::os::daemonize(dopts, ec);
        ::_exit(EXIT_SUCCESS);
    }

    int status = 0;
    ::waitpid(pid, &status, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_TRUE(fs::exists(pid_path));

    std::error_code ec2;
    pid_t           pid2 = ::fork();
    if(pid2 == 0)
    {
        bool ok = hj::os::daemonize(dopts, ec2);
        std::exit(ok ? 0 : 1);
    }

    ::waitpid(pid2, &status, 0);
    EXPECT_TRUE(WIFEXITED(status) && WEXITSTATUS(status) == 0);

    fs::remove(pid_path);
}

TEST(process, wait_and_is_running_eintr_handling)
{
    struct sigaction sa{};
    sa.sa_handler = [](int) {};
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    ::sigaction(SIGUSR1, &sa, nullptr);

    std::string              exe = get_child_helper_path();
    hj::os::process::options opts;
    opts.command = exe;
    opts.args    = {"--sleep"};
    opts.policy  = hj::os::process_policy::manual;

    std::error_code ec;
    auto            proc = hj::os::spawn(opts, ec);
    ASSERT_TRUE(proc.is_valid());

    pthread_t main_thread = ::pthread_self();

    std::thread killer([main_thread]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        ::pthread_kill(main_thread, SIGUSR1);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        ::pthread_kill(main_thread, SIGUSR1);
    });

    EXPECT_TRUE(proc.is_running());

    auto st = proc.wait(ec);
    killer.join();

    EXPECT_FALSE(ec);
    ASSERT_TRUE(st.has_value());
    EXPECT_TRUE(st->exited_normally);

    signal(SIGUSR1, SIG_DFL);
}

TEST(process, detached_no_zombie_linux_status_check)
{
    std::string              exe = get_child_helper_path();
    hj::os::process::options opts;
    opts.command = exe;
    opts.args    = {"--quick-exit"};
    opts.policy  = hj::os::process_policy::manual;

    std::error_code ec;
    auto            proc = hj::os::spawn(opts, ec);
    ASSERT_TRUE(proc.is_valid());

    hj::os::pid_t child_pid = proc.id();

    proc.detach();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    fs::path status_file =
        fs::path("/proc") / std::to_string(child_pid) / "status";
    if(fs::exists(status_file))
    {
        std::ifstream fin(status_file);
        std::string   line;
        bool          is_zombie = false;
        while(std::getline(fin, line))
        {
            if(line.rfind("State:", 0) == 0)
            {
                if(line.find('Z') != std::string::npos
                   || line.find("zombie") != std::string::npos)
                {
                    is_zombie = true;
                }
                break;
            }
        }
        EXPECT_FALSE(is_zombie)
            << "Process " << child_pid << " stuck in Zombie (Z) state!";
    } else
    {
        SUCCEED();
    }
}

TEST(process, daemonize_close_range_large_fds)
{
    std::vector<int> opened_fds;
    for(int i = 0; i < 90; ++i)
    {
        int fd = ::open("/dev/null", O_RDONLY);
        if(fd >= 0)
            opened_fds.push_back(fd);
    }
    ASSERT_GT(opened_fds.size(), 10u);

    hj::os::daemon_options dopts;
    dopts.redirect_stdio = true;
    dopts.auto_close_fds = true;

    pid_t pid = ::fork();
    ASSERT_GE(pid, 0);

    if(pid == 0)
    {
        std::error_code ec;
        if(!hj::os::daemonize(dopts, ec))
        {
            ::_exit(EXIT_FAILURE);
        }

        bool leaked = false;
        for(int fd : opened_fds)
        {
            if(::fcntl(fd, F_GETFD) != -1 || errno != EBADF)
            {
                leaked = true;
                break;
            }
        }

        ::_exit(leaked ? EXIT_FAILURE : EXIT_SUCCESS);
    }

    for(int fd : opened_fds)
    {
        ::close(fd);
    }

    int status = 0;
    ::waitpid(pid, &status, 0);

    EXPECT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), EXIT_SUCCESS)
        << "Daemon process leaked file descriptors!";
}

TEST(process, daemonize_pid_file_contention_ebusy)
{
    std::string pid_path =
        (fs::temp_directory_path() / "test_contention.pid").string();
    std::remove(pid_path.c_str());

    hj::os::daemon_options dopts;
    dopts.pid_file       = pid_path;
    dopts.redirect_stdio = true;

    pid_t pidA = ::fork();
    ASSERT_GE(pidA, 0);

    if(pidA == 0)
    {
        std::error_code ec;
        if(!hj::os::daemonize(dopts, ec))
        {
            ::_exit(EXIT_FAILURE);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
        ::_exit(EXIT_SUCCESS);
    }

    int status = 0;
    ::waitpid(pidA, &status, 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(fs::exists(pid_path));

    std::error_code ecB;

    pid_t pidB = ::fork();
    ASSERT_GE(pidB, 0);

    if(pidB == 0)
    {
        std::error_code ec;
        bool            ok = hj::os::daemonize(dopts, ec);
        if(!ok && ec == std::errc::device_or_resource_busy)
        {
            ::_exit(42);
        }
        ::_exit(ok ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    ::waitpid(pidB, &status, 0);
    EXPECT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 42)
        << "Daemon B should fail with EBUSY contention!";

    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    fs::remove(pid_path);
}
#endif // !defined(_WIN32)

TEST(process, win_exit_code_259_is_running)
{
    std::string              exe = get_child_helper_path();
    hj::os::process::options opts;
    opts.command = exe;
#if defined(_WIN32)
    opts.command = "cmd.exe";
    opts.args    = {"/c", "exit 259"};
#else
    opts.command = exe;
    opts.args    = {"--sleep"};
#endif
    opts.policy = hj::os::process_policy::manual;

    std::error_code ec;
    auto            proc = hj::os::spawn(opts, ec);
    ASSERT_TRUE(proc.is_valid());

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_FALSE(proc.is_running());

    auto st = proc.wait(ec);
    ASSERT_TRUE(st.has_value());
    EXPECT_EQ(st->code(), 259);
}