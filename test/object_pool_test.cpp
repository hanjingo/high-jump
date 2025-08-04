#include <string>

#include <gtest/gtest.h>
#include <libcpp/sync/object_pool.hpp>

struct worker
{
    worker(std::string _name, int _age, double _salary, std::string _email)
        : name{ _name }, age{ _age }, salary{ _salary }, email{ _email }
    {
    }
    ~worker() {}

    std::string name;
    int age;
    double salary;
    std::string email;
};

struct job
{
    job(std::string str) : desc{ str } {}
    ~job() {}

    std::string desc;
};

TEST(object_pool, construct)
{
    libcpp::object_pool<worker> pool1;
    pool1.construct("harry", 30, 10000.0, "hehehunanchina@live.com");
    auto obj1 = pool1.pop();
    ASSERT_EQ(obj1 != nullptr, true);
    ASSERT_EQ(obj1->name == std::string("harry"), true);
    ASSERT_EQ(obj1->age == 30, true);
    ASSERT_EQ(obj1->salary == 10000.0, true);
    ASSERT_EQ(obj1->email == std::string("hehehunanchina@live.com"), true);
    pool1.push(obj1);

    libcpp::object_pool<job> pool2;
    pool2.construct("do something");
    job* obj2 = pool2.pop();
    ASSERT_EQ(obj2 != nullptr, true);
    ASSERT_EQ(obj2->desc == std::string("do something"), true);
    pool2.push(obj2);
}

TEST(object_pool, push) {}

TEST(object_pool, pop) {}

TEST(object_pool, size) {}