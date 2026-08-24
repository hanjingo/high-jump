#include <gtest/gtest.h>
#include <hj/sync/coroutine.hpp>
#include <stdexcept>
#include <type_traits>

static_assert(!std::is_copy_constructible_v<hj::coroutine<void>::yield_context>,
              "yield_context must not be copy constructible");
static_assert(!std::is_copy_assignable_v<hj::coroutine<void>::yield_context>,
              "yield_context must not be copy assignable");
static_assert(!std::is_move_constructible_v<hj::coroutine<void>::yield_context>,
              "yield_context must not be move constructible");
static_assert(!std::is_move_assignable_v<hj::coroutine<void>::yield_context>,
              "yield_context must not be move assignable");

void f1(hj::coroutine<void>::yield_context &ctx)
{
    ASSERT_EQ(true, true);
    ctx();
}

void f3(hj::coroutine<std::string>::yield_context &ctx)
{
    ctx("hello");
    ctx("world");
}

TEST(coroutine, AbstractionAndLifecycle)
{
    hj::coroutine<void> co1(f1);
    EXPECT_TRUE(co1.is_resumable());
    co1();
    EXPECT_TRUE(co1.done());

    HJ_COROUTINE(ASSERT_EQ(true, true););

    hj::coroutine<std::string> co3(f3);
    ASSERT_STREQ(co3.get().c_str(), "hello");
    EXPECT_TRUE(co3);

    co3();
    ASSERT_STREQ(co3.get().c_str(), "world");
    EXPECT_TRUE(co3);

    co3();
    EXPECT_FALSE(co3);

    hj::coroutine<int> co4_lambda([](hj::coroutine<int>::yield_context &ctx) {
        ctx(1);
        ctx(2);
    });

    ASSERT_EQ(co4_lambda.get(), 1);
    co4_lambda();
    ASSERT_EQ(co4_lambda.get(), 2);
    co4_lambda();
    EXPECT_FALSE(co4_lambda);
    EXPECT_THROW(co4_lambda.get(), std::logic_error);
}

TEST(coroutine, EmptyAndStateSemantic)
{
    hj::coroutine<void> co(
        [](hj::coroutine<void>::yield_context &ctx) { ctx(); });

    EXPECT_TRUE(co.is_resumable());
    EXPECT_FALSE(co.done());

    co();

    EXPECT_FALSE(co.is_resumable());
    EXPECT_TRUE(co.done());
    EXPECT_FALSE(co.has_exception());
}

TEST(coroutine, DestroySuspendedCoroutineDoesNotResume)
{
    bool stack_frame_dtor_called = false;
    bool resumed_after_yield     = false;

    {
        hj::coroutine<void> co([&](hj::coroutine<void>::yield_context &ctx) {
            struct ScopedGuard
            {
                bool *flag;
                ~ScopedGuard() { *flag = true; }
            } guard{&stack_frame_dtor_called};

            ctx();

            resumed_after_yield = true;
        });

        EXPECT_TRUE(co.is_resumable());
    }

    EXPECT_FALSE(resumed_after_yield);
    EXPECT_TRUE(stack_frame_dtor_called);
}

TEST(coroutine, MoveAssignmentDestroysTargetWithoutResume)
{
    bool target_dtor_called = false;
    bool target_resumed     = false;

    hj::coroutine<void> target([&](hj::coroutine<void>::yield_context &ctx) {
        struct ScopedGuard
        {
            bool *flag;
            ~ScopedGuard() { *flag = true; }
        } guard{&target_dtor_called};

        ctx();
        target_resumed = true;
    });

    hj::coroutine<void> source(
        [](hj::coroutine<void>::yield_context &ctx) { ctx(); });

    EXPECT_TRUE(target.is_resumable());
    EXPECT_TRUE(source.is_resumable());

    target = std::move(source);

    EXPECT_TRUE(target_dtor_called);
    EXPECT_FALSE(target_resumed);
    EXPECT_TRUE(target.is_resumable());
    EXPECT_FALSE(source);
}

TEST(coroutine, DestroyCoroutineWithExceptionDoesNotThrowOrTerminate)
{
    EXPECT_NO_THROW({
        hj::coroutine<void> co([](hj::coroutine<void>::yield_context &) {
            throw std::runtime_error("unhandled exception inside coroutine");
        });

        co();
        EXPECT_TRUE(co.has_exception());
    });

    EXPECT_NO_THROW({
        hj::coroutine<void> co([](hj::coroutine<void>::yield_context &ctx) {
            ctx();
            throw std::logic_error("error after resume");
        });
    });
}

TEST(coroutine, ExceptionPropagation)
{
    hj::coroutine<void> co([](hj::coroutine<void>::yield_context &) {
        throw std::runtime_error("coroutine test failure");
    });

    co();

    EXPECT_TRUE(co.has_exception());
    EXPECT_NE(co.exception(), nullptr);
    EXPECT_THROW({ co.rethrow_exception(); }, std::runtime_error);
}

TEST(coroutine, HandlerExceptionIsolation)
{
    bool handler_called = false;

    hj::coroutine<void> co(
        [](hj::coroutine<void>::yield_context &) {
            throw std::runtime_error("original error");
        },
        [&handler_called](const std::exception_ptr &) {
            handler_called = true;
            throw std::runtime_error("handler error");
        });

    co();

    EXPECT_TRUE(handler_called);
    EXPECT_TRUE(co.has_exception());
    EXPECT_THROW({ co.rethrow_exception(); }, std::runtime_error);
}

TEST(coroutine, MoveSemantics)
{
    hj::coroutine<void> co_a(
        [](hj::coroutine<void>::yield_context &ctx) { ctx(); });

    EXPECT_TRUE(co_a);

    hj::coroutine<void> co_b(std::move(co_a));
    EXPECT_FALSE(co_a);
    EXPECT_TRUE(co_b);

    hj::coroutine<void> co_c;
    co_c = std::move(co_b);
    EXPECT_FALSE(co_b);
    EXPECT_TRUE(co_c);
}

TEST(coroutine, MoveThenException)
{
    hj::coroutine<void> a(
        [](auto &) { throw std::runtime_error("failure after move"); });

    hj::coroutine<void> b(std::move(a));
    b();

    EXPECT_TRUE(b.has_exception());
    EXPECT_THROW(b.rethrow_exception(), std::runtime_error);
}

TEST(coroutine, GetAfterExceptionSemantic)
{
    hj::coroutine<int> co([](auto &) { throw std::runtime_error("fail"); });

    co();
    EXPECT_TRUE(co.has_exception());

    EXPECT_THROW(co.get(), std::logic_error);
    EXPECT_THROW(co.rethrow_exception(), std::runtime_error);
}