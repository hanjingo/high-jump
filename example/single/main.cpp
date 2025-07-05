#include <iostream>
#include <string>

#define USE_BOOST

#include <libcpp/types/singleton.hpp>

class A
{
public:
    void static StaticSay()
    {
        std::cout << "static say" << std::endl;
    }
    void Say()
    {
        std::cout << "say " << name_ << std::endl;
    }
    void SetName(const std::string& name)
    {
        name_ = name;
    }

private:
    std::string name_;
};

class B : public libcpp::singleton<B>
{
public:
    void static StaticSay()
    {
        std::cout << "static say" << std::endl;
    }
    void Say()
    {
        std::cout << "say " << name_ << std::endl;
    }
    void SetName(const std::string& name)
    {
        name_ = name;
    }

private:
    std::string name_;
};

int main(int argc, char* argv[])
{
    std::cout << "singleton<A>::get_const_instance().StaticSay() >> " << std::endl;
    libcpp::singleton<A>::get_const_instance().StaticSay();

    std::cout << "\nSingleton<A>::get_mutable_instance().Say() >> " << std::endl;
    libcpp::singleton<A>::get_mutable_instance()->SetName("he");
    libcpp::singleton<A>::get_mutable_instance()->Say();

    // WARNNING: the follow usage is not recommended
    B b;
    std::cout << "\nb.get_const_instance().StaticSay() >>"  << std::endl;
    b.get_const_instance().StaticSay();

    std::cout << "\nb.get_mutable_instance().Say() >> " << std::endl;
    b.get_mutable_instance()->SetName("he");
    b.get_mutable_instance()->Say();

    std::cin.get();
    return 0;
}