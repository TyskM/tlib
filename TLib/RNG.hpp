#pragma once
#include <cassert>
#include <random>
#include <stdexcept>
#ifdef BOOST_SERIALIZE
#include <sstream>
#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>
#endif

// Basic rng with convenience functions
// Relies on std::random_device and std::mt19937
class RNG
{
public:
    std::mt19937 generator;

    RNG(unsigned int seedvalue) { seed(seedvalue); }
    RNG() { seed(); }

    // Seeds the generator with the specified seed.
    void seed(unsigned int value) { generator.seed(value); }

    // Seeds the generator with a random seed.
    void seed() { std::random_device rd; generator.seed(rd()); }

    // Returns a random int between min and max, including min and max.
    template <typename T = int>
    int randRangeInt(T min, T max)
    {
        std::uniform_int_distribution<> dist(min, max);
        return dist(generator);
    }

    // Returns a random floating point value between min and max, including min and max.
    // Uses float by default
    template <typename T = float>
    T randRangeReal(T min, T max)
    {
        std::uniform_real_distribution<T> dist(min, max);
        return dist(generator);
    }

    bool randBool() { return randRangeInt(0, 1) == 1; }

    // Returns a random object from a std::vector.
    // It will explode if the vector is empty
    template <typename T>
    T choice(std::vector<T> items)
    {
        assert(items.size() > 0);
        return items[randRangeInt(0, items.size() - 1)];
    }
};