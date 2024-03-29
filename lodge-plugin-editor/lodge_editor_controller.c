#include "lodge_editor_controller.h"

#include "lodge_type.h"
#include "lodge_component_type.h"
#include "lodge_properties.h"
#include "lodge_input.h"
#include "lodge_keys.h"

#include "lodge_system_type.h"
#include "lodge_scene.h"
#include "lodge_transform_component.h"
#include "lodge_camera_component.h"

#include <stdio.h>

lodge_component_type_t LODGE_COMPONENT_TYPE_EDITOR_CONTROLLER = NULL;

static void lodge_editor_controller_component_new_inplace(struct lodge_editor_controller_component *component, void *userdata)
{
	*component = (struct lodge_editor_controller_component) {
		.target_pos = vec3_zero(),
		.last_mouse_pos = vec2_make(-FLT_MAX, -FLT_MAX),
		.move_speed = 0.025f,
		.move_lerp_speed = 0.01f,
		.mouse_sensitivity = 0.0025f,
	};
}

static void lodge_editor_controller_component_move(struct lodge_editor_controller_component *component, const mat4 *view, vec3 delta_pos)
{
	const vec3 forward = mat4_view_get_forward_vector(view);
	const vec3 strafe = mat4_view_get_strafe_vector(view);
	const vec3 up = mat4_view_get_up_vector(view);

	vec3 delta_pos_scaled = vec3_mult_scalar(delta_pos, component->move_speed);
	vec3 delta = vec3_mult_scalar(forward, delta_pos_scaled.z);
	delta = vec3_add(delta, vec3_mult_scalar(strafe, delta_pos_scaled.x));
	delta = vec3_add(delta, vec3_mult_scalar(up, delta_pos_scaled.y));

	component->target_pos = vec3_add(component->target_pos, delta);
}

static void lodge_editor_controller_component_snap_to_target_pos(lodge_scene_t scene, struct lodge_editor_controller_component *component)
{
	lodge_entity_t owner = lodge_scene_get_component_entity(scene, LODGE_COMPONENT_TYPE_EDITOR_CONTROLLER, component);

	struct lodge_transform_component *owner_transform = lodge_scene_get_entity_component(scene, owner, LODGE_COMPONENT_TYPE_TRANSFORM);
	ASSERT(owner_transform);
		
	if(owner_transform) {
		owner_transform->translation = component->target_pos;
	}
}

lodge_component_type_t lodge_editor_controller_component_type_register()
{
	ASSERT(!LODGE_COMPONENT_TYPE_EDITOR_CONTROLLER);

	if(!LODGE_COMPONENT_TYPE_EDITOR_CONTROLLER) {
		LODGE_COMPONENT_TYPE_EDITOR_CONTROLLER = lodge_component_type_register((struct lodge_component_desc) {
			.name = strview_static("editor_controller"),
			.description = strview_static("Controller used for flying around the world."),
			.new_inplace = &lodge_editor_controller_component_new_inplace,
			.free_inplace = NULL,
			.size = sizeof(struct lodge_editor_controller_component),
			.properties = {
				.count = 4,
				.elements = {
					{
						.name = strview_static("target_pos"),
						.type = LODGE_TYPE_VEC3,
						.offset = offsetof(struct lodge_editor_controller_component, target_pos),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
					{
						.name = strview_static("move_speed"),
						.type = LODGE_TYPE_F32,
						.offset = offsetof(struct lodge_editor_controller_component, move_speed),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
					{
						.name = strview_static("move_lerp_speed"),
						.type = LODGE_TYPE_F32,
						.offset = offsetof(struct lodge_editor_controller_component, move_lerp_speed),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
					{
						.name = strview_static("mouse_sensitivity"),
						.type = LODGE_TYPE_F32,
						.offset = offsetof(struct lodge_editor_controller_component, mouse_sensitivity),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
				}
			}
		});
	}

	return LODGE_COMPONENT_TYPE_EDITOR_CONTROLLER;
}

struct lodge_editor_controller_system
{
	vec2 last_mouse_pos;
};

static void lodge_editor_controller_system_new_inplace(struct lodge_editor_controller_system *system, lodge_scene_t scene, struct lodge_editor *plugin)
{
	system->last_mouse_pos = vec2_make(-FLT_MAX, -FLT_MAX);
}

static void lodge_editor_controller_system_update(struct lodge_editor_controller_system *system, lodge_system_type_t type, lodge_scene_t scene, float dt, struct lodge_editor *plugin)
{
	lodge_scene_components_foreach(scene, struct lodge_editor_controller_component*, component, LODGE_COMPONENT_TYPE_EDITOR_CONTROLLER) {
		lodge_entity_t owner = lodge_scene_get_component_entity(scene, LODGE_COMPONENT_TYPE_EDITOR_CONTROLLER, component);
		ASSERT(owner);

		struct lodge_input *input = component->input;

		if(input) {
			const vec2 mouse_pos = lodge_input_get_mouse_position(input);
			const bool last_mouse_pos_valid = (component->last_mouse_pos.x != -FLT_MAX);
			const bool input_mouse_pos_valid = lodge_input_is_mouse_button_position_valid(input);
			const bool right_mouse_down = lodge_input_is_mouse_button_down(input, LODGE_MOUSE_BUTTON_RIGHT);
			const bool left_mouse_down = lodge_input_is_mouse_button_down(input, LODGE_MOUSE_BUTTON_LEFT);

			if(input_mouse_pos_valid && last_mouse_pos_valid) {
				if(right_mouse_down && left_mouse_down) {
					//
					// Mouse move
					//
					const vec2 delta = vec2_sub(mouse_pos, component->last_mouse_pos);
					const vec3 delta_scaled = {
						.x = delta.x,
						.y = delta.y,
						.z = 0.0f,
					};

					const mat4 view = lodge_camera_calc_view_matrix(scene, owner);
					lodge_editor_controller_component_move(component, &view, delta_scaled);
					lodge_editor_controller_component_snap_to_target_pos(scene, component);
				} else if(right_mouse_down) {
					//
					// Mouse look
					//
					const vec3 owner_rot = lodge_get_rotation(scene, owner);
					//const vec2 delta = vec2_sub(component->last_mouse_pos, mouse_pos);
					const vec2 delta = vec2_sub(mouse_pos, component->last_mouse_pos);
					const vec3 delta_scaled = {
						.v = {
							[LODGE_ROTATION_INDEX_YAW] = 0.0f,
							[LODGE_ROTATION_INDEX_ROLL] = delta.x * component->mouse_sensitivity,
							[LODGE_ROTATION_INDEX_PITCH] = delta.y * component->mouse_sensitivity,
						}
					};
					
					lodge_set_rotation(scene, owner, vec3_sub(owner_rot, delta_scaled));
				}
			}
			component->last_mouse_pos = mouse_pos;

			//
			// TODO(TS): Scroll to modify move speed
			//

			//
			// WASD movement
			//
			{
				float speed = 1.0f;

				if(lodge_input_is_key_down(input, LODGE_KEY_LEFT_SHIFT)) {
					speed *= 0.1f;
				} else if(lodge_input_is_key_down(input, LODGE_KEY_LEFT_CONTROL)) {
					speed *= 10.0f;
				}

				vec3 camera_delta = vec3_zero();
				if(lodge_input_is_key_down(input, LODGE_KEY_W)) {
					camera_delta.z -= speed * dt;
				}
				if(lodge_input_is_key_down(input, LODGE_KEY_S)) {
					camera_delta.z += speed * dt;
				}
				if(lodge_input_is_key_down(input, LODGE_KEY_A)) {
					camera_delta.x -= speed * dt;
				}
				if(lodge_input_is_key_down(input, LODGE_KEY_D)) {
					camera_delta.x += speed * dt;
				}

				const mat4 view = lodge_camera_calc_view_matrix(scene, owner);
				lodge_editor_controller_component_move(component, &view, camera_delta);
			}
		}

		if(owner) {
			struct lodge_transform_component *owner_transform = lodge_scene_get_entity_component(scene, owner, LODGE_COMPONENT_TYPE_TRANSFORM);
			ASSERT(owner_transform);

			if(owner_transform) {
				//
				// NOTE(TS): not implemented for local space yet; it is trivial to add
				//
				ASSERT(owner_transform->space == LODGE_TRANSFORM_SPACE_WORLD);

				//
				// Lerp towards `target_pos`
				//
				owner_transform->translation = vec3_lerp(owner_transform->translation, component->target_pos, component->move_lerp_speed*dt);
			}
		}
	}
}

lodge_system_type_t lodge_editor_controller_system_type_register(struct lodge_editor *plugin)
{
	return lodge_system_type_register((struct lodge_system_type_desc) {
		.name = strview_static("editor_controller_system"),
		.size = sizeof(struct lodge_editor_controller_system),
		.new_inplace = &lodge_editor_controller_system_new_inplace,
		.free_inplace = NULL,
		.update = &lodge_editor_controller_system_update,
		.userdata = plugin,
		.properties = {
			.count = 0,
			.elements = { 0 }
		}
	});
}