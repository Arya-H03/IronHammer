#pragma once
#include "ECSCommon.h"


struct BaseArchetype
{
    ArchetypeID archetypeId = InvalidArchetypeID;
    virtual EntityArchetypeLocation AddEntity(uint32_t entityId, void* componentTuple) = 0;
    virtual void RemoveEntity(uint32_t entityId, uint32_t chunkIndex, uint32_t indexInChunk) = 0;
    virtual void DrawDebugGUI( )const = 0;
    virtual ~BaseArchetype() = default;
};
