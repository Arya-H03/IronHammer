#pragma once
#include <format>
#include <string>


struct CPosition
{
    float x, y, z;

    const std::string GetInfo() const
    {
        return std::format("({}, {}, {})",x,y,z);
    }
};

struct CVelocity
{
    float x, y, z;

    const std::string GetInfo() const
    {
        return std::format("({}, {}, {})",x,y,z);
    }
};

struct CRotation
{
    float x, y, z;

    const std::string GetInfo() const
    {
        return std::format("({}, {}, {})",x,y,z);
    }
};
