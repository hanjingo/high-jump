#include <gtest/gtest.h>
#include <libcpp/sync/object_pool.hpp>

#include <string>

class worker
{
public:
    worker(std::string _name, int _age, float _salary, std::string _email)
        : name{_name}, age{_age}, salary{_salary}, email{_email}
    {}
    ~worker(){}

    std::string name;
    int age;
    float salary;
    std::string email;
};

struct c_worker
{
    const char* name;
    int* age;
    float* salary;
    const char* email;
};

TEST(object_pool, construct)
{
    libcpp::object_pool<worker> pool;
    pool.construct("harry", 30, 10000.0, "hehehunanchina@live.com");
    auto obj1 = pool.pop();
    ASSERT_EQ(obj1 != nullptr, true);
    ASSERT_EQ(obj1->name == std::string("harry"), true);
    ASSERT_EQ(obj1->age == 30, true);
    ASSERT_EQ(obj1->salary == 10000.0, true);
    ASSERT_EQ(obj1->email == std::string("hehehunanchina@live.com"), true);
    pool.push(obj1);

    libcpp::object_pool<c_worker> cpool;
    c_worker* cw = cpool.allocate();
    cw->name = "lucy";
    cw->age = new int(18);
    cw->salary = new float(5000.0f);
    cw->email = "lucy@abc.com";
    cpool.push(cw);
    auto cw2 = cpool.pop();
    ASSERT_STREQ(cw2->name, "lucy");
    ASSERT_EQ(*cw2->age, 18);
    ASSERT_FLOAT_EQ(*cw2->salary, 5000.0f);
    ASSERT_STREQ(cw2->email, "lucy@abc.com");
    cpool.push(cw2);
}

TEST(object_pool, push)
{
    libcpp::object_pool<worker> pool;
    pool.construct("tom", 25, 8000.0, "tom@abc.com");
    auto obj = pool.pop();
    ASSERT_NE(obj, nullptr);
    obj->age = 26;
    pool.push(obj);
    auto obj2 = pool.pop();
    ASSERT_EQ(obj2->age, 26);
    pool.push(obj2);

    libcpp::object_pool<c_worker> cpool;
    c_worker* cw = cpool.allocate();
    cw->name = "lucy";
    cw->age = new int(18);
    cw->salary = new float(5000.0f);
    cw->email = "lucy@abc.com";
    cpool.push(cw);
    auto cw2 = cpool.pop();
    ASSERT_STREQ(cw2->name, "lucy");
    ASSERT_EQ(*cw2->age, 18);
    ASSERT_FLOAT_EQ(*cw2->salary, 5000.0f);
    ASSERT_STREQ(cw2->email, "lucy@abc.com");
    cpool.push(cw2);
}

TEST(object_pool, pop)
{
    libcpp::object_pool<worker> pool;
    pool.construct("jack", 40, 12000.0, "jack@abc.com");
    auto obj = pool.pop();
    ASSERT_NE(obj, nullptr);
    ASSERT_EQ(obj->name, "jack");
    pool.push(obj);

    libcpp::object_pool<c_worker> cpool;
    c_worker* cw = cpool.allocate();
    cw->name = "bob";
    cw->age = new int(22);
    cw->salary = new float(6000.0f);
    cw->email = "bob@abc.com";
    cpool.push(cw);
    auto cw2 = cpool.pop();
    ASSERT_NE(cw2, nullptr);
    ASSERT_STREQ(cw2->name, "bob");
    cpool.push(cw2);
}

TEST(object_pool, size)
{
    libcpp::object_pool<worker> pool;
    ASSERT_EQ(pool.size(), 0);
    pool.construct("alice", 35, 9000.0, "alice@abc.com");
    ASSERT_EQ(pool.size(), 1);
    auto obj = pool.pop();
    ASSERT_EQ(pool.size(), 0);
    pool.push(obj);
    ASSERT_EQ(pool.size(), 1);

    libcpp::object_pool<c_worker> cpool;
    ASSERT_EQ(cpool.size(), 0);
    c_worker* cw = cpool.allocate();
    cw->name = "eve";
    cw->age = new int(28);
    cw->salary = new float(7000.0f);
    cw->email = "eve@abc.com";
    cpool.push(cw);
    ASSERT_EQ(cpool.size(), 1);
    auto cw2 = cpool.pop();
    ASSERT_EQ(cpool.size(), 0);
    cpool.push(cw2);
    ASSERT_EQ(cpool.size(), 1);
}