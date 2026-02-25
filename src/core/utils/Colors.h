#pragma once

#include <SFML/Graphics.hpp>
#include <imgui.h>

namespace Colors
{
    // Gunmetal
    inline constexpr sf::Color Gunmetal_SFML { 42, 46, 51, 255 };
    inline constexpr ImVec4 Gunmetal_ImGui { 0.165f, 0.180f, 0.200f, 1.0f };

    // Dark Steel
    inline constexpr sf::Color DarkSteel_SFML { 34, 38, 41, 255 };
    inline constexpr ImVec4 DarkSteel_ImGui { 0.133f, 0.149f, 0.161f, 1.0f };

    // Cold Steel Blue
    inline constexpr sf::Color ColdSteelBlue_SFML { 143, 166, 184, 255 };
    inline constexpr ImVec4 ColdSteelBlue_ImGui { 0.561f, 0.651f, 0.722f, 1.0f };

    // Hazard Yellow
    inline constexpr sf::Color HazardYellow_SFML { 201, 162, 39, 255 };
    inline constexpr ImVec4 HazardYellow_ImGui { 0.788f, 0.635f, 0.153f, 1.0f };

    // Brass
    inline constexpr sf::Color Brass_SFML { 184, 153, 31, 255 };
    inline constexpr ImVec4 Brass_ImGui { 0.722f, 0.600f, 0.122f, 0.8f };

    // Rust Red
    inline constexpr sf::Color RustRed_SFML { 122, 46, 46, 255 };
    inline constexpr ImVec4 RustRed_ImGui { 0.478f, 0.180f, 0.180f, 1.0f };

    // Oxidized Green
    inline constexpr sf::Color OxidizedGreen_SFML { 122, 159, 122, 255 };
    inline constexpr ImVec4 OxidizedGreen_ImGui { 0.478f, 0.624f, 0.478f, 1.0f };

    // Industrial Orange
    inline constexpr sf::Color IndustrialOrange_SFML { 184, 106, 46, 255 };
    inline constexpr ImVec4 IndustrialOrange_ImGui { 0.722f, 0.416f, 0.180f, 1.0f };

    // Near Black Background Variants
    inline constexpr ImVec4 UltraDark_ImGui { 0.055f, 0.060f, 0.065f, 1.00f };
    inline constexpr ImVec4 DeepPanel_ImGui { 0.075f, 0.080f, 0.085f, 1.00f };
    inline constexpr ImVec4 Surface_ImGui { 0.095f, 0.100f, 0.105f, 1.00f };

    // Steel
    inline constexpr ImVec4 NeutralHover_ImGui { 0.18f, 0.19f, 0.20f, 1.00f };
    inline constexpr ImVec4 NeutralActive_ImGui { 0.22f, 0.23f, 0.24f, 1.00f };

    // Concrete Grey
    inline constexpr sf::Color ConcreteGrey_SFML { 181, 184, 186, 255 };
    inline constexpr ImVec4 ConcreteGrey_ImGui { 0.710f, 0.722f, 0.729f, 1.0f };

    // Blue
    inline constexpr sf::Color LinkBlue_SFML { 70, 130, 200, 255 };
    inline constexpr ImVec4 LinkBlue_ImGui { 0.274f, 0.509f, 0.784f, 1.0f };

    inline constexpr ImVec4 ButtonHover_ImGui { 0.650f, 0.520f, 0.130f, 1.0f };
    inline constexpr ImVec4 HeaderHover_ImGui { 0.600f, 0.690f, 0.760f, 1.0f };

    inline constexpr ImVec4 ButtonActive_ImGui { 0.850f, 0.700f, 0.180f, 1.0f };
    inline constexpr ImVec4 HeaderActive_ImGui { 0.480f, 0.560f, 0.620f, 1.0f };

    inline constexpr ImVec4 Disabled_ImGui { 0.350f, 0.360f, 0.370f, 0.600f };
    inline constexpr ImVec4 Selection_ImGui { 0.788f, 0.635f, 0.153f, 0.350f };
    inline constexpr ImVec4 Border_ImGui { 0.478f, 0.180f, 0.180f, 0.800f };
    inline constexpr ImVec4 Separator_ImGui { 0.400f, 0.420f, 0.440f, 1.0f };
}
