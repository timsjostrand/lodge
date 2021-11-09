#ifndef _LODGE_TRS_COMPONENT_H
#define _LODGE_TRS_COMPONENT_H

#include "math4.h"

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_scene;
typedef struct lodge_scene* lodge_scene_t;

struct lodge_entity;
typedef struct lodge_entity* lodge_entity_t;

struct lodge_type;
typedef struct lodge_type* lodge_type_t;

enum lodge_transform_space
{
	LODGE_TRANSFORM_SPACE_LOCAL,
	LODGE_TRANSFORM_SPACE_WORLD,
	LODGE_TRANSFORM_SPACE_MAX,
};

enum lodge_rotation_index
{
	LODGE_ROTATION_INDEX_PITCH = 0,	// X
	LODGE_ROTATION_INDEX_YAW,		// Y
	LODGE_ROTATION_INDEX_ROLL,		// Z
};

struct lodge_transform_component
{
	enum lodge_transform_space	space;
	vec3						translation;
	vec3						rotation;
	vec3						scale;
};

extern lodge_component_type_t	LODGE_COMPONENT_TYPE_TRANSFORM;
extern lodge_type_t				LODGE_TYPE_ENUM_TRANSFORM_SPACE;

lodge_component_type_t			lodge_transform_component_type_register();

//
// Helpers to recursively walk entity hierarchy and get/set world space positions/rotations/scale.
//

vec3							lodge_get_position(lodge_scene_t scene, lodge_entity_t entity);
vec3							lodge_get_rotation(lodge_scene_t scene, lodge_entity_t entity);
vec3							lodge_get_direction_vector(lodge_scene_t scene, lodge_entity_t entity);
vec3							lodge_get_scale(lodge_scene_t scene, lodge_entity_t entity);
mat4							lodge_get_transform(lodge_scene_t scene, lodge_entity_t entity);

void							lodge_set_position(lodge_scene_t scene, lodge_entity_t entity, vec3 position);
void							lodge_set_rotation(lodge_scene_t scene, lodge_entity_t entity, vec3 rotation);
void							lodge_set_scale(lodge_scene_t scene, lodge_entity_t entity, vec3 scale);

mat4							lodge_rotation_to_matrix(const vec3 rotation);

#endif