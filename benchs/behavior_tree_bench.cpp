#include <benchmark/benchmark.h>
#include <hj/algo/behavior_tree.hpp>
#include <memory>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <sstream>

static const char *simple_xml = R"(
<root hjCPP_format="4">
    <BehaviorTree ID="SimpleTree">
        <Sequence name="root">
            <AlwaysSuccess/>
            <TestAction message="Hello"/>
        </Sequence>
    </BehaviorTree>
</root>
)";

static const char *complex_xml = R"(
<root hjCPP_format="4">
    <BehaviorTree ID="ComplexTree">
        <Fallback name="root">
            <Sequence name="primary">
                <TestCondition/>
                <TestAction message="Primary action"/>
                <SetBlackboard output_key="result" value="success"/>
            </Sequence>
            <Sequence name="secondary">
                <TestAction message="Secondary action"/>
                <SetBlackboard output_key="result" value="fallback"/>
            </Sequence>
        </Fallback>
    </BehaviorTree>
</root>
)";

class TestAction : public hj::sync_action_node
{
  public:
    TestAction(const std::string &name, const hj::node_configuration &config)
        : hj::sync_action_node(name, config)
    {
    }

    hj::node_status tick() override
    {
        std::string msg;
        getInput("message", msg);
        volatile int sum = 0;
        for(int i = 0; i < 100; ++i)
        {
            sum += i;
        }
        return hj::node_status::SUCCESS;
    }

    static hj::ports_list providedPorts()
    {
        return {hj::input_port<std::string>("message", "Action message")};
    }
};

class TestCondition : public hj::sync_action_node
{
  public:
    TestCondition(const std::string &name, const hj::node_configuration &config)
        : hj::sync_action_node(name, config)
        , counter_(0)
    {
    }

    hj::node_status tick() override
    {
        counter_++;
        return (counter_ % 3 == 0) ? hj::node_status::SUCCESS
                                   : hj::node_status::FAILURE;
    }

    static hj::ports_list providedPorts() { return {}; }

  private:
    int counter_;
};

class AsyncTestAction : public hj::async_action_node
{
  public:
    AsyncTestAction(const std::string            &name,
                    const hj::node_configuration &config)
        : hj::async_action_node(name, config)
    {
    }

    hj::node_status tick() override
    {
        std::string msg;
        getInput("message", msg);

        std::this_thread::sleep_for(std::chrono::microseconds(10));

        return hj::node_status::SUCCESS;
    }

    static hj::ports_list providedPorts()
    {
        return {hj::input_port<std::string>("message", "Async action message")};
    }
};

class StatefulTestAction : public hj::stateful_action_node
{
  public:
    StatefulTestAction(const std::string            &name,
                       const hj::node_configuration &config)
        : hj::stateful_action_node(name, config)
        , step_(0)
    {
    }

    hj::node_status onStart() override
    {
        step_ = 0;
        return hj::node_status::RUNNING;
    }

    hj::node_status onRunning() override
    {
        step_++;
        if(step_ >= 3)
        {
            return hj::node_status::SUCCESS;
        }
        return hj::node_status::RUNNING;
    }

    void onHalted() override { step_ = 0; }

    static hj::ports_list providedPorts() { return {}; }

  private:
    int step_;
};

static void bm_factory_creation(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::behavior_tree_factory factory;
        benchmark::DoNotOptimize(&factory);
    }
}
BENCHMARK(bm_factory_creation);

static void bm_node_registration(benchmark::State &state)
{
    for(auto _ : state)
    {
        hj::behavior_tree_factory factory;
        factory.registerNodeType<TestAction>("TestAction");
        factory.registerNodeType<TestCondition>("TestCondition");
        factory.registerNodeType<AsyncTestAction>("AsyncTestAction");
        factory.registerNodeType<StatefulTestAction>("StatefulTestAction");
        benchmark::DoNotOptimize(&factory);
    }
}
BENCHMARK(bm_node_registration);

static void bm_simple_tree_creation(benchmark::State &state)
{
    hj::behavior_tree_factory factory;
    factory.registerNodeType<TestAction>("TestAction");

    for(auto _ : state)
    {
        auto tree = factory.createTreeFromText(simple_xml);
        benchmark::DoNotOptimize(&tree);
    }
}
BENCHMARK(bm_simple_tree_creation);

static void bm_complex_tree_creation(benchmark::State &state)
{
    hj::behavior_tree_factory factory;
    factory.registerNodeType<TestAction>("TestAction");
    factory.registerNodeType<TestCondition>("TestCondition");

    for(auto _ : state)
    {
        auto tree = factory.createTreeFromText(complex_xml);
        benchmark::DoNotOptimize(&tree);
    }
}
BENCHMARK(bm_complex_tree_creation);

static void bm_simple_tree_execution(benchmark::State &state)
{
    hj::behavior_tree_factory factory;
    factory.registerNodeType<TestAction>("TestAction");
    auto tree = factory.createTreeFromText(simple_xml);

    for(auto _ : state)
    {
        auto status = tree.tickWhileRunning();
        benchmark::DoNotOptimize(status);
    }
}
BENCHMARK(bm_simple_tree_execution);

static void bm_complex_tree_execution(benchmark::State &state)
{
    hj::behavior_tree_factory factory;
    factory.registerNodeType<TestAction>("TestAction");
    factory.registerNodeType<TestCondition>("TestCondition");
    auto tree = factory.createTreeFromText(complex_xml);

    for(auto _ : state)
    {
        auto status = tree.tickWhileRunning();
        benchmark::DoNotOptimize(status);
    }
}
BENCHMARK(bm_complex_tree_execution);

static void bm_multiple_ticks(benchmark::State &state)
{
    const int tick_count = static_cast<int>(state.range(0));

    hj::behavior_tree_factory factory;
    factory.registerNodeType<TestAction>("TestAction");
    factory.registerNodeType<TestCondition>("TestCondition");
    auto tree = factory.createTreeFromText(complex_xml);

    for(auto _ : state)
    {
        for(int i = 0; i < tick_count; ++i)
        {
            auto status = tree.tickWhileRunning();
            benchmark::DoNotOptimize(status);
        }
    }
}
BENCHMARK(bm_multiple_ticks)->Arg(10)->Arg(50)->Arg(100)->Arg(500);

static void bm_async_node_execution(benchmark::State &state)
{
    hj::behavior_tree_factory factory;
    factory.registerNodeType<AsyncTestAction>("AsyncTestAction");

    const char *async_xml = R"(
    <root hjCPP_format="4">
        <BehaviorTree ID="AsyncTree">
            <AsyncTestAction message="async test"/>
        </BehaviorTree>
    </root>
    )";

    auto tree = factory.createTreeFromText(async_xml);

    for(auto _ : state)
    {
        auto status = tree.tickWhileRunning();
        benchmark::DoNotOptimize(status);
    }
}
BENCHMARK(bm_async_node_execution);

static void bm_stateful_node_execution(benchmark::State &state)
{
    hj::behavior_tree_factory factory;
    factory.registerNodeType<StatefulTestAction>("StatefulTestAction");

    const char *stateful_xml = R"(
    <root hjCPP_format="4">
        <BehaviorTree ID="StatefulTree">
            <StatefulTestAction/>
        </BehaviorTree>
    </root>
    )";

    auto tree = factory.createTreeFromText(stateful_xml);

    for(auto _ : state)
    {
        auto status = tree.tickWhileRunning();
        benchmark::DoNotOptimize(status);
    }
}
BENCHMARK(bm_stateful_node_execution);

static void bm_port_operations(benchmark::State &state)
{
    for(auto _ : state)
    {
        auto in_port1 = hj::input_port<int>("input1", "description1");
        auto in_port2 =
            hj::input_port("input2", "default_value", "description2");
        auto out_port = hj::output_port<std::string>("output1", "description3");

        benchmark::DoNotOptimize(in_port1);
        benchmark::DoNotOptimize(in_port2);
        benchmark::DoNotOptimize(out_port);
    }
}
BENCHMARK(bm_port_operations);

static void bm_mass_node_registration(benchmark::State &state)
{
    const int node_count = static_cast<int>(state.range(0));

    for(auto _ : state)
    {
        hj::behavior_tree_factory factory;

        for(int i = 0; i < node_count; ++i)
        {
            std::string name = "TestAction" + std::to_string(i);
            factory.registerNodeType<TestAction>(name);
        }

        benchmark::DoNotOptimize(&factory);
    }
}
BENCHMARK(bm_mass_node_registration)->Arg(10)->Arg(50)->Arg(100)->Arg(500);

static void bm_dynamic_xml_execution(benchmark::State &state)
{
    const int sequence_length = static_cast<int>(state.range(0));

    hj::behavior_tree_factory factory;
    factory.registerNodeType<TestAction>("TestAction");

    std::ostringstream xml_stream;
    xml_stream
        << R"(<root hjCPP_format="4"><BehaviorTree ID="DynamicTree"><Sequence name="root">)";

    for(int i = 0; i < sequence_length; ++i)
    {
        xml_stream << R"(<TestAction message="action)" << i << R"("/>)";
    }

    xml_stream << "</Sequence></BehaviorTree></root>";
    std::string xml_content = xml_stream.str();

    for(auto _ : state)
    {
        auto tree   = factory.createTreeFromText(xml_content);
        auto status = tree.tickWhileRunning();
        benchmark::DoNotOptimize(status);
    }
}
BENCHMARK(bm_dynamic_xml_execution)->Arg(5)->Arg(10)->Arg(20)->Arg(50);

static void bm_nested_tree_execution(benchmark::State &state)
{
    hj::behavior_tree_factory factory;
    factory.registerNodeType<TestAction>("TestAction");
    factory.registerNodeType<TestCondition>("TestCondition");

    const char *nested_xml = R"(
    <root hjCPP_format="4">
        <BehaviorTree ID="NestedTree">
            <Fallback name="level1">
                <Sequence name="level2_1">
                    <Fallback name="level3_1">
                        <TestCondition/>
                        <TestAction message="deep action 1"/>
                    </Fallback>
                    <TestAction message="level2 action 1"/>
                </Sequence>
                <Sequence name="level2_2">
                    <TestAction message="level2 action 2"/>
                    <Fallback name="level3_2">
                        <TestCondition/>
                        <TestAction message="deep action 2"/>
                    </Fallback>
                </Sequence>
            </Fallback>
        </BehaviorTree>
    </root>
    )";

    auto tree = factory.createTreeFromText(nested_xml);

    for(auto _ : state)
    {
        auto status = tree.tickWhileRunning();
        benchmark::DoNotOptimize(status);
    }
}
BENCHMARK(bm_nested_tree_execution);

static void bm_string_view_operations(benchmark::State &state)
{
    std::string test_string =
        "This is a test string for string_view operations";

    for(auto _ : state)
    {
        hj::string_view sv1(test_string);
        hj::string_view sv2 = sv1.substr(10, 4);
        hj::string_view sv3 = sv1.substr(0, sv1.find(' '));

        benchmark::DoNotOptimize(sv1);
        benchmark::DoNotOptimize(sv2);
        benchmark::DoNotOptimize(sv3);
    }
}
BENCHMARK(bm_string_view_operations);

static void bm_memory_usage(benchmark::State &state)
{
    const int tree_count = static_cast<int>(state.range(0));

    hj::behavior_tree_factory factory;
    factory.registerNodeType<TestAction>("TestAction");
    factory.registerNodeType<TestCondition>("TestCondition");

    for(auto _ : state)
    {
        std::vector<BT::Tree> trees;
        trees.reserve(tree_count);

        for(int i = 0; i < tree_count; ++i)
        {
            trees.emplace_back(factory.createTreeFromText(complex_xml));
        }

        for(auto &tree : trees)
        {
            auto status = tree.tickWhileRunning();
            benchmark::DoNotOptimize(status);
        }
    }
}
BENCHMARK(bm_memory_usage)->Arg(1)->Arg(5)->Arg(10)->Arg(25);

static void bm_factory_thread_safety_simulation(benchmark::State &state)
{
    hj::behavior_tree_factory factory;

    for(auto _ : state)
    {
        factory.registerNodeType<TestAction>("TestAction1");
        auto tree1 = factory.createTreeFromText(simple_xml);

        factory.registerNodeType<TestCondition>("TestCondition1");
        auto tree2 = factory.createTreeFromText(complex_xml);

        auto status1 = tree1.tickWhileRunning();
        auto status2 = tree2.tickWhileRunning();

        benchmark::DoNotOptimize(status1);
        benchmark::DoNotOptimize(status2);
    }
}
BENCHMARK(bm_factory_thread_safety_simulation);