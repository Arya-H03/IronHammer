#include"Archetype.hpp"
#pragma once

class ArchetypeRegistry
{
public:

	std::vector<BaseArchetype*> archetypePtrs;

	template<typename ArchetypeT>
	ArchetypeID Register(ArchetypeT* newArchetypePtr)
	{
		ArchetypeID id = static_cast<ArchetypeID>(archetypePtrs.size());
		newArchetypePtr->id = id;
		archetypePtrs.push_back(newArchetypePtr);
		return id;
	}

	BaseArchetype* GetArchetype(ArchetypeID id)
	{
		return archetypePtrs[id];
	}

};