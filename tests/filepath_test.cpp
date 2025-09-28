#include <gtest/gtest.h>
#include <hj/io/filepath.hpp>

TEST(file_path, pwd)
{
    ASSERT_EQ(hj::filepath::pwd().empty(), false);
}

TEST(file_path, parent)
{
    ASSERT_STREQ(hj::filepath::parent("/usr/local/src").c_str(), "/usr/local");
}

TEST(file_path, absolute)
{
#if !defined(_WIN32)
    ASSERT_STREQ(hj::filepath::absolute("/usr/local/src").c_str(),
                 "/usr/local/src");
#endif

    ASSERT_STREQ(hj::filepath::absolute("./007.txt").c_str(),
                 hj::filepath::join(hj::filepath::pwd(), "./007.txt").c_str());
}

TEST(file_path, relative)
{
    std::string pwd = hj::filepath::pwd();

#if defined(_WIN32)
    ASSERT_STREQ(hj::filepath::relative(pwd + "\\007.txt", pwd).c_str(),
                 "007.txt");
    ASSERT_STREQ(
        hj::filepath::relative(pwd + "\\..\\src\\007.txt", pwd).c_str(),
        "..\\src\\007.txt");
#else
    ASSERT_STREQ(hj::filepath::relative(pwd + "/007.txt", pwd).c_str(),
                 "007.txt");
    ASSERT_STREQ(hj::filepath::relative(pwd + "/../src/007.txt", pwd).c_str(),
                 "../src/007.txt");
#endif
    std::string sibling = hj::filepath::join(pwd, "sibling.txt");
    ASSERT_STREQ(hj::filepath::relative(sibling, pwd).c_str(), "sibling.txt");
}

TEST(file_path, join)
{
#if defined(_WIN32)
    ASSERT_STREQ(hj::filepath::join("/usr/local/src", "007.txt").c_str(),
                 "/usr/local/src\\007.txt");
    std::vector<std::string> args{"/usr/local/src", "007.txt"};
    ASSERT_STREQ(hj::filepath::join(args).c_str(), "/usr/local/src\\007.txt");
#else
    ASSERT_STREQ(hj::filepath::join("/usr/local/src", "007.txt").c_str(),
                 "/usr/local/src/007.txt");
    std::vector<std::string> args{"/usr/local/src", "007.txt"};
    ASSERT_STREQ(hj::filepath::join(args).c_str(), "/usr/local/src/007.txt");
#endif
}

TEST(file_path, file_name)
{
    ASSERT_STREQ(hj::filepath::file_name("/usr/local/src/007.txt").c_str(),
                 "007.txt");
}

TEST(file_path, dir_name)
{
    ASSERT_STREQ(hj::filepath::dir_name("/usr/local/src/007.txt").c_str(),
                 "src");
}

TEST(file_path, path_name)
{
    ASSERT_STREQ(hj::filepath::path_name("/usr/local/src/007.txt").c_str(),
                 "/usr/local/src");
}

TEST(file_path, extension)
{
    ASSERT_STREQ(hj::filepath::extension("/usr/local/src/007.txt").c_str(),
                 ".txt");
}

TEST(file_path, replace_extension)
{
    ASSERT_STREQ(
        hj::filepath::replace_extension("/usr/local/src/007.txt", ".exe")
            .c_str(),
        "/usr/local/src/007.exe");
}

TEST(file_path, is_dir)
{
    ASSERT_EQ(hj::filepath::is_dir(hj::filepath::pwd()), true);
    ASSERT_EQ(hj::filepath::is_dir("/usr/local/src/007.txt"), false);
}

TEST(file_path, is_symlink)
{
    ASSERT_EQ(hj::filepath::is_symlink(hj::filepath::pwd()), false);
}

TEST(file_path, is_exist)
{
    ASSERT_EQ(hj::filepath::is_exist(hj::filepath::pwd()), true);
}

TEST(file_path, last_mod_time)
{
    ASSERT_EQ(hj::filepath::last_mod_time(hj::filepath::pwd()) > 0, true);
}

TEST(file_path, size)
{
    ASSERT_EQ(hj::filepath::size(hj::filepath::pwd()) == -1, true);

    std::string f =
        hj::filepath::join(hj::filepath::pwd(), "file_path_size_test.txt");
    if(!hj::filepath::is_exist(f))
    {
        ASSERT_EQ(hj::filepath::make_file(f), true);
    }

    ASSERT_EQ(hj::filepath::size(f) >= 0, true);
}

TEST(file_path, walk)
{
    int n = 0;
    hj::filepath::walk(hj::filepath::pwd(), [&](const std::string &f) -> bool {
        (void) f;
        n++;
        return true;
    });
    ASSERT_EQ(n > 0, true);
}

TEST(file_path, list)
{
    ASSERT_EQ(hj::filepath::list(hj::filepath::pwd()).size() > 0, true);
}

TEST(file_path, find)
{
    std::string f =
        hj::filepath::join(hj::filepath::pwd(), "file_path_find_test.txt");
    if(!hj::filepath::is_exist(f))
    {
        ASSERT_EQ(hj::filepath::make_file(f), true);
    }

    ASSERT_EQ(hj::filepath::find(hj::filepath::pwd(), "file_path_find_test.txt")
                      .size()
                  > 0,
              true);
}

TEST(file_path, make_dir)
{
    std::string path = hj::filepath::join(hj::filepath::pwd(), "tmp");
    ASSERT_EQ(
        (hj::filepath::is_exist(path) ? true : hj::filepath::make_dir(path)),
        true);
}

TEST(file_path, make_file)
{
    std::string f = hj::filepath::join(hj::filepath::pwd(), "007.txt");
    ASSERT_EQ((hj::filepath::is_exist(f) ? true : hj::filepath::make_file(f)),
              true);
}

TEST(file_path, copy_dir)
{
    std::string from = hj::filepath::join(hj::filepath::pwd(), "tmp");
    std::string to   = hj::filepath::join(hj::filepath::pwd(), "tmp1");
    ASSERT_EQ((hj::filepath::is_exist(from) && (!hj::filepath::is_exist(to))
                   ? hj::filepath::copy_dir(from, to)
                   : true),
              true);
}

TEST(file_path, copy_file)
{
    std::string from = hj::filepath::join(hj::filepath::pwd(), "007.txt");
    std::string to   = hj::filepath::join(hj::filepath::pwd(), "008.txt");
    ASSERT_EQ((hj::filepath::is_exist(from) && (!hj::filepath::is_exist(to))
                   ? hj::filepath::copy_file(from, to)
                   : true),
              true);
}

TEST(file_path, remove)
{
    std::string f = hj::filepath::join(hj::filepath::pwd(), "007.txt");
    ASSERT_EQ((hj::filepath::is_exist(f) ? hj::filepath::remove(f) : true),
              true);
}

TEST(file_path, rename)
{
    std::string from = hj::filepath::join(hj::filepath::pwd(), "007.txt");
    std::string to   = hj::filepath::join(hj::filepath::pwd(), "008.txt");
    ASSERT_STREQ((hj::filepath::is_exist(from) && (!hj::filepath::is_exist(to))
                      ? hj::filepath::rename(from, to).c_str()
                      : to.c_str()),
                 to.c_str());
}
