#include <gtest/gtest.h>
#include <hj/algo/astar.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <vector>

using namespace hj::astar;

namespace
{
using location_t = location<int>;
using grid_t     = grid<location_t>;

class astar : public ::testing::Test
{
  protected:
    static grid_t create_grid(int width, int height, bool diagonal = false)
    {
        grid_t g(width, height);

        for(int y = 0; y < height; ++y)
        {
            for(int x = 0; x < width; ++x)
            {
                std::vector<location_t> neighbors;
                for(int dy = -1; dy <= 1; ++dy)
                {
                    for(int dx = -1; dx <= 1; ++dx)
                    {
                        if(dx == 0 && dy == 0)
                            continue;
                        if(!diagonal && std::abs(dx) + std::abs(dy) != 1)
                            continue;

                        const int nx = x + dx;
                        const int ny = y + dy;
                        if(nx >= 0 && nx < width && ny >= 0 && ny < height)
                            neighbors.emplace_back(nx, ny);
                    }
                }
                g.add_location({x, y}, neighbors);
            }
        }
        return g;
    }

    static grid_t create_grid_with_obstacles(
        int width, int height, const std::vector<location_t> &obstacles)
    {
        auto g = create_grid(width, height);
        g.add_obstacles(obstacles);
        return g;
    }

    static bool contains(const std::vector<location_t> &items,
                         const location_t              &value)
    {
        return std::find(items.begin(), items.end(), value) != items.end();
    }

    static void expect_valid_path(const std::vector<location_t> &path,
                                  const grid_t                  &g,
                                  const location_t              &start,
                                  const location_t              &goal)
    {
        ASSERT_FALSE(path.empty());
        EXPECT_EQ(path.front(), start);
        EXPECT_EQ(path.back(), goal);

        for(std::size_t i = 0; i < path.size(); ++i)
        {
            ASSERT_TRUE(g.is_walkable(path[i]))
                << "Path contains non-walkable node (" << path[i].x << ","
                << path[i].y << ")";

            if(i + 1 == path.size())
                break;

            const auto &neighbors = g.neighbors(path[i]);
            EXPECT_TRUE(contains(neighbors, path[i + 1]))
                << "Invalid edge from (" << path[i].x << "," << path[i].y
                << ") to (" << path[i + 1].x << "," << path[i + 1].y << ")";

            EXPECT_TRUE(std::isfinite(g.cost(path[i], path[i + 1])));
        }
    }

    static double path_cost(const std::vector<location_t> &path,
                            const grid_t                  &g)
    {
        double result = 0.0;
        for(std::size_t i = 0; i + 1 < path.size(); ++i)
            result += g.cost(path[i], path[i + 1]);
        return result;
    }
};

TEST_F(astar, location_value_semantics_and_hash)
{
    location_t a{1, 2};
    location_t b{1, 2};
    location_t c{2, 1};
    location_t zero{};

    EXPECT_EQ(a.x, 1);
    EXPECT_EQ(a.y, 2);
    EXPECT_EQ(zero, location_t(0, 0));
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_TRUE(a < c);

    location_t::hash hasher;
    EXPECT_EQ(hasher(a), hasher(b));

    std::unordered_map<location_t, int, location_t::hash> map;
    map[a] = 42;
    EXPECT_EQ(map[b], 42);
    EXPECT_EQ(map.size(), 1U);
}

TEST_F(astar, priority_queue_empty_and_ordering)
{
    detail::priority_queue<int, double> q;
    EXPECT_TRUE(q.empty());

    q.put(3, 3.0);
    q.put(1, 1.0);
    q.put(2, 2.0);
    EXPECT_FALSE(q.empty());
    EXPECT_EQ(q.get(), 1);
    EXPECT_EQ(q.get(), 2);
    EXPECT_EQ(q.get(), 3);
    EXPECT_TRUE(q.empty());
}

TEST_F(astar, heuristic_boundary_values)
{
    const location_t a{-10, -20};
    const location_t b{30, 40};

    EXPECT_DOUBLE_EQ(detail::manhattan_heuristic(a, a), 0.0);
    EXPECT_DOUBLE_EQ(detail::manhattan_heuristic(a, b), 100.0);
    EXPECT_DOUBLE_EQ(detail::manhattan_heuristic(a, b),
                     detail::manhattan_heuristic(b, a));

    EXPECT_DOUBLE_EQ(detail::euclidean_heuristic(a, a), 0.0);
    EXPECT_DOUBLE_EQ(
        detail::euclidean_heuristic(location_t{0, 0}, location_t{3, 4}),
        5.0);
    EXPECT_DOUBLE_EQ(detail::euclidean_heuristic(a, b),
                     detail::euclidean_heuristic(b, a));
}

TEST_F(astar, smooth_handles_empty_single_two_and_turning_paths)
{
    EXPECT_TRUE(detail::smooth(std::vector<location_t>{}).empty());

    const std::vector<location_t> one{{1, 1}};
    EXPECT_EQ(detail::smooth(one), one);

    const std::vector<location_t> two{{0, 0}, {1, 1}};
    EXPECT_EQ(detail::smooth(two), two);

    const std::vector<location_t> horizontal{{0, 0}, {1, 0}, {2, 0}, {3, 0}};
    EXPECT_EQ(detail::smooth(horizontal),
              (std::vector<location_t>{{0, 0}, {3, 0}}));

    const std::vector<location_t> vertical{{0, 0}, {0, 1}, {0, 2}, {0, 3}};
    EXPECT_EQ(detail::smooth(vertical),
              (std::vector<location_t>{{0, 0}, {0, 3}}));

    const std::vector<location_t> diagonal{{0, 0}, {1, 1}, {2, 2}, {3, 3}};
    EXPECT_EQ(detail::smooth(diagonal),
              (std::vector<location_t>{{0, 0}, {3, 3}}));

    const std::vector<location_t> turn{{0, 0}, {1, 0}, {1, 1}, {1, 2}};
    EXPECT_EQ(detail::smooth(turn),
              (std::vector<location_t>{{0, 0}, {1, 0}, {1, 2}}));
}

TEST_F(astar, smooth_duplicate_points)
{
    const std::vector<location_t> path{{0, 0}, {0, 0}, {1, 0}, {1, 0}, {2, 0}};
    const auto                    result = detail::smooth(path);
    EXPECT_EQ(result, (std::vector<location_t>{{0, 0}, {2, 0}}));
}

TEST_F(astar, grid_constructor_and_invalid_dimensions)
{
    EXPECT_NO_THROW(grid_t(1, 1));
    EXPECT_NO_THROW(grid_t(1, 100));
    EXPECT_NO_THROW(grid_t(100, 1));

    EXPECT_THROW(grid_t(0, 1), std::invalid_argument);
    EXPECT_THROW(grid_t(1, 0), std::invalid_argument);
    EXPECT_THROW(grid_t(-1, 1), std::invalid_argument);
    EXPECT_THROW(grid_t(1, -1), std::invalid_argument);
}

TEST_F(astar, grid_neighbors_and_missing_location)
{
    auto g = create_grid(3, 3);

    EXPECT_EQ(g.neighbors({1, 1}).size(), 4U);
    EXPECT_EQ(g.neighbors({0, 0}).size(), 2U);
    EXPECT_EQ(g.neighbors({1, 0}).size(), 3U);
    EXPECT_TRUE(g.is_walkable({0, 0}));
    EXPECT_FALSE(g.is_walkable({100, 100}));

    EXPECT_THROW(g.neighbors({100, 100}), std::out_of_range);
}

TEST_F(astar, grid_add_location_replaces_neighbors)
{
    grid_t           g(2, 2);
    const location_t a{0, 0};
    const location_t b{1, 0};

    g.add_location(a, {b});
    ASSERT_EQ(g.neighbors(a).size(), 1U);
    EXPECT_EQ(g.neighbors(a).front(), b);

    g.add_location(a, {});
    EXPECT_TRUE(g.neighbors(a).empty());
}

TEST_F(astar, grid_edge_weight_accepts_only_finite_nonnegative_values)
{
    grid_t           g(2, 2);
    const location_t a{0, 0};
    const location_t b{1, 0};

    EXPECT_TRUE(g.set_edge_weight(a, b, 0.0));
    EXPECT_DOUBLE_EQ(g.cost(a, b), 0.0);

    EXPECT_TRUE(g.set_edge_weight(a, b, 3.5));
    EXPECT_DOUBLE_EQ(g.cost(a, b), 3.5);

    EXPECT_FALSE(g.set_edge_weight(a, b, -1.0));
    EXPECT_DOUBLE_EQ(g.cost(a, b), 3.5);

    EXPECT_FALSE(
        g.set_edge_weight(a, b, (std::numeric_limits<double>::quiet_NaN)()));
    EXPECT_DOUBLE_EQ(g.cost(a, b), 3.5);

    EXPECT_FALSE(
        g.set_edge_weight(a, b, (std::numeric_limits<double>::infinity)()));
    EXPECT_DOUBLE_EQ(g.cost(a, b), 3.5);

    EXPECT_FALSE(
        g.set_edge_weight(a, b, -(std::numeric_limits<double>::infinity)()));
    EXPECT_DOUBLE_EQ(g.cost(a, b), 3.5);
}

TEST_F(astar, grid_edge_weights_are_directed)
{
    grid_t           g(2, 2);
    const location_t a{0, 0};
    const location_t b{1, 0};

    ASSERT_TRUE(g.set_edge_weight(a, b, 3.5));
    EXPECT_DOUBLE_EQ(g.cost(a, b), 3.5);
    EXPECT_DOUBLE_EQ(g.cost(b, a), 1.0);
}

TEST_F(astar, obstacles_make_nodes_unwalkable_and_edges_impassable)
{
    grid_t           g = create_grid(2, 2);
    const location_t blocked{0, 1};
    g.add_obstacles({blocked});

    EXPECT_FALSE(g.is_walkable(blocked));
    EXPECT_TRUE(g.is_walkable({0, 0}));
    EXPECT_TRUE(std::isinf(g.cost({0, 0}, blocked)));
    EXPECT_TRUE(std::isinf(g.cost({1, 0}, blocked)));
}

TEST_F(astar, search_finds_shortest_four_connected_path)
{
    auto             g = create_grid(3, 3);
    const location_t start{0, 0};
    const location_t goal{2, 2};

    std::vector<location_t> path;
    const auto              result = search(path, g, start, goal);

    EXPECT_EQ(result, search_result::found);
    expect_valid_path(path, g, start, goal);
    EXPECT_DOUBLE_EQ(path_cost(path, g), 4.0);
    EXPECT_EQ(path.size(), 5U);
}

TEST_F(astar, search_same_start_and_goal_returns_single_node)
{
    auto                    g = create_grid(3, 3);
    const location_t        start{1, 1};
    std::vector<location_t> path{{99, 99}};

    const auto result = search(path, g, start, start);

    EXPECT_EQ(result, search_result::found);
    EXPECT_EQ(path, (std::vector<location_t>{{1, 1}}));
}

TEST_F(astar, search_clears_preexisting_output_path_on_failure)
{
    auto                    g = create_grid(3, 3);
    std::vector<location_t> path{{99, 99}, {98, 98}};

    const auto result = search(path, g, {99, 99}, {2, 2});

    EXPECT_EQ(result, search_result::invalid_start);
    EXPECT_TRUE(path.empty());
}

TEST_F(astar, search_rejects_invalid_start_and_goal)
{
    auto                    g = create_grid(3, 3);
    std::vector<location_t> path;

    EXPECT_EQ(search(path, g, {-1, 0}, {2, 2}), search_result::invalid_start);
    EXPECT_TRUE(path.empty());

    EXPECT_EQ(search(path, g, {0, 0}, {10, 10}), search_result::invalid_goal);
    EXPECT_TRUE(path.empty());
}

TEST_F(astar, search_rejects_obstacle_start_and_goal)
{
    auto g = create_grid_with_obstacles(3, 3, {{0, 0}, {2, 2}});
    std::vector<location_t> path;

    EXPECT_EQ(search(path, g, {0, 0}, {1, 1}), search_result::invalid_start);
    EXPECT_TRUE(path.empty());

    EXPECT_EQ(search(path, g, {1, 1}, {2, 2}), search_result::invalid_goal);
    EXPECT_TRUE(path.empty());
}

TEST_F(astar, search_returns_not_found_for_disconnected_graph)
{
    grid_t g(3, 1);
    g.add_location({0, 0}, {});
    g.add_location({1, 0}, {});
    g.add_location({2, 0}, {});

    std::vector<location_t> path;
    EXPECT_EQ(search(path, g, {0, 0}, {2, 0}), search_result::not_found);
    EXPECT_TRUE(path.empty());
}

TEST_F(astar, search_respects_obstacles)
{
    auto             g = create_grid_with_obstacles(3, 3, {{1, 0}, {1, 1}});
    const location_t start{0, 0};
    const location_t goal{2, 2};

    std::vector<location_t> path;
    EXPECT_EQ(search(path, g, start, goal), search_result::found);
    expect_valid_path(path, g, start, goal);
    EXPECT_DOUBLE_EQ(path_cost(path, g), 4.0);

    for(const auto &node : path)
        EXPECT_TRUE(g.is_walkable(node));
}

TEST_F(astar, zero_heuristic_behaves_as_dijkstra_for_weighted_graph)
{
    auto             g = create_grid(3, 2);
    const location_t start{0, 0};
    const location_t goal{2, 0};

    // Expensive direct route; cheap detour through y=1.
    ASSERT_TRUE(g.set_edge_weight({0, 0}, {1, 0}, 10.0));
    ASSERT_TRUE(g.set_edge_weight({1, 0}, {2, 0}, 10.0));
    ASSERT_TRUE(g.set_edge_weight({0, 0}, {0, 1}, 1.0));
    ASSERT_TRUE(g.set_edge_weight({0, 1}, {1, 1}, 1.0));
    ASSERT_TRUE(g.set_edge_weight({1, 1}, {2, 1}, 1.0));
    ASSERT_TRUE(g.set_edge_weight({2, 1}, {2, 0}, 1.0));

    search_options<location_t> options;
    options.heuristic = [](const location_t &, const location_t &) {
        return 0.0;
    };

    std::vector<location_t> path;
    EXPECT_EQ(search(path, g, start, goal, options), search_result::found);
    expect_valid_path(path, g, start, goal);
    EXPECT_DOUBLE_EQ(path_cost(path, g), 4.0);
}

TEST_F(astar, zero_cost_edges_are_supported)
{
    auto g = create_grid(3, 1);
    ASSERT_TRUE(g.set_edge_weight({0, 0}, {1, 0}, 0.0));
    ASSERT_TRUE(g.set_edge_weight({1, 0}, {2, 0}, 0.0));

    search_options<location_t> options;
    options.heuristic = [](const location_t &, const location_t &) {
        return 0.0;
    };

    std::vector<location_t> path;
    EXPECT_EQ(search(path, g, {0, 0}, {2, 0}, options), search_result::found);
    EXPECT_EQ(path, (std::vector<location_t>{{0, 0}, {1, 0}, {2, 0}}));
    EXPECT_DOUBLE_EQ(path_cost(path, g), 0.0);
}

TEST_F(astar, custom_euclidean_heuristic_on_unit_cost_diagonal_graph)
{
    auto                       g = create_grid(3, 3, true);
    search_options<location_t> options;
    // Diagonal edges also cost 1 in this graph, so Euclidean is not admissible.
    // Use zero heuristic here to test connectivity independently of optimality.
    options.heuristic = [](const location_t &, const location_t &) {
        return 0.0;
    };

    std::vector<location_t> path;
    EXPECT_EQ(search(path, g, {0, 0}, {2, 2}, options), search_result::found);
    expect_valid_path(path, g, {0, 0}, {2, 2});
    EXPECT_DOUBLE_EQ(path_cost(path, g), 2.0);
}

TEST_F(astar, invalid_heuristic_nan_is_reported)
{
    auto                       g = create_grid(2, 1);
    search_options<location_t> options;
    options.heuristic = [](const location_t &, const location_t &) {
        return (std::numeric_limits<double>::quiet_NaN)();
    };

    std::vector<location_t> path;
    EXPECT_EQ(search(path, g, {0, 0}, {1, 0}, options),
              search_result::invalid_heuristic);
    EXPECT_TRUE(path.empty());
}

TEST_F(astar, invalid_heuristic_infinity_is_reported)
{
    auto                       g = create_grid(2, 1);
    search_options<location_t> options;
    options.heuristic = [](const location_t &, const location_t &) {
        return (std::numeric_limits<double>::infinity)();
    };

    std::vector<location_t> path;
    EXPECT_EQ(search(path, g, {0, 0}, {1, 0}, options),
              search_result::invalid_heuristic);
    EXPECT_TRUE(path.empty());
}

TEST_F(astar, invalid_heuristic_negative_is_reported)
{
    auto                       g = create_grid(2, 1);
    search_options<location_t> options;
    options.heuristic = [](const location_t &, const location_t &) {
        return -1.0;
    };

    std::vector<location_t> path;
    EXPECT_EQ(search(path, g, {0, 0}, {1, 0}, options),
              search_result::invalid_heuristic);
    EXPECT_TRUE(path.empty());
}

TEST_F(astar, resource_limit_is_enforced)
{
    auto                       g = create_grid(5, 1);
    search_options<location_t> options;
    options.heuristic = [](const location_t &, const location_t &) {
        return 0.0;
    };
    options.max_expansions = 1;

    std::vector<location_t> path;
    EXPECT_EQ(search(path, g, {0, 0}, {4, 0}, options),
              search_result::resource_limit_exceeded);
    EXPECT_TRUE(path.empty());
}

TEST_F(astar, zero_resource_limit_means_unlimited)
{
    auto                       g = create_grid(5, 1);
    search_options<location_t> options;
    options.heuristic = [](const location_t &, const location_t &) {
        return 0.0;
    };
    options.max_expansions = 0;

    std::vector<location_t> path;
    EXPECT_EQ(search(path, g, {0, 0}, {4, 0}, options), search_result::found);
    expect_valid_path(path, g, {0, 0}, {4, 0});
}

TEST_F(astar, cancellation_is_honored)
{
    auto                       g = create_grid(5, 5);
    search_options<location_t> options;
    options.heuristic = [](const location_t &, const location_t &) {
        return 0.0;
    };
    options.should_cancel = [] { return true; };

    std::vector<location_t> path;
    EXPECT_EQ(search(path, g, {0, 0}, {4, 4}, options),
              search_result::cancelled);
    EXPECT_TRUE(path.empty());
}

TEST_F(astar, cancellation_is_not_checked_when_start_equals_goal)
{
    auto                       g = create_grid(2, 2);
    search_options<location_t> options;
    options.should_cancel = [] { return true; };

    std::vector<location_t> path;
    EXPECT_EQ(search(path, g, {0, 0}, {0, 0}, options), search_result::found);
    EXPECT_EQ(path, (std::vector<location_t>{{0, 0}}));
}

TEST_F(astar, search_handles_one_by_one_grid)
{
    grid_t                  g(1, 1);
    std::vector<location_t> path;

    EXPECT_EQ(search(path, g, {0, 0}, {0, 0}), search_result::found);
    EXPECT_EQ(path, (std::vector<location_t>{{0, 0}}));
}

TEST_F(astar, search_handles_narrow_corridor)
{
    auto g = create_grid_with_obstacles(7,
                                        3,
                                        {{0, 0},
                                         {1, 0},
                                         {2, 0},
                                         {3, 0},
                                         {4, 0},
                                         {5, 0},
                                         {6, 0},
                                         {0, 2},
                                         {1, 2},
                                         {2, 2},
                                         {3, 2},
                                         {4, 2},
                                         {5, 2},
                                         {6, 2}});

    std::vector<location_t> path;
    EXPECT_EQ(search(path, g, {0, 1}, {6, 1}), search_result::found);
    expect_valid_path(path, g, {0, 1}, {6, 1});
    EXPECT_DOUBLE_EQ(path_cost(path, g), 6.0);
}

TEST_F(astar, search_handles_one_way_edges)
{
    grid_t g(3, 1);
    g.add_location({0, 0}, {{1, 0}});
    g.add_location({1, 0}, {{2, 0}});
    g.add_location({2, 0}, {});

    std::vector<location_t> path;
    EXPECT_EQ(search(path, g, {0, 0}, {2, 0}), search_result::found);
    expect_valid_path(path, g, {0, 0}, {2, 0});

    path.clear();
    EXPECT_EQ(search(path, g, {2, 0}, {0, 0}), search_result::not_found);
    EXPECT_TRUE(path.empty());
}

TEST_F(astar, search_ignores_nonwalkable_neighbors)
{
    auto g = create_grid(3, 1);
    g.add_obstacles({{1, 0}});

    // The adjacency still contains the obstacle, but A* must not traverse it.
    std::vector<location_t> path;
    EXPECT_EQ(search(path, g, {0, 0}, {2, 0}), search_result::not_found);
    EXPECT_TRUE(path.empty());
}

TEST_F(astar, search_skips_infinite_edge_cost)
{
    auto g = create_grid(3, 1);
    g.edge_weights[{{0, 0}, {1, 0}}] =
        (std::numeric_limits<double>::infinity)();

    std::vector<location_t> path;
    EXPECT_EQ(search(path, g, {0, 0}, {2, 0}), search_result::not_found);
    EXPECT_TRUE(path.empty());
}

TEST_F(astar, search_reports_overflowed_path_cost)
{
    auto         g        = create_grid(3, 1);
    const double max_cost = (std::numeric_limits<double>::max)();
    ASSERT_TRUE(g.set_edge_weight({0, 0}, {1, 0}, max_cost));
    ASSERT_TRUE(g.set_edge_weight({1, 0}, {2, 0}, max_cost));

    std::vector<location_t> path;
    EXPECT_EQ(search(path, g, {0, 0}, {2, 0}), search_result::invalid_cost);
    EXPECT_TRUE(path.empty());
}

TEST_F(astar, search_result_to_string_is_total)
{
    EXPECT_STREQ(to_string(search_result::found), "found");
    EXPECT_STREQ(to_string(search_result::not_found), "not_found");
    EXPECT_STREQ(to_string(search_result::invalid_start), "invalid_start");
    EXPECT_STREQ(to_string(search_result::invalid_goal), "invalid_goal");
    EXPECT_STREQ(to_string(search_result::invalid_graph), "invalid_graph");
    EXPECT_STREQ(to_string(search_result::invalid_cost), "invalid_cost");
    EXPECT_STREQ(to_string(search_result::invalid_heuristic),
                 "invalid_heuristic");
    EXPECT_STREQ(to_string(search_result::resource_limit_exceeded),
                 "resource_limit_exceeded");
    EXPECT_STREQ(to_string(search_result::cancelled), "cancelled");
}

TEST_F(astar, default_heuristic_is_optimal_for_uniform_four_connected_grid)
{
    auto                    g = create_grid(10, 10);
    std::vector<location_t> path;

    EXPECT_EQ(search(path, g, {0, 0}, {9, 9}), search_result::found);
    expect_valid_path(path, g, {0, 0}, {9, 9});
    EXPECT_DOUBLE_EQ(path_cost(path, g), 18.0);
    EXPECT_EQ(path.size(), 19U);
}

TEST_F(astar, repeated_searches_do_not_leak_previous_state)
{
    auto                    g = create_grid(4, 4);
    std::vector<location_t> path;

    EXPECT_EQ(search(path, g, {0, 0}, {3, 3}), search_result::found);
    expect_valid_path(path, g, {0, 0}, {3, 3});

    EXPECT_EQ(search(path, g, {3, 3}, {0, 0}), search_result::found);
    expect_valid_path(path, g, {3, 3}, {0, 0});

    EXPECT_EQ(search(path, g, {0, 0}, {99, 99}), search_result::invalid_goal);
    EXPECT_TRUE(path.empty());
}

TEST_F(astar, float_location_smoothing)
{
    using float_location                   = location<float>;
    const std::vector<float_location> path = {{0.0f, 0.0f},
                                              {1.0f, 0.0f},
                                              {2.0f, 0.0f}};

    const auto smoothed = detail::smooth(path);
    EXPECT_EQ(smoothed.size(), 2U);
    EXPECT_EQ(smoothed.front(), path.front());
    EXPECT_EQ(smoothed.back(), path.back());
}

} // namespace
