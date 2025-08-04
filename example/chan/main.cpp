#include <chrono>
#include <iostream>
#include <thread>

#include <libcpp/sync/chan.hpp>
struct Num
{
  public:
    inline void Do() { n_++; }
    inline long Value() { return n_; }

  private:
    long n_ = 1;
};

static Num n1{};
static Num n2{};
static Num n3{};

static std::mutex mu;
static long max = 1000000;

int main(int argc, char* argv[])
{
    libcpp::chan<Num*> chan{};
    chan << (&n2);

    // unsafe multi thread operation
    std::thread([]() {
        for (; n1.Value() < max; n1.Do())
        {
        }
    }).detach();
    std::thread([]() {
        for (; n1.Value() < max; n1.Do())
        {
        }
    }).detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    std::cout << "n1.Value() = " << n1.Value() << std::endl;

    // safe multi thread operation with chan
    std::thread([&]() {
        Num* p;
        for (chan >> p; p->Value() < max; chan << p)
        {
            p->Do();
        }
    }).detach();
    std::thread([&]() {
        Num* p;
        for (chan >> p; p->Value() < max; chan << p)
        {
            p->Do();
        }
    }).detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    std::cout << "n2.Value() = " << n2.Value() << std::endl;

    // safe multi thread operation with mutex
    std::thread([]() {
        for (; true;)
        {
            mu.lock();
            if (n3.Value() < max)
            {
                n3.Do();
            }
            else
            {
                mu.unlock();
                return;
            }
            mu.unlock();
        }
    }).detach();
    std::thread([]() {
        for (; true;)
        {
            mu.lock();
            if (n3.Value() < max)
            {
                n3.Do();
            }
            else
            {
                mu.unlock();
                return;
            }
            mu.unlock();
        }
    }).detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    std::cout << "n3.Value() = " << n3.Value() << std::endl;

    std::cin.get();
    return 0;
}