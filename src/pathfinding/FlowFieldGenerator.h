#pragma once

#include "core/utils/Vect2.hpp"
#include "pathfinding/FlowField.h"

#include <cstddef>

class FlowFieldGenerator
{
private:
public:
    FlowField GenerateFlowField(Vect2f origin, size_t width, size_t height, size_t cellSize)
    {
        return FlowField{origin, width, height, cellSize};
    }
};
