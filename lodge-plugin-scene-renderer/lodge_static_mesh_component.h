#ifndef _LODGE_STATIC_MESH_COMPONENT_H
#define _LODGE_STATIC_MESH_COMPONENT_H

#include "math4.h"
#include "lodge_drawable.h"
#include "lodge_static_mesh.h"

// LODGE_STATIC_MESH_REF.h

struct lodge_type_t;
typedef struct lodge_type* lodge_type_t;

// TODO(TS): separate header for _ref
struct lodge_static_mesh_ref
{
	char name[256];
};

extern lodge_type_t LODGE_TYPE_STATIC_MESH_REF;


///////////////

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_shader;
typedef struct lodge_shader* lodge_shader_t;

struct lodge_texture;
typedef struct lodge_texture* lodge_texture_t;

struct fbx_asset;

struct lodge_static_mesh_component
{
	const struct fbx_asset			*asset;
	lodge_shader_t					shader;
	lodge_shader_t					shader_entity_id;
	lodge_texture_t					texture;

	struct lodge_static_mesh_ref	fbx_ref;
	struct lodge_static_mesh_ref	shader_ref;
	struct lodge_static_mesh_ref	texture_ref;
};

extern lodge_component_type_t	LODGE_COMPONENT_TYPE_STATIC_MESH;

lodge_component_type_t			lodge_static_mesh_component_type_register();

#endif