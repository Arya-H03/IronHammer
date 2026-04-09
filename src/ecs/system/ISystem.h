#pragma once

#include "ecs/World.h"
class ISetupSystem
{
  public:
    virtual void SetupSystem(World* worldPtr) = 0;
    virtual ~ISetupSystem() = default;
};
