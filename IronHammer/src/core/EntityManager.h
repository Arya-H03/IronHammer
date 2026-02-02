#include <bitset>
#include <vector>
#include <cassert>
#include "ECSCommon.h"
#include "Archetype.hpp"
#pragma once

class EntityManager
{
	struct EntitySlot
	{
		uint32_t generation = 1;
		bool isOccupied = false;
		uint32_t nextFreeEntityIndex = UINT32_MAX;
	};

private:

	static constexpr size_t initialEntitySize = 1024;
	std::vector<EntitySlot> entitySlots;
	std::vector<EntityLocation> entityLocations;
	uint32_t freeListHeadIndex = UINT32_MAX;

	bool IsEntityValid(Entity entity)const;
	
public:

	EntityManager();
	Entity CreateEntity();
	void DeleteEntity(Entity entity);
};


