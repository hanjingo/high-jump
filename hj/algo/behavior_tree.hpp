/*
 * This file is part of high-jump(hj).
 *
 * Copyright 2025 hanjingo <hehehunanchina@live.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BEHAVIOR_TREE_HPP
#define BEHAVIOR_TREE_HPP

#include <behaviortree_cpp/bt_factory.h>

namespace hj
{

using behavior_tree_factory = BT::BehaviorTreeFactory;

using sync_action_node     = BT::SyncActionNode;
using async_action_node    = BT::ThreadedAction;
using stateful_action_node = BT::StatefulActionNode;
using coro_action_node     = BT::CoroActionNode;

using node_configuration = BT::NodeConfiguration;
using node_status        = BT::NodeStatus;

using ports_list = BT::PortsList;
using port_info  = BT::PortInfo;

using string_view = BT::StringView;

template <typename T = void>
inline std::pair<std::string, port_info>
input_port(string_view name, string_view description = {})
{
    return BT::InputPort(name, description);
}

inline std::pair<std::string, port_info> input_port(
    string_view name, const std::string &default_value, string_view description)
{
    return BT::InputPort(name, default_value, description);
}

template <typename T = void>
inline std::pair<std::string, port_info>
output_port(string_view name, string_view description = {})
{
    return BT::OutputPort(name, description);
}

}

#endif