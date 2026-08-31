#include <gtest/gtest.h>
#include <hj/net/grpc.hpp>
#include <grpcpp/grpcpp.h>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <future>
#include "grpc/grpc_test.grpc.pb.h"

namespace
{

using GrpcLibrary::GrpcService;
using GrpcLibrary::HelloReply;
using GrpcLibrary::HelloRequest;

const char kTestCaCert[] = R"(-----BEGIN CERTIFICATE-----
MIIDAzCCAeugAwIBAgIUBtkR7A8olK8wjiDiYvHIqNpv6dcwDQYJKoZIhvcNAQEL
BQAwETEPMA0GA1UEAwwGVGVzdENBMB4XDTI2MDgzMTE3Mzg0OFoXDTM2MDgyODE3
Mzg0OFowETEPMA0GA1UEAwwGVGVzdENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8A
MIIBCgKCAQEA7FVJ1djGaxPVIoGxbxy5YqxAAEpyl2O85NeSskhg/agKC1weQ7uz
6k33tZy3st7Fer8e8MXPFVU6181xBevgmbO6h7WUmjb5JyPyhGOpjg8yBSeZIE40
aIVhf4oW+podgcPk0nLOkgDkGGaSyjHNpQeQRNEMYZ2C6bfj2nnZTkss+bZiNa+j
r6Nktf2ZiQLOvByhl+L0rGcYKgX8BLIMk3wYtZnEsM5FXrrHEVvVGqQy6QN1ocY1
Oj/7BmIwf0lm9FrUVwRSeWsdzk6ootW5CaVAGLmv3ychdOd+B3geYCm/Hv3OHdH2
uXN5o6hfvnZg6zx9R16DM4EoIsh2NPCthQIDAQABo1MwUTAdBgNVHQ4EFgQUpr7i
jcS7pOnjbAnojhFHkgUTI1owHwYDVR0jBBgwFoAUpr7ijcS7pOnjbAnojhFHkgUT
I1owDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAQEAxyM+062LVN8j
6+5Ns0zDuF3faZgWmuNLTP0hHCPklbwNZgg0LMFMhftrCboXWftXAOHJ5nmSedRr
y8QCbqh/LR/bGqfat7hlIDQVpvnT10bKHX3QhmN0mDO/rbjku0u71rU/RHYdPWCG
diFKRFpxGPePNx9uSeDXDzH+KW1s4fQoFe2yvvntdmleivtGoXiQchJP4WdxxL6E
V7pFh4BvhnT7PA7tDILfLrVlZqjPgQXalQAWYHnmpBcW+3KgHaddsXJltpjeC32v
1Es0jMcR8TZz/t4uuRPG1OPhcxw9fFUq4jj4Nf0zabZUA0W4OOs32/mTwuHAoCja
axo8LKkM3w==
-----END CERTIFICATE-----
)";

const char kTestServerKey[] = R"(-----BEGIN PRIVATE KEY-----
MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCvt7/9nPtnE1El
gX5lbsIlA6Rb3BHD8DKY/Jg0GwKCcnM0g+WGUXAXiC9tFRiKuGaTSPGdo47EtB0j
KIqdlsjRxxxJCVtNANY3hmLq9ySLQ+vXaUfXK5vswWy398dNurwRyi/NQ/o/CIyA
jRvIh43qfsk7xSQboyDWdxoKJmQqx6AfhaWcavCRXaPJzDWSpyYmEFZDa0tQBpIF
1exDlimJi4r7pz6WhdDi9I4sSoerORZSVkReiz7rGlMwI/HldCDbL5XTSEhdJ77v
5ttgz4+8KMKR702ZRRR3HL/jsc5/cAOw8J9pWozXVAPda+RrVCeI3NDt+UlkMRbD
prDb0zzTAgMBAAECggEASa2S34zsrxYe7sqCzJYp6qpBxtCaVF2JYbN8QFDY9wh1
PN5XJcBM71kHhcYUMcpLHFO4F8CmUt0w4HaV1KkOUEOcHRXCxiFlMQf0vw/cr1ZN
89ctq8+ZxCaAZw0KcRGs3Qq5i2iG69oyS46fSTBpdZsu1pR9ZOc8FCEBdqMNWhAD
A+l45Yt4zgKGgh2Rv4ap16HPG8B1iTRtNR3dSOBX0UVF95E/PkQaduZunaWNa18E
f4783DtTEdCTv1EY1e/0T8qhyPZbsuVVBslTvoSigwoC+s1tDgvyGgekyPA089YU
Uhuek9qdd1RHX2NWtkc077Im6tI6rsAlQWhiVbZbMQKBgQDe0jRnDMkaEk8rcxxU
WMtl6bwNIi8arAkHoUib4oToxiEwnVHV10SSByRzvnmvaygsOT6LbNwmcq49eNkp
0GiSk2Z+DFldFqn6SOiFX8+erWg8paF3GMLrM9YPJThHbKRPhxywDnEdkTPFjOzy
JytqhvVI9wxBlIsl9QUP4bv4YwKBgQDJ4f8qz5e3p98k+jhRrdIRcNeihqO4mOeX
w/rrBesJa52ZKiqfVGH63ZMY1wBoOcOtKT8uZzyUf4Y5nJWhPRn0v/aA0INz0XCN
2YGj+dULsMft4LYMZA6o2v6rfAAOjl4znUShHChBHj8SrTPTWGFeVYnOW1UwO8y2
xIui1HT80QKBgCOf2/z10Fk4wCukNc1OfQaIPiajZoCPLMppgOBbcQB+XOQW4vSb
fD0Uo0FWHMJtEXkwlc1Y1EHZZ/4wChhjV7THmEVi2A7IBtkbOwHH9bva4aWAnbYa
4V8VZI37V2oj9ok4gt+rD264vlxBUcjNdmWLuj+2L+F0TGL/L5YAczsdAoGAJbUp
pQRrUSlw24ZFGLR/xTQvgrLlR6h/sXAwoM9VLEhGgoJGvcXUGzhhrkUSVWVcsC5B
F9i+jvoWxxIBPMGd5IIqd7r6oppfDI+w0EDs0ycs4LiVKFFUD6P/52Xc2NssqzVX
yUPqHOeaXlGtmW8rmKQffkS+76hrv0OM1VgbldECgYEAtX0uiWbwpyLJrSexaVtH
R3oo0o8vNswRjfGHpbCHBY1dqpUauaO+gC6GCVGnDio2EYI2o+yHvBwKTSILlgef
kmEbmGPYfANQeqSt6XMap1XSJ5vaJF/5FDwiDYLlO944WBPmDnqmEZ/D0NwJ/QJP
DNDpX1OKncZp4Itsxuly460=
-----END PRIVATE KEY-----)";

const char kTestServerCert[] = R"(-----BEGIN CERTIFICATE-----
MIIDETCCAfmgAwIBAgIUewQ5u+JE40ELEQrih8JVdpAF9r4wDQYJKoZIhvcNAQEL
BQAwETEPMA0GA1UEAwwGVGVzdENBMB4XDTI2MDgzMTE3MzkxNFoXDTM2MDgyODE3
MzkxNFowFDESMBAGA1UEAwwJbG9jYWxob3N0MIIBIjANBgkqhkiG9w0BAQEFAAOC
AQ8AMIIBCgKCAQEAr7e//Zz7ZxNRJYF+ZW7CJQOkW9wRw/AymPyYNBsCgnJzNIPl
hlFwF4gvbRUYirhmk0jxnaOOxLQdIyiKnZbI0cccSQlbTQDWN4Zi6vcki0Pr12lH
1yub7MFst/fHTbq8EcovzUP6PwiMgI0byIeN6n7JO8UkG6Mg1ncaCiZkKsegH4Wl
nGrwkV2jycw1kqcmJhBWQ2tLUAaSBdXsQ5YpiYuK+6c+loXQ4vSOLEqHqzkWUlZE
Xos+6xpTMCPx5XQg2y+V00hIXSe+7+bbYM+PvCjCke9NmUUUdxy/47HOf3ADsPCf
aVqM11QD3Wvka1QniNzQ7flJZDEWw6aw29M80wIDAQABo14wXDAaBgNVHREEEzAR
gglsb2NhbGhvc3SHBH8AAAEwHQYDVR0OBBYEFEdSYGPNpYgxZGwHI2cl6eDMSqGS
MB8GA1UdIwQYMBaAFKa+4o3Eu6Tp42wJ6I4RR5IFEyNaMA0GCSqGSIb3DQEBCwUA
A4IBAQCk0+oCVrMZWSVFCuBtv8jn2Oe3/qJWPBszwoSW9cXET4uZhhY0Bxb+JTag
38YO1bF79uE1cIC3//6fAH57VA66poKrvt4yCGj/JEsahKXmNd9pjZn9W7yDtLcj
06rVtAe5FIIeXjPEulgMLVQYRVIpOT1pBWmglDreO6yMTc3A2Rs1Vxznv3gof+4/
DwWKmTkDzos8iYcZx49naIzqXWIcJBCHNwRm4IGZS1GKM2VqBBAsglx8z/wDTZdj
ppLArt7r5wAxNlL3HqQwdXDTcpIi8GI1yve2GCas3IvQAk5VQVE8R1GCcFfTOPka
cQdJg09+lBCN65IknfyTxZpgp1ZO
-----END CERTIFICATE-----)";

class TestGrpcServiceImpl : public GrpcService::Service
{
  public:
    ::grpc::Status SayHello(::grpc::ServerContext *context,
                            const HelloRequest    *request,
                            HelloReply            *response) override
    {
        (void) context;
        response->set_message("Hello, " + request->name());
        return ::grpc::Status::OK;
    }
};

class GrpcTestFixture : public ::testing::Test
{
  protected:
    TestGrpcServiceImpl service;
};

} // namespace

TEST_F(GrpcTestFixture, say_hello_unary_call)
{
    hj::grpc_server server;
    std::string     address = "127.0.0.1:50062";

    auto err = server.start(address, &service);
    ASSERT_FALSE(err);

    auto channel = hj::grpc_channel::make_shared();
    ASSERT_TRUE(channel->init(address));
    channel->connect();

    ASSERT_FALSE(channel->wait_until_ready(std::chrono::seconds(2)));

    auto stub = GrpcService::NewStub(channel->get());

    HelloRequest req;
    req.set_name("World");
    HelloReply            reply;
    ::grpc::ClientContext ctx;
    ::grpc::Status        status = stub->SayHello(&ctx, req, &reply);

    EXPECT_TRUE(status.ok());
    EXPECT_EQ(reply.message(), "Hello, World");

    EXPECT_FALSE(server.stop());
}

TEST_F(GrpcTestFixture, start_null_service)
{
    hj::grpc_server server;
    std::string     address = "127.0.0.1:50063";

    TestGrpcServiceImpl *null_service = nullptr;
    auto err = server.start<TestGrpcServiceImpl>(address, null_service);

    EXPECT_EQ(err, hj::make_error_code(hj::grpc_errc::invalid_argument));
    EXPECT_FALSE(server.is_running());
}

TEST_F(GrpcTestFixture, duplicate_start)
{
    hj::grpc_server server;
    std::string     address = "127.0.0.1:50064";

    auto err1 = server.start(address, &service);
    EXPECT_FALSE(err1);

    auto err2 = server.start(address, &service);
    EXPECT_EQ(err2, hj::make_error_code(hj::grpc_errc::already_started));

    server.stop();
}

TEST_F(GrpcTestFixture, idempotent_stop)
{
    hj::grpc_server server;
    std::string     address = "127.0.0.1:50065";

    EXPECT_FALSE(server.start(address, &service));

    EXPECT_FALSE(server.stop());
    EXPECT_FALSE(server.is_running());

    EXPECT_FALSE(server.stop());
}

TEST_F(GrpcTestFixture, bind_conflict)
{
    hj::grpc_server server1;
    hj::grpc_server server2;
    std::string     address = "127.0.0.1:50066";

    ASSERT_FALSE(server1.start(address, &service));

    auto err = server2.start(address, &service);
    EXPECT_EQ(err, hj::make_error_code(hj::grpc_errc::bind_failed));

    server1.stop();
}

TEST_F(GrpcTestFixture, rpc_after_server_stop)
{
    hj::grpc_server server;
    std::string     address = "127.0.0.1:50067";

    ASSERT_FALSE(server.start(address, &service));

    auto channel =
        hj::grpc_channel::make_shared(address,
                                      grpc::InsecureChannelCredentials(),
                                      hj::grpc_channel_options());
    ASSERT_TRUE(channel->init(address));
    auto stub = GrpcService::NewStub(channel->get());

    server.stop();

    HelloRequest req;
    req.set_name("World");
    HelloReply            reply;
    ::grpc::ClientContext ctx;
    ctx.set_deadline(std::chrono::system_clock::now()
                     + std::chrono::milliseconds(500));

    ::grpc::Status status = stub->SayHello(&ctx, req, &reply);

    EXPECT_FALSE(status.ok());
}

TEST_F(GrpcTestFixture, channel_uninitialized_state)
{
    auto channel = hj::grpc_channel::make_shared();

    EXPECT_FALSE(channel->is_ready());
    EXPECT_EQ(channel->get(), nullptr);

    auto err = channel->wait_until_ready(std::chrono::milliseconds(100));
    EXPECT_EQ(err, hj::make_error_code(hj::grpc_errc::channel_not_initialized));
}

TEST_F(GrpcTestFixture, invalid_address_rpc)
{
    std::string bad_address = "255.255.255.255:65535";

    auto channel =
        hj::grpc_channel::make_shared(bad_address,
                                      grpc::InsecureChannelCredentials(),
                                      hj::grpc_channel_options());
    channel->connect();

    auto stub = GrpcService::NewStub(channel->get());

    HelloRequest req;
    req.set_name("Test");
    HelloReply            reply;
    ::grpc::ClientContext ctx;
    ctx.set_deadline(std::chrono::system_clock::now()
                     + std::chrono::milliseconds(200));

    ::grpc::Status status = stub->SayHello(&ctx, req, &reply);

    EXPECT_FALSE(status.ok());
    EXPECT_TRUE(status.error_code() == ::grpc::StatusCode::UNAVAILABLE
                || status.error_code()
                       == ::grpc::StatusCode::DEADLINE_EXCEEDED);
}

TEST_F(GrpcTestFixture, concurrent_start_stop_state)
{
    hj::grpc_server   server;
    std::string       address = "127.0.0.1:50068";
    std::atomic<bool> stop_flag{false};

    std::thread reader([&]() {
        while(!stop_flag.load())
        {
            (void) server.is_running();
            (void) server.get_state();
            std::this_thread::yield();
        }
    });

    std::vector<std::thread> workers;
    for(int i = 0; i < 10; ++i)
    {
        workers.emplace_back([&]() {
            if(!server.start(address, &service))
            {
                while(server.get_state() == hj::grpc_server::state::starting)
                {
                    std::this_thread::yield();
                }
                server.stop();
            }
        });
    }

    for(auto &t : workers)
    {
        if(t.joinable())
            t.join();
    }

    stop_flag.store(true);
    if(reader.joinable())
        reader.join();

    EXPECT_FALSE(server.is_running());
    EXPECT_EQ(server.get_state(), hj::grpc_server::state::stopped);
}

TEST_F(GrpcTestFixture, concurrent_rpc_requests)
{
    hj::grpc_server server;
    std::string     address = "127.0.0.1:50069";
    ASSERT_FALSE(server.start(address, &service));

    auto channel =
        hj::grpc_channel::make_shared(address,
                                      grpc::InsecureChannelCredentials(),
                                      hj::grpc_channel_options());
    ASSERT_TRUE(channel->init(address));
    channel->connect();

    ASSERT_FALSE(channel->wait_until_ready(std::chrono::seconds(2)));

    const int                thread_count = 50;
    std::vector<std::thread> threads;
    std::atomic<int>         success_count{0};

    for(int i = 0; i < thread_count; ++i)
    {
        threads.emplace_back([&channel, &success_count, i]() {
            auto         stub = GrpcService::NewStub(channel->get());
            HelloRequest req;
            req.set_name("Thread " + std::to_string(i));
            HelloReply            reply;
            ::grpc::ClientContext ctx;

            ::grpc::Status status = stub->SayHello(&ctx, req, &reply);
            if(status.ok()
               && reply.message() == "Hello, Thread " + std::to_string(i))
            {
                success_count.fetch_add(1);
            }
        });
    }

    for(auto &t : threads)
    {
        if(t.joinable())
            t.join();
    }

    EXPECT_EQ(success_count.load(), thread_count);
    server.stop();
}

TEST_F(GrpcTestFixture, server_lifecycle_stress_test)
{
    hj::grpc_server server;
    std::string     address    = "127.0.0.1:50070";
    const int       iterations = 100;

    for(int i = 0; i < iterations; ++i)
    {
        ASSERT_FALSE(server.start(address, &service));
        EXPECT_TRUE(server.is_running());

        ASSERT_FALSE(server.stop());
        EXPECT_FALSE(server.is_running());
    }
}

TEST_F(GrpcTestFixture, deterministic_stop_during_starting)
{
    hj::grpc_server server;
    std::string     address = "127.0.0.1:50071";

    std::thread start_thread([&]() { server.start(address, &service); });

    while(server.get_state() != hj::grpc_server::state::starting)
    {
        std::this_thread::yield();
    }

    EXPECT_FALSE(server.stop());

    if(start_thread.joinable())
        start_thread.join();

    EXPECT_EQ(server.get_state(), hj::grpc_server::state::stopped);
    EXPECT_FALSE(server.is_running());
}

TEST_F(GrpcTestFixture, restart_after_natural_wait_completion)
{
    hj::grpc_server server;
    std::string     address = "127.0.0.1:50072";

    ASSERT_FALSE(server.start(address, &service));
    EXPECT_TRUE(server.is_running());

    server.stop();
    EXPECT_FALSE(server.is_running());

    ASSERT_FALSE(server.start(address, &service));
    EXPECT_TRUE(server.is_running());

    server.stop();
}

TEST_F(GrpcTestFixture, channel_wait_shutdown_semantic)
{
    auto uninit_channel = hj::grpc_channel::make_shared();

    EXPECT_EQ(uninit_channel->wait_until_ready(std::chrono::milliseconds(50)),
              hj::make_error_code(hj::grpc_errc::channel_not_initialized));

    auto timeout_channel =
        hj::grpc_channel::make_shared("192.0.2.1:12345",
                                      grpc::InsecureChannelCredentials(),
                                      hj::grpc_channel_options());
    timeout_channel->connect();
    EXPECT_EQ(timeout_channel->wait_until_ready(std::chrono::milliseconds(100)),
              hj::make_error_code(hj::grpc_errc::connection_timeout));
}

TEST_F(GrpcTestFixture, start_vs_stop_barrier_race)
{
    hj::grpc_server server;
    std::string     address = "127.0.0.1:50074";

    std::promise<void> start_reached_barrier;
    std::promise<void> allow_start_to_proceed;
    auto allow_future = allow_start_to_proceed.get_future().share();

    // 1. 设置 Hook：在 BuildAndStart() 前卡住 start 线程
    server.set_before_build_hook([&]() {
        start_reached_barrier
            .set_value(); // 通知主线程：start 已经处于 starting 状态并准备 Build
        allow_future.wait(); // 等待主线程放行
    });

    // 2. 异步启动 server.start
    std::future<std::error_code> start_future =
        std::async(std::launch::async,
                   [&]() { return server.start(address, &service); });

    // 3. 确保 start 已经进入了 target 逻辑（处于 starting 状态）
    start_reached_barrier.get_future().wait();
    EXPECT_EQ(server.get_state(), hj::grpc_server::state::starting);

    // 4. 在另一个线程并发调用 stop()（此时 stop 会阻塞在 wait 屏障，等待 state 不为 starting）
    std::future<std::error_code> stop_future =
        std::async(std::launch::async, [&]() { return server.stop(); });

    // 5. 放行 start()
    allow_start_to_proceed.set_value();

    // 6. 验证结果：start 和 stop 都应该正常返回，Server 最终必定为 stopped
    auto start_err = start_future.get();
    auto stop_err  = stop_future.get();

    EXPECT_FALSE(start_err);
    EXPECT_FALSE(stop_err);
    EXPECT_FALSE(server.is_running());
    EXPECT_EQ(server.get_state(), hj::grpc_server::state::stopped);
}

// ==========================================
// 2. Wait() 自然退出与状态转变测试
// ==========================================
TEST_F(GrpcTestFixture, wait_natural_completion_and_restart)
{
    hj::grpc_server server;
    std::string     address = "127.0.0.1:50075";

    ASSERT_FALSE(server.start(address, &service));
    EXPECT_TRUE(server.is_running());

    // 创建后台线程等待 Wait() 自然退出
    std::atomic<bool> wait_finished{false};
    std::thread       wait_thread([&]() {
        server.wait();
        wait_finished.store(true);
    });

    // 从外部 stop 触发服务器关闭
    EXPECT_FALSE(server.stop());

    if(wait_thread.joinable())
        wait_thread.join();

    // 验证 wait() 确实解除了阻塞，且状态正确切回 stopped
    EXPECT_TRUE(wait_finished.load());
    EXPECT_EQ(server.get_state(), hj::grpc_server::state::stopped);

    // 验证自然退出后可以成功 Restart
    ASSERT_FALSE(server.start(address, &service));
    EXPECT_TRUE(server.is_running());
    EXPECT_FALSE(server.stop());
}

// ==========================================
// 3. 完整的 Restart + RPC 校验测试
// ==========================================
TEST_F(GrpcTestFixture, full_restart_with_rpc_verification)
{
    hj::grpc_server server;
    std::string     address = "127.0.0.1:50076";

    auto execute_rpc = [&](const std::string &name_suffix) {
        auto channel =
            hj::grpc_channel::make_shared(address,
                                          grpc::InsecureChannelCredentials(),
                                          hj::grpc_channel_options());
        channel->connect();
        EXPECT_FALSE(channel->wait_until_ready(std::chrono::seconds(2)));

        auto         stub = GrpcService::NewStub(channel->get());
        HelloRequest req;
        req.set_name("Client_" + name_suffix);
        HelloReply            reply;
        ::grpc::ClientContext ctx;

        ::grpc::Status status = stub->SayHello(&ctx, req, &reply);
        EXPECT_TRUE(status.ok());
        EXPECT_EQ(reply.message(), "Hello, Client_" + name_suffix);
    };

    // 第一次 Lifecycle: Start -> RPC -> Stop
    ASSERT_FALSE(server.start(address, &service));
    execute_rpc("Round1");
    EXPECT_FALSE(server.stop());
    EXPECT_FALSE(server.is_running());

    // 第二次 Lifecycle: Start -> RPC -> Stop (重启测试)
    ASSERT_FALSE(server.start(address, &service));
    execute_rpc("Round2");
    EXPECT_FALSE(server.stop());
    EXPECT_FALSE(server.is_running());
}

// ==========================================
// 4. TLS 与 mTLS 传输安全测试
// ==========================================
TEST_F(GrpcTestFixture, tls_and_mtls_rpc_communication)
{
    std::string address = "127.0.0.1:50077";

    // --- 1. 单向 TLS 测试 (Server 加密，Client 验证 Server) ---
    {
        hj::grpc_server server;
        auto            server_creds =
            hj::make_tls_server_credentials("",
                                            kTestServerCert,
                                            kTestServerKey,
                                            false /* require_client_cert */);

        ASSERT_FALSE(server.start(address, &service, server_creds));

        grpc::SslCredentialsOptions ssl_opts;
        ssl_opts.pem_root_certs = kTestCaCert;
        auto client_creds       = grpc::SslCredentials(ssl_opts);

        auto channel =
            hj::grpc_channel::make_shared(address,
                                          client_creds,
                                          hj::grpc_channel_options());
        channel->connect();
        ASSERT_FALSE(channel->wait_until_ready(std::chrono::seconds(2)));

        auto                  stub = GrpcService::NewStub(channel->get());
        HelloRequest          req;
        HelloReply            reply;
        ::grpc::ClientContext ctx;
        req.set_name("TLS_User");

        ::grpc::Status status = stub->SayHello(&ctx, req, &reply);
        EXPECT_TRUE(status.ok());
        EXPECT_EQ(reply.message(), "Hello, TLS_User");

        server.stop();
    }

    // --- 2. 双向 mTLS 测试 (Server & Client 互相校验) ---
    {
        hj::grpc_server server;
        std::string     mtls_address = "127.0.0.1:50078";

        auto mtls_server_creds =
            hj::make_tls_server_credentials(kTestCaCert,
                                            kTestServerCert,
                                            kTestServerKey,
                                            true /* require_client_cert */);

        ASSERT_FALSE(server.start(mtls_address, &service, mtls_server_creds));

        grpc::SslCredentialsOptions ssl_opts;
        ssl_opts.pem_root_certs  = kTestCaCert;
        ssl_opts.pem_cert_chain  = kTestServerCert; // 使用有效的客户端证书链
        ssl_opts.pem_private_key = kTestServerKey;
        auto mtls_client_creds   = grpc::SslCredentials(ssl_opts);

        auto channel =
            hj::grpc_channel::make_shared(mtls_address,
                                          mtls_client_creds,
                                          hj::grpc_channel_options());
        channel->connect();
        ASSERT_FALSE(channel->wait_until_ready(std::chrono::seconds(2)));

        auto                  stub = GrpcService::NewStub(channel->get());
        HelloRequest          req;
        HelloReply            reply;
        ::grpc::ClientContext ctx;
        req.set_name("mTLS_User");

        ::grpc::Status status = stub->SayHello(&ctx, req, &reply);
        EXPECT_TRUE(status.ok());
        EXPECT_EQ(reply.message(), "Hello, mTLS_User");

        server.stop();
    }
}