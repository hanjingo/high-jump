#include <gtest/gtest.h>
#include <libcpp/encoding/json.hpp>

struct json_obj
{
    std::string name;
    int age;
    std::string email;
};

TEST (json, get)
{
    json_obj obj;
    auto js = libcpp::json{{"name", "harry potter"},
                           {"age", 30},
                           {"email", "hehehunanchina@live.com"}};
    js.at ("name").get_to (obj.name);
    ASSERT_EQ (obj.name, std::string ("harry potter"));
    ASSERT_EQ (js.at ("name").get<std::string> (),
               std::string ("harry potter"));

    js.at ("age").get_to (obj.age);
    ASSERT_EQ (obj.age, 30);
    ASSERT_EQ (js.at ("age").get<int> (), 30);

    ASSERT_STREQ (js.at ("email").get<std::string> ().c_str (),
                  "hehehunanchina@live.com");
}

TEST (json, parse)
{
    // from string
    auto js1 = libcpp::json::parse (R"(
        {
            "pi": 3.141,
            "happy": true
        }
    )");

    ASSERT_EQ (js1.contains ("pi"), true);
    ASSERT_EQ (js1["pi"].get<double> (), 3.141);
    ASSERT_EQ (js1.contains ("happy"), true);
    ASSERT_EQ (js1["happy"].get<bool> (), true);

    // from ifstream
    std::ifstream f ("./json_test.json");
    auto js2 = libcpp::json::parse (f);

    ASSERT_EQ (js2.contains ("pi"), true);
    ASSERT_EQ (js2["pi"].get<double> (), 3.141);
    ASSERT_EQ (js2.contains ("happy"), true);
    ASSERT_EQ (js2["happy"].get<bool> (), true);
}

TEST (json, make_object)
{
}

TEST (json, make_array)
{
}