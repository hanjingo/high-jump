#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <hj/testing/exception.hpp>

TEST(exception, custom_exception_class)
{
    try
    {
        throw hj::Exception("custom error message");
    }
    catch(const hj::Exception &e)
    {
        EXPECT_STREQ(e.what(), "custom error message");
        EXPECT_NO_THROW({
            auto trace_obj = e.trace();
            static_cast<void>(trace_obj);
        });
    }

    try
    {
        throw hj::NotFoundException("resource not found");
    }
    catch(const hj::Exception &e)
    {
        EXPECT_STREQ(e.what(), "resource not found");
    }
}

TEST(exception, empty_message)
{
    try
    {
        throw hj::Exception("");
    }
    catch(const hj::Exception &e)
    {
        EXPECT_STREQ(e.what(), "");
    }

    EXPECT_THROW(hj::throw_if_false(false, {}), std::logic_error);
    try
    {
        hj::throw_if_false(false, {});
    }
    catch(const std::logic_error &e)
    {
        EXPECT_STREQ(e.what(), "");
    }
}

TEST(exception, custom_stacktrace_preservation)
{
    boost::stacktrace::stacktrace custom_trace  = hj::current_stacktrace();
    std::size_t                   expected_size = custom_trace.size();

    hj::Exception e("error with custom trace", custom_trace);

    EXPECT_STREQ(e.what(), "error with custom trace");
    EXPECT_EQ(e.trace().size(), expected_size);
}

TEST(exception, string_view_non_null_terminated)
{
    std::string      value = "abcdef";
    std::string_view view(value.data(), 3);

    EXPECT_THROW(hj::throw_if_false(false, view), std::logic_error);

    try
    {
        hj::throw_if_false(false, view);
    }
    catch(const std::logic_error &e)
    {
        EXPECT_STREQ(e.what(), "abc");
    }
}

TEST(exception, throw_if_false)
{
    try
    {
        hj::throw_if_false(false);
    }
    catch(const std::logic_error &e)
    {
        EXPECT_STREQ(e.what(), "false");
    }

    try
    {
        hj::throw_if_false(false, "condition false");
    }
    catch(const std::logic_error &e)
    {
        EXPECT_STREQ(e.what(), "condition false");
    }

    EXPECT_NO_THROW(hj::throw_if_false(true));
}

TEST(exception, throw_if_not_false)
{
    try
    {
        hj::throw_if_not_false(true);
    }
    catch(const std::logic_error &e)
    {
        EXPECT_STREQ(e.what(), "not false");
    }

    EXPECT_NO_THROW(hj::throw_if_not_false(false));
}

TEST(exception, throw_if_equal)
{
    EXPECT_THROW(hj::throw_if_equal(10, 10), std::logic_error);
    EXPECT_NO_THROW(hj::throw_if_equal(10, 20, "equal panic"));
}

TEST(exception, throw_if_not_equal)
{
    EXPECT_THROW(hj::throw_if_not_equal(10, 20), std::logic_error);
    EXPECT_NO_THROW(hj::throw_if_not_equal(10, 10));
}

TEST(exception, throw_if_empty)
{
    std::string str1{};
    std::string str2{"hello"};

    EXPECT_THROW(hj::throw_if_empty(str1), std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_empty(str2));
}

TEST(exception, throw_if_not_empty)
{
    std::string str1{"hello"};
    std::string str2{};

    EXPECT_THROW(hj::throw_if_not_empty(str1), std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_not_empty(str2));
}

TEST(exception, throw_if_null)
{
    int *null_ptr = nullptr;
    int  val      = 100;

    EXPECT_THROW(hj::throw_if_null(null_ptr), std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_null(&val));

    std::shared_ptr<int> empty_shared;
    auto                 valid_shared = std::make_shared<int>(42);

    EXPECT_THROW(hj::throw_if_null(empty_shared, "shared null"),
                 std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_null(valid_shared));

    std::unique_ptr<int> empty_unique;
    auto                 valid_unique = std::make_unique<int>(42);

    EXPECT_THROW(hj::throw_if_null(empty_unique, "unique null"),
                 std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_null(valid_unique));

    std::optional<int> empty_opt;
    std::optional<int> valid_opt = 42;

    EXPECT_THROW(hj::throw_if_null(empty_opt, "optional null"),
                 std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_null(valid_opt));
}

TEST(exception, throw_if_not_null)
{
    int *null_ptr = nullptr;
    int  val      = 100;

    EXPECT_THROW(hj::throw_if_not_null(&val, "not null"),
                 std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_not_null(null_ptr));

    auto                 valid_shared = std::make_shared<int>(42);
    std::shared_ptr<int> empty_shared;

    EXPECT_THROW(hj::throw_if_not_null(valid_shared), std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_not_null(empty_shared));
}

TEST(exception, throw_if_exists)
{
    std::vector<int> vec{1, 2, 3};
    EXPECT_THROW(hj::throw_if_exists(vec, 2), std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_exists(vec, 5));

    std::set<int> set_ct{10, 20, 30};
    EXPECT_THROW(hj::throw_if_exists(set_ct, 20), std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_exists(set_ct, 50));
}

TEST(exception, throw_if_not_exists)
{
    std::vector<int> vec{1, 2, 3};
    EXPECT_THROW(hj::throw_if_not_exists(vec, 0), std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_not_exists(vec, 2));

    std::set<int> set_ct{10, 20, 30};
    EXPECT_THROW(hj::throw_if_not_exists(set_ct, 99), std::invalid_argument);
    EXPECT_NO_THROW(hj::throw_if_not_exists(set_ct, 10));
}

TEST(exception, recover_function_with_return_value)
{
    bool handler_called = false;

    auto res1 = hj::recover([]() -> int { return 42; }, [&](auto, auto) {});
    ASSERT_TRUE(res1.has_value());
    EXPECT_EQ(res1.value(), 42);

    auto res2 = hj::recover(
        []() -> int {
            throw hj::Exception("error in recover");
            return 0;
        },
        [&](const std::exception_ptr            &ep,
            const boost::stacktrace::stacktrace &st) {
            handler_called = true;
            EXPECT_NE(ep, nullptr);
            EXPECT_GT(st.size(), 0);
        });

    EXPECT_FALSE(res2.has_value());
    EXPECT_TRUE(handler_called);
}

TEST(exception, recover_unknown_exception)
{
    bool handler_called = false;

    hj::recover([]() { throw 42; },
                [&](const std::exception_ptr            &ep,
                    const boost::stacktrace::stacktrace &st) {
                    handler_called = true;
                    EXPECT_NE(ep, nullptr);
                    EXPECT_GT(st.size(), 0);
                });

    EXPECT_TRUE(handler_called);
}

TEST(exception, recover_handler_throws_exception)
{
    EXPECT_THROW(
        {
            hj::recover([]() { throw std::runtime_error("primary error"); },
                        [](const std::exception_ptr &,
                           const boost::stacktrace::stacktrace &) {
                            throw std::runtime_error("handler error");
                        });
        },
        std::runtime_error);

    try
    {
        hj::recover([]() { throw std::runtime_error("primary error"); },
                    [](const std::exception_ptr &,
                       const boost::stacktrace::stacktrace &) {
                        throw std::runtime_error("handler error");
                    });
    }
    catch(const std::runtime_error &e)
    {
        EXPECT_STREQ(e.what(), "handler error");
    }
}

TEST(exception, recover_macro)
{
    std::ostringstream ss;

    HJ_RECOVER_WITH_LOG(hj::throw_if_false(false, "macro capture test"), ss);

    std::string log_output = ss.str();
    EXPECT_NE(log_output.find("Exception caught: macro capture test"),
              std::string::npos);
    EXPECT_NE(log_output.find("Stacktrace:"), std::string::npos);
}