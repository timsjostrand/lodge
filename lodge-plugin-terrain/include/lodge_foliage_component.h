#ifndef _LODGE_FOLIAGE_COMPONENT_H
#define _LODGE_FOLIAGE_COMPONENT_H

#include "math4.h"

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

extern lodge_component_type_t LODGE_COMPONENT_TYPE_FOLIAGE;

lodge_component_type_t lodge_foliage_component_type_register();

/// FOLIAGE SYSTEM

#define LODGE_FOLIAGE_LODS_MAX 4

struct lodge_scene;
typedef struct lodge_scene* lodge_scene_t;

struct lodge_scene_render_pass_params;
struct lodge_terrain_system;
struct lodge_terrain_component;

struct lodge_entity;
typedef struct lodge_entity* lodge_entity_t;

struct lodge_shader;
typedef struct lodge_shader* lodge_shader_t;

struct lodge_sampler;
typedef struct lodge_sampler* lodge_sampler_t;

struct lodge_static_mesh;

void lodge_foliage_system_render(struct lodge_foliage_component *foliage, const struct lodge_scene_render_pass_params *pass_params, lodge_shader_t shader, lodge_sampler_t heightfield_sampler, struct lodge_terrain_component *terrain, lodge_entity_t owner, vec3 terrain_scale);

struct lodge_foliage_lods_desc
{
	size_t							count;
	const struct lodge_static_mesh	*elements[LODGE_FOLIAGE_LODS_MAX];
};

void lodge_foliage_set_lods_desc(struct lodge_foliage_component *foliage, struct lodge_foliage_lods_desc *lods_desc);

#endif