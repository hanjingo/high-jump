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

// for more information about the A* algorithm, see:
// https://scm_mos.gitlab.io/motion-planner/a-star/

#ifndef ASTAR_HPP
#define ASTAR_HPP

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <queue>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace hj::astar
{

using cost_t = double;

namespace detail
{
template <typename Location>
inline cost_t manhattan_heuristic(const Location &a, const Location &b)
{
    const auto dx = a.x >= b.x ? a.x - b.x : b.x - a.x;
    const auto dy = a.y >= b.y ? a.y - b.y : b.y - a.y;

    return static_cast<cost_t>(dx) + static_cast<cost_t>(dy);
}

template <typename Location>
inline cost_t euclidean_heuristic(const Location &a, const Location &b)
{
    const long double dx =
        static_cast<long double>(a.x) - static_cast<long double>(b.x);
    const long double dy =
        static_cast<long double>(a.y) - static_cast<long double>(b.y);

    return static_cast<cost_t>(std::sqrt(dx * dx + dy * dy));
}

template <typename Location>
std::vector<Location> smooth(const std::vector<Location> &path)
{
    if(path.size() <= 2)
        return path;

    std::vector<Location> smoothed_path;
    smoothed_path.reserve(path.size());
    smoothed_path.push_back(path.front()); // Always keep the start point

    for(size_t i = 1; i < path.size() - 1; ++i)
    {
        const auto &prev = path[i - 1];
        const auto &curr = path[i];
        const auto &next = path[i + 1];

        const long double dx1 =
            static_cast<long double>(curr.x) - static_cast<long double>(prev.x);
        const long double dy1 =
            static_cast<long double>(curr.y) - static_cast<long double>(prev.y);
        const long double dx2 =
            static_cast<long double>(next.x) - static_cast<long double>(curr.x);
        const long double dy2 =
            static_cast<long double>(next.y) - static_cast<long double>(curr.y);

        if(dx1 * dy2 != dy1 * dx2)
            smoothed_path.push_back(curr);
    }

    smoothed_path.push_back(path.back()); // Always keep the end point
    return smoothed_path;
}

template <typename Grid, typename Location, typename = void>
struct has_is_walkable : std::false_type
{
};

template <typename Grid, typename Location>
struct has_is_walkable<
    Grid,
    Location,
    std::void_t<decltype(std::declval<const Grid &>().is_walkable(
        std::declval<const Location &>()))>> : std::true_type
{
};

template <typename Grid, typename Location>
bool is_walkable(const Grid &grid, const Location &location)
{
    if constexpr(has_is_walkable<Grid, Location>::value)
        return static_cast<bool>(grid.is_walkable(location));
    else
        return true;
}

template <typename T, typename Priority>
struct priority_queue
{
    typedef std::pair<Priority, T> Element;
    std::priority_queue<Element, std::vector<Element>, std::greater<Element>>
        elements;

    inline bool empty() const { return elements.empty(); }
    inline void put(T item, Priority priority)
    {
        elements.emplace(priority, item);
    }
    T get()
    {
        assert(!elements.empty()
               && "priority_queue::get() called on empty queue");
        T best_item = elements.top().second;
        elements.pop();
        return best_item;
    }
};

} // namespace detail
} // namespace hj::astar

namespace hj::astar
{
template <typename T = int>
struct location
{
    T x, y;

    location()
        : x(T{})
        , y(T{})
    {
    }
    location(T x_, T y_)
        : x(x_)
        , y(y_)
    {
    }
    bool operator==(const location &other) const
    {
        return x == other.x && y == other.y;
    }
    bool operator!=(const location &other) const { return !(*this == other); }
    bool operator<(const location &other) const
    {
        if(x != other.x)
            return x < other.x;

        return y < other.y;
    }

    // Hash function for location to use in unordered_map
    struct hash
    {
        std::size_t operator()(const location &loc) const
        {
            std::size_t seed = 0;
            seed ^= std::hash<T>()(loc.x) + 0x9e3779b97f4a7c15ULL + (seed << 6)
                    + (seed >> 2);
            seed ^= std::hash<T>()(loc.y) + 0x9e3779b97f4a7c15ULL + (seed << 6)
                    + (seed >> 2);
            return seed;
        }
    };
};
} // namespace hj::astar

namespace hj::astar
{
// support for std::pair<location, location> as key in unordered_map
template <typename Location>
struct edge_hash
{
    std::size_t
    operator()(const std::pair<Location, Location> &edge) const noexcept
    {
        const typename Location::hash hasher{};
        const std::size_t             h1 = hasher(edge.first);
        const std::size_t             h2 = hasher(edge.second);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

template <typename Location>
struct grid
{
    std::unordered_map<Location, std::vector<Location>, typename Location::hash>
        adjacency_list;

    std::unordered_map<std::pair<Location, Location>,
                       cost_t,
                       edge_hash<Location>>
        edge_weights;

    std::unordered_set<Location, typename Location::hash> obstacles;

    grid() = default;
    grid(int width, int height)
    {
        if(width <= 0 || height <= 0)
            throw std::invalid_argument("invalid grid dimensions");

        for(int y = 0; y < height; y++)
            for(int x = 0; x < width; x++)
                adjacency_list[Location{x, y}];
    }

    virtual ~grid() = default;

    void add_location(const Location              &loc,
                      const std::vector<Location> &neighbors)
    {
        adjacency_list[loc] = neighbors;
    }

    void add_obstacles(const std::vector<Location> &obs)
    {
        for(const auto &o : obs)
            obstacles.insert(o);
    }

    bool is_walkable(const Location &loc) const
    {
        return adjacency_list.find(loc) != adjacency_list.end()
               && obstacles.find(loc) == obstacles.end();
    }

    bool set_edge_weight(const Location &from, const Location &to, cost_t w)
    {
        if(!std::isfinite(w) || w < 0.0)
            return false;

        edge_weights[{from, to}] = w;
        return true;
    }

    virtual const std::vector<Location> &neighbors(const Location &loc) const
    {
        return adjacency_list.at(loc);
    }

    virtual cost_t cost(const Location &from, const Location &to) const
    {
        if(obstacles.count(to))
            return (std::numeric_limits<cost_t>::infinity)();

        auto it = edge_weights.find({from, to});
        return it != edge_weights.end() ? it->second : 1.0;
    }
};

template <typename Location>
using heuristic_fn = std::function<cost_t(const Location &, const Location &)>;

enum class search_result
{
    found,
    not_found,
    invalid_start,
    invalid_goal,
    invalid_graph,
    invalid_cost,
    invalid_heuristic,
    resource_limit_exceeded,
    cancelled
};

inline const char *to_string(search_result r) noexcept
{
    switch(r)
    {
        case search_result::found:
            return "found";
        case search_result::not_found:
            return "not_found";
        case search_result::invalid_start:
            return "invalid_start";
        case search_result::invalid_goal:
            return "invalid_goal";
        case search_result::invalid_graph:
            return "invalid_graph";
        case search_result::invalid_cost:
            return "invalid_cost";
        case search_result::invalid_heuristic:
            return "invalid_heuristic";
        case search_result::resource_limit_exceeded:
            return "resource_limit_exceeded";
        case search_result::cancelled:
            return "cancelled";
    }
    return "unknown";
}

template <typename Location>
struct search_options
{
    heuristic_fn<Location> heuristic = detail::manhattan_heuristic<Location>;
    std::size_t            max_expansions = 0;
    std::function<bool()>  should_cancel  = []() -> bool { return false; };
};

// For optimal-path guarantees, the supplied heuristic must be admissible and
//  consistent for the graph's edge-cost model.
template <typename Grid, typename Location>
search_result search(std::vector<Location>          &path,
                     const Grid                     &grid,
                     const Location                 &start,
                     const Location                 &goal,
                     const search_options<Location> &options = {})
{
    path.clear();

    if(!detail::is_walkable(grid, start))
        return search_result::invalid_start;

    if(!detail::is_walkable(grid, goal))
        return search_result::invalid_goal;

    if(start == goal)
    {
        path.push_back(start);
        return search_result::found;
    }

    detail::priority_queue<Location, cost_t> frontier;
    frontier.put(start, 0);

    std::unordered_map<Location, Location, typename Location::hash> came_from;
    std::unordered_map<Location, cost_t, typename Location::hash>   cost_so_far;
    std::unordered_set<Location, typename Location::hash>           closed;
    came_from[start]   = start;
    cost_so_far[start] = 0;

    std::size_t expansions = 0;
    while(!frontier.empty())
    {
        Location current = frontier.get();
        if(closed.count(current) != 0)
            continue;

        closed.insert(current);
        if(current == goal)
            break;

        if(options.should_cancel && options.should_cancel())
            return search_result::cancelled;

        if(options.max_expansions != 0 && ++expansions > options.max_expansions)
            return search_result::resource_limit_exceeded;

        for(const Location &next : grid.neighbors(current))
        {
            if(!detail::is_walkable(grid, next))
                continue;

            cost_t edge_cost = grid.cost(current, next);
            if(!std::isfinite(edge_cost))
                continue; // impassable (e.g. obstacle)

            cost_t new_cost = cost_so_far[current] + edge_cost;
            if(!std::isfinite(new_cost) || new_cost < 0.0)
                return search_result::invalid_cost;

            auto found_it = cost_so_far.find(next);
            if(found_it == cost_so_far.end() || new_cost < found_it->second)
            {
                cost_so_far[next] = new_cost;
                came_from[next]   = current;
                const cost_t h    = options.heuristic(next, goal);
                if(!std::isfinite(h) || h < 0.0)
                    return search_result::invalid_heuristic;

                frontier.put(next, new_cost + h);
            }
        }
    }

    if(came_from.find(goal) == came_from.end())
        return search_result::not_found;

    Location    current  = goal;
    std::size_t max_hops = came_from.size() + 1;
    std::size_t hops     = 0;
    while(current != start)
    {
        path.push_back(current);
        if(++hops > max_hops)
            return search_result::invalid_graph;

        auto it = came_from.find(current);
        if(it == came_from.end())
            return search_result::invalid_graph;

        current = it->second;
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return search_result::found;
}

} // namespace hj::astar

#endif // ASTAR_HPP