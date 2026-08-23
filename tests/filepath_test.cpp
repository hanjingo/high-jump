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

        hj::filepath::mkdir_options opt;
        opt.succ_if_exist = true;
        opt.recursive     = true;
        hj::filepath::mkdir(test_dir_, opt);
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
    ASSERT_TRUE(hj::filepath::is_dir(test_dir_));
    std::string f = hj::filepath::join(test_dir_, "007.txt");
    hj::filepath::touch(f);
    ASSERT_FALSE(hj::filepath::is_dir(f));
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
    ASSERT_EQ(hj::filepath::touch(f), true);

    auto file_size = hj::filepath::size(f);
    ASSERT_TRUE(file_size.has_value());
    ASSERT_GE(*file_size, 0);
}

TEST_F(FilePathTest, walk)
{
    std::string f = hj::filepath::join(test_dir_, "walk_test.txt");
    hj::filepath::touch(f);

    int                        n = 0;
    hj::filepath::walk_options w_opt;
    w_opt.recursive = true;
    hj::filepath::walk(
        test_dir_,
        [&](const std::string &path, std::size_t depth) -> bool {
            (void) path;
            (void) depth;
            n++;
            return true;
        },
        w_opt);
    ASSERT_EQ(n > 0, true);
}

TEST_F(FilePathTest, walk_callback_termination)
{
    hj::filepath::touch(hj::filepath::join(test_dir_, "f1.txt"));
    hj::filepath::touch(hj::filepath::join(test_dir_, "f2.txt"));

    int                        count = 0;
    hj::filepath::walk_options w_opt;
    w_opt.recursive = false;

    hj::filepath::walk(
        test_dir_,
        [&](const std::string &, std::size_t) -> bool {
            count++;
            return false;
        },
        w_opt);

    ASSERT_EQ(count, 1);
}

TEST_F(FilePathTest, walk_error_handler_termination)
{
    hj::filepath::walk_options w_opt;
    w_opt.continue_on_error = false;

    bool error_handled = false;
    auto err_handler   = [&](const std::string &, const std::error_code &) {
        error_handled = true;
        return false;
    };

    std::string bad_path =
        hj::filepath::join(test_dir_, "non_existent_walk_target");
    ASSERT_NO_THROW(hj::filepath::walk(
        bad_path,
        [](const std::string &, std::size_t) { return true; },
        w_opt,
        err_handler));
    ASSERT_TRUE(error_handled);
}

TEST_F(FilePathTest, list)
{
    std::string f = hj::filepath::join(test_dir_, "list_test.txt");
    hj::filepath::touch(f);

    ASSERT_EQ(hj::filepath::list(test_dir_).size(), 1);
}

TEST_F(FilePathTest, find)
{
    std::string f = hj::filepath::join(test_dir_, "find_test.txt");
    hj::filepath::touch(f);

    hj::filepath::find_options opt;
    opt.recursive = true;
    auto results  = hj::filepath::find(test_dir_, "find_test.txt", opt);
    ASSERT_EQ(results.size(), 1);
}

TEST_F(FilePathTest, mkdir_succ_if_exist)
{
    std::string sub_dir = hj::filepath::join(test_dir_, "sub_existing");

    hj::filepath::mkdir_options opt;
    opt.recursive     = true;
    opt.succ_if_exist = false;

    ASSERT_EQ(hj::filepath::mkdir(sub_dir, opt), true);
    ASSERT_EQ(hj::filepath::is_dir(sub_dir), true);

    std::error_code ec;
    bool            second_create = hj::filepath::mkdir(sub_dir, opt, ec);
    ASSERT_FALSE(second_create);
    ASSERT_TRUE(ec);

    opt.succ_if_exist = true;
    ec.clear();
    bool succ_if_exist_create = hj::filepath::mkdir(sub_dir, opt, ec);
    ASSERT_TRUE(succ_if_exist_create);
    ASSERT_FALSE(ec);
}

TEST_F(FilePathTest, mkdir_force_semantic)
{
    std::string existing_dir = hj::filepath::join(test_dir_, "nested_dir");
    std::string dummy_file   = hj::filepath::join(existing_dir, "data.txt");

    hj::filepath::mkdir_options opt_create;
    opt_create.recursive = true;
    hj::filepath::mkdir(existing_dir, opt_create);
    {
        std::ofstream ofs(dummy_file);
        ofs << "keep me";
    }

    hj::filepath::mkdir_options opt_force;
    opt_force.recursive     = true;
    opt_force.succ_if_exist = true;

    std::error_code ec;
    bool            res = hj::filepath::mkdir(existing_dir, opt_force, ec);
    ASSERT_TRUE(res);
    ASSERT_FALSE(ec);
    ASSERT_TRUE(hj::filepath::exists(dummy_file));
}

TEST_F(FilePathTest, touch)
{
    std::string f = hj::filepath::join(test_dir_, "file.txt");
    ASSERT_EQ(hj::filepath::touch(f), true);
    ASSERT_EQ(hj::filepath::exists(f), true);
}

TEST_F(FilePathTest, copy_dir)
{
    std::string from      = hj::filepath::join(test_dir_, "from_dir");
    std::string to        = hj::filepath::join(test_dir_, "to_dir");
    std::string empty_sub = hj::filepath::join(from, "empty_subdir");

    ASSERT_TRUE(hj::filepath::mkdir(empty_sub));
    ASSERT_TRUE(hj::filepath::copy_dir(from, to));
    ASSERT_TRUE(hj::filepath::is_dir(hj::filepath::join(to, "empty_subdir")));

    std::string nested_file = hj::filepath::join(from, "a", "b", "file.txt");
    hj::filepath::mkdir_options opt;
    opt.recursive = true;
    hj::filepath::mkdir(hj::filepath::parent(nested_file), opt);


    {
        std::ofstream ofs(nested_file);
        ofs << "original content";
    }

    std::string target_to = hj::filepath::join(test_dir_, "to_dir_nested");
    ASSERT_TRUE(hj::filepath::copy_dir(from, target_to));
    std::string copied_nested =
        hj::filepath::join(target_to, "a", "b", "file.txt");
    ASSERT_TRUE(hj::filepath::exists(copied_nested));

    std::string conflict_file =
        hj::filepath::join(target_to, "a", "b", "file.txt");
    {
        std::ofstream ofs(conflict_file);
        ofs << "new modified content";
    }

    ASSERT_TRUE(hj::filepath::copy_dir(from, target_to, false));
    {
        std::ifstream ifs(conflict_file);
        std::string   content((std::istreambuf_iterator<char>(ifs)),
                              std::istreambuf_iterator<char>());
        ASSERT_EQ(content, "new modified content");
    }

    ASSERT_TRUE(hj::filepath::copy_dir(from, target_to, true));
    {
        std::ifstream ifs(conflict_file);
        std::string   content((std::istreambuf_iterator<char>(ifs)),
                              std::istreambuf_iterator<char>());
        ASSERT_EQ(content, "original content");
    }

#if !defined(_WIN32)
    std::string link_target = hj::filepath::join(from, "link_target.txt");
    hj::filepath::touch(link_target);
    std::string     symlink_path = hj::filepath::join(from, "my_symlink");
    std::error_code sym_ec;
    std::filesystem::create_symlink(link_target, symlink_path, sym_ec);

    if(!sym_ec)
    {
        std::string link_to = hj::filepath::join(test_dir_, "to_dir_link");
        ASSERT_TRUE(hj::filepath::copy_dir(from, link_to));
        ASSERT_TRUE(hj::filepath::is_symlink(
            hj::filepath::join(link_to, "my_symlink")));
    }
#endif

    std::string non_exist_from =
        hj::filepath::join(test_dir_, "non_exist_from");
    std::string fail_to = hj::filepath::join(test_dir_, "fail_to");
    ASSERT_FALSE(hj::filepath::copy_dir(non_exist_from, fail_to));
}

TEST_F(FilePathTest, copy_file)
{
    std::string from = hj::filepath::join(test_dir_, "from.txt");
    std::string to   = hj::filepath::join(test_dir_, "to.txt");

    ASSERT_TRUE(hj::filepath::touch(from));

    ASSERT_FALSE(hj::filepath::exists(to));

    ASSERT_TRUE(hj::filepath::copy_file(from, to));
    ASSERT_TRUE(hj::filepath::exists(to));
}

TEST_F(FilePathTest, remove)
{
    std::string f = hj::filepath::join(test_dir_, "remove.txt");
    hj::filepath::touch(f);
    ASSERT_EQ(hj::filepath::remove(f), true);
    ASSERT_EQ(hj::filepath::exists(f), false);
}

TEST_F(FilePathTest, remove_all_failure_semantics)
{
    std::string     missing_path = hj::filepath::join(test_dir_, "missing_dir");
    std::error_code ec;
    auto            count_missing = hj::filepath::remove_all(missing_path, ec);
    ASSERT_EQ(count_missing, 0);

    std::string file_path = hj::filepath::join(test_dir_, "single_file.txt");
    hj::filepath::touch(file_path);
    auto count_file = hj::filepath::remove_all(file_path, ec);
    ASSERT_EQ(count_file, 1);
    ASSERT_FALSE(hj::filepath::exists(file_path));

    std::string deep_dir = hj::filepath::join(test_dir_, "a", "b", "c");
    hj::filepath::mkdir_options opt;
    opt.recursive = true;
    hj::filepath::mkdir(deep_dir, opt);
    hj::filepath::touch(hj::filepath::join(deep_dir, "leaf.txt"));

    auto count_dir =
        hj::filepath::remove_all(hj::filepath::join(test_dir_, "a"), ec);
    ASSERT_GE(count_dir, 2);
    ASSERT_FALSE(hj::filepath::exists(hj::filepath::join(test_dir_, "a")));
}

TEST_F(FilePathTest, rename)
{
    std::string from = hj::filepath::join(test_dir_, "from_rename.txt");
    std::string to   = hj::filepath::join(test_dir_, "to_rename.txt");

    ASSERT_TRUE(hj::filepath::touch(from));

    ASSERT_FALSE(hj::filepath::exists(to));

    ASSERT_TRUE(hj::filepath::rename(from, to));
    ASSERT_TRUE(hj::filepath::exists(to));
    ASSERT_FALSE(hj::filepath::exists(from));
}

TEST_F(FilePathTest, find_by_regex)
{
    std::string f1 = hj::filepath::join(test_dir_, "match1.txt");
    std::string f2 = hj::filepath::join(test_dir_, "match2.log");
    hj::filepath::touch(f1);
    hj::filepath::touch(f2);

    hj::filepath::find_options opt;
    opt.recursive = true;
    auto txts     = hj::filepath::find_by_regex(test_dir_, R"(.*\.txt$)", opt);
    ASSERT_EQ(txts.size(), 1);
    ASSERT_EQ(txts[0], f1);
}

TEST_F(FilePathTest, find_options_depth_and_limit)
{
    // test_dir_/
    // ├── level1.txt
    // └── sub1/
    //     ├── level2.txt
    //     └── sub2/
    //         └── level3.txt

    std::string sub1 = hj::filepath::join(test_dir_, "sub1");
    std::string sub2 = hj::filepath::join(sub1, "sub2");

    hj::filepath::mkdir_options opt_mkdir;
    opt_mkdir.recursive = true;
    hj::filepath::mkdir(sub2, opt_mkdir);

    std::string f1 = hj::filepath::join(test_dir_, "level1.txt");
    std::string f2 = hj::filepath::join(sub1, "level2.txt");
    std::string f3 = hj::filepath::join(sub2, "level3.txt");

    hj::filepath::touch(f1);
    hj::filepath::touch(f2);
    hj::filepath::touch(f3);

    hj::filepath::find_options opt_depth;
    opt_depth.recursive = true;
    opt_depth.max_depth = 1;

    auto results_depth = hj::filepath::find(test_dir_, "level2.txt", opt_depth);
    ASSERT_TRUE(results_depth.empty());

    auto results_depth_ok =
        hj::filepath::find(test_dir_, "level1.txt", opt_depth);
    ASSERT_EQ(results_depth_ok.size(), 1);

    std::string extra_f1 = hj::filepath::join(sub1, "level1.txt");
    hj::filepath::touch(extra_f1);

    hj::filepath::find_options opt_limit;
    opt_limit.recursive  = true;
    opt_limit.max_result = 1;

    auto results_limit = hj::filepath::find(test_dir_, "level1.txt", opt_limit);
    ASSERT_EQ(results_limit.size(), 1);

    hj::filepath::find_options opt_regex;
    opt_regex.recursive  = true;
    opt_regex.max_depth  = 1;
    opt_regex.max_result = 1;

    auto regex_results =
        hj::filepath::find_by_regex(test_dir_, R"(.*\.txt$)", opt_regex);
    ASSERT_EQ(regex_results.size(), 1);
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

    std::string drive_rel = "C:foo\\bar.txt";
    ASSERT_STREQ(hj::filepath::file_name(drive_rel).c_str(), "bar.txt");
    ASSERT_STREQ(hj::filepath::extension(drive_rel).c_str(), ".txt");

    std::string mixed_path = "C:/foo/bar.txt";
    ASSERT_STREQ(hj::filepath::parent(mixed_path).c_str(), "C:/foo");
    ASSERT_STREQ(hj::filepath::file_name(mixed_path).c_str(), "bar.txt");

    std::string unc_path = "\\\\server\\share\\dir\\file.txt";
    ASSERT_STREQ(hj::filepath::parent(unc_path).c_str(),
                 "\\\\server\\share\\dir");
    ASSERT_STREQ(hj::filepath::file_name(unc_path).c_str(), "file.txt");

    std::string long_path = "\\\\?\\C:\\very_long_path\\file.txt";
    ASSERT_STREQ(hj::filepath::file_name(long_path).c_str(), "file.txt");
    ASSERT_STREQ(hj::filepath::extension(long_path).c_str(), ".txt");

    std::string ext_unc_path = "\\\\?\\UNC\\server\\share\\dir\\file.txt";
    ASSERT_STREQ(hj::filepath::file_name(ext_unc_path).c_str(), "file.txt");
    ASSERT_STREQ(hj::filepath::parent(ext_unc_path).c_str(),
                 "\\\\?\\UNC\\server\\share\\dir");
#endif
}

TEST(file_path, exception_safety)
{
    std::error_code ec;
    ASSERT_NO_THROW(hj::filepath::list("/not_exist_dir_1234567890", ec));

    hj::filepath::find_options opt;
    opt.recursive = false;
    ASSERT_NO_THROW(hj::filepath::find_by_regex("/not_exist_dir_1234567890",
                                                ".*",
                                                opt,
                                                ec));

    opt.reset();
    opt.recursive = false;
    ASSERT_NO_THROW(
        hj::filepath::find("/not_exist_dir_1234567890", "foo", opt, ec));

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
    hj::filepath::mkdir_options opt;
    opt.succ_if_exist   = true;
    opt.recursive       = true;
    std::string sub_dir = hj::filepath::join(test_dir_, "subdir");
    hj::filepath::mkdir(sub_dir);

    std::string f1 = hj::filepath::join(test_dir_, "target.txt");
    std::string f2 = hj::filepath::join(sub_dir, "target.txt");
    hj::filepath::touch(f1);
    hj::filepath::touch(f2);

    hj::filepath::find_options find_opt;
    find_opt.recursive = true;
    auto filename_results =
        hj::filepath::find_by_regex(test_dir_, R"(.*target\.txt$)", find_opt);
    ASSERT_EQ(filename_results.size(), 2);

    find_opt.reset();
    find_opt.recursive = true;
    find_opt.target    = hj::filepath::match_target::path;
    auto path_results = hj::filepath::find_by_regex(test_dir_,
                                                    R"(.*subdir.*target\.txt$)",
                                                    find_opt);
    ASSERT_EQ(path_results.size(), 1);
    ASSERT_EQ(path_results[0], f2);
}

TEST(file_path, exception_safety_and_error_codes)
{
    std::error_code ec;
    auto list_res = hj::filepath::list("/not_exist_dir_1234567890", ec);
    ASSERT_TRUE(ec);
    ASSERT_EQ(ec, std::errc::no_such_file_or_directory);
    ASSERT_TRUE(list_res.empty());

    ec.clear();
    hj::filepath::find_options opt;
    opt.recursive = false;
    auto find_res =
        hj::filepath::find("/not_exist_dir_1234567890", "foo", opt, ec);
    ASSERT_TRUE(ec);
    ASSERT_EQ(ec, std::errc::no_such_file_or_directory);
    ASSERT_TRUE(find_res.empty());

    ec.clear();
    opt.reset();
    opt.recursive = false;
    opt.target    = hj::filepath::match_target::filename;
    auto regex_res =
        hj::filepath::find_by_regex("/not_exist_dir_1234567890", ".*", opt, ec);
    ASSERT_TRUE(ec);
    ASSERT_EQ(ec, std::errc::no_such_file_or_directory);
    ASSERT_TRUE(regex_res.empty());

    ec.clear();
    std::uintmax_t file_size =
        hj::filepath::size("/not_exist_dir_1234567890", ec);
    ASSERT_TRUE(ec);
    ASSERT_EQ(ec, std::errc::no_such_file_or_directory);
    ASSERT_EQ(file_size, 0);
}