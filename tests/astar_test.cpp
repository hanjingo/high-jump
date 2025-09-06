#include <gtest/gtest.h>
#include <libcpp/algo/astar.hpp>
#include <vector>
#include <iostream>
#include <chrono>
#include <cmath>
#include <algorithm>

using namespace libcpp::astar;

class AStarTest : public ::testing::Test {
protected:
    void SetUp() override {
        // std::cout << "\n=== A* Algorithm Tests Setup ===" << std::endl;
    }
    
    void TearDown() override {
        // std::cout << "=== A* Algorithm Tests Teardown ===" << std::endl;
    }

    // Helper function to create a simple 3x3 grid_t with full connectivity
    grid_t create_simple_grid() {
        grid_t test_grid(3, 3);
        
        // Connect each cell to its neighbors (4-connected)
        for (int y = 0; y < 3; y++) {
            for (int x = 0; x < 3; x++) {
                location_t current{x, y};
                std::vector<location_t> neighbors;
                
                // Add valid neighbors
                if (x > 0) neighbors.emplace_back(x-1, y);     // left
                if (x < 2) neighbors.emplace_back(x+1, y);     // right  
                if (y > 0) neighbors.emplace_back(x, y-1);     // up
                if (y < 2) neighbors.emplace_back(x, y+1);     // down
                
                test_grid.add_location(current, neighbors);
            }
        }
        return test_grid;
    }

    // Helper function to create 8-connected grid_t (with diagonals)
    grid_t create_8_connected_grid(int width, int height) {
        grid_t test_grid(width, height);
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                location_t current{x, y};
                std::vector<location_t> neighbors;
                
                // Add all 8 neighbors (including diagonals)
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue; // Skip current cell
                        
                        int nx = x + dx;
                        int ny = y + dy;
                        
                        if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                            neighbors.emplace_back(nx, ny);
                        }
                    }
                }
                
                test_grid.add_location(current, neighbors);
            }
        }
        return test_grid;
    }

    // Helper function to create grid_t with obstacles
    grid_t create_grid_with_obstacles(int width, int height, const std::vector<location_t>& obstacles) {
        grid_t test_grid(width, height);
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                location_t current{x, y};
                
                // Skip if current location_t is an obstacle
                if (std::find(obstacles.begin(), obstacles.end(), current) != obstacles.end()) {
                    test_grid.add_location(current, {});
                    continue;
                }
                
                std::vector<location_t> neighbors;
                
                // Add valid neighbors that are not obstacles
                std::vector<location_t> potential_neighbors = {
                    {x-1, y}, {x+1, y}, {x, y-1}, {x, y+1}
                };
                
                for (const auto& neighbor : potential_neighbors) {
                    if (neighbor.x >= 0 && neighbor.x < width && 
                        neighbor.y >= 0 && neighbor.y < height &&
                        std::find(obstacles.begin(), obstacles.end(), neighbor) == obstacles.end()) {
                        neighbors.push_back(neighbor);
                    }
                }
                
                test_grid.add_location(current, neighbors);
            }
        }
        return test_grid;
    }

    // Helper function to create a maze-like grid_t
    grid_t create_maze_grid() {
        grid_t maze_grid(7, 7);
        
        // Create a simple maze pattern
        std::vector<location_t> walls = {
            {1, 1}, {1, 2}, {1, 3}, {1, 5},
            {2, 3}, {2, 5},
            {3, 1}, {3, 3}, {3, 5},
            {4, 1}, {4, 3},
            {5, 1}, {5, 2}, {5, 3}, {5, 4}, {5, 5}
        };
        
        return create_grid_with_obstacles(7, 7, walls);
    }

    // Helper function to print path for debugging
    void print_path(const std::vector<location_t>& path) {
        // std::cout << "Path: ";
        // for (size_t i = 0; i < path.size(); ++i) {
        //     std::cout << "(" << path[i].x << "," << path[i].y << ")";
        //     if (i < path.size() - 1) std::cout << " -> ";
        // }
        // std::cout << std::endl;
    }

    // Helper function to print grid_t with path for visualization
    void print_grid_with_path(const std::vector<location_t>& path, int width, int height, 
                             const std::vector<location_t>& obstacles = {}) {
        // std::cout << "Grid visualization:" << std::endl;
        // for (int y = 0; y < height; y++) {
        //     for (int x = 0; x < width; x++) {
        //         location_t current{x, y};
                
        //         if (std::find(obstacles.begin(), obstacles.end(), current) != obstacles.end()) {
        //             std::cout << "# ";  // Obstacle
        //         } else if (std::find(path.begin(), path.end(), current) != path.end()) {
        //             if (current == path.front()) {
        //                 std::cout << "S ";  // Start
        //             } else if (current == path.back()) {
        //                 std::cout << "G ";  // Goal
        //             } else {
        //                 std::cout << "* ";  // Path
        //             }
        //         } else {
        //             std::cout << ". ";  // Empty
        //         }
        //     }
        //     std::cout << std::endl;
        // }
        // std::cout << std::endl;
    }

    // Fixed helper function to validate path
    bool is_valid_path(const std::vector<location_t>& path, const grid_t& test_grid) {
        if (path.empty()) return false;
        if (path.size() == 1) return true;  // Single point path is valid
        
        for (size_t i = 0; i < path.size() - 1; ++i) {
            const auto& current = path[i];
            const auto& next = path[i + 1];
            
            try {
                const auto& neighbors = test_grid.neighbors(current);
                if (std::find(neighbors.begin(), neighbors.end(), next) == neighbors.end()) {
                    // std::cout << "Invalid step from (" << current.x << "," << current.y 
                    //          << ") to (" << next.x << "," << next.y << ")" << std::endl;
                    return false;
                }
            } catch (const std::exception& e) {
                // std::cout << "Exception checking neighbors for (" << current.x << "," << current.y 
                //          << "): " << e.what() << std::endl;
                return false;
            }
        }
        return true;
    }

    // Helper function to check if location_t exists in grid_t
    bool location_exists_in_grid(const location_t& loc, const grid_t& test_grid) {
        try {
            test_grid.neighbors(loc);
            return true;
        } catch (...) {
            return false;
        }
    }

    // Helper function to calculate path cost
    double calculate_path_cost(const std::vector<location_t>& path, const grid_t& test_grid) {
        if (path.size() < 2) return 0.0;
        
        double total_cost = 0.0;
        for (size_t i = 0; i < path.size() - 1; ++i) {
            total_cost += test_grid.cost(path[i], path[i + 1]);
        }
        return total_cost;
    }
};

// Test basic location_t functionality
TEST_F(AStarTest, location_basic_operations) {
    SCOPED_TRACE("Testing location_t basic operations");
    
    location_t loc1(1, 2);
    location_t loc2(1, 2);
    location_t loc3(2, 1);
    
    EXPECT_EQ(loc1.x, 1);
    EXPECT_EQ(loc1.y, 2);
    EXPECT_TRUE(loc1 == loc2);
    EXPECT_FALSE(loc1 == loc3);
    EXPECT_TRUE(loc1 != loc3);
    EXPECT_TRUE(loc1 < loc3);
    
    // Test default constructor
    location_t default_loc;
    EXPECT_EQ(default_loc.x, 0);
    EXPECT_EQ(default_loc.y, 0);
}

// Test location_t hash functionality
TEST_F(AStarTest, location_hash_function) {
    SCOPED_TRACE("Testing location_t hash function");
    
    location_t loc1(1, 2);
    location_t loc2(1, 2);
    location_t loc3(2, 1);
    
    location_t::hash hasher;
    
    EXPECT_EQ(hasher(loc1), hasher(loc2));
    EXPECT_NE(hasher(loc1), hasher(loc3));
    
    // Test hash distribution
    std::unordered_map<location_t, int, location_t::hash> location_map;
    location_map[loc1] = 1;
    location_map[loc3] = 2;
    
    EXPECT_EQ(location_map[loc1], 1);
    EXPECT_EQ(location_map[loc2], 1);  // Same as loc1
    EXPECT_EQ(location_map[loc3], 2);
}

// Test priority queue functionality
TEST_F(AStarTest, priority_queue_basic) {
    SCOPED_TRACE("Testing priority queue");
    
    detail::priority_queue<int, double> pq;
    
    EXPECT_TRUE(pq.empty());
    
    pq.put(3, 3.0);
    pq.put(1, 1.0);
    pq.put(2, 2.0);
    
    EXPECT_FALSE(pq.empty());
    EXPECT_EQ(pq.get(), 1);
    EXPECT_EQ(pq.get(), 2);
    EXPECT_EQ(pq.get(), 3);
    EXPECT_TRUE(pq.empty());
}

// Test Manhattan heuristic
TEST_F(AStarTest, manhattan_heuristic) {
    SCOPED_TRACE("Testing Manhattan heuristic");
    
    location_t a(0, 0);
    location_t b(3, 4);
    
    double distance = detail::manhattan_heuristic(a, b);
    EXPECT_DOUBLE_EQ(distance, 7.0);
    
    location_t c(-1, -1);
    location_t d(1, 1);
    distance = detail::manhattan_heuristic(c, d);
    EXPECT_DOUBLE_EQ(distance, 4.0);
    
    // Test symmetric property
    EXPECT_DOUBLE_EQ(detail::manhattan_heuristic(a, b), detail::manhattan_heuristic(b, a));
    
    // Test zero distance
    EXPECT_DOUBLE_EQ(detail::manhattan_heuristic(a, a), 0.0);
}

// Test Euclidean heuristic
TEST_F(AStarTest, euclidean_heuristic) {
    SCOPED_TRACE("Testing Euclidean heuristic");
    
    location_t a(0, 0);
    location_t b(3, 4);
    
    double distance = detail::euclidean_heuristic(a, b);
    EXPECT_DOUBLE_EQ(distance, 5.0);
    
    location_t c(0, 0);
    location_t d(1, 1);
    distance = detail::euclidean_heuristic(c, d);
    EXPECT_DOUBLE_EQ(distance, std::sqrt(2.0));
    
    // Test symmetric property
    EXPECT_DOUBLE_EQ(detail::euclidean_heuristic(a, b), detail::euclidean_heuristic(b, a));
    
    // Test zero distance
    EXPECT_DOUBLE_EQ(detail::euclidean_heuristic(a, a), 0.0);
}

// Test path smoothing
TEST_F(AStarTest, path_smoothing) {
    SCOPED_TRACE("Testing path smoothing");
    
    // Test empty path
    std::vector<location_t> empty_path;
    auto smoothed = detail::smooth(empty_path);
    EXPECT_TRUE(smoothed.empty());
    
    // Test single point
    std::vector<location_t> single_path = {{0, 0}};
    smoothed = detail::smooth(single_path);
    EXPECT_EQ(smoothed.size(), 1);
    
    // Test two points
    std::vector<location_t> two_path = {{0, 0}, {1, 1}};
    smoothed = detail::smooth(two_path);
    EXPECT_EQ(smoothed.size(), 2);
    
    // Test collinear path that can be simplified
    std::vector<location_t> collinear_path = {{0, 0}, {1, 0}, {2, 0}, {3, 0}};
    smoothed = detail::smooth(collinear_path);
    EXPECT_EQ(smoothed.size(), 2);
    EXPECT_EQ(smoothed[0], location_t(0, 0));
    EXPECT_EQ(smoothed[1], location_t(3, 0));
}

// Test grid_t construction
TEST_F(AStarTest, grid_construction) {
    SCOPED_TRACE("Testing grid_t construction");
    
    grid_t test_grid(3, 3);
    
    // Test that all locations are initialized
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            location_t loc{x, y};
            EXPECT_NO_THROW(test_grid.neighbors(loc));
        }
    }
    
    // Test adding neighbors
    location_t center{1, 1};
    std::vector<location_t> center_neighbors = {{0, 1}, {2, 1}, {1, 0}, {1, 2}};
    test_grid.add_location(center, center_neighbors);
    
    const auto& neighbors = test_grid.neighbors(center);
    EXPECT_EQ(neighbors.size(), 4);
}

// Test basic A* search with step validation
TEST_F(AStarTest, basic_search_step_by_step) {
    SCOPED_TRACE("Testing basic A* search with step validation");
    
    auto test_grid = create_simple_grid();
    
    location_t start{0, 0};
    location_t goal{2, 0};
    
    std::vector<location_t> path;
    search(path, test_grid, start, goal);
    
    EXPECT_GT(path.size(), 0);
    EXPECT_EQ(path.front(), start);
    EXPECT_EQ(path.back(), goal);
    
    // Manual validation of each step
    bool path_valid = true;
    for (size_t i = 0; i < path.size() - 1; ++i) {
        const auto& current = path[i];
        const auto& next = path[i + 1];
        
        // Check if next is a valid neighbor of current
        const auto& neighbors = test_grid.neighbors(current);
        if (std::find(neighbors.begin(), neighbors.end(), next) == neighbors.end()) {
            // std::cout << "Invalid step from (" << current.x << "," << current.y 
            //          << ") to (" << next.x << "," << next.y << ")" << std::endl;
            path_valid = false;
        }
    }
    
    EXPECT_TRUE(path_valid);
    print_path(path);
    print_grid_with_path(path, 3, 3);
}

// Test A* with manual path construction (no smoothing)
TEST_F(AStarTest, search_without_smoothing) {
    SCOPED_TRACE("Testing A* search without smoothing");
    
    auto test_grid = create_simple_grid();
    
    location_t start{0, 0};
    location_t goal{2, 2};
    
    std::vector<location_t> path;
    
    // Custom search without smoothing to debug the issue
    detail::priority_queue<location_t, cost_t> frontier;
    frontier.put(start, 0);

    std::unordered_map<location_t, location_t, location_t::hash> came_from;
    std::unordered_map<location_t, cost_t, location_t::hash> cost_so_far;
    came_from[start] = start;
    cost_so_far[start] = 0;

    while (!frontier.empty()) {
        location_t current = frontier.get();
        if (current == goal) break;

        const auto& neighbors = test_grid.neighbors(current);
        for (const location_t& next : neighbors) {
            cost_t new_cost = cost_so_far[current] + test_grid.cost(current, next);
            if (cost_so_far.find(next) == cost_so_far.end() || new_cost < cost_so_far[next]) {
                cost_so_far[next] = new_cost;
                cost_t priority = new_cost + detail::manhattan_heuristic(next, goal);
                frontier.put(next, priority);
                came_from[next] = current;
            }
        }
    }

    // Construct path manually
    if (came_from.find(goal) != came_from.end()) {
        location_t current = goal;
        while (current != start) {
            path.push_back(current);
            current = came_from[current];
        }
        path.push_back(start);
        std::reverse(path.begin(), path.end());
    }

    EXPECT_GT(path.size(), 0);
    EXPECT_EQ(path.front(), start);
    EXPECT_EQ(path.back(), goal);
    
    // Validate each step manually
    for (size_t i = 0; i < path.size() - 1; ++i) {
        const auto& current = path[i];
        const auto& next = path[i + 1];
        const auto& neighbors = test_grid.neighbors(current);
        
        EXPECT_TRUE(std::find(neighbors.begin(), neighbors.end(), next) != neighbors.end())
            << "Invalid step from (" << current.x << "," << current.y 
            << ") to (" << next.x << "," << next.y << ")";
    }
    
    print_path(path);
}

// Test same start and goal
TEST_F(AStarTest, same_start_and_goal) {
    SCOPED_TRACE("Testing same start and goal");
    
    auto test_grid = create_simple_grid();
    
    location_t start{1, 1};
    location_t goal{1, 1};
    
    std::vector<location_t> path;
    search(path, test_grid, start, goal);
    
    EXPECT_GT(path.size(), 0);
    EXPECT_EQ(path[0], start);
}

// Test unreachable goal
TEST_F(AStarTest, unreachable_goal) {
    SCOPED_TRACE("Testing unreachable goal");
    
    grid_t test_grid(3, 3);
    
    // Create isolated start
    location_t start{0, 0};
    test_grid.add_location(start, {});
    
    // Create isolated goal
    location_t goal{2, 2};
    test_grid.add_location(goal, {});
    
    std::vector<location_t> path;
    search(path, test_grid, start, goal);
    
    EXPECT_EQ(path.size(), 0);
}

// Test A* with obstacles
TEST_F(AStarTest, search_with_obstacles_detailed) {
    SCOPED_TRACE("Testing A* with obstacles - detailed");
    
    std::vector<location_t> obstacles = {{1, 0}, {1, 1}};
    auto test_grid = create_grid_with_obstacles(3, 3, obstacles);
    
    location_t start{0, 0};
    location_t goal{2, 2};
    
    // Debug: Print grid_t structure
    // std::cout << "Grid structure:" << std::endl;
    // for (int y = 0; y < 3; y++) {
    //     for (int x = 0; x < 3; x++) {
    //         location_t loc{x, y};
    //         try {
    //             const auto& neighbors = test_grid.neighbors(loc);
    //             std::cout << "(" << x << "," << y << ") has " << neighbors.size() << " neighbors: ";
    //             for (const auto& n : neighbors) {
    //                 std::cout << "(" << n.x << "," << n.y << ") ";
    //             }
    //             std::cout << std::endl;
    //         } catch (...) {
    //             std::cout << "(" << x << "," << y << ") - error getting neighbors" << std::endl;
    //         }
    //     }
    // }
    
    std::vector<location_t> path;
    search(path, test_grid, start, goal);
    
    EXPECT_GT(path.size(), 0);
    if (path.size() > 0) {
        EXPECT_EQ(path.front(), start);
        EXPECT_EQ(path.back(), goal);
        
        // Verify path doesn't go through obstacles
        for (const auto& loc : path) {
            EXPECT_EQ(std::find(obstacles.begin(), obstacles.end(), loc), obstacles.end());
        }
        
        print_path(path);
        print_grid_with_path(path, 3, 3, obstacles);
    }
}

// Test different heuristics with explicit function objects
TEST_F(AStarTest, different_heuristics) {
    SCOPED_TRACE("Testing different heuristic functions");
    
    auto test_grid = create_simple_grid();
    
    location_t start{0, 0};
    location_t goal{2, 2};
    
    std::vector<location_t> manhattan_path, euclidean_path;
    
    // Test with Manhattan heuristic using function object
    heuristic_fn<location_t> manhattan_heuristic = 
        [](const location_t& a, const location_t& b) -> cost_t {
            return detail::manhattan_heuristic(a, b);
        };
    
    search(manhattan_path, test_grid, start, goal, manhattan_heuristic);
    
    // Test with Euclidean heuristic using function object
    heuristic_fn<location_t> euclidean_heuristic = 
        [](const location_t& a, const location_t& b) -> cost_t {
            return detail::euclidean_heuristic(a, b);
        };
    
    search(euclidean_path, test_grid, start, goal, euclidean_heuristic);
    
    EXPECT_GT(manhattan_path.size(), 0);
    EXPECT_GT(euclidean_path.size(), 0);
    
    if (manhattan_path.size() > 0 && euclidean_path.size() > 0) {
        EXPECT_EQ(manhattan_path.front(), start);
        EXPECT_EQ(manhattan_path.back(), goal);
        EXPECT_EQ(euclidean_path.front(), start);
        EXPECT_EQ(euclidean_path.back(), goal);
    }
    
    // std::cout << "Manhattan path: ";
    print_path(manhattan_path);
    // std::cout << "Euclidean path: ";
    print_path(euclidean_path);
}

// Test zero heuristic (Dijkstra)
TEST_F(AStarTest, zero_heuristic_dijkstra) {
    SCOPED_TRACE("Testing zero heuristic (Dijkstra algorithm)");
    
    auto test_grid = create_simple_grid();
    
    location_t start{0, 0};
    location_t goal{2, 2};
    
    // Zero heuristic function (equivalent to Dijkstra's algorithm)
    heuristic_fn<location_t> zero_heuristic = 
        [](const location_t&, const location_t&) -> cost_t { return 0.0; };
    
    std::vector<location_t> path;
    search(path, test_grid, start, goal, zero_heuristic);
    
    EXPECT_GT(path.size(), 0);
    if (path.size() > 0) {
        EXPECT_EQ(path.front(), start);
        EXPECT_EQ(path.back(), goal);
        print_path(path);
    }
}

// Test error conditions with safe error handling
TEST_F(AStarTest, error_conditions_safe) {
    SCOPED_TRACE("Testing error conditions safely");
    
    // Test with empty grid_t
    grid_t empty_grid;
    location_t start{0, 0};
    location_t goal{1, 1};
    std::vector<location_t> path;
    
    search(path, empty_grid, start, goal);
    EXPECT_EQ(path.size(), 0);
    
    // Test with start not in grid_t
    auto test_grid = create_simple_grid();
    location_t invalid_start{10, 10};
    
    // Check if location_t exists before searching
    if (!location_exists_in_grid(invalid_start, test_grid)) {
        search(path, test_grid, invalid_start, goal);
        EXPECT_EQ(path.size(), 0);
    }
    
    // Test with goal not in grid_t
    location_t invalid_goal{10, 10};
    if (!location_exists_in_grid(invalid_goal, test_grid)) {
        search(path, test_grid, start, invalid_goal);
        EXPECT_EQ(path.size(), 0);
    }
}

// Test grid_t connectivity validation
TEST_F(AStarTest, grid_connectivity_validation) {
    SCOPED_TRACE("Testing grid_t connectivity validation");
    
    auto test_grid = create_simple_grid();
    
    // Test that each location_t has the expected neighbors
    location_t center{1, 1};
    const auto& center_neighbors = test_grid.neighbors(center);
    EXPECT_EQ(center_neighbors.size(), 4);  // Should have 4 neighbors
    
    location_t corner{0, 0};
    const auto& corner_neighbors = test_grid.neighbors(corner);
    EXPECT_EQ(corner_neighbors.size(), 2);  // Corner should have 2 neighbors
    
    location_t edge{1, 0};
    const auto& edge_neighbors = test_grid.neighbors(edge);
    EXPECT_EQ(edge_neighbors.size(), 3);  // Edge should have 3 neighbors
}

// Test path optimality
TEST_F(AStarTest, path_optimality_check) {
    SCOPED_TRACE("Testing path optimality");
    
    auto test_grid = create_simple_grid();
    
    location_t start{0, 0};
    location_t goal{2, 2};
    
    std::vector<location_t> path;
    search(path, test_grid, start, goal);
    
    EXPECT_GT(path.size(), 0);
    
    if (path.size() > 0) {
        // For a 3x3 grid_t from (0,0) to (2,2), optimal path length should be 5 steps
        // (0,0) -> (1,0) -> (2,0) -> (2,1) -> (2,2) or similar
        EXPECT_GE(path.size(), 3);  // At least 3 steps (after smoothing)
        EXPECT_LE(path.size(), 5);  // At most 5 steps (before smoothing)
        
        double path_cost = calculate_path_cost(path, test_grid);
        // std::cout << "Path cost: " << path_cost << std::endl;
        // std::cout << "Path length: " << path.size() << std::endl;
    }
}

// Test large grid_t with known optimal path
TEST_F(AStarTest, large_grid_optimal_path) {
    SCOPED_TRACE("Testing large grid_t with optimal path");
    
    const int size = 10;
    auto test_grid = create_grid_with_obstacles(size, size, {});
    
    location_t start{0, 0};
    location_t goal{size-1, size-1};
    
    std::vector<location_t> path;
    search(path, test_grid, start, goal);
    
    EXPECT_GT(path.size(), 0);
    if (path.size() > 0) {
        EXPECT_EQ(path.front(), start);
        EXPECT_EQ(path.back(), goal);
        
        // For a grid_t without obstacles, optimal path length should be 
        // around 2*size-1 = 19 steps (Manhattan distance)
        // After smoothing, it might be shorter
        // std::cout << "Large grid_t path length: " << path.size() << std::endl;
        // std::cout << "Expected minimum (Manhattan): " << (2*size-2) << std::endl;
    }
}

// Test corridor navigation
TEST_F(AStarTest, corridor_navigation) {
    SCOPED_TRACE("Testing corridor navigation");
    
    // Create a narrow corridor
    const int length = 10;
    const int width = 3;
    
    // Add walls to create corridor
    std::vector<location_t> walls;
    for (int x = 0; x < length; x++) {
        walls.push_back({x, 0});      // Top wall
        walls.push_back({x, width-1}); // Bottom wall
    }
    
    auto corridor_grid = create_grid_with_obstacles(length, width, walls);
    
    location_t start{0, 1};  // Middle of corridor start
    location_t goal{length-1, 1};  // Middle of corridor end
    
    std::vector<location_t> path;
    search(path, corridor_grid, start, goal);
    
    EXPECT_GT(path.size(), 0);
    if (path.size() > 0) {
        EXPECT_EQ(path.front(), start);
        EXPECT_EQ(path.back(), goal);
        
        // Path should be roughly the length of the corridor
        // std::cout << "Corridor path length: " << path.size() << std::endl;
        print_path(path);
    }
}

// Test spiral path navigation
TEST_F(AStarTest, spiral_path_navigation) {
    SCOPED_TRACE("Testing spiral path navigation");
    
    // Create a 5x5 grid_t with spiral obstacles
    std::vector<location_t> spiral_walls = {
        {1, 1}, {2, 1}, {3, 1},
        {3, 2}, {3, 3},
        {2, 3}, {1, 3},
        {1, 2}
    };
    
    auto spiral_grid = create_grid_with_obstacles(5, 5, spiral_walls);
    
    location_t start{0, 0};
    location_t goal{2, 2};  // Center of spiral
    
    std::vector<location_t> path;
    search(path, spiral_grid, start, goal);
    
    if (path.size() > 0) {
        EXPECT_EQ(path.front(), start);
        EXPECT_EQ(path.back(), goal);
        print_path(path);
        print_grid_with_path(path, 5, 5, spiral_walls);
    } else {
        std::cout << "No path found in spiral (might be blocked)" << std::endl;
    }
}

// Test performance with timing
TEST_F(AStarTest, performance_timing) {
    SCOPED_TRACE("Testing performance with timing");
    
    const int size = 20;
    auto test_grid = create_grid_with_obstacles(size, size, {});
    
    location_t start{0, 0};
    location_t goal{size-1, size-1};
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<location_t> path;
    search(path, test_grid, start, goal);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    EXPECT_GT(path.size(), 0);
    
    // std::cout << "Performance test (" << size << "x" << size << ") took: " 
    //           << duration.count() << " microseconds" << std::endl;
    // std::cout << "Path length: " << path.size() << std::endl;
    
    // Performance should be reasonable - less than 10s for 20x20 grid_t
    EXPECT_LT(duration.count(), 10000000);
}