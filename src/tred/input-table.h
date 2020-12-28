#pragma once

#include <string>
#include <map>


struct Table
{
    void Set(const std::string& name, float value);

    std::map<std::string, float> data;
};