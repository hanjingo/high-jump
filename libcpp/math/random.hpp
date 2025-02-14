#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>

namespace libcpp
{

static std::random_device default_seed;

class random
{

public:
    random() {}
    ~random() {}

    template <int64_t Min, int64_t Max>
    static int64_t range(std::random_device& seed = default_seed)
    {
        std::default_random_engine engine(seed());

        std::uniform_int_distribution<int64_t> uniform_dist(Min, Max);
        return uniform_dist(engine);
    }

    template <typename T>
    static T range(T min, T max, std::random_device& seed = default_seed)
    {
        std::default_random_engine engine(seed());

        std::uniform_int_distribution<T> uniform_dist(min, max);
        return uniform_dist(engine);
    }
};

}

#endif