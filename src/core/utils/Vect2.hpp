#pragma once
#include <SFML/Window/Joystick.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include <math.h>
#include <SFML/Graphics.hpp>

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
    };
    Vect2(T X, T Y) : x(X), y(Y) { }
    Vect2(const sf::Vector2<T>& sfVector2) : x(sfVector2.x), y(sfVector2) { }

    Vect2 operator+(const Vect2& rhs) const { return Vect2(x + rhs.x, y + rhs.y); }

    Vect2 operator-(const Vect2& rhs) const { return Vect2(x - rhs.x, y - rhs.y); }

    Vect2 operator*(const T value) const { return Vect2(x * value, y * value); }

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

    float Length() { return std::sqrt((x * x) + (y * y)); }

    float Distance(Vect2 rhs) { return std::sqrt((x - rhs.x) * (x - rhs.x) + (y - rhs.y) * (y - rhs.y)); }
    float DistanceSquare(Vect2 rhs) { return (x - rhs.x) * (x - rhs.x) + (y - rhs.y) * (y - rhs.y); }

    float DotProduct(const Vect2& other) { return (x * other.x) + (y * other.y); }

    Vect2 Normalize()
    {
        float length = Length();
        if (length == 0) return Vect2(0, 0);

        return Vect2(x / length, y / length);
    }

    Vect2 Abs() { return Vect2(std::abs(x), std::abs(y)); }

    Vect2<int> Floor() { return Vect2<int>(std::floor(x), std::floor(y)); }

    Vect2<int> Ceil() { return Vect2<int>(std::ceil(x), std::ceil(y)); }

    void Print() { std::cerr << "Vector2: (" << x << ", " << y << ")"; }
};

using Vect2f = Vect2<float>;
