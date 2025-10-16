#include <gtest/gtest.h>
#include <hj/time/timer.hpp>
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>

using namespace std::chrono_literals;

TEST(timer, basic_callback)
{
    static std::atomic<int> tm_flag1{0};
    tm_flag1 = 0; // Reset flag for this test
    
    {
        auto t = std::make_shared<hj::timer>(1000); // 1ms
        t->start([] { tm_flag1 = 42; });

        // Wait for callback with timeout
        auto start = std::chrono::steady_clock::now();
        while(tm_flag1 != 42 && 
              std::chrono::steady_clock::now() - start < std::chrono::milliseconds(100))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        ASSERT_EQ(tm_flag1, 42);
        
        // Let timer destructor run
    }
    
    // Additional cleanup time
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

TEST(timer, reset_and_wait)
{
    static std::atomic<int> tm_flag2{0};
    tm_flag2 = 0; // Reset flag for this test
    
    {
        auto t = std::make_shared<hj::timer>(10000); // 10ms initial delay
        t->start([] { tm_flag2 = 1; });
        t->reset(1000, [] { tm_flag2 = 2; }); // Reset to 1ms with new callback
        
        // Wait for the reset callback
        auto start = std::chrono::steady_clock::now();
        while(tm_flag2 != 2 && 
              std::chrono::steady_clock::now() - start < std::chrono::milliseconds(100))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        ASSERT_EQ(tm_flag2, 2);
        
        // Let timer destructor run
    }
    
    // Additional cleanup time
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

TEST(timer, cancel)
{
    static std::atomic<int> tm_flag3{0};
    tm_flag3 = 0; // Reset flag for this test
    
    {
        auto t = std::make_shared<hj::timer>(10000); // 10ms delay
        t->start([] { tm_flag3 = 1; });
        t->cancel();
        
        // Give some time to ensure cancel takes effect
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        
        // Timer should be cancelled and callback should not execute
        ASSERT_EQ(tm_flag3, 0);
        
        // Let timer destructor run before test ends
    }
    
    // Additional sleep to ensure cleanup completes
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST(timer, shared_ptr_transfer)
{
    static std::atomic<int> tm_flag4{0};
    tm_flag4 = 0; // Reset flag for this test
    
    {
        // Test transferring ownership of shared_ptr (not moving the timer object itself)
        std::shared_ptr<hj::timer> t1 = std::make_shared<hj::timer>(1000); // 1ms
        t1->start([] { tm_flag4 = 9; });
        
        // Transfer ownership to t2 (this is safe)
        std::shared_ptr<hj::timer> t2 = t1;
        t1.reset(); // Release original reference
        
        // Wait for callback with timeout
        auto start = std::chrono::steady_clock::now();
        while(tm_flag4 != 9 && 
              std::chrono::steady_clock::now() - start < std::chrono::milliseconds(100))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        ASSERT_EQ(tm_flag4, 9);
        
        // Let timer destructor run
    }
    
    // Additional cleanup time
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST(timer, move_constructor)
{
    static std::atomic<int> tm_flag5{0};
    tm_flag5 = 0; // Reset flag for this test
    
    {
        // Test timer move constructor with proper shared_ptr management
        // Create two shared_ptr instances to test move semantics at the shared_ptr level
        auto create_timer = []() {
            return std::make_shared<hj::timer>(1000); // 1ms
        };
        
        auto t1 = create_timer();
        auto t2 = std::move(t1); // Move the shared_ptr, not the timer object
        
        // t1 should now be null, t2 should have the timer
        ASSERT_FALSE(t1);
        ASSERT_TRUE(t2);
        
        // Start the timer through the moved shared_ptr
        t2->start([] { tm_flag5 = 10; });
        
        // Wait for callback with timeout
        auto start = std::chrono::steady_clock::now();
        while(tm_flag5 != 10 && 
              std::chrono::steady_clock::now() - start < std::chrono::milliseconds(100))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        ASSERT_EQ(tm_flag5, 10);
        
        // Let timer destructor run
    }
    
    // Additional cleanup time
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}