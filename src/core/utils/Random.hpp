#pragma once
#include "core/utils/Vect2.hpp"

#include <SFML/Graphics/Color.hpp>
#include <random>

class Random
{
  private:
    inline static std::mt19937 m_engine; // Random engine

  public:
    static void Init()
    {
        std::random_device rd;
        m_engine.seed(rd());
    }

    static int Int(int min, int max)
    {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(m_engine);
    }

    static float Float(float min, float max)
    {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(m_engine);
    }

    static Vect2f Vect2f(Vect2f xBounds, Vect2f yBounds)
    {
        float x = Float(xBounds.x, xBounds.y);
        float y = Float(yBounds.x, yBounds.y);

        return {x, y};
    }

    static sf::Color Color()
    {
        return sf::Color(Int(0, 255), Int(0, 255), Int(0, 255), 255);
    }
};
