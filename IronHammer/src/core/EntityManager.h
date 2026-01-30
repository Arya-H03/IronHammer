#include <bitset>
#include <vector>
#include <cassert>
#include "SlabAllocator.hpp"

struct Entity
{
	uint32_t id;
	uint32_t generation;
};

class EntityManager
{
	struct EntitySlot
	{
		uint32_t generation = 0;
		bool isOccupied = false;
		uint32_t nextFreeEntityIndex = UINT32_MAX;
	};

private:

	static constexpr size_t initialEntitySize = 1024;
	std::vector<EntitySlot> entitySlots;
	uint32_t freeListHeadIndex = UINT32_MAX;

	bool IsEntityValid(Entity entity)const
	{
		if (entity.id >= entitySlots.size()) return false;
		return entitySlots[entity.id].isOccupied && entitySlots[entity.id].generation == entity.generation;
	}

public:

	EntityManager()
	{
		entitySlots.reserve(initialEntitySize);
	}

	Entity CreateEntity()
	{
		//No free slots
		if (freeListHeadIndex == UINT32_MAX)
		{
			entitySlots.push_back({ 1,true,UINT32_MAX });
			uint32_t id = (unsigned int)entitySlots.size() - 1;
			return Entity{id, entitySlots[id].generation };
		}
		//Reuse a slot
		else
		{
			uint32_t newEntityIndex = freeListHeadIndex;
			EntitySlot& slot = entitySlots[newEntityIndex];
			freeListHeadIndex = slot.nextFreeEntityIndex;
			slot.isOccupied = true;
			++slot.generation;
			return Entity{newEntityIndex,slot.generation };
		}
	}

	void DeleteEntity(Entity entity)
	{
		assert(IsEntityValid(entity) && "Tried to delete invalid Entity");

		EntitySlot& slot = entitySlots[entity.id];
		slot.isOccupied = false;
		slot.nextFreeEntityIndex = freeListHeadIndex;
		freeListHeadIndex = entity.id;
	}
};


