#pragma once

#include <chrono>


void
PrintProfiles();


struct FrameProfileScope
{
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    TimePoint start;

    FrameProfileScope();
    ~FrameProfileScope();
};


#define PROFILE_FRAME() auto dnu__profiler = FrameProfileScope{}
