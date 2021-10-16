#ifndef _LODGE_CAMERA_COMPONENT_H
#define _LODGE_CAMERA_COMPONENT_H

#include "math4.h"
#include "lodge_perspective.h"
#include "lodge_plugin_scene_renderer.h" // FIXME(TS): only for camera_params

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_type;
typedef struct lodge_type* lodge_type_t;

struct lodge_scene;
typedef struct lodge_scene* lodge_scene_t;

struct lodge_entity;
typedef struct lodge_entity* lodge_entity_t;

enum lodge_projection
{
	LODGE_PROJECTION_PERSPECTIVE = 0,
	LODGE_PROJECTION_ORTHOGRAPHIC,
	LODGE_PROJECTION_MAX,
};

struct lodge_camera_component 
{
	enum lodge_projection		projection;
	struct lodge_perspective	perspective;
	//struct lodge_ortho		ortho;
	bool						use_default;
};

extern lodge_type_t				LODGE_TYPE_PERSPECTIVE;
extern lodge_component_type_t	LODGE_COMPONENT_TYPE_CAMERA;

lodge_component_type_t			lodge_camera_component_type_register();

mat4							lodge_camera_calc_view_matrix(lodge_scene_t scene, lodge_entity_t camera);
struct lodge_camera_params		lodge_camera_params_make(lodge_scene_t scene, lodge_entity_t camera, float aspect_ratio);

#endif