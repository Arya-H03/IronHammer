#pragma once
#include <sstream>
#include <format>
#include "imgui.h"
#include <SFML/Graphics/Color.hpp>

namespace EntityInspectorHelpers
{
    inline float inputFieldWidth = 96;
    inline float colorFieldWidth = 120;

    template <typename T>
    inline void TypeHeader(const std::string& typeName, void* ptr)
    {
        std::ostringstream oss;
        oss << ptr;
        std::string typeInfo = std::format("{} Bytes at {}", sizeof(T), oss.str());
        ImGui::SeparatorText(typeName.c_str());
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("%s", typeInfo.c_str());
            ImGui::EndTooltip();
        }
    }

    inline void TableNextField(const char* tag)
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%s", tag);
        ImGui::TableSetColumnIndex(1);
    }

    inline void DragScalar(const char* label, size_t* value)
    {
        ImGui::Text(" ");
        ImGui::SameLine();
        ImGui::PushItemWidth(inputFieldWidth);
        ImGui::DragScalar(label, ImGuiDataType_U64, value);
        ImGui::PopItemWidth();
    }

    inline void DragFloat2(const char* labelX, float* x, const char* labelY, float* y, float speed = 0.1f)
    {
        ImGui::PushItemWidth(inputFieldWidth);
        ImGui::Text("x");
        ImGui::SameLine();
        ImGui::DragFloat(labelX, x, speed);
        ImGui::SameLine();
        ImGui::Text("y");
        ImGui::SameLine();
        ImGui::DragFloat(labelY, y, speed);
        ImGui::PopItemWidth();
    }

    inline void DragFloatWithLimits(const char* label, float* value, float speed, float min = 0.0f, float max = 10000.0f)
    {
        ImGui::PushItemWidth(inputFieldWidth);
        ImGui::Text(" ");
        ImGui::SameLine();
        ImGui::DragFloat(label, value, speed, min, max);
        ImGui::PopItemWidth();
    }

    inline void ColorEdit4(const char* label, sf::Color& color)
    {
        float col[4] = { color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f };
        ImGui::Text(" ");
        ImGui::SameLine();
        ImGui::PushItemWidth(colorFieldWidth);
        if (ImGui::ColorEdit4(label, col))
        {
            color = sf::Color(static_cast<uint8_t>(col[0] * 255),
                static_cast<uint8_t>(col[1] * 255),
                static_cast<uint8_t>(col[2] * 255),
                static_cast<uint8_t>(col[3] * 255));
        }
        ImGui::PopItemWidth();
    }

    inline void Checkbox(const char* label, bool* value)
    {
        ImGui::Text(" ");
        ImGui::SameLine();
        ImGui::Checkbox(label, value);
    }

    inline void InputText(const char* label, std::string& str)
    {
        char buffer[256] {};
        strncpy(buffer, str.c_str(), sizeof(buffer) - 1);
        ImGui::Text(" ");
        ImGui::SameLine();
        ImGui::PushItemWidth(inputFieldWidth);
        if (ImGui::InputText(label, buffer, sizeof(buffer)))
        {
            str = buffer;
        }
        ImGui::PopItemWidth();
    }
}
