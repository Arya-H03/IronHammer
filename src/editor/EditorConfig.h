#pragma once

#include <cstdint>
#include "core/utils/Vect2.hpp"

namespace EditorConfig
{
    // Base Configuration
    inline constexpr uint16_t MenuBarHeight = 25;
    inline constexpr uint16_t EdgePadding = 2;
    inline constexpr uint16_t PanelSpacing = 6;

    inline constexpr uint16_t InspectorWidth = 400;
    inline constexpr uint16_t DebugWidth = 450;
    inline constexpr uint16_t LogHeight = 200;

    struct Layout
    {
        // Window size (must be filled externally)
        uint16_t WindowWidth = 0;
        uint16_t WindowHeight = 0;

        // Menu Bar
        uint16_t Menu_X = 0;
        uint16_t Menu_Y = 0;
        uint16_t Menu_Width = 0;
        uint16_t Menu_Height = MenuBarHeight;

        // Debug Panel (Left)
        uint16_t Debug_X = 0;
        uint16_t Debug_Y = 0;
        uint16_t Debug_Width = DebugWidth;
        uint16_t Debug_Height = 0;

        // Inspector Panel (Right)
        uint16_t Inspector_X = 0;
        uint16_t Inspector_Y = 0;
        uint16_t Inspector_Width = InspectorWidth;
        uint16_t Inspector_Height = 0;

        // Logs Panel (Bottom Left)
        uint16_t Log_X = 0;
        uint16_t Log_Y = 0;
        uint16_t Log_Width = 0;
        uint16_t Log_Height = LogHeight;

        // Viewport (Center)
        uint16_t Viewport_X = 0;
        uint16_t Viewport_Y = 0;
        uint16_t Viewport_Width = 0;
        uint16_t Viewport_Height = 0;

        // Constructor
        Layout(Vect2<uint16_t> windowSize) : WindowWidth(windowSize.x), WindowHeight(windowSize.y)
        {
            // Menu
            Menu_Width = WindowWidth;

            // Vertical Calculations
            const uint16_t ContentTop = MenuBarHeight + PanelSpacing;

            const uint16_t ContentHeight = WindowHeight - MenuBarHeight - LogHeight - PanelSpacing;

            // Debug
            Debug_X = 0;
            Debug_Y = ContentTop;
            Debug_Height = ContentHeight;

            // Inspector
            Inspector_X = WindowWidth - InspectorWidth;
            Inspector_Y = ContentTop;
            Inspector_Height = WindowHeight - MenuBarHeight - PanelSpacing;

            // Logs
            Log_X = 0;
            Log_Y = WindowHeight - LogHeight;
            Log_Width = WindowWidth - InspectorWidth;


            // Viewport
            Viewport_X = DebugWidth;
            Viewport_Y = ContentTop;
            Viewport_Width = WindowWidth - DebugWidth - InspectorWidth;
            Viewport_Height = ContentHeight;
        }
    };

}
