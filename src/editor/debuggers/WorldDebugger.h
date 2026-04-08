#pragma once

#include "editor/debuggers/IDebugTab.h"

#include <gelf.h>
#include <imgui.h>

class World;
class Archetype;
class EntityInspector;

class WorldDebugger : public IDebugTab
{
  private:
    World* m_worldPtr = nullptr;

    void DrawIndividualArchetypeGUI(World* worldPtr, Archetype* archetypePtr, EntityInspector* entityInspector) const;

  public:
    WorldDebugger();

    void DrawWorldGuiTab(World* worldPtr, EntityInspector* entityInspector) const;
    void DrawTab(DebugTabContext& context) override;

    void RegisterWorld(World* worldPtr) { m_worldPtr = worldPtr; }
    void UnRegisterWorld() { m_worldPtr = nullptr; }
};
