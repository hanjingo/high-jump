#include <gtest/gtest.h>
#include <hj/types/singleton.hpp>

static int singleton_num = 0;

class A
{
public:
    void static static_inc()
    {
        singleton_num += 1;
    }
    void inc()
    {
        _tmp += 2;
    }
    int value() { return _tmp; }

private:
    int _tmp = 0;
};


class B : public hj::singleton<B>
{
public:
    void static static_inc()
    {
        singleton_num += 10;
    }
    void inc()
    {
        _tmp += 100;
    }
    int value() { return _tmp; }

private:
    int _tmp = 0;
};

TEST(singleton, singleton)
{
    hj::singleton<A>::get_const_instance().static_inc();
    ASSERT_EQ(singleton_num, 1);

    hj::singleton<A>::get_mutable_instance()->inc();
    ASSERT_EQ(singleton_num, 1);
    ASSERT_EQ(hj::singleton<A>::get_mutable_instance()->value(), 2);

    B b;
    b.get_const_instance().static_inc();
    ASSERT_EQ(singleton_num, 11);

    b.get_mutable_instance()->inc();
    ASSERT_EQ(b.get_mutable_instance()->value(), 100);
}