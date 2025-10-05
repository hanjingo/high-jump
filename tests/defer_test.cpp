#include <gtest/gtest.h>
#include <hj/util/defer.hpp>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <chrono>

class test_resource
{
  public:
    test_resource(int &counter)
        : counter_(counter)
    {
        counter_++;
    }

    ~test_resource() { counter_--; }

    void setValue(int value) { value_ = value; }

    int getValue() const { return value_; }

  private:
    int &counter_;
    int  value_ = 0;
};

TEST(defer_test, basic_defer)
{
    int counter = 0;

    {
        DEFER(counter = 42;);
        EXPECT_EQ(counter, 0);
    }

    EXPECT_EQ(counter, 42);
}

TEST(defer_test, multiple_defers_execution_order)
{
    std::vector<int> execution_order;

    {
        DEFER(execution_order.push_back(1););
        DEFER(execution_order.push_back(2););
        DEFER(execution_order.push_back(3););
    }

    ASSERT_EQ(execution_order.size(), 3);
    EXPECT_EQ(execution_order[0], 3);
    EXPECT_EQ(execution_order[1], 2);
    EXPECT_EQ(execution_order[2], 1);
}

TEST(defer_test, complex_expressions)
{
    int         a = 1, b = 2, c = 3;
    std::string result;

    {
        DEFER(result = std::to_string(a + b + c); a *= 2; b *= 3; c *= 4;);

        a = 10;
        b = 20;
        c = 30;
    }

    EXPECT_EQ(result, "60"); // 10 + 20 + 30 = 60
    EXPECT_EQ(a, 20);        // 10 * 2
    EXPECT_EQ(b, 60);        // 20 * 3
    EXPECT_EQ(c, 120);       // 30 * 4
}

TEST(defer_test, resource_management)
{
    int resource_counter = 0;

    {
        auto resource = std::make_unique<test_resource>(resource_counter);
        EXPECT_EQ(resource_counter, 1);

        DEFER(resource.reset(););

        EXPECT_EQ(resource_counter, 1);
    }

    EXPECT_EQ(resource_counter, 0);
}

TEST(defer_test, exception_safety)
{
    int  cleanup_counter  = 0;
    bool exception_thrown = false;

    try
    {
        DEFER(cleanup_counter++;);
        DEFER(cleanup_counter++;);

        throw std::runtime_error("test exception");
    }
    catch(const std::exception &)
    {
        exception_thrown = true;
    }

    EXPECT_TRUE(exception_thrown);
    EXPECT_EQ(cleanup_counter, 2);
}

TEST(defer_test, exception_in_defer)
{
    int  counter          = 0;
    bool normal_execution = false;

    {
        DEFER(counter = 1;);
        DEFER(throw std::runtime_error("exception in defer"););
        DEFER(counter = 2;);

        normal_execution = true;
    }

    EXPECT_TRUE(normal_execution);
    EXPECT_EQ(counter, 1);
}

TEST(defer_test, nested_scopes)
{
    std::vector<std::string> execution_log;

    {
        DEFER(execution_log.push_back("outer"););

        {
            DEFER(execution_log.push_back("inner1"););

            {
                DEFER(execution_log.push_back("innermost"););
            }

            DEFER(execution_log.push_back("inner2"););
        }
    }

    ASSERT_EQ(execution_log.size(), 4);
    EXPECT_EQ(execution_log[0], "innermost");
    EXPECT_EQ(execution_log[1], "inner2");
    EXPECT_EQ(execution_log[2], "inner1");
    EXPECT_EQ(execution_log[3], "outer");
}

TEST(defer_test, variable_capture)
{
    int x              = 10;
    int captured_value = 0;

    {
        int y = 20;
        DEFER(captured_value = x + y;);

        x = 100;
        y = 200;
    }

    EXPECT_EQ(captured_value, 300);
}

TEST(defer_test, directdefer_class_usage)
{
    int counter = 0;

    {
        hj::defer d1([&]() { counter += 1; });
        hj::defer d2([&]() { counter += 10; });

        EXPECT_EQ(counter, 0);
    }

    EXPECT_EQ(counter, 11); // 1 + 10
}

TEST(defer_test, move_semantics)
{
    int counter = 0;

    {
        auto      lambda = [&]() { counter = 42; };
        hj::defer d(std::move(lambda));

        EXPECT_EQ(counter, 0);
    }

    EXPECT_EQ(counter, 42);
}

TEST(defer_test, empty_defer)
{
    bool no_crash = true;

    {
        hj::defer empty_defer;
    }

    EXPECT_TRUE(no_crash);
}

TEST(defer_test, edge_cases)
{
    std::vector<int> *vec_ptr = nullptr;

    {
        std::vector<int> local_vec = {1, 2, 3};
        vec_ptr                    = &local_vec;

        DEFER(vec_ptr = nullptr;);
    }

    EXPECT_EQ(vec_ptr, nullptr);
}