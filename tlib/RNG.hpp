#pragma once
#include <cassert>
#include <random>
#include <stdexcept>

class RNG
{
protected:
	std::mt19937 generator;
	std::random_device rd;

public:
	RNG()
	{ generator = std::mt19937(rd()); }

	// Seeds the generator with the specified seed.
	void seed(unsigned int value) { generator.seed(value); }

	// Seeds the generator with a random seed.
	void seed() { generator.seed(rd()); }

	// Returns a random int between min and max, including min and max.
	int randRangeInt(int min, int max)
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

	// Returns a random object from a std::vector.
	// It will explode if the vector is empty
	template <typename T>
	T choice(std::vector<T> items)
	{
		assert(items.size() > 0);
		return items[randRangeInt(0, items.size() - 1)];
	}
};