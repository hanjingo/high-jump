#include <gtest/gtest.h>
// #include <hj/algo/behavior_tree.hpp>

// static const char* xml_text = R"(
// <root hjCPP_format="4" >
//     <BehaviorTree ID="MainTree">
//         <Sequence name="root">
//             <AlwaysSuccess/>
//             <say_something   message="this works too" />
//             <think_what_to_say text="{the_answer}"/>
//             <say_something   message="{the_answer}" />
//         </Sequence>
//     </BehaviorTree>
// </root>
// )";
   
// class say_something : public hj::sync_action_node
// {
// public:
//     say_something(const std::string& name, const hj::node_configuration& config) :
//         hj::sync_action_node(name, config)
//     {}
   
//     hj::node_status tick() override
//     {
//         std::string msg;
//         getInput("message", msg);
//         std::cout << "say_something: " << msg << std::endl;
//         return hj::node_status::SUCCESS;
//     }
   
//     static hj::ports_list providedPorts()
//     {
//         return {hj::input_port<std::string>("message")};
//     }
// };
   
// class think_what_to_say : public hj::sync_action_node
// {
// public:
//     think_what_to_say(const std::string& name, const hj::node_configuration& config) :
//         hj::sync_action_node(name, config)
//     {}
   
//     hj::node_status tick() override
//     {
//         setOutput("text", "The answer is 42");
//         return hj::node_status::SUCCESS;
//     }
   
//     static hj::ports_list providedPorts()
//     {
//         return {hj::output_port<std::string>("text")};
//     }
// };

// TEST(behavior_tree, register_nodes)
// {
//     hj::behavior_tree_factory factory;
//     factory.registerNodeType<say_something>("say_something");
//     factory.registerNodeType<think_what_to_say>("think_what_to_say");
// }