#include "GUISystem.h"

void GUISystem::HandleGUISystem()
{
    ImGui::Begin("ECS Debug");

    if (ImGui::BeginTabBar("ESC"))
    {
        if (ImGui::BeginTabItem("Archetypes"))
        {
            int archetypeTreeNodeId = 0;
            for (const auto& [signature, archPtr] : m_entityManager.archetypeRegistry.GetSignatureToPtrMap())
            {
                ImGui::PushID(archetypeTreeNodeId);
                if(ImGui::TreeNode(m_entityManager.archetypeRegistry.GetSignatureToNameMap().at(signature).c_str()))
                {
                    archPtr->DrawDebugGUI();
                    ImGui::TreePop();
                }
                ImGui::PopID();
                ++archetypeTreeNodeId;
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();

}
