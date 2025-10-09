#include <gtest/gtest.h>
#include <hj/algo/behavior_tree.hpp>

static const char *xml_text = R"(
<root hjCPP_format="4" >
    <BehaviorTree ID="MainTree">
        <Sequence name="root">
            <AlwaysSuccess/>
            <say_something   message="this works too" />
            <think_what_to_say text="{the_answer}"/>
            <say_something   message="{the_answer}" />
        </Sequence>
    </BehaviorTree>
</root>
)";

class say_something : public hj::sync_action_node
{
  public:
    say_something(const std::string &name, const hj::node_configuration &config)
        : hj::sync_action_node(name, config)
    {
    }

    hj::node_status tick() override
    {
        std::string msg;
        getInput("message", msg);
        std::cout << "say_something: " << msg << std::endl;
        return hj::node_status::SUCCESS;
    }

    static hj::ports_list providedPorts()
    {
        return {hj::input_port<std::string>("message")};
    }
};

class think_what_to_say : public hj::sync_action_node
{
  public:
    think_what_to_say(const std::string            &name,
                      const hj::node_configuration &config)
        : hj::sync_action_node(name, config)
    {
    }

    hj::node_status tick() override
    {
        setOutput("text", "The answer is 42");
        return hj::node_status::SUCCESS;
    }

    static hj::ports_list providedPorts()
    {
        return {hj::output_port<std::string>("text")};
    }
};

TEST(behavior_tree, type_aliases)
{
    hj::behavior_tree_factory factory;
    hj::sync_action_node     *node_ptr = nullptr;
    hj::node_status           status   = hj::node_status::IDLE;
    hj::node_configuration    config;
    hj::ports_list            ports;
    hj::port_info             info;
    hj::string_view           sv{"test"};
    EXPECT_EQ(status, BT::NodeStatus::IDLE);
}

TEST(behavior_tree, input_output_port)
{
    auto in_port  = hj::input_port<int>("in", "desc");
    auto in_port2 = hj::input_port<std::string>("in2", "desc2");
    auto out_port = hj::output_port<std::string>("out", "desc");
    EXPECT_EQ(in_port.first, "in");
    EXPECT_EQ(in_port2.first, "in2");
    EXPECT_EQ(out_port.first, "out");
    EXPECT_TRUE(!in_port.second.description().empty());
    EXPECT_TRUE(!out_port.second.description().empty());
}

TEST(behavior_tree, register_nodes)
{
    hj::behavior_tree_factory factory;
    factory.registerNodeType<say_something>("say_something");
    factory.registerNodeType<think_what_to_say>("think_what_to_say");
}