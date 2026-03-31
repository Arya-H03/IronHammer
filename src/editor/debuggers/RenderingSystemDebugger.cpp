#include "editor/debuggers/RenderingSystemDebugger.h"

#include "engine/Engine.h"

RenderingSystemDebugger::RenderingSystemDebugger() { m_displayMode = EngineMode::Both; }

void RenderingSystemDebugger::DrawTab(DebugTabContext& context)
{

    if (ImGui::BeginTabItem("Rendering")) {

        if (!m_renderingSystemPtr) { return; }

        bool canDrawSprites = m_renderingSystemPtr->GetCanDrawSprites();
        if (ImGui::Checkbox("Draw Sprites", &canDrawSprites)) m_renderingSystemPtr->SetCanDrawSprites(canDrawSprites);

        ImGui::Separator();

        bool canDrawText = m_renderingSystemPtr->GetCanDrawText();
        if (ImGui::Checkbox("Draw Texts", &canDrawText)) m_renderingSystemPtr->SetCanDrawTest(canDrawText);

        ImGui::Separator();

        bool canDrawShapes = m_renderingSystemPtr->GetCanDrawShapes();
        if (ImGui::Checkbox("Draw Shapes", &canDrawShapes)) m_renderingSystemPtr->SetCanDrawShapes(canDrawShapes);

        ImGui::Separator();

        bool canDrawColliders = m_renderingSystemPtr->GetCanDrawColliders();
        if (ImGui::Checkbox("Draw Colliders", &canDrawColliders))
            m_renderingSystemPtr->SetCanDrawColliders(canDrawColliders);

        ImGui::EndTabItem();
    }
}

void RenderingSystemDebugger::RegisterRenderingSystem(RenderingSystem* renderingSystem)
{
    m_renderingSystemPtr = renderingSystem;
}
void RenderingSystemDebugger::UnRegisterRenderingSystem() { m_renderingSystemPtr = nullptr; }
