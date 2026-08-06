#include <gtest/gtest.h>
#include <hj/net/zmq.hpp>
#include <zmq.h>
#include <thread>
#include <chrono>
#include <vector>

TEST(zmq, context_and_message_basic)
{
    EXPECT_NO_THROW({
        auto ctx = hj::zmq::context::create();
        ASSERT_NE(ctx->get(), nullptr);
    });

    hj::zmq::message msg1;
    ASSERT_NE(msg1.get(), nullptr);
    ASSERT_EQ(msg1.size(), 0u);

    hj::zmq::message msg2(32);
    ASSERT_NE(msg2.get(), nullptr);
    ASSERT_EQ(msg2.size(), 32u);
    ASSERT_NE(msg2.data(), nullptr);

    hj::zmq::message msg3 = std::move(msg2);
    ASSERT_EQ(msg3.size(), 32u);
    ASSERT_EQ(msg2.size(), 0u);

    static bool freed = false;
    {
        char            *buffer = new char[100];
        hj::zmq::message zero_copy_msg(
            buffer,
            100,
            [](void *data, void *hint) {
                delete[] static_cast<char *>(data);
                bool *p_freed = static_cast<bool *>(hint);
                if(p_freed)
                    *p_freed = true;
            },
            &freed);
        ASSERT_EQ(zero_copy_msg.size(), 100u);
    }
    ASSERT_TRUE(freed);
}

TEST(zmq, exception_handling)
{
    auto ctx = hj::zmq::context::create();
    EXPECT_THROW(
        { hj::zmq::r_chan r_ch(ctx, "invalid_protocol://address"); },
        hj::zmq::zmq_error);
}

TEST(zmq, chan_basic)
{
    auto            ctx = hj::zmq::context::create();
    hj::zmq::w_chan w_ch(ctx);
    hj::zmq::r_chan r_ch = w_ch.make_r_chan();

    std::string msg = "hello zmq chan";
    ASSERT_TRUE(w_ch << msg);

    std::string recv_str;
    ASSERT_TRUE(r_ch >> recv_str);
    ASSERT_EQ(recv_str, msg);
}

TEST(zmq, producer_consumer_basic)
{
    auto              ctx = hj::zmq::context::create();
    hj::zmq::producer prod(ctx);
    hj::zmq::consumer cons(ctx);
    std::string       addr = "inproc://test-pc-basic";

    ASSERT_EQ(prod.bind(addr), 0);
    ASSERT_EQ(cons.connect(addr), 0);

    std::string msg = "producer_consumer_data";
    ASSERT_GT(prod.push(msg), 0);

    std::string recv;
    ASSERT_GT(cons.pull(recv), 0);
    ASSERT_EQ(recv, msg);
}

TEST(zmq, multipart_message_basic)
{
    auto              ctx = hj::zmq::context::create();
    hj::zmq::producer prod(ctx);
    hj::zmq::consumer cons(ctx);
    std::string       addr = "inproc://test-multipart-basic";

    ASSERT_EQ(prod.bind(addr), 0);
    ASSERT_EQ(cons.connect(addr), 0);

    std::vector<std::string_view> send_frames = {"Frame1", "Frame2", "Frame3"};
    ASSERT_TRUE(prod.send_multipart(send_frames));

    std::vector<std::string> recv_frames;
    ASSERT_TRUE(cons.recv_multipart(recv_frames));

    ASSERT_EQ(recv_frames.size(), 3u);
    ASSERT_EQ(recv_frames[0], "Frame1");
    ASSERT_EQ(recv_frames[1], "Frame2");
    ASSERT_EQ(recv_frames[2], "Frame3");
}

TEST(zmq, producer_consumer_safe_pipeline)
{
    auto        ctx  = hj::zmq::context::create();
    std::string addr = "inproc://test-safe-pc-pipeline";

    hj::zmq::w_chan w_ch_in(ctx);
    hj::zmq::w_chan w_ch_out(ctx);

    hj::zmq::r_chan r_ch_out = w_ch_out.make_r_chan();

    std::thread cons_thread([ctx, &addr, &w_ch_out]() {
        hj::zmq::consumer cons(ctx);
        ASSERT_EQ(cons.connect(addr), 0);
        cons.safe_pull(w_ch_out, 0);
    });

    std::thread prod_thread(
        [ctx, &addr, r_ch = w_ch_in.make_r_chan()]() mutable {
            hj::zmq::producer prod(ctx);
            ASSERT_EQ(prod.bind(addr), 0);
            prod.safe_push(r_ch, 0);
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    w_ch_in << "msg_1";
    w_ch_in << "msg_2";
    w_ch_in << "";

    std::string res1, res2, res3;

    ASSERT_TRUE(r_ch_out >> res1);
    ASSERT_EQ(res1, "msg_1");

    ASSERT_TRUE(r_ch_out >> res2);
    ASSERT_EQ(res2, "msg_2");

    ASSERT_TRUE(r_ch_out >> res3);
    ASSERT_TRUE(res3.empty());

    if(prod_thread.joinable())
        prod_thread.join();
    if(cons_thread.joinable())
        cons_thread.join();
}

TEST(zmq, publisher_subscriber_basic)
{
    auto                ctx = hj::zmq::context::create();
    hj::zmq::publisher  pub(ctx);
    hj::zmq::subscriber sub(ctx);
    std::string         addr = "inproc://test-ps-basic";

    ASSERT_EQ(pub.bind(addr), 0);
    ASSERT_EQ(sub.connect(addr), 0);
    ASSERT_EQ(sub.sub(""), 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::string msg = "pubmsg";
    ASSERT_GT(pub.pub(msg), 0);

    std::string recv;
    ASSERT_GT(sub.recv(recv), 0);
    ASSERT_EQ(recv, msg);
}

TEST(zmq, publisher_subscriber_safe_pipeline)
{
    auto        ctx  = hj::zmq::context::create();
    std::string addr = "inproc://test-safe-ps-pipeline";

    hj::zmq::w_chan w_ch_in(ctx);
    hj::zmq::w_chan w_ch_out(ctx);

    hj::zmq::r_chan r_ch_out = w_ch_out.make_r_chan();

    std::thread sub_thread([ctx, &addr, &w_ch_out]() {
        hj::zmq::subscriber sub(ctx);
        ASSERT_EQ(sub.connect(addr), 0);
        ASSERT_EQ(sub.sub(""), 0);
        sub.safe_recv(w_ch_out, 0);
    });

    std::thread pub_thread(
        [ctx, &addr, r_ch = w_ch_in.make_r_chan()]() mutable {
            hj::zmq::publisher pub(ctx);
            ASSERT_EQ(pub.bind(addr), 0);
            pub.safe_pub(r_ch, 0);
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    w_ch_in << "pub_data_1";
    w_ch_in << "";

    std::string res1, res2;

    ASSERT_TRUE(r_ch_out >> res1);
    ASSERT_EQ(res1, "pub_data_1");

    ASSERT_TRUE(r_ch_out >> res2);
    ASSERT_TRUE(res2.empty());

    if(pub_thread.joinable())
        pub_thread.join();
    if(sub_thread.joinable())
        sub_thread.join();
}

TEST(zmq, broker_basic)
{
    auto            ctx = hj::zmq::context::create();
    hj::zmq::socket xpub(ctx, ZMQ_XPUB);
    hj::zmq::socket xsub(ctx, ZMQ_XSUB);
    hj::zmq::broker bk(xpub.get(), xsub.get());
    std::string     xpub_addr = "inproc://xpub-broker";
    std::string     xsub_addr = "inproc://xsub-broker";

    ASSERT_EQ(bk.bind(xpub_addr, xsub_addr), 0);
}

TEST(zmq, poller_basic)
{
    auto              ctx = hj::zmq::context::create();
    hj::zmq::producer prod(ctx);
    hj::zmq::consumer cons(ctx);
    std::string       addr = "inproc://test-poller-basic";

    ASSERT_EQ(prod.bind(addr), 0);
    ASSERT_EQ(cons.connect(addr), 0);

    hj::zmq::poller poller;
    int             user_tag = 100;
    poller.add(cons, &user_tag, ZMQ_POLLIN);

    int rc1 = poller.poll(10);
    ASSERT_EQ(rc1, 0);

    ASSERT_GT(prod.push("poller_test"), 0);

    int rc2 = poller.poll(1000);
    ASSERT_EQ(rc2, 1);
    ASSERT_TRUE(poller.items()[0].revents & ZMQ_POLLIN);
    ASSERT_EQ(*static_cast<int *>(poller.user_data(0)), 100);

    std::string recv_data;
    ASSERT_GT(cons.pull(recv_data), 0);
    ASSERT_EQ(recv_data, "poller_test");
}