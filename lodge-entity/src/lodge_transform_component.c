#include "lodge_transform_component.h"

#include "math4.h"

#include "lodge_component_type.h"
#include "lodge_platform.h"
#include "lodge_scene.h"

lodge_component_type_t LODGE_COMPONENT_TYPE_TRANSFORM = NULL;
lodge_type_t LODGE_TYPE_ENUM_TRANSFORM_SPACE = NULL;

static void lodge_transform_component_new_inplace(struct lodge_transform_component *component, void *userdata)
{
	*component = (struct lodge_transform_component) {
		.space = LODGE_TRANSFORM_SPACE_LOCAL,
		.translation = vec3_zero(),
		.rotation = vec3_zero(),
		.scale = vec3_ones(),
	};
}

vec3 lodge_get_scale(lodge_scene_t scene, lodge_entity_t entity)
{
	struct lodge_transform_component* transform = lodge_scene_get_entity_component(scene, entity, LODGE_COMPONENT_TYPE_TRANSFORM);
	if(!transform) {
		return vec3_ones();
	}

	switch(transform->space)
	{
	case LODGE_TRANSFORM_SPACE_LOCAL:
	{
		lodge_entity_t parent = lodge_scene_get_entity_parent(scene, entity);
		if(!parent) {
			return transform->scale;
		}
		return vec3_mult(lodge_get_scale(scene, parent), transform->scale);
	}
	case LODGE_TRANSFORM_SPACE_WORLD:
	default:
	{
		return transform->scale;
	}
	}
}

vec3 lodge_get_rotation(lodge_scene_t scene, lodge_entity_t entity)
{
	struct lodge_transform_component* transform = lodge_scene_get_entity_component(scene, entity, LODGE_COMPONENT_TYPE_TRANSFORM);
	if(!transform) {
		return vec3_zero();
	}

	switch(transform->space)
	{
	case LODGE_TRANSFORM_SPACE_LOCAL:
	{
		lodge_entity_t parent = lodge_scene_get_entity_parent(scene, entity);
		if(!parent) {
			return transform->rotation;
		}
		return vec3_add(lodge_get_rotation(scene, parent), transform->rotation);
	}
	case LODGE_TRANSFORM_SPACE_WORLD:
	default:
	{
		return transform->rotation;
	}
	}
}

vec3 lodge_get_direction_vector(lodge_scene_t scene, lodge_entity_t entity)
{
	return vec3_norm(lodge_get_rotation(scene, entity));
}

vec3 lodge_get_position(lodge_scene_t scene, lodge_entity_t entity)
{
	struct lodge_transform_component* transform = lodge_scene_get_entity_component(scene, entity, LODGE_COMPONENT_TYPE_TRANSFORM);
	if(!transform) {
		return vec3_zero();
	}

	switch(transform->space)
	{
	case LODGE_TRANSFORM_SPACE_LOCAL:
	{
		lodge_entity_t parent = lodge_scene_get_entity_parent(scene, entity);
		if(!parent) {
			return transform->translation;
		}
		const vec3 parent_pos = lodge_get_position(scene, parent);
		const vec3 parent_scale = lodge_get_scale(scene, parent);
		const vec3 parent_offset = vec3_mult(transform->translation, parent_scale);
		return vec3_add(parent_pos, parent_offset);
	}
	case LODGE_TRANSFORM_SPACE_WORLD:
	default:
	{
		return transform->translation;
	}
	}
}

mat4 lodge_rotation_to_matrix(const vec3 rotation)
{
	const mat4 pitch = mat4_rotation_x(rotation.v[LODGE_ROTATION_INDEX_PITCH]);
	const mat4 yaw = mat4_rotation_y(rotation.v[LODGE_ROTATION_INDEX_YAW]);
	const mat4 roll = mat4_rotation_z(rotation.v[LODGE_ROTATION_INDEX_ROLL]);
	return mat4_mult(mat4_mult(pitch, roll), yaw);
}

static mat4 lodge_transform_component_to_trs(struct lodge_transform_component *transform)
{
	const mat4 t = mat4_translation(xyz_of(transform->translation));
	const mat4 r = lodge_rotation_to_matrix(transform->rotation);
	const mat4 s = mat4_scaling(xyz_of(transform->scale));

#if 1
	return mat4_mult(t, mat4_mult(r, s));
#else
	return mat4_mult(s, mat4_mult(r, t));
#endif
}

mat4 lodge_get_transform(lodge_scene_t scene, lodge_entity_t entity)
{
	struct lodge_transform_component *transform = lodge_scene_get_entity_component(scene, entity, LODGE_COMPONENT_TYPE_TRANSFORM);
	if(!transform) {
		return mat4_identity();
	}

	const mat4 trs = lodge_transform_component_to_trs(transform);

	switch(transform->space)
	{
	case LODGE_TRANSFORM_SPACE_LOCAL:
	{
		lodge_entity_t parent = lodge_scene_get_entity_parent(scene, entity);
		if(!parent) {
			return trs;
		}
		const mat4 parent_transform = lodge_get_transform(scene, parent);
		return mat4_mult(parent_transform, trs);
	}
	case LODGE_TRANSFORM_SPACE_WORLD:
	default:
	{
		return trs;
	}
	}
}

void lodge_set_rotation(lodge_scene_t scene, lodge_entity_t entity, vec3 rotation)
{
	struct lodge_transform_component *transform = lodge_scene_get_entity_component(scene, entity, LODGE_COMPONENT_TYPE_TRANSFORM);
	ASSERT(transform);
	if(!transform) {
		return;
	}

	switch(transform->space)
	{
	case LODGE_TRANSFORM_SPACE_LOCAL:
	{
		ASSERT_NOT_IMPLEMENTED();
	}
	case LODGE_TRANSFORM_SPACE_WORLD:
	default:
	{
		transform->rotation = vec3_make(fmod(rotation.x, M_PI*2), fmod(rotation.y, M_PI*2), fmod(rotation.z, M_PI*2));
	}
	}
}

lodge_component_type_t lodge_transform_component_type_register()
{
	ASSERT(!LODGE_TYPE_ENUM_TRANSFORM_SPACE);
	if(!LODGE_TYPE_ENUM_TRANSFORM_SPACE) {
		LODGE_TYPE_ENUM_TRANSFORM_SPACE = lodge_type_register_enum(strview_static("transform_space"), (struct lodge_enum_desc) {
			.count = 2,
			.elements = {
				{
					.name = strview_static("Local"),
					.value = LODGE_TRANSFORM_SPACE_LOCAL,
				},
				{
					.name = strview_static("World"),
					.value = LODGE_TRANSFORM_SPACE_WORLD,
				},
			}
		});
	}

	ASSERT(!LODGE_COMPONENT_TYPE_TRANSFORM);

	if(!LODGE_COMPONENT_TYPE_TRANSFORM) {
		LODGE_COMPONENT_TYPE_TRANSFORM = lodge_component_type_register((struct lodge_component_desc) {
			.name = strview_static("transform"),
			.description = strview_static("Contains translation, rotation and scale components for an entity."),
			.new_inplace = &lodge_transform_component_new_inplace,
			.free_inplace = NULL,
			.size = sizeof(struct lodge_transform_component),
			.properties = {
				.count = 4,
				.elements = {
					{
						.name = strview_static("space"),
						.type = LODGE_TYPE_ENUM_TRANSFORM_SPACE,
						.offset = offsetof(struct lodge_transform_component, space),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
					{
						.name = strview_static("translation"),
						.type = LODGE_TYPE_VEC3,
						.offset = offsetof(struct lodge_transform_component, translation),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
					{
						.name = strview_static("rotation"),
						.type = LODGE_TYPE_VEC3, // FIXME(TS): LODGE_TYPE_VEC3_ROTATION: switch degrees, mod TAU etc
						.offset = offsetof(struct lodge_transform_component, rotation),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
					{
						.name = strview_static("scale"),
						.type = LODGE_TYPE_VEC3,
						.offset = offsetof(struct lodge_transform_component, scale),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = NULL,
					},
				}
			}
		});
	}

	return LODGE_COMPONENT_TYPE_TRANSFORM;
}
