#include "lodge_entity.h"

#include "membuf.h"
#include "lodge_entity_type.h"
#include "lodge_platform.h"

#include <stddef.h>

//
// entity_type
//
//		- Reusable entity archetypes
//		- Used when spawning entities, but only as a blueprint for the base state:
//		- Individual entities can modify this setup as needed
//		- Useful for iterating all entities of the "enemy" type
//
// entities
//
//		- Entity "manager"
//		- 1 per scene, where entities of all types go
//		- Owns entity memory
//		- Owns entity component memory
//
// entity (instance)
//
//		- List of components
//		- Variables? (use variants api?)
//		- Expose variables to graphs?
//		- Updatable
//		- Drawable
//
// entity_component
//
//		- Preallocated buffer of all entity components
//		- Components can be updated in "one go", eg all controller entites are iterated at the same time
//		- Components can query other components through their entity handle
//
// Graphs
//
//		- Most/All callbacks should be substituable for node graphs:
//		- BeginPlay/Tick/EndPlay <-- unique per entity type
//
//	Serialization of entities
//
//

#if 0
typedef size_t lodge_entity_id_t;
typedef size_t lodge_component_id_t;

struct lodge_entity
{
	lodge_entity_type_t		type;
	lodge_entity_id_t		id;
	lodge_component_id_t	components[32];
};

struct lodge_entities
{
	size_t					entity_size;
	size_t					entities_count;
	void					*entity_data;
};

void lodge_entities_add(struct lodge_entities *entities, lodge_entity_type_t type)
{
	ASSERT_NOT_IMPLEMENTED();
}

void lodge_entities_remove(struct lodge_entities *entities)
{
	ASSERT_NOT_IMPLEMENTED();
}
#endif