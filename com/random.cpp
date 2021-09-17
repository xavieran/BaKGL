#include "com/random.hpp"

#include <random>

unsigned GetRandomNumber(unsigned min, unsigned max)
{
    static std::random_device random_device;
    static std::mt19937 engine{random_device()};
    std::uniform_int_distribution<> dist(min, max);
    return dist(engine);
}
