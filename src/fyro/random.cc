#include "fyro/random.h"

#include <random>

#include "fyro/cint.h"


struct RandomImpl
{
	std::random_device rd;
	std::mt19937 gen;

	RandomImpl()
		: gen(rd())
	{
	}

	double get_including(double max = 1.0)
	{
		std::uniform_real_distribution<> random(0.0, max);
		return random(gen);
	}

	float get_including(float max = 1.0)
	{
		return static_cast<float>(get_including(static_cast<double>(max)));
	}

	int get_excluding(int max)
	{
		std::uniform_int_distribution<> random(0, max-1);
		return random(gen);
	}

	std::size_t get_excluding(std::size_t max)
	{
		return Cint_to_sizet(get_excluding(Csizet_to_int(max)));
	}
};


Random::Random()
	: r(std::make_unique<RandomImpl>())
{
}

Random::~Random() = default;

float Random::get_including(float max) { return r->get_including(max); }
double Random::get_including(double max) { return r->get_including(max); }
int Random::get_excluding(int max) { return r->get_excluding(max); }
size_t Random::get_excluding(std::size_t max) { return r->get_excluding(max); }
