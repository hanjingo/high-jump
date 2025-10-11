#include <gtest/gtest.h>
#include <hj/net/zmq.hpp>
#include <zmq.h>

TEST(zmq, chan_basic)
{
    hj::zmq::context ctx;
    hj::zmq::chan    ch(ctx.get());
    std::string      msg = "hello zmq";
    ASSERT_TRUE(ch << msg);
    std::string recv;
    ASSERT_TRUE(ch >> recv);
    ASSERT_EQ(recv, msg);
}

TEST(zmq, producer_consumer_basic)
{
    hj::zmq::context  ctx;
    hj::zmq::producer prod(ctx.get());
    hj::zmq::consumer cons(ctx.get());
    std::string       addr = "inproc://test-pc";
    ASSERT_EQ(prod.bind(addr), 0);
    ASSERT_EQ(cons.connect(addr), 0);
    std::string msg = "data";
    ASSERT_GT(prod.push(msg), 0);
    std::string recv;
    ASSERT_GT(cons.pull(recv), 0);
    ASSERT_EQ(recv, msg);
}

TEST(zmq, publisher_subscriber_basic)
{
    hj::zmq::context    ctx;
    hj::zmq::publisher  pub(ctx.get());
    hj::zmq::subscriber sub(ctx.get());
    std::string         addr = "inproc://test-ps";
    ASSERT_EQ(pub.bind(addr), 0);
    ASSERT_EQ(sub.connect(addr), 0);
    ASSERT_EQ(sub.sub(""), 0);
    std::string msg = "pubmsg";
    ASSERT_GT(pub.pub(msg), 0);
    std::string recv;
    ASSERT_GT(sub.recv(recv), 0);
    ASSERT_EQ(recv, msg);
}

// TEST(zmq, pubsub_broker_lifecycle)
// {
//     hj::zmq::context ctx;
//     auto             xpub = zmq_socket(&ctx, ZMQ_XPUB);
//     auto             xsub = zmq_socket(&ctx, ZMQ_XSUB);
//     hj::zmq::broker  bk(xpub, xsub);
//     std::string      xpub_addr = "inproc://xpub";
//     std::string      xsub_addr = "inproc://xsub";
//     ASSERT_EQ(bk.bind(xpub_addr, xsub_addr), 0);
// }