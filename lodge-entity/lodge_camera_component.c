#include "lodge_camera_component.h"

#include "lodge_component_type.h"

#include "lodge_scene.h"
#include "lodge_transform_component.h"

lodge_type_t LODGE_TYPE_PERSPECTIVE = NULL;
lodge_component_type_t LODGE_COMPONENT_TYPE_CAMERA = NULL;

void lodge_camera_component_new_inplace(struct lodge_camera_component *component, void *userdata)
{
	component->perspective.fov_degrees = 60.0f;
	component->perspective.z_near = 0.1f;
	component->perspective.z_far = 10000.0f;
	component->use_default = false;
}

static mat4 lodge_camera_calc_view_matrix_impl(vec3 camera_pos, vec3 camera_rot)
{
	const vec3 camera_pos_neg = vec3_negate(camera_pos);
	return mat4_mult(lodge_rotation_to_matrix(camera_rot), mat4_translation(xyz_of(camera_pos_neg)));
}

mat4 lodge_camera_calc_view_matrix(lodge_scene_t scene, lodge_entity_t camera)
{
	const vec3 camera_pos = lodge_get_position(scene, camera);
	const vec3 camera_rot = lodge_get_rotation(scene, camera);
	return lodge_camera_calc_view_matrix_impl(camera_pos, camera_rot);
}

struct lodge_camera_params lodge_camera_params_make(lodge_scene_t scene, lodge_entity_t camera, float aspect_ratio)
{
	struct lodge_camera_component *component = lodge_scene_get_entity_component(scene, camera, LODGE_COMPONENT_TYPE_CAMERA);
	ASSERT(component);

	const vec3 camera_pos = lodge_get_position(scene, camera);
	const vec3 camera_rot = lodge_get_rotation(scene, camera);
	const mat4 view = lodge_camera_calc_view_matrix_impl(camera_pos, camera_rot);
	const mat4 projection = lodge_perspective_calc_projection(&component->perspective, aspect_ratio);
	const mat4 view_projection = mat4_mult(projection, view);

	return (struct lodge_camera_params) {
		.pos = camera_pos,
		.view = view,
		.projection = projection,
		.view_projection = view_projection,
		.inv_view = mat4_inverse(view, NULL),
		.inv_projection = mat4_inverse(projection, NULL),
		.inv_view_projection = mat4_inverse(view_projection, NULL),
		.fov_y = radians(component->perspective.fov_degrees),
	};
}

lodge_component_type_t lodge_camera_component_type_register()
{
	ASSERT(!LODGE_TYPE_PERSPECTIVE);

	if(!LODGE_TYPE_PERSPECTIVE) {
		LODGE_TYPE_PERSPECTIVE = lodge_type_register_property_object(
			strview_static("perspective"),
			sizeof(struct lodge_perspective),
			&(struct lodge_properties) {
				.count = 3,
				.elements = {
					{
						.name = strview_static("fov"),
						.type = LODGE_TYPE_F32,
						.offset = offsetof(struct lodge_perspective, fov_degrees),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
					{
						.name = strview_static("z_near"),
						.type = LODGE_TYPE_F32,
						.offset = offsetof(struct lodge_perspective, z_near),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
					{
						.name = strview_static("z_far"),
						.type = LODGE_TYPE_F32,
						.offset = offsetof(struct lodge_perspective, z_far),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
				}
			}
		);
	}

	ASSERT(!LODGE_COMPONENT_TYPE_CAMERA);

	if(!LODGE_COMPONENT_TYPE_CAMERA) {
		LODGE_COMPONENT_TYPE_CAMERA = lodge_component_type_register((struct lodge_component_desc) {
			.name = strview_static("camera"),
			.description = strview_static("A camera that can be used for rendering the scene."),
			.new_inplace = lodge_camera_component_new_inplace,
			.free_inplace = NULL,
			.size = sizeof(struct lodge_camera_component),
			.properties = {
				.count = 2,
				.elements = {
					{
						.name = strview_static("perspective"),
						.type = LODGE_TYPE_PERSPECTIVE,
						.offset = offsetof(struct lodge_camera_component, perspective),
					},
					{
						.name = strview_static("use_default"),
						.type = LODGE_TYPE_BOOL,
						.offset = offsetof(struct lodge_camera_component, use_default),
					},
				}
			}
		});
	}

	return LODGE_COMPONENT_TYPE_CAMERA;
}
