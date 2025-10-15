#include <gtest/gtest.h>
#include <hj/testing/error_handler.hpp>
#include <system_error>

enum class err1
{
    unknow = -1,
    ok     = 0,
    timeout,
    mem_leak,
};

std::error_code e_fn()
{
    return std::error_code(static_cast<int>(err1::timeout),
                           std::generic_category());
}

std::error_code e_fn_fix_timeout()
{
    return std::error_code(static_cast<int>(err1::mem_leak),
                           std::generic_category());
}

std::error_code e_fn_fix_memleak()
{
    return std::error_code();
}

TEST(error_handler, match)
{
    err1                    last_err = err1::unknow;
    err1                    ok       = err1::ok;
    err1                    timeout  = err1::timeout;
    err1                    mem_leak = err1::mem_leak;
    hj::error_handler<err1> h(
        [](const err1 &e) -> bool { return e == err1::ok; });

    // idle + ok -> succed
    last_err = err1::unknow;
    h.match(ok);
    EXPECT_EQ(last_err, err1::unknow);
    EXPECT_TRUE(h.status() == hj::err_status::succed);

    // succed + timeout -> handling
    last_err = err1::unknow;
    h.match(timeout, [&](const err1 &e) { last_err = e; });
    EXPECT_EQ(last_err, timeout);
    EXPECT_TRUE(h.status() == hj::err_status::handling);

    // handling + mem_leak -(defer)-> handling
    last_err = err1::unknow;
    h.match(mem_leak, [&](const err1 &e) { last_err = e; });
    EXPECT_EQ(last_err, err1::unknow);
    EXPECT_TRUE(h.status() == hj::err_status::handling);
    h.match(ok); // last handling state finished, deferred event processed
    EXPECT_EQ(last_err, mem_leak);
    EXPECT_TRUE(h.status() == hj::err_status::handling);

    // handling + abort -> failed
    last_err = err1::unknow;
    h.abort();
    EXPECT_EQ(last_err, err1::unknow);
    EXPECT_TRUE(h.status() == hj::err_status::failed);

    // failed + reset -> idle
    last_err = err1::unknow;
    h.reset();
    EXPECT_EQ(last_err, err1::unknow);
    EXPECT_TRUE(h.status() == hj::err_status::idle);

    // idle + mem_leak -> handling
    last_err = err1::unknow;
    h.match(mem_leak, [&](const err1 &e) { last_err = e; });
    EXPECT_EQ(last_err, mem_leak);
    EXPECT_TRUE(h.status() == hj::err_status::handling);

    // handling + ok -> succed
    last_err = err1::unknow;
    h.match(ok);
    EXPECT_EQ(last_err, err1::unknow);
    EXPECT_TRUE(h.status() == hj::err_status::succed);
}

TEST(error_handler, match_std_error_code)
{
    std::error_code unknow(static_cast<int>(err1::unknow),
                           std::generic_category());
    std::error_code ok;
    std::error_code timeout(static_cast<int>(err1::timeout),
                            std::generic_category());
    std::error_code mem_leak(static_cast<int>(err1::mem_leak),
                             std::generic_category());
    std::error_code last_err = unknow;
    hj::error_handler<std::error_code> h(
        [](const std::error_code &e) -> bool { return !e; });

    // idle + ok -> succed
    last_err = unknow;
    h.match(ok);
    EXPECT_EQ(last_err, unknow);
    EXPECT_TRUE(h.status() == hj::err_status::succed);

    // succed + timeout -> handling
    last_err = unknow;
    h.match(timeout, [&](const std::error_code &e) { last_err = e; });
    EXPECT_EQ(last_err, timeout);
    EXPECT_TRUE(h.status() == hj::err_status::handling);

    // handling + mem_leak -(defer)-> handling
    last_err = unknow;
    h.match(mem_leak, [&](const std::error_code &e) { last_err = e; });
    EXPECT_EQ(last_err, unknow);
    EXPECT_TRUE(h.status() == hj::err_status::handling);
    h.match(ok); // last handling state finished, deferred event processed
    EXPECT_EQ(last_err, mem_leak);
    EXPECT_TRUE(h.status() == hj::err_status::handling);

    // handling + abort -> failed
    last_err = unknow;
    h.abort();
    EXPECT_EQ(last_err, unknow);
    EXPECT_TRUE(h.status() == hj::err_status::failed);

    // failed + reset -> idle
    last_err = unknow;
    h.reset();
    EXPECT_EQ(last_err, unknow);
    EXPECT_TRUE(h.status() == hj::err_status::idle);

    // idle + mem_leak -> handling
    last_err = unknow;
    h.match(mem_leak, [&](const std::error_code &e) { last_err = e; });
    EXPECT_EQ(last_err, mem_leak);
    EXPECT_TRUE(h.status() == hj::err_status::handling);

    // handling + ok -> succed
    last_err = unknow;
    h.match(ok);
    EXPECT_EQ(last_err, unknow);
    EXPECT_TRUE(h.status() == hj::err_status::succed);
}

TEST(error_handler, linked_match)
{
    static int                         err_match_n = 0;
    std::error_code                    last_err;
    std::error_code                    ok;
    hj::error_handler<std::error_code> h(ok);
    last_err = e_fn();
    h.match(last_err,
            [&](const std::error_code &e) {
                if(e.value() == static_cast<int>(err1::timeout))
                {
                    err_match_n++;
                    last_err = e_fn_fix_timeout();
                }

                h.reset();
            })
        .match(last_err,
               [&](const std::error_code &e) {
                   if(e.value() == static_cast<int>(err1::mem_leak))
                   {
                       err_match_n++;
                       last_err = e_fn_fix_memleak();
                   }

                   h.reset();
               })
        .match(last_err, [&](const std::error_code &e) {
            if(!e)
            {
                err_match_n++;
                h.abort(); // failed many times, abort it
            }
        });

    ASSERT_EQ(err_match_n, 2);
    ASSERT_TRUE(h.is_succed());
}