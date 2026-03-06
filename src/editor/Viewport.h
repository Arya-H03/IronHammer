#pragma once

#include "core/utils/Vect2.hpp"
#include <cstdint>
class Viewport
{
  private:

    inline static Vect2f m_position { 0.f, 0.f };
    inline static Vect2<uint16_t> m_size { 0, 0 };

  public:

    inline static void SetViewport(const Vect2f& position, const Vect2<uint16_t>& size)
    {
        m_position = position;
        m_size = size;
    }

    inline static Vect2f GetPosition() { return m_position; }

    inline static Vect2<uint16_t> GetSize() { return m_size; }

    inline static bool IsMouseInside()
    {
        Vect2f mouse = ImGui::GetMousePos();

        return mouse.x >= m_position.x && mouse.y >= m_position.y && mouse.x <= m_position.x + m_size.x && mouse.y <= m_position.y + m_size.y;
    }

    inline static Vect2f ScreenToViewport(const Vect2f& screenPos)
    {
        float x = screenPos.x - m_position.x;
        float y = screenPos.y - m_position.y;

        if (x < 0 || y < 0 || x > m_size.x || y > m_size.y) return { -1.0f, -1.0f };

        // flip Y because render texture
        y = m_size.y - y;

        return { x, y };
    }

    inline static Vect2f ScreenToViewportMouse() { return ScreenToViewport(ImGui::GetMousePos()); }

    inline static Vect2f ScreenToViewportNormalized(const Vect2f& screenPos)
    {
        if (m_size.x == 0 || m_size.y == 0) return Vect2f(-1.f, -1.f);

        Vect2f vect = ScreenToViewport(screenPos);
        if (vect.x < 0 || vect.y < 0) return Vect2f(-1.f, -1.f);

        return { vect.x / m_size.x, vect.y / m_size.y };
    }

    inline static Vect2f ScreenToViewportNormalizedMouse() { return ScreenToViewportNormalized(ImGui::GetMousePos()); }
};
