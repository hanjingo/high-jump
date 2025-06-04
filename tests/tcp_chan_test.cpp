#include <gtest/gtest.h>
#include <libcpp/net/tcp.hpp>
#include <thread>

 TEST(tcp_chan, wait_dequeue)
 {
     static libcpp::tcp_chan<int> ch{2};
    
     static int n = 0;
     std::thread t1([&](){
         int i = 0;
         ch >> i;
         n += i;
         ch >> i;
         n += i;
         ch >> i;
         n += i;
     });

     std::thread t2([&](){
         int i = 0;
         ch >> i;
         n += i;
         ch >> i;
         n += i;
     });

     ch << 1;
     ch << 2;
     ch << 3;
     ch << 4;
     ch << 5;

     t1.join();
     t2.join();
     ASSERT_EQ(n == 15, true);
 }

 TEST(tcp_chan, enqueue)
 {
     static int n = 0;
     static int i = 0;
     static libcpp::tcp_chan<int> ch{2};

     std::thread t1([&](){
         ch << 1;
         ch << 2;
         ch << 3;
     });

     std::thread t2([&](){
         ch << 4;
         ch << 5;
     });

     ch >> i;
     n += i;

     ch >> i;
     n += i;

     ch >> i;
     n += i;
 ;
     ch >> i;
     n += i;

     ch >> i;
     n += i;

     ASSERT_EQ(n == 15, true);
     t1.join();
     t2.join();
 }