#include <string>
#include <iostream>
#include <thread>

#include <libcpp/net/zmq/zmq_chan.hpp>
#include <libcpp/net/zmq/zmq_publisher.hpp>
#include <libcpp/net/zmq/zmq_subscriber.hpp>

static void* ctx = zmq_ctx_new();

static libcpp::zmq_publisher puber{ctx};
static libcpp::zmq_subscriber suber1{ctx};
static libcpp::zmq_subscriber suber2{ctx};

void pub_sub()
{
    std::cout << "pub-sub example" << std::endl;

    std::cout << "puber bind tcp://*:10087 with ret = " 
        << puber.bind("tcp://*:10087") << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "suber1 connect: tcp://localhost:10087 with ret = " 
        << suber1.connect("tcp://localhost:10087") << std::endl;
    std::cout << "suber2 connect: tcp://localhost:10087 with ret = " 
        << suber2.connect("tcp://localhost:10087") << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "suber1 sub hello with ret = " << suber1.sub("hello") << std::endl;
    std::cout << "suber2 sub hello with ret = " << suber2.sub("hello") << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::string msg{"hello;001"};
    std::cout << "puber unsafe_pub with ret = " << puber.unsafe_pub(msg) 
        << ", msg = " << msg << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::string recv{};
    std::cout << "suber1 ret:" << suber1.recv(recv) << ", recv:" << recv << std::endl;
    std::cout << "suber2 ret:" << suber2.recv(recv) << ", recv:" << recv << std::endl;

    // std::string msg1{"hello;002"};
    // if (!puber.pub(msg1)) {
    //     return;
    // }
    // std::cout << "puber pub: " << "hello;002" << std::endl;
    // std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // std::cout << "suber1 ret:" << suber1.recv(recv) << ", recv:" << recv << std::endl;
    // std::cout << "suber2 ret:" << suber2.recv(recv) << ", recv:" << recv << std::endl;

    std::cout << "pub-sub example end\n" << std::endl;
}

int main(int argc, char* argv[])
{
    pub_sub();

    std::cin.get();
    return 0;
}