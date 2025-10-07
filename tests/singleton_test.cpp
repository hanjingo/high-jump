#include <gtest/gtest.h>
#include <hj/types/singleton.hpp>

static int singleton_num = 0;

class A
{
  public:
    static void static_inc() { singleton_num += 1; }
    void        inc() { _tmp += 2; }
    int         value() const { return _tmp; }

  private:
    int _tmp = 0;
};

TEST(singleton, boost_singleton_basic)
{
    hj::singleton<A>::get_const_instance().static_inc();
    ASSERT_EQ(singleton_num, 1);
    hj::singleton<A>::get_mutable_instance().inc();
    ASSERT_EQ(singleton_num, 1);
    ASSERT_EQ(hj::singleton<A>::get_mutable_instance().value(), 2);
}
