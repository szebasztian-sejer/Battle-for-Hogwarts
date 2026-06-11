#include <helpers.h>
#include <random>
#include <chrono>

int getRandomInt(int min, int max)
{
	unsigned static seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::ranlux24_base rng(seed++);
	std::uniform_int_distribution<int> dist(min, max);
	return dist(rng);
}