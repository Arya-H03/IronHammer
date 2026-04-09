#pragma once

#include "core/utils/Vect2.hpp"
#include "imgui.h"

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstdint>
class Viewport
{
  private:
    inline static Vect2f m_position{0.f, 0.f};
    inline static Vect2<uint16_t> m_size{0, 0};
    inline static sf::RenderTexture* m_renderTexture;
    inline static ImVec2 m_viewportImagePos;
    inline static ImVec2 m_viewportImageDrawSize;

  public:
    inline static void SetViewportSize(const Vect2f& position, const Vect2<uint16_t>& size)
    {
        m_position = position;
        m_size = size;
    }

    inline static void SetViewportTexture(sf::RenderTexture* renderTexture) { m_renderTexture = renderTexture; }
    inline static void UpdateViewportImage(ImVec2 pos, ImVec2 drawSize)
    {
        m_viewportImagePos = pos;
        m_viewportImageDrawSize = drawSize;
    };

    inline static Vect2f GetPosition() { return m_position; }

    inline static Vect2<uint16_t> GetSize() { return m_size; }

    inline static bool IsMouseInsideViewport()
    {
        Vect2f mouse = ImGui::GetMousePos();

        return mouse.x >= m_position.x && mouse.y >= m_position.y && mouse.x <= m_position.x + m_size.x &&
               mouse.y <= m_position.y + m_size.y;
    }

    inline static Vect2f ScreenToViewport(const Vect2f& screenPos)
    {
        float x = screenPos.x - m_position.x;
        float y = screenPos.y - m_position.y;

        if (x < 0 || y < 0 || x > m_size.x || y > m_size.y) return {-1.0f, -1.0f};

        return {x, y};
    }

    inline static Vect2f ScreenToViewportMouse() { return ScreenToViewport(ImGui::GetMousePos()); }

    inline static Vect2f ScreenToViewportNormalized(const Vect2f& screenPos)
    {
        if (m_size.x == 0 || m_size.y == 0) return Vect2f(-1.f, -1.f);

        Vect2f vect = ScreenToViewport(screenPos);
        if (vect.x < 0 || vect.y < 0) return Vect2f(-1.f, -1.f);

        return {vect.x / m_size.x, vect.y / m_size.y};
    }

    inline static Vect2f ScreenToViewportNormalizedMouse() { return ScreenToViewportNormalized(ImGui::GetMousePos()); }

    inline static ImVec2 WorldToViewportGui(const Vect2f& worldPos)
    {
        sf::Vector2i texturePixel = m_renderTexture->mapCoordsToPixel({worldPos.x, worldPos.y});
        sf::Vector2u textureSize = m_renderTexture->getSize();

        float scaleX = m_viewportImageDrawSize.x / (float)textureSize.x;
        float scaleY = m_viewportImageDrawSize.y / (float)textureSize.y;

        return ImVec2(m_viewportImagePos.x + (float)texturePixel.x * scaleX, m_viewportImagePos.y + (float)texturePixel.y * scaleY);
    }
};
