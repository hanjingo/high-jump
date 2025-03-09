#include <iostream>

#include <libcpp/io/filepath.hpp>
#include <libcpp/io/file.hpp>

using namespace libcpp;

int main(int argc, char* argv[])
{
    std::cout << std::endl;
    std::cout << "KB(1) / BYTE(1) = " << KB(1) / BYTE(1) << std::endl;
    std::cout << "MB(1) / KB(1) = " << MB(1) / KB(1) << std::endl;
    std::cout << "GB(1) / MB(1) = " << GB(1) / MB(1) << std::endl;
    std::cout << "TB(1) / GB(1) = " << TB(1) / GB(1) << std::endl;


    std::cout << "pwd() = " << filepath::pwd() << std::endl;

    std::cout << "parent(\"/var/tmp/hello.txt\") = " << filepath::parent("/var/tmp/hello.txt") << std::endl;

    std::cout << "parent(\"./\") = " << filepath::absolute("./") << std::endl;

    std::cout << "relative(\"/var\", \"/var/tmp/src/\") = " << filepath::relative("/", "/var/tmp/src/") << std::endl;

    std::cout << "join({\"/usr/local\", \"bin\"}) = " << filepath::join("/usr/local", "bin") << std::endl;

    std::cout << "make_file(\"/var/tmp/hello.txt\") = "
              << filepath::make_file("/var/tmp/hello.txt") << std::endl;

    std::cout << "file_name(\"/var/tmp/hello.txt\") = " << filepath::file_name("/var/tmp/hello.txt") << std::endl;
    std::cout << "file_name(\"/var/tmp/hello.txt\", false) = "
              << filepath::file_name("/var/tmp/hello.txt", false) << std::endl;

    std::cout << "dir_name(\"/var/tmp/hello.txt\") = " << filepath::dir_name("/var/tmp/hello.txt") << std::endl;

    std::cout << "path_name(\"/var/tmp/hello.txt\") = " << filepath::path_name("/var/tmp/hello.txt") << std::endl;

    std::cout << "extension(\"/var/tmp/hello.txt\") = " << filepath::extension("/var/tmp/hello.txt") << std::endl;

    std::cout << "replace_extension(\"/var/tmp/hello.txt\", \".hpp\") = "
              << filepath::replace_extension("/var/tmp/hello.txt", ".hpp") << std::endl;

    std::cout << "is_dir(\"/var/tmp/hello.txt\") = " << filepath::is_dir("/var/tmp/hello.txt") << std::endl;
    std::cout << "is_dir(\"/var/tmp/\") = " << filepath::is_dir("/var/tmp/") << std::endl;

    std::cout << "is_symlink(\"/var/tmp/hello.txt\") = " << filepath::is_symlink("/var/tmp/hello.txt") << std::endl;

    std::cout << "is_exist(\"/var/tmp/hello.txt\") = " << filepath::is_exist("/var/tmp/hello.txt") << std::endl;
    std::cout << "is_exist(\"/var/tmp/\") = " << filepath::is_exist("/var/tmp/") << std::endl;

    std::cout << "last_mod_time(\"/var/tmp/\") = " << filepath::last_mod_time("/var/tmp/") << std::endl;

    std::cout << "size(\"/var/tmp/hello.txt\") = " << filepath::size("/var/tmp/hello.txt") << std::endl;

    std::cout << "\nWalk(\"/var/tmp\", [](const std::string& e)->bool{ std::cout << e << std::endl; return true;}) >>" << std::endl;
    filepath::walk("/var/tmp", [](const std::string & e)->bool{ std::cout << e << std::endl; return true;});

    std::cout << "\nList(\"/var/tmp\") >>" << std::endl;
    auto ret1 = filepath::list("/var/tmp");
    for (auto itr = ret1.begin(); itr != ret1.end(); itr++) {
        std::cout << *itr << std::endl;
    }

    std::cout << "\nlist(\"/var/tmp\", [](const std::string & a, const std::string & b)->bool {return a.length() < b.length();}) >>" << std::endl;
    auto ret2 = filepath::list("/var/tmp", [](const std::string & a, const std::string & b)->bool {return a.length() < b.length();});
    for (auto itr = ret2.begin(); itr != ret2.end(); itr++) {
        std::cout << *itr << std::endl;
    }

    std::cout << "\nfind(\"/var/tmp\", \"hello.txt\", true) >>" << std::endl;
    auto ret3 = filepath::find("/var/tmp", "hello.txt", true);
    for (auto itr = ret3.begin(); itr != ret3.end(); itr++) {
        std::cout << *itr << std::endl;
    }

    std::cout << "filepath::make_dir(\"/var/tmp/world/test/\")"
              << filepath::make_dir("/var/tmp/world/test/") << std::endl;

    std::cout << "remove(\"/var/tmp/cp\") = " << filepath::remove("/var/tmp/cp") << std::endl;

    std::cout << "copy_dir(\"/var/tmp/world/test/\", \"/var/tmp/cp\") = "
              << filepath::copy_dir("/var/tmp/world/test/", "/var/tmp/cp") << std::endl;

    std::cout << "copy_file(\"/var/tmp/hello.txt\", \"/var/tmp/hello.cp.txt\") = "
              << filepath::copy_file("/var/tmp/hello.txt", "/var/tmp/hello.cp.txt") << std::endl;

    std::cout << "rename(\"/var/tmp/hello.txt\", \"/var/tmp/007.txt\")" <<
              filepath::rename("/var/tmp/hello.txt", "/var/tmp/007.txt") << std::endl;

    std::cin.get();
    return 0;
}