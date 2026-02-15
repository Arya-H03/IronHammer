#pragma once

#include "ecs/archetype/Archetype.h"
#include "ecs/common/ECSCommon.h"
#include "ecs/component/ComponentRegistry.hpp"
#include <vector>
class BaseSystem
{
  protected:
    ComponentSignatureMask m_signatureMask;
    std::vector<Archetype*> m_matchingArchetypes;

    template<typename... Components>
    void MakeSignatureMask()
    {
        (m_signatureMask.set(ComponentRegistry::GetComponentID<Components>()),...);
    }

  public:
    void TryAddMatchingArchetype(Archetype* archetype)
    {
        if ((archetype->GetComponentSignature() & m_signatureMask) == m_signatureMask)
        {
            m_matchingArchetypes.push_back(archetype);
        }
    }

    virtual ~BaseSystem() = default;
};
