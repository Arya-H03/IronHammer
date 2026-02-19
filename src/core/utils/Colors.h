#pragma once

#include <SFML/Graphics.hpp>
#include <imgui.h>

namespace Colors
{
    // 🛡 Gunmetal - Primary Background
    inline constexpr sf::Color Gunmetal_SFML{42, 46, 51, 255};
    inline constexpr ImVec4 Gunmetal_ImGui{0.165f, 0.180f, 0.200f, 1.0f};

    // 🔩 Dark Steel - Secondary Panels
    inline constexpr sf::Color DarkSteel_SFML{34, 38, 41, 255};
    inline constexpr ImVec4 DarkSteel_ImGui{0.133f, 0.149f, 0.161f, 1.0f};

    // ⚔ Cold Steel Blue - Active / Selection / Headers
    inline constexpr sf::Color ColdSteelBlue_SFML{143, 166, 184, 255};
    inline constexpr ImVec4 ColdSteelBlue_ImGui{0.561f, 0.651f, 0.722f, 1.0f};

    // ☣ Hazard Yellow - Primary Accent
    inline constexpr sf::Color HazardYellow_SFML{201, 162, 39, 255};
    inline constexpr ImVec4 HazardYellow_ImGui{0.788f, 0.635f, 0.153f, 1.0f};

    // 🟡 Brass - Secondary Accent
    inline constexpr sf::Color Brass_SFML{184, 153, 31, 255};
    inline constexpr ImVec4 Brass_ImGui{0.722f, 0.600f, 0.122f, 1.0f};

    // 🧱 Rust Red - Danger / Destructive
    inline constexpr sf::Color RustRed_SFML{122, 46, 46, 255};
    inline constexpr ImVec4 RustRed_ImGui{0.478f, 0.180f, 0.180f, 1.0f};

    // 🟢 Oxidized Green - Strings / Secondary Highlight
    inline constexpr sf::Color OxidizedGreen_SFML{122, 159, 122, 255};
    inline constexpr ImVec4 OxidizedGreen_ImGui{0.478f, 0.624f, 0.478f, 1.0f};

    // 🔥 Industrial Orange - Numbers / Alerts
    inline constexpr sf::Color IndustrialOrange_SFML{184, 106, 46, 255};
    inline constexpr ImVec4 IndustrialOrange_ImGui{0.722f, 0.416f, 0.180f, 1.0f};

    // 🪨 Concrete Grey - Default Text
    inline constexpr sf::Color ConcreteGrey_SFML{181, 184, 186, 255};
    inline constexpr ImVec4 ConcreteGrey_ImGui{0.710f, 0.722f, 0.729f, 1.0f};

    inline constexpr ImVec4 ButtonHover_ImGui{0.650f, 0.520f, 0.130f, 1.0f};
    inline constexpr ImVec4 HeaderHover_ImGui{0.600f, 0.690f, 0.760f, 1.0f};

    inline constexpr ImVec4 ButtonActive_ImGui{0.850f, 0.700f, 0.180f, 1.0f};
    inline constexpr ImVec4 HeaderActive_ImGui{0.480f, 0.560f, 0.620f, 1.0f};

    inline constexpr ImVec4 Disabled_ImGui{0.350f, 0.360f, 0.370f, 0.600f};
    inline constexpr ImVec4 Selection_ImGui{0.788f, 0.635f, 0.153f, 0.350f};
    inline constexpr ImVec4 Border_ImGui{0.478f, 0.180f, 0.180f, 0.800f};
    inline constexpr ImVec4 Separator_ImGui{0.400f, 0.420f, 0.440f, 1.0f};

    inline void ApplyImGuiTheme()
    {
        ImGuiStyle& style = ImGui::GetStyle();

        // ---- Shape & Spacing ----
        style.WindowRounding = 2.0f;
        style.FrameRounding = 2.0f;
        style.ScrollbarRounding = 2.0f;
        style.GrabRounding = 1.0f;

        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;

        style.WindowPadding = ImVec2(12, 10);
        style.FramePadding = ImVec2(8, 4);
        style.ItemSpacing = ImVec2(8, 6);

        // ---- Colors ----

        ImVec4* c = style.Colors;

        // Backgrounds
        c[ImGuiCol_WindowBg] = Gunmetal_ImGui;
        c[ImGuiCol_ChildBg] = DarkSteel_ImGui;
        c[ImGuiCol_PopupBg] = DarkSteel_ImGui;

        // Borders
        c[ImGuiCol_Border] = Border_ImGui;
        c[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

        // Text
        c[ImGuiCol_Text] = ConcreteGrey_ImGui;
        c[ImGuiCol_TextDisabled] = Disabled_ImGui;

        // Headers (TreeNodes, Collapsing)
        c[ImGuiCol_Header] = ColdSteelBlue_ImGui;
        c[ImGuiCol_HeaderHovered] = ImVec4(HazardYellow_ImGui.x, HazardYellow_ImGui.y, HazardYellow_ImGui.z, 0.65f);
        c[ImGuiCol_HeaderActive] = ImVec4(HazardYellow_ImGui.x, HazardYellow_ImGui.y, HazardYellow_ImGui.z, 0.85f);

        // Buttons
        c[ImGuiCol_Button] = Brass_ImGui;
        c[ImGuiCol_ButtonHovered] = ImVec4(HazardYellow_ImGui.x, HazardYellow_ImGui.y, HazardYellow_ImGui.z, 0.70f);
        c[ImGuiCol_ButtonActive] = ImVec4(HazardYellow_ImGui.x, HazardYellow_ImGui.y, HazardYellow_ImGui.z, 0.95f);

        // Frame BG (inputs, sliders)
        c[ImGuiCol_FrameBg] = DarkSteel_ImGui;
        c[ImGuiCol_FrameBgHovered] = ImVec4(HazardYellow_ImGui.x, HazardYellow_ImGui.y, HazardYellow_ImGui.z, 0.45f);
        c[ImGuiCol_FrameBgActive] = ImVec4(HazardYellow_ImGui.x, HazardYellow_ImGui.y, HazardYellow_ImGui.z, 0.75f);

        // Tabs
        c[ImGuiCol_Tab] = DarkSteel_ImGui;
        c[ImGuiCol_TabHovered] = ImVec4(HazardYellow_ImGui.x, HazardYellow_ImGui.y, HazardYellow_ImGui.z, 0.65f);
        c[ImGuiCol_TabActive] = ColdSteelBlue_ImGui;
        c[ImGuiCol_TabUnfocused] = DarkSteel_ImGui;
        c[ImGuiCol_TabUnfocusedActive] = ColdSteelBlue_ImGui;

        // Title bar
        c[ImGuiCol_TitleBg] = DarkSteel_ImGui;
        c[ImGuiCol_TitleBgActive] = ColdSteelBlue_ImGui;
        c[ImGuiCol_TitleBgCollapsed] = DarkSteel_ImGui;

        // Scrollbar
        c[ImGuiCol_ScrollbarBg] = DarkSteel_ImGui;
        c[ImGuiCol_ScrollbarGrab] = Border_ImGui;
        c[ImGuiCol_ScrollbarGrabHovered] =
            ImVec4(HazardYellow_ImGui.x, HazardYellow_ImGui.y, HazardYellow_ImGui.z, 0.60f);
        c[ImGuiCol_ScrollbarGrabActive] =
            ImVec4(HazardYellow_ImGui.x, HazardYellow_ImGui.y, HazardYellow_ImGui.z, 0.90f);

        // Separators
        c[ImGuiCol_Separator] = Separator_ImGui;
        c[ImGuiCol_SeparatorHovered] = ImVec4(HazardYellow_ImGui.x, HazardYellow_ImGui.y, HazardYellow_ImGui.z, 0.70f);
        c[ImGuiCol_SeparatorActive] = ImVec4(HazardYellow_ImGui.x, HazardYellow_ImGui.y, HazardYellow_ImGui.z, 0.90f);
    }
} // namespace Colors
