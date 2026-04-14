#pragma once
#include "core/utils/Debug.h"
#include "ecs/common/ECSCommon.h"

#include <cstdlib>

template <size_t size>
struct InlineEntityList
{
    Entity list[size];

    size_t count = 0;
    bool overFlow = false; // For debugging

    void Push(Entity entity)
    {
        if (count < size)
        {
            list[count++] = entity;
        }
        else
        {
            overFlow = true;
#ifndef NDEBUG
            LOG_WARNING("InlineEntityList is overflowing")
#endif
        }
    }
    void clear()
    {
        count = 0;
        overFlow = false;
    }

    Entity* Begin()
    {
        return list;
    }

    Entity* End()
    {
        return list + count;
    }
};
