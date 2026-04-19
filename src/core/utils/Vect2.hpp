#pragma once
#include "imgui.h"

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include <math.h>
#include <sys/types.h>

template <typename T>
class Vect2
{
  public:
    T x;
    T y;

    Vect2()
    {
        x = 0;
        y = 0;
    }
    Vect2(T X, T Y) : x(X), y(Y)
    {
    }

    Vect2(const sf::Vector2<T>& sfVector2) : x(sfVector2.x), y(sfVector2.y)
    {
    }

    Vect2(const ImVec2& imGuiVec2) : x(static_cast<T>(imGuiVec2.x)), y(static_cast<T>(imGuiVec2.y))
    {
    }

    operator ImVec2() const
    {
        return ImVec2(static_cast<float>(x), static_cast<float>(y));
    }

    Vect2 operator+(const Vect2& rhs) const
    {
        return Vect2(x + rhs.x, y + rhs.y);
    }

    Vect2 operator-(const Vect2& rhs) const
    {
        return Vect2(x - rhs.x, y - rhs.y);
    }

    Vect2 operator*(const T value) const
    {
        return Vect2(x * value, y * value);
    }

    Vect2 operator/(const T value) const
    {
        assert(value != 0 || value != 0 && "Attempted to divide by zero");
        return Vect2(x / value, y / value);
    }

    void operator+=(const Vect2& rhs)
    {
        x += rhs.x;
        y += rhs.y;
    }

    void operator-=(const Vect2& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
    }

    void operator*=(const T value)
    {
        x *= value;
        y *= value;
    }

    void operator/=(const T value)
    {
        assert(value != 0 && "Attempted to divide by zero");
        x /= value;
        y /= value;
    }

    bool operator==(const Vect2<T>& rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }

    float Length()
    {
        return std::sqrt((x * x) + (y * y));
    }

    float Distance(Vect2 rhs)
    {
        return std::sqrt((x - rhs.x) * (x - rhs.x) + (y - rhs.y) * (y - rhs.y));
    }
    float DistanceSquare(Vect2 rhs)
    {
        return (x - rhs.x) * (x - rhs.x) + (y - rhs.y) * (y - rhs.y);
    }

    float DotProduct(const Vect2& other)
    {
        return (x * other.x) + (y * other.y);
    }

    Vect2 Normalize()
    {
        float length = Length();
        if (length == 0) return Vect2(0, 0);

        return Vect2(x / length, y / length);
    }

    Vect2 Abs()
    {
        return Vect2(std::abs(x), std::abs(y));
    }

    Vect2<int> Floor()
    {
        return Vect2<int>(std::floor(x), std::floor(y));
    }

    Vect2<int> Ceil()
    {
        return Vect2<int>(std::ceil(x), std::ceil(y));
    }

    void Print()
    {
        std::cerr << "Vector2: (" << x << ", " << y << ")";
    }

    static const Vect2<T> Zero;
    static const Vect2<T> One;
};

using Vect2f = Vect2<float>;
using Vect2int = Vect2<int>;
using Vect2s = Vect2<size_t>;

template <typename T>
const inline Vect2<T> Vect2<T>::Zero = Vect2<T>{0, 0};

template <typename T>
const inline Vect2<T> Vect2<T>::One = Vect2<T>{1, 1};
