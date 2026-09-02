#include <gtest/gtest.h>
#include <hj/encoding/ini.hpp>

TEST(ini, parse)
{
    char text[] = "[section1] \na=123 \nb=abc \nc=123.456";
    auto ini    = hj::ini::try_parse(text);
    ASSERT_TRUE(ini.has_value());

    auto sec1 = ini->try_get_child("section1");
    ASSERT_TRUE(sec1.has_value());

    ASSERT_EQ(sec1->get<int>("a", 0), 123);
    ASSERT_EQ(sec1->get<std::string>("b", ""), "abc");
    ASSERT_FLOAT_EQ(sec1->get<float>("c", 0.0f), 123.456f);

    char text1[] = "[section1] \nserver.ip = 192.168.0.1 \nserver.port = 8080";
    auto ini1    = hj::ini::try_parse(text1);
    ASSERT_TRUE(ini1.has_value());
    ASSERT_FALSE(ini1->get<std::string>("section1.server.ip", "")
                 == "192.168.0.1");
    ASSERT_TRUE(ini1->get<std::string>("section1/server.ip", "")
                == "192.168.0.1");
}

TEST(ini, get_set)
{
    hj::ini ini;
    ini.put("section1/a", 42);
    ini.put("section1/b", "hello");
    ini.put("section1/c", 3.14);

    ASSERT_EQ(ini.get<int>("section1/a", 0), 42);
    ASSERT_EQ(ini.get<std::string>("section1/b", ""), "hello");
    ASSERT_DOUBLE_EQ(ini.get<double>("section1/c", 0.0), 3.14);
    ASSERT_EQ(ini.get<int>("section1/not_exist", 99), 99);
}

TEST(ini, set_method)
{
    hj::ini ini;
    ini.put("section2/x", 100);
    ini.put("section2/y.z", std::string("world"));

    ASSERT_TRUE(ini.get<int>("section2/x", 0) == 100);
    ASSERT_TRUE(ini.get<std::string>("section2/y.z", "") == "world");
}

TEST(ini, str_and_parse_roundtrip)
{
    hj::ini ini;
    ini.put("person/name", "hanjingo");
    ini.put("person/age", 30);
    ini.put("person/income", 10000.123);

    std::string s    = ini.str().value_or("");
    auto        ini2 = hj::ini::try_parse(s.c_str());
    ASSERT_TRUE(ini2.has_value());

    ASSERT_TRUE(ini2->get<std::string>("person/name", "") == "hanjingo");
    ASSERT_EQ(ini2->get<int>("person/age", 0), 30);
    ASSERT_FLOAT_EQ(ini2->get<float>("person/income", 0.0f), 10000.123f);
}

TEST(ini, read_file)
{
    char text[] = "[person] \nname=hanjingo \nage=30 \nincome=10000.123";
    auto cfg1   = hj::ini::try_parse(text);
    ASSERT_TRUE(cfg1.has_value());

    ASSERT_EQ(cfg1->write_file("cfg1.ini"), hj::ini_errc::success);

    hj::ini cfg2;
    ASSERT_EQ(cfg2.read_file("cfg1.ini"), hj::ini_errc::success);
    ASSERT_EQ(cfg2.get<std::string>("person/name", ""), "hanjingo");
    ASSERT_EQ(cfg2.get<int>("person/age", 0), 30);
    ASSERT_FLOAT_EQ(cfg2.get<float>("person/income", 0.0f), 10000.123f);
}

TEST(ini, write_file)
{
    char text[] = "[person] \nname=hanjingo \nage=30 \nincome=10000.123";
    auto cfg1   = hj::ini::try_parse(text);
    ASSERT_TRUE(cfg1.has_value());

    cfg1->put("person/email", "hehehunanchina@live.com");
    ASSERT_EQ(cfg1->write_file("cfg2.ini"), hj::ini_errc::success);

    hj::ini cfg2;
    ASSERT_EQ(cfg2.read_file("cfg2.ini"), hj::ini_errc::success);
    ASSERT_EQ(cfg2.get<std::string>("person/email", ""),
              "hehehunanchina@live.com");
}

TEST(ini, str)
{
    char text[] = "[person] \nname=hanjingo";
    auto cfg1   = hj::ini::try_parse(text);
    ASSERT_TRUE(cfg1.has_value());

    cfg1->put("person/email", "hehehunanchina@live.com");

    auto str_val = cfg1->str();
    ASSERT_TRUE(str_val.has_value());
    auto cfg2 = hj::ini::try_parse(str_val.value());
    ASSERT_TRUE(cfg2.has_value());
    ASSERT_EQ(cfg2->get<std::string>("person/email", ""),
              "hehehunanchina@live.com");
}

TEST(ini, type_conversion_failure)
{
    char text[] = "[section]\nport=abc";
    auto ini    = hj::ini::try_parse(text);
    ASSERT_TRUE(ini.has_value());

    int port = ini->get<int>("section/port", 8080);
    ASSERT_EQ(port, 8080);

    std::error_code get_ec;
    auto            val = ini->get<int>("section/port", get_ec);
    ASSERT_FALSE(val.has_value());
    ASSERT_EQ(get_ec, hj::ini_errc::bad_data_error);
}

TEST(ini, empty_values)
{
    char text[] = "[section]\nkey=\nint_key=";
    auto ini    = hj::ini::try_parse(text);
    ASSERT_TRUE(ini.has_value());

    std::string s_val = ini->get<std::string>("section/key", "default_str");
    int         i_val = ini->get<int>("section/int_key", 42);
    ASSERT_EQ(i_val, 42);
}

TEST(ini, syntax_edge_cases)
{
    const char *complex_ini = "; This is a semicolon comment line\n"
                              "[section_with_spaces]\n"
                              "key_with_spaces = value_with_spaces\n"
                              "[special_chars]\n"
                              "path = C:\\Windows\\System32\n"
                              "url = https://example.com?query=1&foo=bar\n";

    std::error_code ec;
    auto            ini = hj::ini::parse(complex_ini, ec);
    ASSERT_TRUE(ini.has_value());
    ASSERT_EQ(ec, hj::ini_errc::success);

    ASSERT_EQ(ini->get<std::string>("section_with_spaces/key_with_spaces", ""),
              "value_with_spaces");

    ASSERT_EQ(ini->get<std::string>("special_chars/path", ""),
              "C:\\Windows\\System32");
    ASSERT_EQ(ini->get<std::string>("special_chars/url", ""),
              "https://example.com?query=1&foo=bar");
}

TEST(ini, duplicate_keys_error)
{
    const char *duplicate_ini = "[section]\n"
                                "k = first\n"
                                "k = second\n";

    std::error_code ec;
    auto            ini = hj::ini::parse(duplicate_ini, ec);
    ASSERT_FALSE(ini.has_value());
    ASSERT_EQ(ec, hj::ini_errc::parser_error);
}

TEST(ini, empty_file_and_section)
{
    auto ini_empty = hj::ini::try_parse("");
    ASSERT_TRUE(ini_empty.has_value());
    ASSERT_TRUE(ini_empty->empty());

    const char *empty_sec = "[empty_section]";
    auto        ini_sec   = hj::ini::try_parse(empty_sec);
    ASSERT_TRUE(ini_sec.has_value());
    ASSERT_TRUE(ini_sec->empty());
}

TEST(ini, filesystem_errors)
{
    hj::ini ini;
    ASSERT_EQ(ini.read_file("not_exist_file.ini"),
              hj::ini_errc::file_not_found);

    ASSERT_NE(ini.read_file(""), hj::ini_errc::success);
    ASSERT_NE(ini.read_file(static_cast<const char *>(nullptr)),
              hj::ini_errc::success);

    std::filesystem::path current_dir = std::filesystem::current_path();
    std::error_code       dir_ec      = ini.read_file(current_dir);
    ASSERT_EQ(dir_ec, hj::ini_errc::filesystem_error);

    std::error_code write_ec = ini.write_file(current_dir);
    ASSERT_NE(write_ec, hj::ini_errc::success);
}

TEST(ini, unicode_and_paths)
{
    std::string unicode_ini = "[user]\n"
                              "name=张三\n"
                              "[config]\n"
                              "path=C:\\中文\\配置\n";

    std::error_code ec;
    auto            ini = hj::ini::parse(unicode_ini, ec);
    ASSERT_TRUE(ini.has_value());
    ASSERT_EQ(ec, hj::ini_errc::success);

    ASSERT_EQ(ini->get<std::string>("user/name", ""), "张三");
    ASSERT_EQ(ini->get<std::string>("config/path", ""), "C:\\中文\\配置");
}

TEST(ini, smoke_extra_long_value)
{
    std::string large_val(1024 * 1024, 'X');
    std::string ini_content = "[limits]\nkey=" + large_val + "\n";

    std::error_code ec;
    auto            ini = hj::ini::parse(ini_content, ec);
    ASSERT_TRUE(ini.has_value());
    ASSERT_EQ(ec, hj::ini_errc::success);

    auto result_val = ini->get<std::string>("limits/key", "");
    ASSERT_EQ(result_val.size(), 1024 * 1024);
    ASSERT_EQ(result_val[0], 'X');
    ASSERT_EQ(result_val.back(), 'X');
}

TEST(ini, smoke_performance_large_key_count)
{
    hj::ini   ini;
    const int num_keys = 10000;
    for(int i = 0; i < num_keys; ++i)
    {
        std::string path = "perf_section/key_" + std::to_string(i);
        ini.put(path, i);
    }

    auto opt_str = ini.str();
    ASSERT_TRUE(opt_str.has_value());
    ASSERT_FALSE(opt_str->empty());

    ASSERT_EQ(ini.get<int>("perf_section/key_0", -1), 0);
    ASSERT_EQ(ini.get<int>("perf_section/key_9999", -1), 9999);

    auto ini_parsed = hj::ini::try_parse(opt_str.value());
    ASSERT_TRUE(ini_parsed.has_value());
    ASSERT_EQ(ini_parsed->get<int>("perf_section/key_5000", -1), 5000);
}

TEST(ini, error_code_initial_state_contract)
{
    std::error_code ec = hj::ini_errc::bad_path_error;

    const char *valid_text = "[section]\nkey=value";
    auto        ini        = hj::ini::parse(valid_text, ec);

    ASSERT_TRUE(ini.has_value());
    ASSERT_FALSE(ec);
    ASSERT_EQ(ec, hj::ini_errc::success);
}

TEST(ini, error_handling)
{
    hj::ini ini;
    ASSERT_EQ(ini.read_file("not_exist_file.ini"),
              hj::ini_errc::file_not_found);
    ASSERT_EQ(ini.get<int>("no.such.key", 12345), 12345);

    const char *valid_ini = "[[[malformed_section_syntax]]";
    auto        ini2      = hj::ini::try_parse(valid_ini);
    ASSERT_TRUE(ini2.has_value());

    {
        std::error_code ec;
        const char     *bad_ini1 = "[unclosed_section";
        auto            ret      = hj::ini::parse(bad_ini1, ec);
        ASSERT_FALSE(ret.has_value());
        ASSERT_EQ(ec, hj::ini_errc::parser_error);

        const char *valid_ini_inner = "unclosed_section = [section";
        ret                         = hj::ini::parse(valid_ini_inner, ec);
        ASSERT_TRUE(ret.has_value());
        ASSERT_EQ(ec, hj::ini_errc::success);
    }

    {
        std::error_code ec;
        const char     *bad_ini2 = "just_a_bare_word_without_equals";
        auto            ret      = hj::ini::parse(bad_ini2, ec);
        ASSERT_FALSE(ret.has_value());
        ASSERT_EQ(ec, hj::ini_errc::parser_error);
    }
}