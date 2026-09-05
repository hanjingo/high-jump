#include <gtest/gtest.h>
#include <hj/os/process.hpp>
#include <chrono>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <future>
#include <filesystem>

namespace fs = std::filesystem;

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

inline std::string get_child_helper_path()
{
#if defined(_WIN32)
    char buf[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buf);
    return std::string(buf) + "\\child_helpers.exe";
#else
    return "./child_helpers";
#endif
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
        ::_exit(EXIT_SUCCESS);
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
        EXPECT_TRUE(is_process_alive(pid));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(is_process_alive(pid));

    hj::os::terminate(pid);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
    hj::os::daemon_options opts;
    std::error_code        ec;
    bool                   res = hj::os::daemonize(opts, ec);

#if defined(_WIN32)
    EXPECT_FALSE(res);
    EXPECT_EQ(ec, std::make_error_code(std::errc::not_supported));
#else
    SUCCEED();
#endif
}