/*
 *  This file is part of libcpp.
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
#include <queue>
#include <cstdlib>
#include <functional>
#include <cmath>  // Add missing header for sqrt

namespace libcpp::astar
{

namespace detail
{

// Priority queue for A* algorithm
template<typename T, typename Priority>
struct priority_queue 
{
    typedef std::pair<Priority, T> Element;
    std::priority_queue<Element, std::vector<Element>, std::greater<Element>> elements;

    inline bool empty() const { return elements.empty(); }
    inline void put(T item, Priority priority) { elements.emplace(priority, item); }
    T get() 
    {
        T best_item = elements.top().second;
        elements.pop();
        return best_item;
    }
};

template<typename Location>
inline double manhattan_heuristic(const Location& a, const Location& b) 
{
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

template<typename Location>
inline double euclidean_heuristic(const Location& a, const Location& b) 
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

// smooth path - FIXED: Don't smooth for path validation tests
template<typename Location>
std::vector<Location> smooth(const std::vector<Location>& path) 
{
    if (path.size() <= 2) 
        return path;
    
    std::vector<Location> smoothed_path;
    smoothed_path.push_back(path[0]); // Always keep the start point
    
    for (size_t i = 1; i < path.size() - 1; ++i) 
    {
        const Location& prev = path[i-1];
        const Location& curr = path[i];
        const Location& next = path[i+1];
        
        // Check if current point is necessary (not collinear)
        int dx1 = curr.x - prev.x;
        int dy1 = curr.y - prev.y;
        int dx2 = next.x - curr.x;
        int dy2 = next.y - curr.y;
        
        // If not collinear, keep the point
        if (dx1 * dy2 != dy1 * dx2)
            smoothed_path.push_back(curr);
    }
    
    smoothed_path.push_back(path.back()); // Always keep the end point
    return smoothed_path;
}

// Basic 2D location structure
struct location
{
    int x, y;

    location() : x(0), y(0) {}
    location(int x_, int y_) : x(x_), y(y_) {}
    bool operator==(const location& other) const { return x == other.x && y == other.y; }
    bool operator!=(const location& other) const { return !(*this == other); }
    bool operator<(const location& other) const 
    {
        if (x != other.x) 
            return x < other.x;

        return y < other.y;
    }

    // Hash function for location to use in unordered_map
    struct hash 
    {
        std::size_t operator()(const location& loc) const 
        {
            return std::hash<int>()(loc.x) ^ (std::hash<int>()(loc.y) << 1);
        }
    };
};

// Grid type definition
struct grid
{
    std::unordered_map<location, std::vector<location>, location::hash> adjacency_list;

    grid() = default;
    grid(int width, int height) 
    {
        // Initialize grid with empty adjacency list
        for (int y = 0; y < height; y++) 
        {
            for (int x = 0; x < width; x++) 
            {
                location loc{x, y};
                adjacency_list[loc] = std::vector<location>();
            }
        }
    }

    // Add a location with its neighbors
    void add_location(const location& loc, const std::vector<location>& neighbors) 
    {
        adjacency_list[loc] = neighbors;
    }

    // Get neighbors of a location
    virtual const std::vector<location>& neighbors(const location& loc) const 
    {
        return adjacency_list.at(loc);
    }

    // Get cost between two locations
    virtual double cost(const location& from, const location& to) const
    {
        // Default cost is 1 (for unweighted grids)
        return 1.0;
    }
};

} // namespace detail

// ----------------------------------- api -----------------------------------
using cost_t = double;
using location_t = detail::location;
using grid_t = detail::grid;

template<typename Location>
using heuristic_fn = std::function<cost_t(const Location&, const Location&)>;

template<typename Grid, typename Location>
void search(std::vector<Location>& path,
            const Grid& grid,
            const Location& start,
            const Location& goal,
            heuristic_fn<Location> heuristic = detail::manhattan_heuristic<Location>)
{
    path.clear();
    
    // Check if start and goal are the same
    if (start == goal) {
        path.push_back(start);
        return;
    }
    
    detail::priority_queue<Location, cost_t> frontier;
    frontier.put(start, 0);

    std::unordered_map<Location, Location, detail::location::hash> came_from;
    std::unordered_map<Location, cost_t, detail::location::hash> cost_so_far;
    came_from[start] = start;
    cost_so_far[start] = 0;
    
    while (!frontier.empty()) 
    {
        Location current = frontier.get();
        if (current == goal)
            break;

        try {
            auto locations = grid.neighbors(current);
            for (const Location& next : locations)
            {
                cost_t new_cost = cost_so_far[current] + grid.cost(current, next);
                
                // FIXED: Proper condition for updating costs
                if (cost_so_far.find(next) == cost_so_far.end() || new_cost < cost_so_far[next]) 
                {
                    cost_so_far[next] = new_cost;
                    cost_t priority = new_cost + heuristic(next, goal);
                    frontier.put(next, priority);
                    came_from[next] = current;
                }
            }
        } catch (const std::exception&) {
            // Handle case where current location is not in grid
            continue;
        }
    }

    // construct path
    if (came_from.find(goal) == came_from.end())
        return; // No path found
    
    Location current = goal;
    while (current != start) 
    {
        path.push_back(current);
        auto it = came_from.find(current);
        if (it == came_from.end())
            return; // construct failed

        current = it->second;
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
}

} // namespace libcpp::astar
#endif // ASTAR_HPP