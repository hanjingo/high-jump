#include <benchmark/benchmark.h>

#include <hj/net/zmq.hpp>
#include <string>
#include <atomic>
#include <cstdlib>

static std::atomic_uint_fast64_t s_zmq_id{0};

static bool bench_allow_zmq()
{
    return std::getenv("HJ_BENCH_ALLOW_ZMQ") != nullptr;
}

static std::string make_inproc_addr()
{
    auto               id = s_zmq_id.fetch_add(1, std::memory_order_relaxed);
    std::ostringstream ss;
    ss << "inproc://zmq_bench_" << std::hex << id;
    return ss.str();
}

// chan basic: push a string and pop it
static void bm_zmq_chan_basic(benchmark::State &state)
{
    if(!bench_allow_zmq())
        state.SkipWithError("HJ_BENCH_ALLOW_ZMQ not set");

    for(auto _ : state)
    {
        hj::zmq::context ctx;
        hj::zmq::chan    ch(ctx.get());
        std::string      msg = "hello zmq";
        bool             ok  = ch << msg;
        benchmark::DoNotOptimize(ok);
        std::string recv;
        ok = ch >> recv;
        benchmark::DoNotOptimize(recv);
        benchmark::ClobberMemory();
    }
}

// producer/consumer push/pull using inproc address
static void bm_zmq_producer_consumer(benchmark::State &state)
{
    if(!bench_allow_zmq())
        state.SkipWithError("HJ_BENCH_ALLOW_ZMQ not set");

    for(auto _ : state)
    {
        hj::zmq::context  ctx;
        std::string       addr = make_inproc_addr();
        hj::zmq::producer prod(ctx.get());
        hj::zmq::consumer cons(ctx.get());

        int b = prod.bind(addr);
        benchmark::DoNotOptimize(b);
        int c = cons.connect(addr);
        benchmark::DoNotOptimize(c);

        std::string msg    = "data";
        int         pushed = prod.push(msg);
        benchmark::DoNotOptimize(pushed);

        std::string recv;
        int         pulled = cons.pull(recv);
        benchmark::DoNotOptimize(pulled);
        benchmark::DoNotOptimize(recv);
        benchmark::ClobberMemory();
    }
}

// publisher/subscriber
static void bm_zmq_pubsub(benchmark::State &state)
{
    if(!bench_allow_zmq())
        state.SkipWithError("HJ_BENCH_ALLOW_ZMQ not set");

    for(auto _ : state)
    {
        hj::zmq::context    ctx;
        std::string         addr = make_inproc_addr();
        hj::zmq::publisher  pub(ctx.get());
        hj::zmq::subscriber sub(ctx.get());

        int pb = pub.bind(addr);
        benchmark::DoNotOptimize(pb);
        int sc = sub.connect(addr);
        benchmark::DoNotOptimize(sc);
        sub.sub("");

        std::string msg  = "pubmsg";
        int         sent = pub.pub(msg);
        benchmark::DoNotOptimize(sent);

        std::string recv;
        int         r = sub.recv(recv);
        benchmark::DoNotOptimize(r);
        benchmark::DoNotOptimize(recv);
        benchmark::ClobberMemory();
    }
}

// broker bind cost
static void bm_zmq_broker_bind(benchmark::State &state)
{
    if(!bench_allow_zmq())
        state.SkipWithError("HJ_BENCH_ALLOW_ZMQ not set");

    for(auto _ : state)
    {
        hj::zmq::context ctx;
        std::string      xpub = make_inproc_addr();
        std::string      xsub = make_inproc_addr();

        hj::zmq::socket xpub_sock(ctx.get(), ZMQ_XPUB);
        hj::zmq::socket xsub_sock(ctx.get(), ZMQ_XSUB);
        hj::zmq::broker bk(xpub_sock.get(), xsub_sock.get());

        int b = bk.bind(xpub, xsub);
        benchmark::DoNotOptimize(b);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(bm_zmq_chan_basic)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_zmq_producer_consumer)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_zmq_pubsub)->Unit(benchmark::kMicrosecond);
BENCHMARK(bm_zmq_broker_bind)->Unit(benchmark::kMicrosecond);
