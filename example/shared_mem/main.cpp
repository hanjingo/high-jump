#include <iostream>
#include <thread>

#include <libcpp/sync/shared_memory.hpp>

int main(int argc, char* argv[])
{
    // [shared memory] single byte model
    boost::interprocess::shared_memory_object::remove("mem");
    boost::interprocess::shared_memory_object mem(boost::interprocess::open_or_create, 
                                                  "mem", 
                                                  boost::interprocess::read_write);
    mem.truncate(1024);
    boost::interprocess::offset_t sz;
    if (mem.get_size(sz))
        std::cout << mem.get_name() << ":" << sz << std::endl;

    boost::interprocess::mapped_region rg1(mem, boost::interprocess::read_write);
    int *addr = static_cast<int*>(rg1.get_address());
    *addr = 123;

    boost::interprocess::mapped_region rg2(mem, boost::interprocess::read_write);
    int *ptr2 = static_cast<int*>(rg2.get_address());
    std::cout << *ptr2 << std::endl;
    
    std::cout << "mem " << (boost::interprocess::shared_memory_object::remove("mem") ? "remove succ" : "remove fail") << std::endl;


    // [shared memory] mgr model
    boost::interprocess::shared_memory_object::remove("mgr_mem");
    boost::interprocess::managed_shared_memory mgr_mem(boost::interprocess::open_or_create, 
                                                       "mgr_mem", 
                                                       1024);
    addr = mgr_mem.construct<int>("i1")(456);
    std::cout << addr << ":" << *addr << std::endl;
    std::pair<int*, std::size_t> ptr3 = mgr_mem.find<int>("i1");
    if (ptr3.first)
        std::cout << *ptr3.first << std::endl;

    addr = mgr_mem.find_or_construct<int>("arr1")[5](0);
    for (int i = 0; i < 5; i++, addr++)
        *addr = i;
    std::pair<int*, std::size_t> ptr4 = mgr_mem.find<int>("arr1");
    for (int i = 0; i < 5; i++, addr++)
        std::cout << "[" << i << "]:" << *(ptr4.first + i) << std::endl;
    boost::interprocess::shared_memory_object::remove("mgr_mem");


    // [shared memory] mutex
    boost::interprocess::managed_shared_memory mgr_mem1(boost::interprocess::open_or_create, 
                                                        "mgr_mem1", 
                                                        1024);
    addr = mgr_mem.find_or_construct<int>("arr1")[5](0);
    for (int i = 0; i < 5; i++, addr++)
        *addr = i;
    boost::interprocess::named_mutex mu(boost::interprocess::open_or_create, "mu");
    mu.lock();
    for (int i = 0; i < 5; i++, addr++)
        *addr = i;
    ptr4 = mgr_mem.find<int>("arr1");
    for (int i = 0; i < 5; i++, addr++)
        std::cout << "[" << i << "]:" << *(ptr4.first + i) << std::endl;
    mu.unlock();
    boost::interprocess::shared_memory_object::remove("mgr_mem1");

    boost::interprocess::named_condition cond(boost::interprocess::open_or_create, "cond");
    boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(mu);
    ptr4 = mgr_mem.find<int>("arr1");
    while (*ptr4.first < 5) 
    { 
        if (*ptr4.first % 2 == 0) 
        { 
            std::cout << *ptr4.first << std::endl; 
            ptr4.first++;
            cond.notify_all();
        } 
        else 
        { 
            std::cout << *ptr4.first << std::endl; 
            ptr4.first++;
            cond.notify_all(); 
        } 
        cond.wait(lock); 
    }

    std::cin.get();
    return 0;
}