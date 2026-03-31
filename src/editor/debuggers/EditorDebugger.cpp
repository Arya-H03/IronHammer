#include "editor/debuggers/EditorDebugger.h"

#include "editor/Editor.h"
#include "engine/Engine.h"

EditorDebugger::EditorDebugger() { m_displayMode = EngineMode::Both; }

void EditorDebugger::RegisterEditor(Editor* editor) { m_editorPtr = editor; }
void EditorDebugger::UnRegisterEditor() { m_editorPtr = nullptr; }

void EditorDebugger::DrawTab(DebugTabContext& context)
{
    if (!m_editorPtr) { return; }

    EditorContext& editorContext = m_editorPtr->m_editorContext;

    if (ImGui::BeginTabItem("Editor")) {

        float tabWidth = ImGui::GetContentRegionAvail().x;

        ImGui::SeparatorText("Grid");
        ImGui::RadioButton("16", &editorContext.editorGrid.GetCellSize(), 16);
        ImGui::SameLine();
        ImGui::RadioButton("32", &editorContext.editorGrid.GetCellSize(), 32);
        ImGui::SameLine();
        ImGui::RadioButton("48", &editorContext.editorGrid.GetCellSize(), 48);
        ImGui::SameLine();
        ImGui::RadioButton("64", &editorContext.editorGrid.GetCellSize(), 64);
        ImGui::SameLine();

        ImGui::SetCursorPosX(tabWidth - 2 * (ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x +
                                             ImGui::CalcTextSize("Snap to Grid").x));
        bool& canSnapToGrid = editorContext.editorGrid.GetCanSnapToGrid();
        ImGui::Checkbox("Snap to Grid", &canSnapToGrid);

        ImGui::SameLine();

        ImGui::SetCursorPosX(tabWidth - (ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x +
                                         ImGui::CalcTextSize("Show Grid").x));
        bool& canShowGrid = editorContext.editorGrid.GetCanShowGrid();
        ImGui::Checkbox("Show Grid", &canShowGrid);

        ImGui::Spacing();
        ImGui::Spacing();

        sf::Color& color  = editorContext.editorGrid.GetCellColor();
        float      col[4] = {color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f};
        ImGui::PushItemWidth(225);
        if (ImGui::ColorEdit4("Color", col)) {
            color = sf::Color(static_cast<uint8_t>(col[0] * 255), static_cast<uint8_t>(col[1] * 255),
                              static_cast<uint8_t>(col[2] * 255), static_cast<uint8_t>(col[3] * 255));
        }
        ImGui::PopItemWidth();

        ImGui::EndTabItem();
    }
}
