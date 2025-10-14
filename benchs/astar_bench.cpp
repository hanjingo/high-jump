#include <benchmark/benchmark.h>
#include <hj/algo/astar.hpp>
#include <vector>
#include <random>
#include <unordered_set>

using namespace hj::astar;

// Helper functions for benchmark setup
static std::mt19937 rng(12345);

// Create a grid with full 4-connectivity (no obstacles)
static grid<location<int>> create_benchmark_grid(int width, int height)
{
    grid<location<int>> test_grid(width, height);
    
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            location<int> current{x, y};
            std::vector<location<int>> neighbors;
            
            // Add 4-connected neighbors
            if(x > 0) neighbors.emplace_back(x - 1, y);     // left
            if(x < width - 1) neighbors.emplace_back(x + 1, y);  // right
            if(y > 0) neighbors.emplace_back(x, y - 1);     // up
            if(y < height - 1) neighbors.emplace_back(x, y + 1);  // down
            
            test_grid.add_location(current, neighbors);
        }
    }
    return test_grid;
}

// Create a grid with 8-connectivity (including diagonals)
static grid<location<int>> create_8connected_grid(int width, int height)
{
    grid<location<int>> test_grid(width, height);
    
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            location<int> current{x, y};
            std::vector<location<int>> neighbors;
            
            // Add all 8 neighbors
            for(int dy = -1; dy <= 1; dy++)
            {
                for(int dx = -1; dx <= 1; dx++)
                {
                    if(dx == 0 && dy == 0) continue;
                    
                    int nx = x + dx;
                    int ny = y + dy;
                    
                    if(nx >= 0 && nx < width && ny >= 0 && ny < height)
                    {
                        neighbors.emplace_back(nx, ny);
                        
                        // Set diagonal edge weights to sqrt(2)
                        if(abs(dx) + abs(dy) == 2)
                        {
                            test_grid.set_edge_weight(current, location<int>{nx, ny}, std::sqrt(2.0));
                        }
                    }
                }
            }
            
            test_grid.add_location(current, neighbors);
        }
    }
    return test_grid;
}

// Create a grid with random obstacles
static grid<location<int>> create_grid_with_random_obstacles(int width, int height, double obstacle_ratio)
{
    auto test_grid = create_benchmark_grid(width, height);
    
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    std::vector<location<int>> obstacles;
    
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            if(dist(rng) < obstacle_ratio)
            {
                obstacles.emplace_back(x, y);
            }
        }
    }
    
    test_grid.add_obstacles(obstacles);
    return test_grid;
}

// Create a maze-like grid with corridors
static grid<location<int>> create_maze_grid(int width, int height)
{
    auto test_grid = create_benchmark_grid(width, height);
    std::vector<location<int>> walls;
    std::uniform_real_distribution<double> wall_dist(0.0, 1.0);
    
    // Create a simple maze pattern
    for(int y = 1; y < height - 1; y += 2)
    {
        for(int x = 1; x < width - 1; x += 2)
        {
            walls.emplace_back(x, y);
            
            // Add random wall extensions
            if(wall_dist(rng) < 0.5 && x + 1 < width - 1)
                walls.emplace_back(x + 1, y);
            if(wall_dist(rng) < 0.5 && y + 1 < height - 1)
                walls.emplace_back(x, y + 1);
        }
    }
    
    test_grid.add_obstacles(walls);
    return test_grid;
}

// Generate random start and goal positions
static std::pair<location<int>, location<int>> generate_random_endpoints(int width, int height)
{
    std::uniform_int_distribution<int> x_dist(0, width - 1);
    std::uniform_int_distribution<int> y_dist(0, height - 1);
    
    location<int> start{x_dist(rng), y_dist(rng)};
    location<int> goal{x_dist(rng), y_dist(rng)};
    
    return {start, goal};
}

// Static heuristic functions
static heuristic_fn<location<int>> manhattan_heuristic_fn = [](const location<int>& a, const location<int>& b) { 
    return detail::manhattan_heuristic(a, b); 
};

static heuristic_fn<location<int>> euclidean_heuristic_fn = [](const location<int>& a, const location<int>& b) { 
    return detail::euclidean_heuristic(a, b); 
};

static heuristic_fn<location<int>> zero_heuristic_fn = [](const location<int>&, const location<int>&) { 
    return 0.0; 
};

// Static grids for benchmarks (initialized once)
static grid<location<int>> small_grid_4 = create_benchmark_grid(10, 10);
static grid<location<int>> medium_grid_4 = create_benchmark_grid(50, 50);
static grid<location<int>> large_grid_4 = create_benchmark_grid(100, 100);
static grid<location<int>> xlarge_grid_4 = create_benchmark_grid(200, 200);

static grid<location<int>> small_grid_8 = create_8connected_grid(10, 10);
static grid<location<int>> medium_grid_8 = create_8connected_grid(50, 50);
static grid<location<int>> large_grid_8 = create_8connected_grid(100, 100);

static grid<location<int>> obstacle_grid_small = create_grid_with_random_obstacles(20, 20, 0.2);
static grid<location<int>> obstacle_grid_medium = create_grid_with_random_obstacles(50, 50, 0.3);
static grid<location<int>> obstacle_grid_large = create_grid_with_random_obstacles(100, 100, 0.25);

static grid<location<int>> maze_small = create_maze_grid(21, 21);
static grid<location<int>> maze_medium = create_maze_grid(51, 51);
static grid<location<int>> maze_large = create_maze_grid(101, 101);

// Benchmark: Small grid pathfinding (10x10)
static void bm_astar_small_grid_manhattan(benchmark::State &state)
{
    location<int> start{0, 0};
    location<int> goal{9, 9};
    
    for(auto _ : state)
    {
        std::vector<location<int>> path;
        auto result = hj::astar::search(path, small_grid_4, start, goal, manhattan_heuristic_fn);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(path);
    }
}
BENCHMARK(bm_astar_small_grid_manhattan);

// Benchmark: Medium grid pathfinding (50x50)
static void bm_astar_medium_grid_manhattan(benchmark::State &state)
{
    location<int> start{0, 0};
    location<int> goal{49, 49};
    
    for(auto _ : state)
    {
        std::vector<location<int>> path;
        auto result = hj::astar::search(path, medium_grid_4, start, goal, manhattan_heuristic_fn);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(path);
    }
}
BENCHMARK(bm_astar_medium_grid_manhattan);

// Benchmark: Large grid pathfinding (100x100)
static void bm_astar_large_grid_manhattan(benchmark::State &state)
{
    location<int> start{0, 0};
    location<int> goal{99, 99};
    
    for(auto _ : state)
    {
        std::vector<location<int>> path;
        auto result = hj::astar::search(path, large_grid_4, start, goal, manhattan_heuristic_fn);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(path);
    }
}
BENCHMARK(bm_astar_large_grid_manhattan);

// Benchmark: Extra large grid pathfinding (200x200)
static void bm_astar_xlarge_grid_manhattan(benchmark::State &state)
{
    location<int> start{0, 0};
    location<int> goal{199, 199};
    
    for(auto _ : state)
    {
        std::vector<location<int>> path;
        auto result = hj::astar::search(path, xlarge_grid_4, start, goal, manhattan_heuristic_fn);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(path);
    }
}
BENCHMARK(bm_astar_xlarge_grid_manhattan);

// Benchmark: Euclidean heuristic comparison
static void bm_astar_medium_grid_euclidean(benchmark::State &state)
{
    location<int> start{0, 0};
    location<int> goal{49, 49};
    
    for(auto _ : state)
    {
        std::vector<location<int>> path;
        auto result = hj::astar::search(path, medium_grid_4, start, goal, euclidean_heuristic_fn);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(path);
    }
}
BENCHMARK(bm_astar_medium_grid_euclidean);

// Benchmark: Zero heuristic (Dijkstra's algorithm)
static void bm_astar_medium_grid_dijkstra(benchmark::State &state)
{
    location<int> start{0, 0};
    location<int> goal{49, 49};
    
    for(auto _ : state)
    {
        std::vector<location<int>> path;
        auto result = hj::astar::search(path, medium_grid_4, start, goal, zero_heuristic_fn);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(path);
    }
}
BENCHMARK(bm_astar_medium_grid_dijkstra);

// Benchmark: 8-connected grid
static void bm_astar_8connected_small(benchmark::State &state)
{
    location<int> start{0, 0};
    location<int> goal{9, 9};
    
    for(auto _ : state)
    {
        std::vector<location<int>> path;
        auto result = hj::astar::search(path, small_grid_8, start, goal, euclidean_heuristic_fn);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(path);
    }
}
BENCHMARK(bm_astar_8connected_small);

static void bm_astar_8connected_medium(benchmark::State &state)
{
    location<int> start{0, 0};
    location<int> goal{49, 49};
    
    for(auto _ : state)
    {
        std::vector<location<int>> path;
        auto result = hj::astar::search(path, medium_grid_8, start, goal, euclidean_heuristic_fn);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(path);
    }
}
BENCHMARK(bm_astar_8connected_medium);

static void bm_astar_8connected_large(benchmark::State &state)
{
    location<int> start{0, 0};
    location<int> goal{99, 99};
    
    for(auto _ : state)
    {
        std::vector<location<int>> path;
        auto result = hj::astar::search(path, large_grid_8, start, goal, euclidean_heuristic_fn);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(path);
    }
}
BENCHMARK(bm_astar_8connected_large);

// Benchmark: Pathfinding with obstacles
static void bm_astar_obstacles_small(benchmark::State &state)
{
    location<int> start{0, 0};
    location<int> goal{19, 19};
    
    for(auto _ : state)
    {
        std::vector<location<int>> path;
        auto result = hj::astar::search(path, obstacle_grid_small, start, goal, manhattan_heuristic_fn);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(path);
    }
}
BENCHMARK(bm_astar_obstacles_small);

static void bm_astar_obstacles_medium(benchmark::State &state)
{
    location<int> start{0, 0};
    location<int> goal{49, 49};
    
    for(auto _ : state)
    {
        std::vector<location<int>> path;
        auto result = hj::astar::search(path, obstacle_grid_medium, start, goal, manhattan_heuristic_fn);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(path);
    }
}
BENCHMARK(bm_astar_obstacles_medium);

static void bm_astar_obstacles_large(benchmark::State &state)
{
    location<int> start{0, 0};
    location<int> goal{99, 99};
    
    for(auto _ : state)
    {
        std::vector<location<int>> path;
        auto result = hj::astar::search(path, obstacle_grid_large, start, goal, manhattan_heuristic_fn);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(path);
    }
}
BENCHMARK(bm_astar_obstacles_large);

// Benchmark: Maze navigation
static void bm_astar_maze_small(benchmark::State &state)
{
    location<int> start{0, 0};
    location<int> goal{20, 20};
    
    for(auto _ : state)
    {
        std::vector<location<int>> path;
        auto result = hj::astar::search(path, maze_small, start, goal, manhattan_heuristic_fn);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(path);
    }
}
BENCHMARK(bm_astar_maze_small);

static void bm_astar_maze_medium(benchmark::State &state)
{
    location<int> start{0, 0};
    location<int> goal{50, 50};
    
    for(auto _ : state)
    {
        std::vector<location<int>> path;
        auto result = hj::astar::search(path, maze_medium, start, goal, manhattan_heuristic_fn);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(path);
    }
}
BENCHMARK(bm_astar_maze_medium);

static void bm_astar_maze_large(benchmark::State &state)
{
    location<int> start{0, 0};
    location<int> goal{100, 100};
    
    for(auto _ : state)
    {
        std::vector<location<int>> path;
        auto result = hj::astar::search(path, maze_large, start, goal, manhattan_heuristic_fn);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(path);
    }
}
BENCHMARK(bm_astar_maze_large);

// Benchmark: Random endpoints stress test
static void bm_astar_random_endpoints(benchmark::State &state)
{
    static std::vector<std::pair<location<int>, location<int>>> endpoints;
    if(endpoints.empty())
    {
        // Pre-generate 100 random endpoint pairs
        for(int i = 0; i < 100; ++i)
        {
            endpoints.push_back(generate_random_endpoints(50, 50));
        }
    }
    
    size_t endpoint_idx = 0;
    for(auto _ : state)
    {
        const auto& [start, goal] = endpoints[endpoint_idx % endpoints.size()];
        endpoint_idx++;
        
        std::vector<location<int>> path;
        auto result = hj::astar::search(path, medium_grid_4, start, goal, manhattan_heuristic_fn);
        benchmark::DoNotOptimize(result);
        benchmark::DoNotOptimize(path);
    }
}
BENCHMARK(bm_astar_random_endpoints);

// Benchmark: Multiple path searches (batch processing)
static void bm_astar_batch_pathfinding(benchmark::State &state)
{
    const int batch_size = 10;
    std::vector<location<int>> starts, goals;
    
    for(int i = 0; i < batch_size; ++i)
    {
        auto [start, goal] = generate_random_endpoints(30, 30);
        starts.push_back(start);
        goals.push_back(goal);
    }
    
    for(auto _ : state)
    {
        for(int i = 0; i < batch_size; ++i)
        {
            std::vector<location<int>> path;
            auto result = hj::astar::search(path, create_benchmark_grid(30, 30), starts[i], goals[i]);
            benchmark::DoNotOptimize(result);
            benchmark::DoNotOptimize(path);
        }
    }
}
BENCHMARK(bm_astar_batch_pathfinding);

// Benchmark: Path smoothing performance
static void bm_astar_path_smoothing(benchmark::State &state)
{
    // Create a long zigzag path for smoothing
    std::vector<location<int>> zigzag_path;
    for(int i = 0; i < 100; ++i)
    {
        zigzag_path.emplace_back(i, i % 2);
    }
    
    for(auto _ : state)
    {
        auto smoothed = detail::smooth(zigzag_path);
        benchmark::DoNotOptimize(smoothed);
    }
}
BENCHMARK(bm_astar_path_smoothing);

// Benchmark: Heuristic function performance
static void bm_manhattan_heuristic(benchmark::State &state)
{
    location<int> a{0, 0};
    location<int> b{100, 100};
    
    for(auto _ : state)
    {
        double distance = detail::manhattan_heuristic(a, b);
        benchmark::DoNotOptimize(distance);
    }
}
BENCHMARK(bm_manhattan_heuristic);

static void bm_euclidean_heuristic(benchmark::State &state)
{
    location<int> a{0, 0};
    location<int> b{100, 100};
    
    for(auto _ : state)
    {
        double distance = detail::euclidean_heuristic(a, b);
        benchmark::DoNotOptimize(distance);
    }
}
BENCHMARK(bm_euclidean_heuristic);

// Benchmark: Priority queue operations
static void bm_priority_queue_operations(benchmark::State &state)
{
    for(auto _ : state)
    {
        detail::priority_queue<location<int>, double> pq;
        
        // Fill priority queue
        for(int i = 0; i < 1000; ++i)
        {
            pq.put(location<int>{i, i}, static_cast<double>(1000 - i));
        }
        
        // Empty priority queue
        while(!pq.empty())
        {
            auto item = pq.get();
            benchmark::DoNotOptimize(item);
        }
    }
}
BENCHMARK(bm_priority_queue_operations);