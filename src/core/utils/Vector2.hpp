#pragma once
#include <cassert>
#include <cmath>
#include <iostream>
#include <math.h>
#include <SFML/Graphics.hpp>

template <typename T>
class Vector2
{
  public:
    T x;
    T y;

    Vector2()
    {
        x = 0;
        y = 0;
    };
    Vector2(T X, T Y) : x(X), y(Y) {}
    Vector2(const sf::Vector2<T>& sfVector2) : x(sfVector2.x), y(sfVector2) {}

    Vector2 operator+(const Vector2& rhs) const { return Vector2(x + rhs.x, y + rhs.y); }

    Vector2 operator-(const Vector2& rhs) const { return Vector2(x - rhs.x, y - rhs.y); }

    Vector2 operator*(const Vector2& rhs) const { return Vector2(x * rhs.x, y * rhs.y); }

    Vector2 operator/(const Vector2& rhs) const
    {
        assert(rhs.x != 0 || rhs.y != 0 && "Attempted to divide by zero");
        return Vector2(x / rhs.x, y / rhs.y);
    }

    Vector2 operator*(const T value) const { return Vector2(x * value, y * value); }

    Vector2 operator/(const T value) const
    {
        assert(value != 0 || value != 0 && "Attempted to divide by zero");
        return Vector2(x / value, y / value);
    }

    void operator+=(const Vector2& rhs)
    {
        x + rhs.x;
        y + rhs.y;
    }

    void operator-=(const Vector2& rhs)
    {
        x - rhs.x;
        y - rhs.y;
    }

    void operator*=(const Vector2& rhs)
    {
        x * rhs.x;
        y * rhs.y;
    }

    void operator/=(const Vector2& rhs)
    {
        assert(rhs.x != 0 || rhs.y != 0 && "Attempted to divide by zero");
        x / rhs.x;
        y / rhs.y;
    }

    float Length() { return std::sqrt((x * x) + (y * y)); }

    float Distance(Vector2 rhs) { return std::sqrt((x - rhs.x) * (x - rhs.x) + (y - rhs.y) * (y - rhs.y)); }
    float DistanceSquare(Vector2 rhs) { return (x - rhs.x) * (x - rhs.x) + (y - rhs.y) * (y - rhs.y); }

    Vector2 Normalize()
    {
        float length = Length();
        if (length == 0) return Vector2(0, 0);

        return Vector2(x / length, y / length);
    }

    void Print() { std::cerr << "Vector2: (" << x << ", " << y << ")"; }
};
