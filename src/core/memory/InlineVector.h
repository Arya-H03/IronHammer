#pragma once
#include "core/utils/Debug.h"
#include <cstdlib>

template <typename T, size_t size>
struct InlineVector
{
    T list[size];

    size_t count = 0;
    bool overFlow = false; // For debugging

    void Push(T t)
    {
        if (count < size)
        {
            list[count++] = t;
        }
        else
        {
            overFlow = true;
#ifndef NDEBUG
            LOG_WARNING("InlineVector is overflowing")
#endif
        }
    }

    void clear()
    {
        count = 0;
        overFlow = false;
    }

    T* Begin() const
    {
        return list;
    }

    T* End() const
    {
        return list + count;
    }

    size_t Size() const
    {
        return count;
    }
};
