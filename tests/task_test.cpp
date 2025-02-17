#include <gtest/gtest.h>
#include <libcpp/io/task.hpp>

TEST(task, create)
{
    ASSERT_EQ(libcpp::task::create() != nullptr, true);
}

TEST(task, then)
{
    libcpp::task* tk = libcpp::task::create();
    tk->then(
        [](int arg2){ ASSERT_EQ(arg2 == 123, true); }, 123
    );
    tk->run();
}

TEST(task, skip_next)
{
    int num = 0;
    libcpp::task* tk = libcpp::task::create();
    tk->then([](libcpp::task* arg){
        arg->skip_next();
    }, tk);
    tk->then([&](){
        num += 1;
        ASSERT_FALSE(false);
    });
    tk->then([&](){
        num += 1;
        ASSERT_TRUE(true);
    });
    tk->run();
    ASSERT_EQ(num, 1);
}

TEST(task, skip_all)
{
    int num = 0;
    libcpp::task* tk = libcpp::task::create();
    tk->then([](libcpp::task* arg){
        arg->skip_all();
    }, tk);
    tk->then([&](){
        num += 1;
        ASSERT_FALSE(false);
    });
    tk->then([&](){
        num += 1;
        ASSERT_FALSE(false);
    });
    tk->run();
    ASSERT_EQ(num, 0);
}

TEST(task, pause)
{

}

TEST(task, resume)
{

}

TEST(task, stop)
{
    int num = 0;
    libcpp::task* tk = libcpp::task::create();
    tk->then([](libcpp::task* arg){
        arg->stop();
    }, tk);
    tk->then([&](){
        num += 1;
        ASSERT_FALSE(false);
    });
    tk->then([&](){
        num += 1;
        ASSERT_FALSE(false);
    });
    tk->run();
    ASSERT_EQ(num, 0);
}

TEST(task, run)
{

}

TEST(task, run_one)
{

}

TEST(task, run_for)
{

}

TEST(task, async_run)
{

}

TEST(task, size)
{
    libcpp::task* tk = libcpp::task::create();
    tk->then([](libcpp::task* arg){
        ASSERT_EQ(arg->size(), 2);
    }, tk);
    tk->then([](libcpp::task* arg){
        ASSERT_EQ(arg->size(), 1);
    }, tk);
    tk->then([](libcpp::task* arg){
        ASSERT_EQ(arg->size(), 0);
    }, tk);
    tk->run();
    ASSERT_EQ(tk->size(), 0);
}