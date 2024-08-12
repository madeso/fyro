#pragma once

constexpr float rgb_i2f(int i)
{
	return static_cast<float>(i) / 255.0f;
}

struct Rgb
{
	float r;
	float g;
	float b;

	Rgb() = default;

	constexpr Rgb(float ir, float ig, float ib)
		: r(ir)
		, g(ig)
		, b(ib)
	{
	}

	constexpr Rgb(int ir, int ig, int ib)
		: r(rgb_i2f(ir))
		, g(rgb_i2f(ig))
		, b(rgb_i2f(ib))
	{
	}
};
