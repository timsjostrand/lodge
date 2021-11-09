#include "lodge_plugin_debug_draw.h"

#include "lodge.h"
#include "lodge_debug_draw.h"
#include "lodge_system_type.h"
#include "lodge_component_type.h"
#include "lodge_transform_component.h"
#include "lodge_scene.h"
#include "lodge_debug_draw.h"
#include "lodge_shader.h"
#include "lodge_plugins.h"
#include "lodge_assets2.h"

#include "lodge_plugin_scene_renderer.h"

#include "math4.h"
#include "geometry.h"
#include "gruvbox.h"
#include "coordinate_systems.h"

#include <stdio.h>

lodge_component_type_t LODGE_COMPONENT_TYPE_DEBUG_SPHERE = NULL;

struct lodge_debug_sphere
{
	struct sphere			sphere;
	vec4					color;
};

struct lodge_debug_draw_system
{
	lodge_shader_t			shaders[3];
	bool					draw_spheres;
	bool					draw_view_gizmo;
	struct lodge_debug_draw	*debug_draw;
};

struct lodge_plugin_debug_draw
{
	struct lodge_assets2	*shaders;
	lodge_system_type_t		system_type;
	lodge_component_type_t	sphere_component_type;
};

static void lodge_debug_sphere_component_new_inplace(struct lodge_debug_sphere *component, void *userdata)
{
	*component = (struct lodge_debug_sphere) {
		.sphere = {
			.pos = vec3_zero(),
			.r = 10.0f,
		},
		.color = GRUVBOX_BRIGHT_ORANGE,
	};
}

static void lodge_debug_draw_system_update(struct lodge_debug_draw_system *system, lodge_system_type_t type, lodge_scene_t scene, float dt, struct lodge_plugin_debug_draw *plugin)
{
	//
	// Load shaders?
	//
	{
		if(!system->shaders[0]) {
			lodge_asset_t debug_draw_shader_asset = lodge_assets2_register(plugin->shaders, strview("debug_draw"));
			system->shaders[0] = lodge_assets2_get(plugin->shaders, debug_draw_shader_asset);
		}
		if(!system->shaders[1]) {
			lodge_asset_t debug_draw_sphere_shader_asset = lodge_assets2_register(plugin->shaders, strview("debug_draw_sphere"));
			system->shaders[1] = lodge_assets2_get(plugin->shaders, debug_draw_sphere_shader_asset);
		}
		if(!system->shaders[2]) {
			lodge_asset_t debug_draw_texture_shader_asset = lodge_assets2_register(plugin->shaders, strview("debug_draw_texture"));
			system->shaders[2] = lodge_assets2_get(plugin->shaders, debug_draw_texture_shader_asset);
		}
	}

	lodge_debug_draw_update(system->debug_draw, dt);

	if(system->draw_spheres) {
#if 0
		lodge_scene_entities_foreach(scene, entity) {
			struct sphere *debug_sphere = lodge_scene_get_entity_component(scene, entity, LODGE_COMPONENT_TYPE_DEBUG_SPHERE);
			if(!debug_sphere) {
				continue;
			}

			const vec3 scale = lodge_get_scale(scene, entity);
			struct sphere tmp = {
				.pos = lodge_get_position(scene, entity),
				// TODO(TS): should support scale vector instead
				.r = debug_sphere->r * max(scale.x, max(scale.y, scale.z)),
			};

			lodge_debug_draw_sphere(system->debug_draw, tmp, GRUVBOX_BRIGHT_ORANGE, 0.0f);
		}
#else
		lodge_scene_components_foreach(scene, struct lodge_debug_sphere*, debug_sphere, LODGE_COMPONENT_TYPE_DEBUG_SPHERE) {
			lodge_entity_t entity = lodge_scene_get_component_entity(scene, LODGE_COMPONENT_TYPE_DEBUG_SPHERE, debug_sphere);
			ASSERT_OR(entity) { continue; }

			const vec3 entity_pos = lodge_get_position(scene, entity);

			lodge_debug_draw_sphere(system->debug_draw, (struct sphere) {
				.pos = vec3_add(entity_pos, debug_sphere->sphere.pos),
				.r = debug_sphere->sphere.r
			}, debug_sphere->color, 0.0f);
		}
#endif
	}
}

static vec3 make_view_gizmo_coord(vec2 p, const struct lodge_scene_render_pass_params *pass_params)
{
	return vec2_screen_01_to_world_space(p, &pass_params->camera.inv_view_projection);
}

static void lodge_debug_draw_system_render(lodge_scene_t scene, const struct lodge_scene_render_pass_params *pass_params, struct lodge_debug_draw_system *system)
{
	if(system->draw_view_gizmo) {
		struct line x_axis = {
			.p0 = make_view_gizmo_coord(vec2_make(0.1f, 0.1f), pass_params), //vec2_screen_01_to_world_space(vec2_make(0.1f, 0.1f), &m),
			.p1 = make_view_gizmo_coord(vec2_make(0.2f, 0.1f), pass_params),
		};

		struct line y_axis = {
			.p0 = make_view_gizmo_coord(vec2_make(0.1f, 0.1f), pass_params),
			.p1 = make_view_gizmo_coord(vec2_make(0.1f, 0.2f), pass_params),
		};

		lodge_debug_draw_line(system->debug_draw, x_axis, GRUVBOX_BRIGHT_RED, 0.0f);
		lodge_debug_draw_line(system->debug_draw, y_axis, GRUVBOX_BRIGHT_GREEN, 0.0f);
	}

	struct mvp mvp = {
		.model = mat4_identity(),
		.view = pass_params->camera.view,
		.projection = pass_params->camera.projection
	};
	lodge_debug_draw_render(system->debug_draw, system->shaders, mvp);
}

static void lodge_debug_draw_system_new_inplace(struct lodge_debug_draw_system *system, lodge_scene_t scene, struct lodge_plugin_debug_draw *plugin)
{
	system->debug_draw = (struct lodge_debug_draw *) malloc(lodge_debug_draw_sizeof());
	lodge_debug_draw_new_inplace(system->debug_draw);

	system->draw_spheres = true;
	system->draw_view_gizmo = false;

	lodge_scene_add_render_pass_func(scene, LODGE_SCENE_RENDER_SYSTEM_PASS_FORWARD_TRANSPARENT, &lodge_debug_draw_system_render, system);
}

static lodge_component_type_t lodge_debug_sphere_component_type_register()
{
	ASSERT(!LODGE_COMPONENT_TYPE_DEBUG_SPHERE);

	if(!LODGE_COMPONENT_TYPE_DEBUG_SPHERE) {
		LODGE_COMPONENT_TYPE_DEBUG_SPHERE = lodge_component_type_register((struct lodge_component_desc) {
			.name = strview_static("debug_sphere"),
			.description = strview_static("Draws a debug sphere."),
			.new_inplace = lodge_debug_sphere_component_new_inplace,
			.free_inplace = NULL,
			.size = sizeof(struct lodge_debug_sphere),
			.properties = {
				.count = 3,
				.elements = {
					{
						.name = strview_static("pos"),
						.type = LODGE_TYPE_VEC3,
						.offset = offsetof(struct lodge_debug_sphere, sphere) + offsetof(struct sphere, pos),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
					{
						.name = strview_static("r"),
						.type = LODGE_TYPE_F32,
						.offset = offsetof(struct lodge_debug_sphere, sphere) + offsetof(struct sphere, r),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
					{
						.name = strview_static("color"),
						.type = LODGE_TYPE_VEC4,
						.hints = {
							.enable = true,
							.vec4 = {
								.color = true,
							},
						},
						.offset = offsetof(struct lodge_debug_sphere, color),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
				}
			}
		});
	}

	return LODGE_COMPONENT_TYPE_DEBUG_SPHERE;
}

static lodge_system_type_t lodge_debug_draw_system_type_register(struct lodge_plugin_debug_draw *plugin)
{
	return lodge_system_type_register((struct lodge_system_type_desc) {
		.name = strview_static("debug_draw"),
		.size = sizeof(struct lodge_debug_draw_system),
		.new_inplace = lodge_debug_draw_system_new_inplace,
		.free_inplace = NULL,
		.update = lodge_debug_draw_system_update,
		.render = NULL,
		.userdata = plugin,
		.properties = {
			.count = 1,
			.elements = {
				{
					.name = strview_static("draw_spheres"),
					.type = LODGE_TYPE_BOOL,
					.offset = offsetof(struct lodge_debug_draw_system, draw_spheres),
					.flags = LODGE_PROPERTY_FLAG_NONE,
					.on_modified = NULL,
				},
			}
		}
	});
}

lodge_system_type_t lodge_plugin_debug_draw_get_system_type(struct lodge_plugin_debug_draw *plugin)
{
	ASSERT(plugin);
	return plugin ? plugin->system_type : NULL;
}

lodge_component_type_t lodge_plugin_debug_draw_get_sphere_component_type(struct lodge_plugin_debug_draw *plugin)
{
	ASSERT(plugin);
	return plugin ? plugin->sphere_component_type : NULL;
}

struct lodge_ret lodge_debug_draw_plugin_new_inplace(struct lodge_plugin_debug_draw *plugin, struct lodge_plugins *plugins, const struct lodge_argv *args)
{
	plugin->shaders = lodge_plugins_depend(plugins, plugin, strview_static("shaders"));
	ASSERT(plugin->shaders);

	plugin->system_type = lodge_debug_draw_system_type_register(plugin);
	ASSERT(plugin->system_type);

	plugin->sphere_component_type = lodge_debug_sphere_component_type_register();
	ASSERT(plugin->system_type);

	return lodge_success();
}

static void lodge_debug_draw_plugin_free_inplace(struct lodge_plugin_debug_draw *plugin)
{
	//ASSERT_NOT_IMPLEMENTED();
}

struct lodge_debug_draw* lodge_debug_draw_system_get_batcher(struct lodge_debug_draw_system *system)
{
	return system ? system->debug_draw : NULL;
}

LODGE_PLUGIN_IMPL(lodge_plugin_debug_draw)
{
	return (struct lodge_plugin_desc) {
		.version = LODGE_PLUGIN_VERSION,
		.size = sizeof(struct lodge_plugin_debug_draw),
		.name = strview_static("debug_draw"),
		.new_inplace = lodge_debug_draw_plugin_new_inplace,
		.free_inplace = lodge_debug_draw_plugin_free_inplace,
		.update = NULL,
		.render = NULL,
	};
}
