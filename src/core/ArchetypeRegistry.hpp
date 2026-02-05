#include"Archetype.hpp"
#pragma once

using ArchetypeComponentSignature = std::bitset<MaxComponents>;

class ArchetypeRegistry
{
private:

	std::vector<BaseArchetype*> archetypePtrs;
	std::unordered_map<ArchetypeComponentSignature, BaseArchetype*> archetypeSignatureMap;

	template<typename... Components>
	ArchetypeComponentSignature MakeASignature()
	{
		ArchetypeComponentSignature signature;

		(signature.set(GetComponentID<Components>()), ...);

		return signature;
	}

	template<typename ArchetypeT>
	ArchetypeID RegisterArchetype(ArchetypeT* newArchetypePtr)
	{
		ArchetypeID id = static_cast<ArchetypeID>(archetypePtrs.size());
		newArchetypePtr->id = id;
		archetypePtrs.push_back(newArchetypePtr);
		return id;
	}

public:


	BaseArchetype* GetArchetype(ArchetypeID id)
	{
		return archetypePtrs[id];
	}

	template<typename... Components>
	BaseArchetype* FindOrCreateArchetype()
	{	
		// ArchetypeComponentSignature signature = MakeASignature<Components...>();
		// std::cout << signature;
		
		// if (archetypeSignatureMap.contains(signature))
		// {
		// 	return archetypeSignatureMap[signature];
		// }
		// else
		// {
		// 	//Create Archetype
		// 	//Archetype<64,Components...> newArchetype = new Archetype();

		// 	//Register Archetype

		// 	//return
		// }

		return nullptr;
	}

	
};