#include <gtest/gtest.h>
#include <hj/testing/error_handler.hpp>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

enum class err1
{
    unknow = -1,
    ok     = 0,
    timeout,
    mem_leak,
};

TEST(error_handler_factory, named_constructors)
{
    std::error_code ok_ec;
    std::error_code err_ec(static_cast<int>(err1::timeout),
                           std::generic_category());

    auto h1 = hj::error_handler<std::error_code>::with_ok_value(ok_ec);
    h1.match(ok_ec);
    EXPECT_TRUE(h1.is_success());

    auto h2 = hj::error_handler<std::error_code>::with_checker(
        [](const std::error_code &e) { return !e; });
    h2.match(err_ec);
    EXPECT_TRUE(h2.is_handling());

    bool transition_hook_fired = false;
    auto h3                    = hj::error_handler<std::error_code>::with_hooks(
        [&](std::string_view, std::string_view) {
            transition_hook_fired = true;
        });
    h3.match(err_ec);
    EXPECT_TRUE(transition_hook_fired);
    EXPECT_TRUE(h3.is_handling());
}

TEST(error_handler_matrix, state_event_coverage)
{
    std::error_code ok_ec;
    std::error_code err_ec(static_cast<int>(err1::timeout),
                           std::generic_category());

    {
        hj::error_handler<std::error_code> h;

        // idle + resolve() -> idle
        h.resolve();
        EXPECT_TRUE(h.is_idle());

        // idle + fail() -> failed
        h.reset();
        h.fail();
        EXPECT_TRUE(h.is_failed());

        // idle + abort() -> failed
        h.reset();
        h.abort();
        EXPECT_TRUE(h.is_failed());

        // idle + match(ok) -> success
        h.reset();
        h.match(ok_ec);
        EXPECT_TRUE(h.is_success());

        // idle + match(err) -> handling
        h.reset();
        h.match(err_ec);
        EXPECT_TRUE(h.is_handling());
    }

    {
        hj::error_handler<std::error_code> h;

        // handling + resolve() -> success
        h.match(err_ec);
        h.resolve();
        EXPECT_TRUE(h.is_success());

        // handling + fail() -> failed
        h.reset();
        h.match(err_ec);
        h.fail();
        EXPECT_TRUE(h.is_failed());

        // handling + abort() -> failed
        h.reset();
        h.match(err_ec);
        h.abort();
        EXPECT_TRUE(h.is_failed());

        // handling + reset() -> idle
        h.reset();
        h.match(err_ec);
        h.reset();
        EXPECT_TRUE(h.is_idle());

        // handling + match(ok) -> success
        h.reset();
        h.match(err_ec);
        h.match(ok_ec);
        EXPECT_TRUE(h.is_success());
    }

    {
        hj::error_handler<std::error_code> h;

        // success + resolve() -> success
        h.match(ok_ec);
        h.resolve();
        EXPECT_TRUE(h.is_success());

        // success + fail() -> success (ignore)
        h.fail();
        EXPECT_TRUE(h.is_success());

        // success + abort() -> success (ignore)
        h.abort();
        EXPECT_TRUE(h.is_success());

        // success + match(err) -> handling
        h.match(err_ec);
        EXPECT_TRUE(h.is_handling());

        // success + reset() -> idle
        h.reset();
        h.match(ok_ec);
        h.reset();
        EXPECT_TRUE(h.is_idle());
    }

    {
        hj::error_handler<std::error_code> h;

        h.abort();
        EXPECT_TRUE(h.is_failed());

        h.resolve();
        EXPECT_TRUE(h.is_failed());

        h.match(ok_ec);
        EXPECT_TRUE(h.is_failed());

        h.match(err_ec);
        EXPECT_TRUE(h.is_failed());

        h.reset();
        EXPECT_TRUE(h.is_idle());
    }
}

TEST(error_handler_exception, user_callback_and_transition_safety)
{
    bool ex_captured = false;
    auto h           = hj::error_handler<std::error_code>::with_hooks(
        [](std::string_view, std::string_view) {
            throw std::logic_error("Transition hook exploded!");
        },
        [&](const std::exception_ptr &ex) {
            if(ex)
            {
                try
                {
                    std::rethrow_exception(ex);
                }
                catch(const std::exception &)
                {
                    ex_captured = true;
                }
            }
        });

    std::error_code err(static_cast<int>(err1::timeout),
                        std::generic_category());

    EXPECT_NO_THROW(h.match(err, [](const std::error_code &) {
        throw std::runtime_error("Match callback exploded!");
    }));

    EXPECT_TRUE(ex_captured);
    EXPECT_TRUE(h.is_handling());
}

TEST(error_handler_move, move_construction_and_assignment)
{
    std::error_code err(static_cast<int>(err1::timeout),
                        std::generic_category());

    // Move Construction
    {
        hj::error_handler<std::error_code> h1;
        h1.match(err);
        EXPECT_TRUE(h1.is_handling());

        hj::error_handler<std::error_code> h2(std::move(h1));
        EXPECT_TRUE(h2.is_handling());

        h2.resolve();
        EXPECT_TRUE(h2.is_success());
    }

    // Move Assignment
    {
        hj::error_handler<std::error_code> h1;
        h1.match(err);

        hj::error_handler<std::error_code> h2;
        h2 = std::move(h1);
        EXPECT_TRUE(h2.is_handling());

        h2.resolve();
        EXPECT_TRUE(h2.is_success());
    }
}

TEST(error_handler_defer, reset_clears_deferred_events)
{
    bool            deferred_cb_executed = false;
    std::error_code err_a(static_cast<int>(err1::timeout),
                          std::generic_category());
    std::error_code err_b(static_cast<int>(err1::mem_leak),
                          std::generic_category());
    std::error_code ok_ec;

    hj::error_handler<std::error_code> h;

    h.match(err_a);
    EXPECT_TRUE(h.is_handling());

    h.match(err_b,
            [&](const std::error_code &) { deferred_cb_executed = true; });
    EXPECT_FALSE(deferred_cb_executed);
    EXPECT_TRUE(h.is_handling());

    h.reset();
    EXPECT_TRUE(h.is_idle());

    h.match(ok_ec);
    EXPECT_TRUE(h.is_success());

    EXPECT_FALSE(deferred_cb_executed);
}

TEST(error_handler_defer, bounded_defer_queue_overflow_protection)
{
    hj::error_handler<std::error_code,
                      std::function<bool(const std::error_code &)>,
                      3>
                    h;
    std::error_code err(static_cast<int>(err1::timeout),
                        std::generic_category());

    h.match(err);
    EXPECT_TRUE(h.is_handling());

    EXPECT_NO_THROW(h.match(err));
    EXPECT_NO_THROW(h.match(err));
    EXPECT_NO_THROW(h.match(err));

    EXPECT_THROW(h.match(err), std::runtime_error);
}