#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdint.h>
#include <utility>
#include "ecs/common/ECSCommon.h"

struct BaseTemplatedArchetype
{
    ArchetypeId archetypeId = InvalidArchetypeID;
    size_t totalCount = 0;
    virtual EntityStorageLocation AddEntity(Entity entity, void* componentTuple) = 0;
    virtual std::pair<Entity, EntityStorageLocation> RemoveEntity(
        Entity entity, uint32_t chunkIndex, uint32_t indexInChunk) = 0;
    virtual void DrawArchetypeGUI(const std::function<void(Entity)>& deleteEntityCallBack) const = 0;
    virtual ~BaseTemplatedArchetype() = default;
};
