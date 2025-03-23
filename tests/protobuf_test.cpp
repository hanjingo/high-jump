#include <gtest/gtest.h>
// #include <libcpp/encoding/protobuf.hpp>
// #include "person.pb.h"

// TEST(protobuf, protobuf)
// {
//     test::Person p;
//     p.set_name("test");
//     p.set_id(100);
//     p.set_email("hehehunanchina@live.com");

//     std::string buf;
//     p.SerializeToString(&buf);

//     test::Person p1;
//     p1.ParseFromString(buf);
//     ASSERT_EQ(p1.name() == std::string("test"), true);
//     ASSERT_EQ(p1.email() == std::string("hehehunanchina@live.com"), true);
//     ASSERT_EQ(p1.id() == 100, true);
// }