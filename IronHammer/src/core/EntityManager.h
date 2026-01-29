#include <bitset>
#include <vector>
#include <cassert>
#include "SlabAllocator.hpp"

struct Entity
{
	size_t id;
	//size_t generation;
};

class EntityManager
{
public:
	//Create Entity
	Entity* CreateEntity()
	{
		Entity* e = entityPool.Allocate();
		e->id = counter;
		++counter;

		return e;
	}
	//Delete Entity

	void DeleteEntity(Entity* entity)
	{
		entityPool.DeAllocate(entity);
	}

	//Add Component To Entity

	//Remove Component from Entity
private:
	SlabAllocator<Entity, 64> entityPool;
	size_t counter = 0;

};


