/*
 *  This file is part of high-jump(hj).
 *  Copyright (C) 2025 hanjingo <hehehunanchina@live.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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