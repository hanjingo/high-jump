#include <gtest/gtest.h>
#include <libcpp/io/filepath.hpp>

TEST(file_path, pwd)
{
    ASSERT_EQ(libcpp::filepath::pwd().empty(), false);
}

TEST(file_path, parent)
{
    ASSERT_STREQ(
        libcpp::filepath::parent("/usr/local/src").c_str(), 
        "/usr/local"
    );
}

TEST(file_path, absolute)
{
#if !defined(_WIN32)
    ASSERT_STREQ(
        libcpp::filepath::absolute("/usr/local/src").c_str(), 
        "/usr/local/src"
    );
#endif

    ASSERT_STREQ(
        libcpp::filepath::absolute("./007.txt").c_str(), 
        libcpp::filepath::join(libcpp::filepath::pwd(), "./007.txt").c_str()
    );
}

TEST(file_path, relative)
{
#if defined(_WIN32)
    ASSERT_STREQ(
        libcpp::filepath::relative("/usr/local/src/007.txt").c_str(), 
        "..\\..\\..\\usr\\local\\src\\007.txt"
    );
#else
    ASSERT_STREQ(
        libcpp::filepath::relative("/usr/local/src/007.txt").c_str(),
        "../../../../007.txt"
    );
#endif
}

TEST(file_path, join)
{
#if defined(_WIN32)
    ASSERT_STREQ(
        libcpp::filepath::join("/usr/local/src", "007.txt").c_str(), 
        "/usr/local/src\\007.txt"
    );
    std::vector<std::string> args{"/usr/local/src", "007.txt"};
    ASSERT_STREQ(
        libcpp::filepath::join(args).c_str(), 
        "/usr/local/src\\007.txt"
    );
#else
    ASSERT_STREQ(
        libcpp::filepath::join("/usr/local/src", "007.txt").c_str(),
        "/usr/local/src/007.txt"
    );
    std::vector<std::string> args{ "/usr/local/src", "007.txt" };
    ASSERT_STREQ(
        libcpp::filepath::join(args).c_str(),
        "/usr/local/src/007.txt"
    );
#endif
}

TEST(file_path, file_name)
{
    ASSERT_STREQ(
        libcpp::filepath::file_name("/usr/local/src/007.txt").c_str(), 
        "007.txt"
    );
}

TEST(file_path, dir_name)
{
    ASSERT_STREQ(
        libcpp::filepath::dir_name("/usr/local/src/007.txt").c_str(), 
        "src"
    );
}

TEST(file_path, path_name)
{
    ASSERT_STREQ(
        libcpp::filepath::path_name("/usr/local/src/007.txt").c_str(), 
        "/usr/local/src"
    );
}

TEST(file_path, extension)
{
    ASSERT_STREQ(
        libcpp::filepath::extension("/usr/local/src/007.txt").c_str(), 
        ".txt"
    );
}

TEST(file_path, replace_extension)
{
    ASSERT_STREQ(
        libcpp::filepath::replace_extension("/usr/local/src/007.txt", ".exe").c_str(), 
        "/usr/local/src/007.exe"
    );
}

TEST(file_path, is_dir)
{
    ASSERT_EQ(libcpp::filepath::is_dir(libcpp::filepath::pwd()), true);
    ASSERT_EQ(libcpp::filepath::is_dir("/usr/local/src/007.txt"), false);
}

TEST(file_path, is_symlink)
{
    ASSERT_EQ(libcpp::filepath::is_symlink(libcpp::filepath::pwd()), false);
}

TEST(file_path, is_exist)
{
    ASSERT_EQ(libcpp::filepath::is_exist(libcpp::filepath::pwd()), true);
}

TEST(file_path, last_mod_time)
{
    ASSERT_EQ(libcpp::filepath::last_mod_time(libcpp::filepath::pwd()) > 0, true);
}

TEST(file_path, size)
{
    ASSERT_EQ(libcpp::filepath::size(libcpp::filepath::pwd()) == -1, true);

    std::string f = libcpp::filepath::join(libcpp::filepath::pwd(), "file_path_size_test.txt");
    if (!libcpp::filepath::is_exist(f))
        ASSERT_EQ(libcpp::filepath::make_file(f), true);

    ASSERT_EQ(libcpp::filepath::size(f) >= 0, true);
}

TEST(file_path, walk)
{
    int n = 0;
    libcpp::filepath::walk(libcpp::filepath::pwd(), [&](const std::string& f)->bool{
        n++;
        return true;
    });
    ASSERT_EQ(n > 0, true);
}

TEST(file_path, list)
{
    ASSERT_EQ(libcpp::filepath::list(libcpp::filepath::pwd()).size() > 0, true);
}

TEST(file_path, find)
{
    std::string f = libcpp::filepath::join(libcpp::filepath::pwd(), "file_path_find_test.txt");
    if (!libcpp::filepath::is_exist(f))
        ASSERT_EQ(libcpp::filepath::make_file(f), true);

    ASSERT_EQ(libcpp::filepath::find(libcpp::filepath::pwd(), "file_path_find_test.txt").size() > 0, true);
}

TEST(file_path, make_dir)
{
    std::string path = libcpp::filepath::join(libcpp::filepath::pwd(), "tmp");
    ASSERT_EQ(
        (libcpp::filepath::is_exist(path) ? true : libcpp::filepath::make_dir(path)), 
        true
    );
}

TEST(file_path, make_file)
{
    std::string f = libcpp::filepath::join(libcpp::filepath::pwd(), "007.txt");
    ASSERT_EQ(
        (libcpp::filepath::is_exist(f) ? true : libcpp::filepath::make_file(f)), 
        true
    );
}

TEST(file_path, copy_dir)
{
    std::string from = libcpp::filepath::join(libcpp::filepath::pwd(), "tmp");
    std::string to = libcpp::filepath::join(libcpp::filepath::pwd(), "tmp1");
    ASSERT_EQ(
        (libcpp::filepath::is_exist(from)&&(!libcpp::filepath::is_exist(to)) ? libcpp::filepath::copy_dir(from, to) : true), 
        true
    );
}

TEST(file_path, copy_file)
{
    std::string from = libcpp::filepath::join(libcpp::filepath::pwd(), "007.txt");
    std::string to = libcpp::filepath::join(libcpp::filepath::pwd(), "008.txt");
    ASSERT_EQ(
        (libcpp::filepath::is_exist(from)&&(!libcpp::filepath::is_exist(to)) ? libcpp::filepath::copy_file(from, to) : true), 
        true
    );
}

TEST(file_path, remove)
{
    std::string f = libcpp::filepath::join(libcpp::filepath::pwd(), "007.txt");
    ASSERT_EQ(
        (libcpp::filepath::is_exist(f) ? libcpp::filepath::remove(f) : true), 
        true
    );
}

TEST(file_path, rename)
{
    std::string from = libcpp::filepath::join(libcpp::filepath::pwd(), "007.txt");
    std::string to = libcpp::filepath::join(libcpp::filepath::pwd(), "008.txt");
    ASSERT_STREQ(
        (libcpp::filepath::is_exist(from)&&(!libcpp::filepath::is_exist(to)) ? libcpp::filepath::rename(from, to).c_str() : to.c_str()), 
        to.c_str()
    );
}
