#pragma once
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

	// Returns a random object from a std::vector.
	// It will throw an exception if the vector is empty.
	template <typename T>
	T choice(std::vector<T> vec)
	{
		if (vec.size() <= 0) throw std::runtime_error("Vector was empty!!");
		return vec[randRangeInt(0, vec.size() - 1)];
	}
};