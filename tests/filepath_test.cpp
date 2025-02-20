#include <gtest/gtest.h>
#include <libcpp/io/filepath.hpp>

TEST(file_path, pwd)
{
    ASSERT_EQ(libcpp::file_path::pwd().empty(), false);
}

TEST(file_path, parent)
{
    ASSERT_STREQ(
        libcpp::file_path::parent("/usr/local/src").c_str(), 
        "/usr/local"
    );
}

TEST(file_path, absolute)
{
    ASSERT_STREQ(
        libcpp::file_path::absolute("/usr/local/src").c_str(), 
        "/usr/local/src"
    );

    ASSERT_STREQ(
        libcpp::file_path::absolute("./007.txt").c_str(), 
        libcpp::file_path::join(libcpp::file_path::pwd(), "./007.txt").c_str()
    );
}

// TEST(file_path, relative)
// {
//     ASSERT_STREQ(
//         libcpp::file_path::relative("/usr/local/src/007.txt").c_str(), 
//         "../../../../007.txt"
//     );
// }

// TEST(file_path, join)
// {
//     ASSERT_STREQ(
//         libcpp::file_path::join("/usr/local/src", "007.txt").c_str(), 
//         "/usr/local/src/007.txt"
//     );

//     std::vector<std::string> args{"/usr/local/src", "007.txt"};
//     ASSERT_STREQ(
//         libcpp::file_path::join(args).c_str(), 
//         "/usr/local/src/007.txt"
//     );
// }

// TEST(file_path, file_name)
// {
//     ASSERT_STREQ(
//         libcpp::file_path::file_name("/usr/local/src/007.txt").c_str(), 
//         "007.txt"
//     );
// }

// TEST(file_path, dir_name)
// {
//     ASSERT_STREQ(
//         libcpp::file_path::dir_name("/usr/local/src/007.txt").c_str(), 
//         "src"
//     );
// }

// TEST(file_path, path_name)
// {
//     ASSERT_STREQ(
//         libcpp::file_path::path_name("/usr/local/src/007.txt").c_str(), 
//         "/usr/local/src"
//     );
// }

// TEST(file_path, extension)
// {
//     ASSERT_STREQ(
//         libcpp::file_path::extension("/usr/local/src/007.txt").c_str(), 
//         ".txt"
//     );
// }

// TEST(file_path, replace_extension)
// {
//     ASSERT_STREQ(
//         libcpp::file_path::replace_extension("/usr/local/src/007.txt", ".exe").c_str(), 
//         "/usr/local/src/007.exe"
//     );
// }

// TEST(file_path, is_dir)
// {
//     ASSERT_EQ(libcpp::file_path::is_dir("/usr/local/src"), true);
//     ASSERT_EQ(libcpp::file_path::is_dir("/usr/local/src/007.txt"), false);
// }

// TEST(file_path, is_symlink)
// {
//     ASSERT_EQ(libcpp::file_path::is_symlink(libcpp::file_path::pwd()), false);
// }

// TEST(file_path, is_exist)
// {
//     ASSERT_EQ(libcpp::file_path::is_exist(libcpp::file_path::pwd()), true);
// }

// TEST(file_path, last_mod_time)
// {
//     ASSERT_EQ(libcpp::file_path::last_mod_time(libcpp::file_path::pwd()) > 0, true);
// }

// // TEST(file_path, size)
// // {
// //     ASSERT_EQ(libcpp::file_path::size(libcpp::file_path::pwd()) > 0, true);
// // }

// TEST(file_path, walk)
// {
//     int n = 0;
//     libcpp::file_path::walk(libcpp::file_path::pwd(), [&](const std::string& f)->bool{
//         n++;
//         return true;
//     });
//     ASSERT_EQ(n > 0, true);
// }

// TEST(file_path, list)
// {
//     ASSERT_EQ(libcpp::file_path::list(libcpp::file_path::pwd()).size() > 0, true);
// }

// TEST(file_path, find)
// {
//     ASSERT_EQ(libcpp::file_path::find(libcpp::file_path::pwd(), "tests").size() > 0, true);
// }

// TEST(file_path, make_dir)
// {
//     std::string path = libcpp::file_path::join(libcpp::file_path::pwd(), "tmp");
//     ASSERT_EQ(
//         (libcpp::file_path::is_exist(path) ? true : libcpp::file_path::make_dir(path)), 
//         true
//     );
// }

// TEST(file_path, make_file)
// {
//     std::string f = libcpp::file_path::join(libcpp::file_path::pwd(), "007.txt");
//     ASSERT_EQ(
//         (libcpp::file_path::is_exist(f) ? true : libcpp::file_path::make_file(f)), 
//         true
//     );
// }

// TEST(file_path, copy_dir)
// {
//     std::string from = libcpp::file_path::join(libcpp::file_path::pwd(), "tmp");
//     std::string to = libcpp::file_path::join(libcpp::file_path::pwd(), "tmp1");
//     ASSERT_EQ(
//         (libcpp::file_path::is_exist(from)&&(!libcpp::file_path::is_exist(to)) ? libcpp::file_path::copy_dir(from, to) : true), 
//         true
//     );
// }

// TEST(file_path, copy_file)
// {
//     std::string from = libcpp::file_path::join(libcpp::file_path::pwd(), "007.txt");
//     std::string to = libcpp::file_path::join(libcpp::file_path::pwd(), "008.txt");
//     ASSERT_EQ(
//         (libcpp::file_path::is_exist(from)&&(!libcpp::file_path::is_exist(to)) ? libcpp::file_path::copy_file(from, to) : true), 
//         true
//     );
// }

// TEST(file_path, remove)
// {
//     std::string f = libcpp::file_path::join(libcpp::file_path::pwd(), "007.txt");
//     ASSERT_EQ(
//         (libcpp::file_path::is_exist(f) ? libcpp::file_path::remove(f) : true), 
//         true
//     );
// }

// TEST(file_path, rename)
// {
//     std::string from = libcpp::file_path::join(libcpp::file_path::pwd(), "007.txt");
//     std::string to = libcpp::file_path::join(libcpp::file_path::pwd(), "008.txt");
//     ASSERT_STREQ(
//         (libcpp::file_path::is_exist(from)&&(!libcpp::file_path::is_exist(to)) ? libcpp::file_path::rename(from, to).c_str() : to.c_str()), 
//         to.c_str()
//     );
// }
