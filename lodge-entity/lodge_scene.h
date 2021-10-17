#ifndef _LODGE_SCENE_H
#define _LODGE_SCENE_H

#include "strview.h"

#include <stdbool.h>

struct lodge_scene;
typedef struct lodge_scene* lodge_scene_t;

struct lodge_system_type;
typedef struct lodge_system_type* lodge_system_type_t;

struct lodge_system;
typedef struct lodge_system* lodge_system_t;

struct lodge_entity_type;
typedef struct lodge_entity_type* lodge_entity_type_t;

struct lodge_component;
typedef struct lodge_component* lodge_component_t;

struct lodge_entity;
typedef struct lodge_entity* lodge_entity_t;

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_component_it
{
	void					*value;
	lodge_component_type_t	type;
	size_t					index;
};

struct lodge_system_it
{
	void					*value;
	lodge_system_type_t		type;
};

struct lodge_entity_desc
{
	// 0 to auto-generate a unique ID.
	size_t					id;
	lodge_entity_t			parent;
	char					name[256];
};

void						lodge_scene_new_inplace(lodge_scene_t scene);
void						lodge_scene_free_inplace(lodge_scene_t scene);
size_t						lodge_scene_sizeof();

lodge_entity_t				lodge_scene_add_entity_from_type(lodge_scene_t scene, lodge_entity_type_t entity_type);
lodge_entity_t				lodge_scene_add_entity_from_desc(lodge_scene_t scene, const struct lodge_entity_desc *entity_desc, const struct lodge_entity_components_desc *components_desc);
lodge_component_t			lodge_scene_add_entity_component(lodge_scene_t scene, lodge_entity_t entity_handle, lodge_component_type_t component_type);
bool						lodge_scene_remove_entity(lodge_scene_t scene, lodge_entity_t entity_type);

lodge_system_t				lodge_scene_add_system(lodge_scene_t scene, lodge_system_type_t system_type);
void*						lodge_scene_get_system(lodge_scene_t scene, lodge_system_type_t system_type);

void*						lodge_scene_get_entity_component(lodge_scene_t scene, lodge_entity_t entity, lodge_component_type_t type);
strview_t					lodge_scene_get_entity_name(lodge_scene_t scene, lodge_entity_t entity);
lodge_entity_t				lodge_scene_get_entity_parent(lodge_scene_t scene, lodge_entity_t entity);

lodge_entity_t				lodge_scene_get_component_entity(lodge_scene_t scene, lodge_component_type_t type, const void *component);

void						lodge_scene_set_entity_parent(lodge_scene_t scene, lodge_entity_t entity, lodge_entity_t parent);

lodge_entity_t				lodge_scene_entities_begin(lodge_scene_t scene);
lodge_entity_t				lodge_scene_entities_next(lodge_scene_t scene, lodge_entity_t entity_handle);

void*						lodge_scene_components_begin(lodge_scene_t scene, lodge_component_type_t type);
void*						lodge_scene_components_next(lodge_scene_t scene, lodge_component_type_t type, void *component);

struct lodge_component_it	lodge_scene_entity_components_begin(lodge_scene_t scene, lodge_entity_t entity);
struct lodge_component_it	lodge_scene_entity_components_next(lodge_scene_t scene, lodge_entity_t entity, struct lodge_component_it previous);

struct lodge_system_it		lodge_scene_systems_begin(lodge_scene_t scene);
struct lodge_system_it		lodge_scene_systems_next(lodge_scene_t scene, struct lodge_system_it current_system);

void						lodge_scene_update(lodge_scene_t scene, float dt);
float						lodge_scene_get_time(lodge_scene_t scene);

void*						lodge_system_get_plugin(lodge_system_t system);

#if 1
struct mvp;
void						lodge_scene_render(lodge_scene_t scene, struct lodge_system_render_params *render_params);
#endif

#define						lodge_scene_entities_foreach(SCENE, IT) \
	for(lodge_entity_t IT = lodge_scene_entities_begin(SCENE); IT; IT = lodge_scene_entities_next(SCENE, IT))

#define						lodge_scene_components_foreach(SCENE, C_TYPE, IT, COMPONENT_TYPE) \
	for(C_TYPE IT = lodge_scene_components_begin(SCENE, COMPONENT_TYPE); IT; IT = lodge_scene_components_next(SCENE, COMPONENT_TYPE, IT))

#define						lodge_scene_entity_components_foreach(SCENE, ENTITY, IT) \
	for(struct lodge_component_it IT = lodge_scene_entity_components_begin(SCENE, ENTITY); IT.value; IT = lodge_scene_entity_components_next(SCENE, ENTITY, IT))

#define						lodge_scene_systems_foreach(SCENE, IT) \
	for(struct lodge_system_it IT = lodge_scene_systems_begin(SCENE); IT.value; IT = lodge_scene_systems_next(SCENE, IT))

#endif