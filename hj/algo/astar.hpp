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
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <cstdlib>
#include <functional>
#include <cmath>

namespace hj::astar
{
namespace detail
{
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

template <typename Location>
inline double manhattan_heuristic(const Location &a, const Location &b)
{
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

template <typename Location>
inline double euclidean_heuristic(const Location &a, const Location &b)
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

template <typename Location>
std::vector<Location> smooth(const std::vector<Location> &path)
{
    if(path.size() <= 2)
        return path;

    std::vector<Location> smoothed_path;
    smoothed_path.push_back(path[0]); // Always keep the start point

    for(size_t i = 1; i < path.size() - 1; ++i)
    {
        const Location &prev = path[i - 1];
        const Location &curr = path[i];
        const Location &next = path[i + 1];

        int dx1 = curr.x - prev.x;
        int dy1 = curr.y - prev.y;
        int dx2 = next.x - curr.x;
        int dy2 = next.y - curr.y;

        if(dx1 * dy2 != dy1 * dx2)
            smoothed_path.push_back(curr);
    }

    smoothed_path.push_back(path.back()); // Always keep the end point
    return smoothed_path;
}
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
            seed ^=
                std::hash<T>()(loc.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^=
                std::hash<T>()(loc.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
};
} // namespace hj::astar

// support for std::pair<location, location> as key in unordered_map
namespace std
{
template <typename T>
struct hash<std::pair<hj::astar::location<T>, hj::astar::location<T>>>
{
    std::size_t operator()(
        const std::pair<hj::astar::location<T>, hj::astar::location<T>> &p)
        const noexcept
    {
        std::size_t h1 = typename hj::astar::location<T>::hash{}(p.first);
        std::size_t h2 = typename hj::astar::location<T>::hash{}(p.second);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};
}

namespace hj::astar
{
template <typename Location>
struct grid
{
    std::unordered_map<Location, std::vector<Location>, typename Location::hash>
                                                              adjacency_list;
    std::unordered_map<std::pair<Location, Location>, double> edge_weights;
    std::unordered_set<Location, typename Location::hash>     obstacles;

    grid() = default;
    grid(int width, int height)
    {
        assert(width > 0 && height > 0);
        for(int y = 0; y < height; y++)
        {
            for(int x = 0; x < width; x++)
            {
                Location loc{x, y};
                adjacency_list[loc] = std::vector<Location>();
            }
        }
    }

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

    void set_edge_weight(const Location &from, const Location &to, double w)
    {
        edge_weights[{from, to}] = w;
    }

    virtual const std::vector<Location> &neighbors(const Location &loc) const
    {
        return adjacency_list.at(loc);
    }

    virtual double cost(const Location &from, const Location &to) const
    {
        if(obstacles.count(to))
            return std::numeric_limits<double>::infinity();

        auto it = edge_weights.find({from, to});
        return it != edge_weights.end() ? it->second : 1.0;
    }
};

using cost_t = double;

template <typename Location>
using heuristic_fn = std::function<cost_t(const Location &, const Location &)>;

enum class search_result
{
    found,
    not_found,
    error
};

template <typename Grid, typename Location>
search_result
search(std::vector<Location> &path,
       const Grid            &grid,
       const Location        &start,
       const Location        &goal,
       heuristic_fn<Location> heuristic = detail::manhattan_heuristic<Location>)
{
    path.clear();

    if(start == goal)
    {
        path.push_back(start);
        return search_result::found;
    }

    detail::priority_queue<Location, cost_t> frontier;
    frontier.put(start, 0);

    std::unordered_map<Location, Location, typename Location::hash> came_from;
    std::unordered_map<Location, cost_t, typename Location::hash>   cost_so_far;
    came_from[start]   = start;
    cost_so_far[start] = 0;

    while(!frontier.empty())
    {
        Location current = frontier.get();
        if(current == goal)
            break;

        try
        {
            auto locations = grid.neighbors(current);
            for(const Location &next : locations)
            {
                cost_t new_cost =
                    cost_so_far[current] + grid.cost(current, next);

                if(cost_so_far.find(next) == cost_so_far.end()
                   || new_cost < cost_so_far[next])
                {
                    cost_so_far[next] = new_cost;
                    cost_t priority   = new_cost + heuristic(next, goal);
                    frontier.put(next, priority);
                    came_from[next] = current;
                }
            }
        }
        catch(const std::exception &e)
        {
            std::cerr << "[astar] Exception: " << e.what() << std::endl;
            continue;
        }
    }

    if(came_from.find(goal) == came_from.end())
        return search_result::not_found;

    Location current = goal;
    while(current != start)
    {
        path.push_back(current);
        auto it = came_from.find(current);
        if(it == came_from.end())
            return search_result::error;

        current = it->second;
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return search_result::found;
}

} // namespace hj::astar

#endif // ASTAR_HPP