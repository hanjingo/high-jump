#ifndef BEHAVIOR_TREE_HPP
#define BEHAVIOR_TREE_HPP

#include <behaviortree_cpp/bt_factory.h>

namespace libcpp {

using behavior_tree_factory = BT::BehaviorTreeFactory;

using sync_action_node = BT::SyncActionNode;
using async_action_node = BT::ThreadedAction;
using stateful_action_node = BT::StatefulActionNode;
using coro_action_node = BT::CoroActionNode;

using node_configuration = BT::NodeConfiguration;
using node_status = BT::NodeStatus;

using ports_list = BT::PortsList;
using port_info = BT::PortInfo;

using string_view = BT::StringView;

template <typename T = void>
inline std::pair<std::string, port_info> input_port(
    string_view name,
    string_view description = {})
{
    return BT::InputPort(name, description);
}

template <typename T = void>
inline std::pair<std::string, port_info> input_port(string_view name,
                                                    const T& default_value,
                                                    string_view description)
{
    return BT::InputPort(name, default_value, description);
}

template <typename T = void>
inline std::pair<std::string, port_info> output_port(
    string_view name,
    string_view description = {})
{
    return BT::OutputPort(name, description);
}

}  // namespace libcpp

#endif