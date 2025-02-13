#pragma once
#include <cassert>
#include <random>
#include <stdexcept>
#include <TLib/Containers/Vector.hpp>

// Basic rng with convenience functions
// Relies on std::random_device and std::mt19937
class RNG
{
    std::mt19937 generator;
    uint32_t     currentSeed = 0;

public:

    RNG(uint32_t seedvalue) { seed(seedvalue); }
    RNG() { seed(); }

    // Seeds the generator with the specified seed.
    void seed(uint32_t value)
    {
        generator.seed(value);
        currentSeed = value;
    }

    // Seeds the generator with a random seed.
    void seed()
    {
        std::random_device rd;
        currentSeed = rd();
        generator.seed(currentSeed);
    }

    // Returns 0 if no seed is set
    uint32_t getSeed() const
    { return currentSeed; }

    [[nodiscard]]
    String getState() const
    {
        std::stringstream ss;
        ss << generator;
        return ss.str();
    }

    void setState(const String& state)
    {
        std::stringstream ss;
        ss << state;
        ss >> generator;
    }

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

    // Don't use lol
    // Returns a random object from a std::vector.
    // It will explode if the vector is empty
    template <typename Cont>
    Cont::value_type& choice(Cont items)
    {
        ASSERT(items.size() > 0);
        return items[randRangeInt<size_t>(size_t(0), items.size() - 1)];
    }
};