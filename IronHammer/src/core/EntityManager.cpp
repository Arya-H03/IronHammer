#include"EntityManager.h"

bool EntityManager::IsEntityValid(Entity entity) const
{
	if (entity.id >= entitySlots.size()) return false;
	return entitySlots[entity.id].isOccupied && entitySlots[entity.id].generation == entity.generation;
}

EntityManager::EntityManager()
{
	entitySlots.reserve(initialEntitySize);
	entityLocations.reserve(initialEntitySize);
}

Entity EntityManager::CreateEntity()
{
	//No free slots
	if (freeListHeadIndex == UINT32_MAX)
	{
		entitySlots.push_back({ 1,true,UINT32_MAX });
		entityLocations.push_back({InvalidArchetypeID,UINT32_MAX,UINT32_MAX });

		uint32_t id = (unsigned int)entitySlots.size() - 1;
		return Entity{ id, entitySlots[id].generation };
	}
	//Reuse a slot
	else
	{
		uint32_t newEntityIndex = freeListHeadIndex;
		EntitySlot& slot = entitySlots[newEntityIndex];
		freeListHeadIndex = slot.nextFreeEntityIndex;
		slot.isOccupied = true;
		++slot.generation;

		entityLocations[newEntityIndex].archetypeId = InvalidArchetypeID;
		entityLocations[newEntityIndex].chunkIndex = UINT32_MAX;
		entityLocations[newEntityIndex].indexInChunk = UINT32_MAX;

		return Entity{ newEntityIndex,slot.generation };
	}
}

void EntityManager::DeleteEntity(Entity entity)
{
	assert(IsEntityValid(entity) && "Tried to delete invalid Entity");

	EntitySlot& slot = entitySlots[entity.id];
	slot.isOccupied = false;
	slot.nextFreeEntityIndex = freeListHeadIndex;

	entityLocations[entity.id].archetypeId = InvalidArchetypeID;
	entityLocations[entity.id].chunkIndex = UINT32_MAX;
	entityLocations[entity.id].indexInChunk = UINT32_MAX;

	freeListHeadIndex = entity.id;
}