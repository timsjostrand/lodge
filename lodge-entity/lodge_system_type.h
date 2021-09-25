#ifndef _LODGE_SYSTEM_H
#define _LODGE_SYSTEM_H

#include "lodge_properties.h"
#include "lodge_rect.h"

struct lodge_system_type;
typedef struct lodge_system_type* lodge_system_type_t;

struct lodge_component;
typedef struct lodge_component* lodge_component_t;

struct lodge_component_type;
typedef struct lodge_component_type* lodge_component_type_t;

struct lodge_scene;
typedef struct lodge_scene* lodge_scene_t;

struct lodge_window;
typedef struct lodge_window* lodge_window_t;

struct lodge_framebuffer;
typedef struct lodge_framebuffer* lodge_framebuffer_t;

struct lodge_system_render_params
{
	struct lodge_recti						window_rect;
	lodge_framebuffer_t						framebuffer;
};

typedef void								(*lodge_system_new_inplace_func_t)(void *system, lodge_scene_t scene);
typedef void								(*lodge_system_free_inplace_func_t)(void *system, lodge_scene_t scene);
typedef void								(*lodge_system_update_func_t)(void *system, lodge_system_type_t type, lodge_scene_t scene, float dt);
typedef void								(*lodge_system_render_func_t)(void *system, lodge_scene_t scene, struct lodge_system_render_params *render_params);
typedef lodge_component_t					(*lodge_system_add_component_func_t)(void *system);
typedef void								(*lodge_system_remove_component_func_t)(void *system, lodge_component_t component);
typedef const void*							(*lodge_system_get_component_func_t)(void *system, lodge_component_t component);

struct lodge_system_plugins_desc
{
	size_t									count;
	strview_t								elements[8];
};

struct lodge_system_type_desc
{
	strview_t								name;
	size_t									size;

	lodge_system_new_inplace_func_t			new_inplace;
	lodge_system_free_inplace_func_t		free_inplace;
	
	lodge_system_update_func_t				update;

	// FIXME(TS):
	// - probably want to add render hooks instead
	// - support different render passes (transparent, deferred, forward)
	lodge_system_render_func_t				render;

	//
	// Systems are expected to be registered via a plugin, that can hold
	// dependencies to other systems.
	//
	void*									plugin;

	struct lodge_properties					properties;
};

lodge_system_type_t							lodge_system_type_register(struct lodge_system_type_desc desc);

void										lodge_system_type_new_inplace(lodge_system_type_t system_type, void *dst, lodge_scene_t scene);
void										lodge_system_type_free_inplace(lodge_system_type_t system_type, void *dst, lodge_scene_t scene);
size_t										lodge_system_type_sizeof(lodge_system_type_t system_type);

void										lodge_system_type_update(lodge_system_type_t system_type, void *system, lodge_scene_t scene, float dt);
void										lodge_system_type_render(lodge_system_type_t system_type, void *system, lodge_scene_t scene, struct lodge_system_render_params *render_params);

strview_t									lodge_system_type_get_name(lodge_system_type_t system_type);
struct lodge_properties*					lodge_system_type_get_properties(lodge_system_type_t system_type);
void*										lodge_system_type_get_plugin(lodge_system_type_t system_type);

lodge_system_type_t							lodge_system_type_find(strview_t name);

lodge_system_type_t							lodge_system_type_it_begin();
lodge_system_type_t							lodge_system_type_it_next(lodge_system_type_t it);

#endif