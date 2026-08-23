#include <gtest/gtest.h>
#include <hj/io/filepath.hpp>
#include <filesystem>

class FilePathTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        namespace fs = std::filesystem;
        auto temp_base =
            fs::temp_directory_path() / "high_jump" / "filepath_test_sandbox";
        std::string unique_name =
            "test_"
            + std::to_string(
                std::chrono::steady_clock::now().time_since_epoch().count());
        test_dir_ = (temp_base / unique_name).string();

        hj::filepath::make_dir(test_dir_, true);
    }

    void TearDown() override
    {
        if(hj::filepath::exists(test_dir_))
        {
            hj::filepath::remove_all(test_dir_);
        }
    }

    std::string test_dir_;
};

TEST_F(FilePathTest, pwd)
{
    ASSERT_EQ(hj::filepath::pwd().empty(), false);
}

TEST_F(FilePathTest, parent)
{
    ASSERT_STREQ(hj::filepath::parent("/usr/local/src").c_str(), "/usr/local");
}

TEST_F(FilePathTest, absolute)
{
#if !defined(_WIN32)
    ASSERT_STREQ(hj::filepath::absolute("/usr/local/src").c_str(),
                 "/usr/local/src");
#endif

    std::string target = hj::filepath::join(test_dir_, "007.txt");
    ASSERT_FALSE(hj::filepath::absolute(target).empty());
}

TEST_F(FilePathTest, relative)
{
    std::string sub_file = hj::filepath::join(test_dir_, "sub", "007.txt");
    std::string rel      = hj::filepath::relative(sub_file, test_dir_);
#if defined(_WIN32)
    ASSERT_STREQ(rel.c_str(), "sub\\007.txt");
#else
    ASSERT_STREQ(rel.c_str(), "sub/007.txt");
#endif
}

TEST_F(FilePathTest, join)
{
#if defined(_WIN32)
    ASSERT_STREQ(hj::filepath::join(test_dir_, "007.txt").c_str(),
                 (test_dir_ + "\\007.txt").c_str());
    std::vector<std::string> args{test_dir_, "007.txt"};
    ASSERT_STREQ(hj::filepath::join(args).c_str(),
                 (test_dir_ + "\\007.txt").c_str());
#else
    ASSERT_STREQ(hj::filepath::join(test_dir_, "007.txt").c_str(),
                 (test_dir_ + "/007.txt").c_str());
    std::vector<std::string> args{test_dir_, "007.txt"};
    ASSERT_STREQ(hj::filepath::join(args).c_str(),
                 (test_dir_ + "/007.txt").c_str());
#endif
}

TEST_F(FilePathTest, file_name)
{
    std::string p = hj::filepath::join(test_dir_, "src", "007.txt");
    ASSERT_STREQ(hj::filepath::file_name(p).c_str(), "007.txt");
}

TEST_F(FilePathTest, dir_name)
{
    std::string p = hj::filepath::join(test_dir_, "src", "007.txt");
    ASSERT_STREQ(hj::filepath::dir_name(p).c_str(), "src");
}

TEST_F(FilePathTest, path_name)
{
    std::string p = hj::filepath::join(test_dir_, "src", "007.txt");
    ASSERT_STREQ(hj::filepath::path_name(p).c_str(),
                 hj::filepath::join(test_dir_, "src").c_str());
}

TEST_F(FilePathTest, extension)
{
    std::string p = hj::filepath::join(test_dir_, "007.txt");
    ASSERT_STREQ(hj::filepath::extension(p).c_str(), ".txt");
}

TEST_F(FilePathTest, replace_extension)
{
    std::string p = hj::filepath::join(test_dir_, "007.txt");
    ASSERT_STREQ(hj::filepath::replace_extension(p, ".exe").c_str(),
                 hj::filepath::join(test_dir_, "007.exe").c_str());
}

TEST_F(FilePathTest, is_dir)
{
    ASSERT_EQ(hj::filepath::is_dir(test_dir_), true);
    std::string f = hj::filepath::join(test_dir_, "007.txt");
    hj::filepath::create_file(f);
    ASSERT_EQ(hj::filepath::is_dir(f), false);
}

TEST_F(FilePathTest, is_symlink)
{
    ASSERT_EQ(hj::filepath::is_symlink(test_dir_), false);
}

TEST_F(FilePathTest, exists)
{
    ASSERT_EQ(hj::filepath::exists(test_dir_), true);
}

TEST_F(FilePathTest, last_mod_time)
{
    ASSERT_EQ(hj::filepath::last_mod_time(test_dir_) > 0, true);
}

TEST_F(FilePathTest, size)
{
    auto dir_size = hj::filepath::size(test_dir_);
    ASSERT_FALSE(dir_size.has_value());

    std::string f = hj::filepath::join(test_dir_, "size_test.txt");
    ASSERT_EQ(hj::filepath::create_file(f), true);

    auto file_size = hj::filepath::size(f);
    ASSERT_TRUE(file_size.has_value());
    ASSERT_GE(*file_size, 0);
}

TEST_F(FilePathTest, walk)
{
    std::string f = hj::filepath::join(test_dir_, "walk_test.txt");
    hj::filepath::create_file(f);

    int n = 0;
    hj::filepath::walk(
        test_dir_,
        [&](const std::string &path) -> bool {
            (void) path;
            n++;
            return true;
        },
        true);
    ASSERT_EQ(n > 0, true);
}

TEST_F(FilePathTest, list)
{
    std::string f = hj::filepath::join(test_dir_, "list_test.txt");
    hj::filepath::create_file(f);

    ASSERT_EQ(hj::filepath::list(test_dir_).size(), 1);
}

TEST_F(FilePathTest, find)
{
    std::string f = hj::filepath::join(test_dir_, "find_test.txt");
    hj::filepath::create_file(f);

    auto results = hj::filepath::find(test_dir_, "find_test.txt", true);
    ASSERT_EQ(results.size(), 1);
}

TEST_F(FilePathTest, make_dir)
{
    std::string sub = hj::filepath::join(test_dir_, "sub_tmp");
    ASSERT_EQ(hj::filepath::make_dir(sub), true);
    ASSERT_EQ(hj::filepath::is_dir(sub), true);
}

TEST_F(FilePathTest, create_file)
{
    std::string f = hj::filepath::join(test_dir_, "file.txt");
    ASSERT_EQ(hj::filepath::create_file(f), true);
    ASSERT_EQ(hj::filepath::exists(f), true);
}

TEST_F(FilePathTest, copy_dir)
{
    std::string from = hj::filepath::join(test_dir_, "from_dir");
    std::string to   = hj::filepath::join(test_dir_, "to_dir");

    ASSERT_TRUE(hj::filepath::make_dir(from));
    std::string sub_file = hj::filepath::join(from, "a.txt");
    ASSERT_TRUE(hj::filepath::create_file(sub_file));

    ASSERT_FALSE(hj::filepath::exists(to));

    ASSERT_TRUE(hj::filepath::copy_dir(from, to));
    ASSERT_TRUE(hj::filepath::is_dir(to));
    std::string copied_sub_file = hj::filepath::join(to, "a.txt");
    ASSERT_TRUE(hj::filepath::exists(copied_sub_file));
}

TEST_F(FilePathTest, copy_file)
{
    std::string from = hj::filepath::join(test_dir_, "from.txt");
    std::string to   = hj::filepath::join(test_dir_, "to.txt");

    ASSERT_TRUE(hj::filepath::create_file(from));

    ASSERT_FALSE(hj::filepath::exists(to));

    ASSERT_TRUE(hj::filepath::copy_file(from, to));
    ASSERT_TRUE(hj::filepath::exists(to));
}

TEST_F(FilePathTest, remove)
{
    std::string f = hj::filepath::join(test_dir_, "remove.txt");
    hj::filepath::create_file(f);
    ASSERT_EQ(hj::filepath::remove(f), true);
    ASSERT_EQ(hj::filepath::exists(f), false);
}

TEST_F(FilePathTest, rename)
{
    std::string from = hj::filepath::join(test_dir_, "from_rename.txt");
    std::string to   = hj::filepath::join(test_dir_, "to_rename.txt");

    ASSERT_TRUE(hj::filepath::create_file(from));

    ASSERT_FALSE(hj::filepath::exists(to));

    ASSERT_TRUE(hj::filepath::rename(from, to));
    ASSERT_TRUE(hj::filepath::exists(to));
    ASSERT_FALSE(hj::filepath::exists(from));
}

TEST_F(FilePathTest, find_by_regex)
{
    std::string f1 = hj::filepath::join(test_dir_, "match1.txt");
    std::string f2 = hj::filepath::join(test_dir_, "match2.log");
    hj::filepath::create_file(f1);
    hj::filepath::create_file(f2);

    auto txts = hj::filepath::find_by_regex(test_dir_, R"(.*\.txt$)");
    ASSERT_EQ(txts.size(), 1);
    ASSERT_EQ(txts[0], f1);
}

TEST(file_path, path_parsing_contract)
{
    ASSERT_EQ(hj::filepath::extension(""), "");
    ASSERT_EQ(hj::filepath::file_name(""), "");
    ASSERT_EQ(hj::filepath::parent(""), "");

    ASSERT_EQ(hj::filepath::extension("."), "");
    ASSERT_EQ(hj::filepath::extension(".."), "");
    ASSERT_EQ(hj::filepath::extension("/"), "");

    ASSERT_EQ(hj::filepath::file_name("/tmp/"), "");
    ASSERT_EQ(hj::filepath::extension("/tmp/"), "");

    ASSERT_STREQ(hj::filepath::extension(".gitignore").c_str(), "");
    ASSERT_STREQ(hj::filepath::file_name(".gitignore").c_str(), ".gitignore");

    ASSERT_STREQ(hj::filepath::extension("foo.tar.gz").c_str(), ".gz");
    ASSERT_STREQ(hj::filepath::file_name("foo.tar.gz").c_str(), "foo.tar.gz");
    ASSERT_STREQ(hj::filepath::extension("foo.").c_str(), ".");
}

TEST(file_path, windows_specific_contract)
{
#if defined(_WIN32)
    ASSERT_EQ(hj::filepath::exists("C:\\"), true);

    std::string unc_path = "\\\\server\\share\\dir\\file.txt";
    ASSERT_STREQ(hj::filepath::parent(unc_path).c_str(),
                 "\\\\server\\share\\dir");
    ASSERT_STREQ(hj::filepath::file_name(unc_path).c_str(), "file.txt");

    std::string long_path = "\\\\?\\C:\\very_long_path\\file.txt";
    ASSERT_STREQ(hj::filepath::file_name(long_path).c_str(), "file.txt");
    ASSERT_STREQ(hj::filepath::extension(long_path).c_str(), ".txt");
#endif
}

TEST(file_path, exception_safety)
{
    std::error_code ec;
    ASSERT_NO_THROW(hj::filepath::list("/not_exist_dir_1234567890", ec));
    ASSERT_NO_THROW(
        hj::filepath::find_by_regex("/not_exist_dir_1234567890",
                                    ".*",
                                    false,
                                    hj::filepath::match_target::filename,
                                    ec));
    ASSERT_NO_THROW(
        hj::filepath::find("/not_exist_dir_1234567890", "foo", false, ec));

    ASSERT_NO_THROW(hj::filepath::is_dir("/not_exist_dir_1234567890"));
    ASSERT_NO_THROW(hj::filepath::exists("/not_exist_dir_1234567890"));
    ASSERT_NO_THROW(hj::filepath::size("/not_exist_dir_1234567890"));
    ASSERT_NO_THROW(hj::filepath::remove("/not_exist_dir_1234567890"));
}

TEST(file_path, edge_cases)
{
    ASSERT_EQ(hj::filepath::file_name("").empty(), true);
    ASSERT_EQ(hj::filepath::dir_name("").empty(), true);
    ASSERT_EQ(hj::filepath::extension("").empty(), true);
    ASSERT_EQ(hj::filepath::replace_extension("", ".log"), ".log");
    ASSERT_EQ(hj::filepath::is_dir(""), false);
    ASSERT_EQ(hj::filepath::exists(""), false);
    ASSERT_EQ(hj::filepath::size("").has_value(), false);
}

TEST_F(FilePathTest, find_by_regex_target)
{
    std::string sub_dir = hj::filepath::join(test_dir_, "subdir");
    hj::filepath::make_dir(sub_dir);

    std::string f1 = hj::filepath::join(test_dir_, "target.txt");
    std::string f2 = hj::filepath::join(sub_dir, "target.txt");
    hj::filepath::create_file(f1);
    hj::filepath::create_file(f2);

    auto filename_results =
        hj::filepath::find_by_regex(test_dir_, R"(.*target\.txt$)", true);
    ASSERT_EQ(filename_results.size(), 2);

    auto path_results =
        hj::filepath::find_by_regex(test_dir_,
                                    R"(.*subdir.*target\.txt$)",
                                    true,
                                    hj::filepath::match_target::path);
    ASSERT_EQ(path_results.size(), 1);
    ASSERT_EQ(path_results[0], f2);
}