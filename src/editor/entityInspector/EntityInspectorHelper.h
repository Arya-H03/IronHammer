#pragma once
#include "assets/AssetManager.h"
#include "imgui.h"

#include <cstdint>
#include <cstring>
#include <format>
#include <functional>
#include <SFML/Graphics/Color.hpp>
#include <sstream>

namespace EntityInspectorHelpers {
inline float inputFieldWidth = 100;
inline float colorFieldWidth = 225;

template <typename T>
inline bool ComponentHeader(const char* label, T* ptr, const std::function<void()>& RemoveComponentCallback,
                            const std::function<void()>& ResetCallback, bool* isDirty = nullptr)
{
    std::ostringstream oss;
    oss << ptr;
    std::string typeInfo = std::format("{} Bytes at {}", sizeof(T), oss.str());

    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    bool open = ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen);
    ImGui::PopStyleColor();
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("%s", typeInfo.c_str());
        ImGui::EndTooltip();
    }

    if (open) {
        float headerWidth = ImGui::GetContentRegionAvail().x;

        std::string resetLabel  = std::string("Reset##") + label;
        std::string removeLabel = std::string("Remove##") + label;

        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

        ImGui::SetCursorPosX(headerWidth - 35);
        if (ImGui::ImageButton(resetLabel.c_str(), AssetManager::Instance().GetTextureID("ResetBtn"), ImVec2(12.5, 12.5))) {
            ResetCallback();
            if (isDirty != nullptr) *isDirty = true;
        }

        ImGui::SameLine();
        ImGui::SetCursorPosX(headerWidth - 10);
        if (ImGui::ImageButton(removeLabel.c_str(), AssetManager::Instance().GetTextureID("CrossBtn"), ImVec2(12.5, 12.5))) {
            RemoveComponentCallback();
            if (isDirty != nullptr) *isDirty = true;
        }

        ImGui::PopStyleColor(2);
    }
    return open;
}
inline void TableNextField(const char* tag)
{
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("%s", tag);
    ImGui::TableSetColumnIndex(1);
    ImGui::SetCursorPosX(135);
}

inline void DragScalar(const char* label, size_t* value, bool* isDirty = nullptr)
{
    ImGui::Text(" ");
    ImGui::SameLine();
    ImGui::PushItemWidth(inputFieldWidth);
    if (ImGui::DragScalar(label, ImGuiDataType_U64, value)) {
        if (isDirty != nullptr) *isDirty = true;
    }
    ImGui::PopItemWidth();
}

inline void DragFloat2(const char* labelX, float* x, const char* labelY, float* y, float speed = 0.1f,
                       bool* isDirty = nullptr)
{
    ImGui::PushItemWidth(inputFieldWidth);
    ImGui::Text("x");
    ImGui::SameLine();
    if (ImGui::DragFloat(labelX, x, speed)) {
        if (isDirty != nullptr) *isDirty = true;
    }

    ImGui::SameLine();
    ImGui::Text("y");
    ImGui::SameLine();
    if (ImGui::DragFloat(labelY, y, speed)) {
        if (isDirty != nullptr) *isDirty = true;
    }

    ImGui::PopItemWidth();
}

inline void DragFloatWithLimits(const char* label, float* value, float speed, float min = 0.0f, float max = 10000.0f,
                                bool* isDirty = nullptr)
{
    ImGui::PushItemWidth(inputFieldWidth);
    ImGui::Text(" ");
    ImGui::SameLine();
    if (ImGui::DragFloat(label, value, speed, min, max)) {
        if (isDirty != nullptr) *isDirty = true;
    }
    ImGui::PopItemWidth();
}

inline void ColorEdit4(const char* label, sf::Color& color, bool* isDirty = nullptr)
{
    float col[4] = {color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f};
    ImGui::Text(" ");
    ImGui::SameLine();
    ImGui::PushItemWidth(colorFieldWidth);
    if (ImGui::ColorEdit4(label, col)) {
        color = sf::Color(static_cast<uint8_t>(col[0] * 255), static_cast<uint8_t>(col[1] * 255),
                          static_cast<uint8_t>(col[2] * 255), static_cast<uint8_t>(col[3] * 255));

        if (isDirty != nullptr) *isDirty = true;
    }
    ImGui::PopItemWidth();
}

inline void Checkbox(const char* label, bool* value, bool* isDirty = nullptr)
{
    ImGui::Text(" ");
    ImGui::SameLine();
    if (ImGui::Checkbox(label, value)) {
        if (isDirty != nullptr) *isDirty = true;
    }
}

inline void InputText(const char* label, std::string& str, bool* isDirty = nullptr)
{
    char buffer[256]{};
    strncpy(buffer, str.c_str(), sizeof(buffer) - 1);
    ImGui::Text(" ");
    ImGui::SameLine();
    ImGui::PushItemWidth(inputFieldWidth);
    if (ImGui::InputText(label, buffer, sizeof(buffer))) {
        str = buffer;
        if (isDirty != nullptr) *isDirty = true;
    }
    ImGui::PopItemWidth();
}

inline void InputTextWithHint(const char* label, const char* hint, std::string& str, bool* isDirty = nullptr)
{
    char buffer[256]{};
    strncpy(buffer, str.c_str(), sizeof(buffer) - 1);
    ImGui::Text(" ");
    ImGui::SameLine();
    ImGui::PushItemWidth(inputFieldWidth);
    if (ImGui::InputTextWithHint(label, hint, buffer, sizeof(buffer))) {
        str = buffer;
        if (isDirty != nullptr) *isDirty = true;
    }
    ImGui::PopItemWidth();
}

inline void ComboBoxSingleSelect(const char* label, const char* currentOption, const char* const* options, size_t count,
                                 const std::function<void(size_t selectedIndex)>& SelectCombo, bool* isDirty = nullptr)
{
    ImGui::Text(" ");
    ImGui::SameLine();
    ImGui::PushItemWidth(inputFieldWidth);
    if (ImGui::BeginCombo(label, currentOption, ImGuiComboFlags_WidthFitPreview)) {
        for (size_t i = 0; i < count; i++) {
            bool isSelected = strcmp(currentOption, options[i]) == 0;
            if (ImGui::Selectable(options[i], isSelected)) {
                SelectCombo(i);
                if (isDirty != nullptr) *isDirty = true;
            }

            if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();
}
inline void ComboBoxMultipleSelect(const char* label, const char* firstOption, const char* const* options, size_t count,
                                   bool* selectedOptions, const std::function<void(size_t selectedIndex)>& SelectCombo,
                                   bool* isDirty = nullptr)
{
    ImGui::Text(" ");
    ImGui::SameLine();
    ImGui::PushItemWidth(inputFieldWidth);
    if (ImGui::BeginCombo(label, firstOption, ImGuiComboFlags_WidthFitPreview)) {
        for (size_t i = 0; i < count; i++) {
            bool isSelected = selectedOptions[i];
            if (ImGui::Selectable(options[i], isSelected)) {
                SelectCombo(i);
                if (isDirty != nullptr) *isDirty = true;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();
}

} // namespace EntityInspectorHelpers
