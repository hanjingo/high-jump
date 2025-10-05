#include <gtest/gtest.h>
#include <hj/util/init.hpp>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <atomic>
#include <chrono>
#include <functional>

static int              global_init_counter = 0;
static std::vector<int> global_init_order;
static std::atomic<int> atomic_counter{0};

INIT(global_init_counter += 1; global_init_order.push_back(1););
INIT(global_init_counter += 1; global_init_order.push_back(2););
INIT(global_init_counter += 1; global_init_order.push_back(3););

INIT(
    try { throw std::runtime_error("test exception in init"); } catch(...) {
        global_init_counter += 10;
    });

INIT(atomic_counter.fetch_add(1););
INIT(atomic_counter.fetch_add(1););

class init_test_resource
{
  public:
    static int instance_count;

    init_test_resource() { instance_count++; }

    ~init_test_resource() { instance_count--; }

    void setValue(int value) { value_ = value; }

    int getValue() const { return value_; }

  private:
    int value_ = 0;
};

int init_test_resource::instance_count = 0;

static std::unique_ptr<init_test_resource> global_resource;
INIT(global_resource = std::make_unique<init_test_resource>();
     global_resource->setValue(42););

TEST(init, global_init_execution)
{
    EXPECT_EQ(global_init_counter, 13); // 3 + 10 = 13
    EXPECT_EQ(global_init_order.size(), 3);
    EXPECT_EQ(global_init_order[0], 1);
    EXPECT_EQ(global_init_order[1], 2);
    EXPECT_EQ(global_init_order[2], 3);
}

TEST(init, atomic_initialization)
{
    EXPECT_EQ(atomic_counter.load(), 2);
}

TEST(init, global_resource_initialization)
{
    ASSERT_NE(global_resource, nullptr);
    EXPECT_EQ(global_resource->getValue(), 42);
    EXPECT_EQ(init_test_resource::instance_count, 1);
}

TEST(init, direct_init_class_usage)
{
    int counter = 0;

    {
        hj::init init_obj([&counter]() { counter = 100; });

        EXPECT_EQ(counter, 100);
    }

    EXPECT_EQ(counter, 100);
}

TEST(init, multiple_init_objects)
{
    std::vector<int> execution_order;

    {
        hj::init init1([&execution_order]() { execution_order.push_back(1); });

        hj::init init2([&execution_order]() { execution_order.push_back(2); });

        hj::init init3([&execution_order]() { execution_order.push_back(3); });

        ASSERT_EQ(execution_order.size(), 3);
        EXPECT_EQ(execution_order[0], 1);
        EXPECT_EQ(execution_order[1], 2);
        EXPECT_EQ(execution_order[2], 3);
    }
}

TEST(init, exception_in_init)
{
    int  counter          = 0;
    bool exception_caught = false;

    {
        hj::init init_obj([&counter, &exception_caught]() {
            counter = 1;
            try
            {
                throw std::runtime_error("test exception");
            }
            catch(...)
            {
                exception_caught = true;
                counter          = 2;
            }
        });

        EXPECT_EQ(counter, 2);
        EXPECT_TRUE(exception_caught);
    }
}

TEST(init, complex_initialization)
{
    struct ComplexData
    {
        std::vector<std::string> strings;
        std::vector<int>         numbers;
        bool                     initialized = false;
    };

    ComplexData data;

    {
        hj::init complex_init([&data]() {
            data.strings = {"hello", "world", "init"};
            data.numbers = {1, 2, 3, 4, 5};

            for(int i = 0; i < 10; ++i)
            {
                data.numbers.push_back(i * i);
            }

            data.initialized = true;
        });

        EXPECT_TRUE(data.initialized);
        EXPECT_EQ(data.strings.size(), 3);
        EXPECT_EQ(data.numbers.size(), 15); // 5 + 10
        EXPECT_EQ(data.strings[0], "hello");
        EXPECT_EQ(data.numbers[5], 0);  // 0*0
        EXPECT_EQ(data.numbers[9], 16); // 4*4
    }
}

TEST(init, resource_management)
{
    int resource_counter = 0;

    {
        hj::init resource_init([&resource_counter]() {
            auto resource    = std::make_unique<init_test_resource>();
            resource_counter = init_test_resource::instance_count;
        });

        EXPECT_GE(resource_counter, 1);
    }
}

TEST(init, move_semantics)
{
    int counter = 0;

    {
        auto lambda = [&counter]() { counter = 999; };

        hj::init init_obj(std::move(lambda));
        EXPECT_EQ(counter, 999);
    }
}

TEST(init, empty_callback)
{
    bool no_crash = true;

    {
        std::function<void()> empty_func;
    }

    EXPECT_TRUE(no_crash);
}

TEST(init, init_with_other_objects)
{
    struct TestObject
    {
        int  value       = 0;
        bool initialized = false;

        void initialize()
        {
            value       = 123;
            initialized = true;
        }
    };

    TestObject obj;

    {
        hj::init obj_init([&obj]() { obj.initialize(); });

        EXPECT_TRUE(obj.initialized);
        EXPECT_EQ(obj.value, 123);
    }
}

class InitTestCleanup : public ::testing::Test
{
  protected:
    void TearDown() override
    {
        if(global_resource)
        {
            EXPECT_EQ(global_resource->getValue(), 42);
        }
    }
};

TEST_F(InitTestCleanup, GlobalResourcePersistence)
{
    ASSERT_NE(global_resource, nullptr);
    EXPECT_EQ(global_resource->getValue(), 42);
}