#pragma once
#include <imgui.h>
#include "EntityManager.hpp"

class GUISystem
{
  private:
    const EntityManager& m_entityManager;

  public:
  GUISystem(EntityManager& entityManager):m_entityManager(entityManager){}
    void HandleGUISystem();
};
