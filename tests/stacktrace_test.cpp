#include <gtest/gtest.h>
#include <hj/testing/stacktrace.hpp>
#include <boost/config.hpp>
#include <atomic>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// --- 去除 static，防止 MSVC PDB 生成修饰性内部符号 ---

BOOST_NOINLINE std::string capture_deep_stacktrace_baz()
{
    return hj::current_stacktrace();
}

BOOST_NOINLINE std::string capture_deep_stacktrace_bar()
{
    return capture_deep_stacktrace_baz();
}

BOOST_NOINLINE std::string capture_deep_stacktrace_foo()
{
    return capture_deep_stacktrace_bar();
}

BOOST_NOINLINE void throw_exception_baz()
{
    throw std::runtime_error("deep exception error");
}

BOOST_NOINLINE void throw_exception_bar()
{
    throw_exception_baz();
}

BOOST_NOINLINE void throw_exception_foo()
{
    throw_exception_bar();
}

BOOST_NOINLINE std::string catch_and_diagnose_foo()
{
    try
    {
        throw_exception_foo();
    }
    catch(...)
    {
        return hj::current_exception_diagnostic();
    }
    return "";
}

BOOST_NOINLINE void throw_path_alpha()
{
    throw std::runtime_error("alpha failure: network timeout");
}

BOOST_NOINLINE void throw_path_beta()
{
    throw std::invalid_argument("beta failure: invalid buffer size");
}

BOOST_NOINLINE void concurrent_worker_path_a(int thread_id, int iter)
{
    throw std::runtime_error("thread_a_err_tid_" + std::to_string(thread_id)
                             + "_iter_" + std::to_string(iter));
}

BOOST_NOINLINE void concurrent_worker_path_b(int thread_id, int iter)
{
    throw std::invalid_argument("thread_b_err_tid_" + std::to_string(thread_id)
                                + "_iter_" + std::to_string(iter));
}

TEST(stacktrace, current_stacktrace_contains_test_function)
{
    std::string st = hj::current_stacktrace();

    EXPECT_FALSE(st.empty());
    EXPECT_NE(st.find("current_stacktrace_contains_test_function"),
              std::string::npos)
        << "Stacktrace output:\n"
        << st;
}

TEST(stacktrace, nested_callchain_verification)
{
    std::string st = capture_deep_stacktrace_foo();

    EXPECT_NE(st.find("capture_deep_stacktrace_baz"), std::string::npos)
        << "Failed to capture leaf function 'baz'\nStacktrace:\n"
        << st;

    EXPECT_NE(st.find("capture_deep_stacktrace_bar"), std::string::npos)
        << "Failed to capture middle function 'bar'\nStacktrace:\n"
        << st;

    EXPECT_NE(st.find("capture_deep_stacktrace_foo"), std::string::npos)
        << "Failed to capture root function 'foo'\nStacktrace:\n"
        << st;

    size_t pos_baz = st.find("capture_deep_stacktrace_baz");
    size_t pos_bar = st.find("capture_deep_stacktrace_bar");
    size_t pos_foo = st.find("capture_deep_stacktrace_foo");

    EXPECT_LT(pos_baz, pos_bar)
        << "Stacktrace order incorrect: 'baz' should appear above 'bar'";
    EXPECT_LT(pos_bar, pos_foo)
        << "Stacktrace order incorrect: 'bar' should appear above 'foo'";
}

TEST(stacktrace, nested_exception_diagnostic_verification)
{
    std::string diag = catch_and_diagnose_foo();

    EXPECT_NE(diag.find("runtime_error"), std::string::npos);
    EXPECT_NE(diag.find("deep exception error"), std::string::npos);

    bool captured_throw_site =
        (diag.find("throw_exception_baz") != std::string::npos);
    bool captured_catch_site =
        (diag.find("catch_and_diagnose_foo") != std::string::npos);

    EXPECT_TRUE(captured_throw_site || captured_catch_site)
        << "Diagnostic failed to capture valid stack frame.\nDiagnostic:\n"
        << diag;
}

TEST(stacktrace, multiple_executions_state_isolation)
{
    std::string diag_alpha;
    std::string diag_beta;

    try
    {
        throw_path_alpha();
    }
    catch(...)
    {
        diag_alpha = hj::current_exception_diagnostic();
    }

    try
    {
        throw_path_beta();
    }
    catch(...)
    {
        diag_beta = hj::current_exception_diagnostic();
    }

    EXPECT_NE(diag_alpha.find("runtime_error"), std::string::npos);
    EXPECT_NE(diag_alpha.find("alpha failure: network timeout"),
              std::string::npos);
    EXPECT_EQ(diag_alpha.find("beta failure"), std::string::npos);

    EXPECT_NE(diag_beta.find("invalid_argument"), std::string::npos);
    EXPECT_NE(diag_beta.find("beta failure: invalid buffer size"),
              std::string::npos);
    EXPECT_EQ(diag_beta.find("alpha failure"), std::string::npos);
}

TEST(stacktrace, multiple_iterations_resource_and_refresh_integrity)
{
    constexpr int kIterations = 50;

    for(int i = 0; i < kIterations; ++i)
    {
        std::string expected_msg =
            "iterative failure count: " + std::to_string(i);
        std::string diag;

        try
        {
            if(i % 2 == 0)
            {
                throw std::runtime_error(expected_msg);
            } else
            {
                throw std::logic_error(expected_msg);
            }
        }
        catch(...)
        {
            diag = hj::current_exception_diagnostic();
        }

        if(i % 2 == 0)
        {
            EXPECT_NE(diag.find("runtime_error"), std::string::npos);
        } else
        {
            EXPECT_NE(diag.find("logic_error"), std::string::npos);
        }

        EXPECT_NE(diag.find(expected_msg), std::string::npos)
            << "Failed to refresh dynamic message at iteration " << i;
    }
}

TEST(stacktrace, concurrent_multithreaded_safety_cxx17)
{
    constexpr int kThreadCount         = 16;
    constexpr int kIterationsPerThread = 50;

    std::atomic<bool> ready_flag{false};
    std::atomic<int>  failure_count{0};

    std::vector<std::thread> threads;
    threads.reserve(kThreadCount);

    for(int t = 0; t < kThreadCount; ++t)
    {
        threads.emplace_back([t,
                              kIterationsPerThread,
                              &ready_flag,
                              &failure_count]() {
            while(!ready_flag.load(std::memory_order_acquire))
            {
                std::this_thread::yield();
            }

            for(int i = 0; i < kIterationsPerThread; ++i)
            {
                std::string       diag;
                const std::string expected_token =
                    "tid_" + std::to_string(t) + "_iter_" + std::to_string(i);

                try
                {
                    if((t + i) % 2 == 0)
                    {
                        concurrent_worker_path_a(t, i);
                    } else
                    {
                        concurrent_worker_path_b(t, i);
                    }
                }
                catch(...)
                {
                    diag = hj::current_exception_diagnostic();
                }

                if((t + i) % 2 == 0)
                {
                    // 核心逻辑校验：校验异常类型与该线程特有的 token Message
                    bool valid_type =
                        (diag.find("runtime_error") != std::string::npos);
                    bool valid_msg =
                        (diag.find("thread_a_err_" + expected_token)
                         != std::string::npos);

                    // 数据隔离校验：不得夹带 Path B 的任何 Message 污染
                    bool clean =
                        (diag.find("thread_b_err_") == std::string::npos);

                    if(!valid_type || !valid_msg || !clean)
                    {
                        failure_count.fetch_add(1, std::memory_order_relaxed);
                    }
                } else
                {
                    bool valid_type =
                        (diag.find("invalid_argument") != std::string::npos);
                    bool valid_msg =
                        (diag.find("thread_b_err_" + expected_token)
                         != std::string::npos);

                    bool clean =
                        (diag.find("thread_a_err_") == std::string::npos);

                    if(!valid_type || !valid_msg || !clean)
                    {
                        failure_count.fetch_add(1, std::memory_order_relaxed);
                    }
                }
            }
        });
    }

    ready_flag.store(true, std::memory_order_release);

    for(auto &th : threads)
    {
        if(th.joinable())
        {
            th.join();
        }
    }

    EXPECT_EQ(failure_count.load(), 0)
        << "Detected " << failure_count.load()
        << " thread-safety or data-leakage failures during high-concurrency "
           "execution!";
}