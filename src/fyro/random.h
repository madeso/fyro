#pragma once

#include <memory>


struct RandomImpl;

struct Random
{
    Random();
    ~Random();

    float get_including(float max = 1.0f);
    double get_including(double max = 1.0);
    int get_excluding(int max);
    std::size_t get_excluding(std::size_t max);

    std::unique_ptr<RandomImpl> r;
};
