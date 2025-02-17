#include <iostream>

#include <libcpp/io/filepath.hpp>
#include <libcpp/io/fsize.hpp>

using namespace libcpp;

int main(int argc, char* argv[])
{
    std::cout << std::endl;
    std::cout << "KB(1) / BYTE(1) = " << KB(1) / BYTE(1) << std::endl;
    std::cout << "MB(1) / KB(1) = " << MB(1) / KB(1) << std::endl;
    std::cout << "GB(1) / MB(1) = " << GB(1) / MB(1) << std::endl;
    std::cout << "TB(1) / GB(1) = " << TB(1) / GB(1) << std::endl;


    std::cout << "pwd() = " << file_path::pwd() << std::endl;

    std::cout << "parent(\"/var/tmp/hello.txt\") = " << file_path::parent("/var/tmp/hello.txt") << std::endl;

    std::cout << "parent(\"./\") = " << file_path::absolute("./") << std::endl;

    std::cout << "relative(\"/var\", \"/var/tmp/src/\") = " << file_path::relative("/", "/var/tmp/src/") << std::endl;

    std::cout << "join({\"/usr/local\", \"bin\"}) = " << file_path::join("/usr/local", "bin") << std::endl;

    std::cout << "make_file(\"/var/tmp/hello.txt\") = "
              << file_path::make_file("/var/tmp/hello.txt") << std::endl;

    std::cout << "file_name(\"/var/tmp/hello.txt\") = " << file_path::file_name("/var/tmp/hello.txt") << std::endl;
    std::cout << "file_name(\"/var/tmp/hello.txt\", false) = "
              << file_path::file_name("/var/tmp/hello.txt", false) << std::endl;

    std::cout << "dir_name(\"/var/tmp/hello.txt\") = " << file_path::dir_name("/var/tmp/hello.txt") << std::endl;

    std::cout << "path_name(\"/var/tmp/hello.txt\") = " << file_path::path_name("/var/tmp/hello.txt") << std::endl;

    std::cout << "extension(\"/var/tmp/hello.txt\") = " << file_path::extension("/var/tmp/hello.txt") << std::endl;

    std::cout << "replace_extension(\"/var/tmp/hello.txt\", \".hpp\") = "
              << file_path::replace_extension("/var/tmp/hello.txt", ".hpp") << std::endl;

    std::cout << "is_dir(\"/var/tmp/hello.txt\") = " << file_path::is_dir("/var/tmp/hello.txt") << std::endl;
    std::cout << "is_dir(\"/var/tmp/\") = " << file_path::is_dir("/var/tmp/") << std::endl;

    std::cout << "is_symlink(\"/var/tmp/hello.txt\") = " << file_path::is_symlink("/var/tmp/hello.txt") << std::endl;

    std::cout << "is_exist(\"/var/tmp/hello.txt\") = " << file_path::is_exist("/var/tmp/hello.txt") << std::endl;
    std::cout << "is_exist(\"/var/tmp/\") = " << file_path::is_exist("/var/tmp/") << std::endl;

    std::cout << "last_mod_time(\"/var/tmp/\") = " << file_path::last_mod_time("/var/tmp/") << std::endl;

    std::cout << "size(\"/var/tmp/hello.txt\") = " << file_path::size("/var/tmp/hello.txt") << std::endl;

    std::cout << "\nWalk(\"/var/tmp\", [](const std::string& e)->bool{ std::cout << e << std::endl; return true;}) >>" << std::endl;
    file_path::walk("/var/tmp", [](const std::string & e)->bool{ std::cout << e << std::endl; return true;});

    std::cout << "\nList(\"/var/tmp\") >>" << std::endl;
    auto ret1 = file_path::list("/var/tmp");
    for (auto itr = ret1.begin(); itr != ret1.end(); itr++) {
        std::cout << *itr << std::endl;
    }

    std::cout << "\nlist(\"/var/tmp\", [](const std::string & a, const std::string & b)->bool {return a.length() < b.length();}) >>" << std::endl;
    auto ret2 = file_path::list("/var/tmp", [](const std::string & a, const std::string & b)->bool {return a.length() < b.length();});
    for (auto itr = ret2.begin(); itr != ret2.end(); itr++) {
        std::cout << *itr << std::endl;
    }

    std::cout << "\nfind(\"/var/tmp\", \"hello.txt\", true) >>" << std::endl;
    auto ret3 = file_path::find("/var/tmp", "hello.txt", true);
    for (auto itr = ret3.begin(); itr != ret3.end(); itr++) {
        std::cout << *itr << std::endl;
    }

    std::cout << "file_path::make_dir(\"/var/tmp/world/test/\")"
              << file_path::make_dir("/var/tmp/world/test/") << std::endl;

    std::cout << "remove(\"/var/tmp/cp\") = " << file_path::remove("/var/tmp/cp") << std::endl;

    std::cout << "copy_dir(\"/var/tmp/world/test/\", \"/var/tmp/cp\") = "
              << file_path::copy_dir("/var/tmp/world/test/", "/var/tmp/cp") << std::endl;

    std::cout << "copy_file(\"/var/tmp/hello.txt\", \"/var/tmp/hello.cp.txt\") = "
              << file_path::copy_file("/var/tmp/hello.txt", "/var/tmp/hello.cp.txt") << std::endl;

    std::cout << "rename(\"/var/tmp/hello.txt\", \"/var/tmp/007.txt\")" <<
              file_path::rename("/var/tmp/hello.txt", "/var/tmp/007.txt") << std::endl;

    std::cin.get();
    return 0;
}