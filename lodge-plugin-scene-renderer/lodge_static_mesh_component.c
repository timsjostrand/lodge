#include "lodge_static_mesh_component.h"

#include "math4.h"
#include "strbuf.h"

#include "lodge_system_type.h"
#include "lodge_component_type.h"
#include "lodge_transform_component.h"
#include "lodge_scene.h"
#include "lodge_assets.h"

#include "lodge_gui_property_widget_factory.h"

#include "lodge_json.h"
#include "lodge_serialize_json.h"

#include <stdio.h>

lodge_type_t LODGE_TYPE_STATIC_MESH_REF = NULL;
lodge_component_type_t LODGE_COMPONENT_TYPE_STATIC_MESH = NULL;

static void lodge_static_mesh_component_new_inplace(struct lodge_static_mesh_component *component)
{
	*component = (struct lodge_static_mesh_component) { 0 };
}

static void lodge_static_mesh_component_free_inplace(struct lodge_static_mesh_component *component)
{
#if 0
	// TODO(TS): these should live in resource managers instead
	if(component->asset) {
		lodge_res_release( component->asset );
	}
#endif
}

static void on_modified_static_mesh_ref(struct lodge_property *property, const struct lodge_static_mesh_component *component)
{
	((struct lodge_static_mesh_component*)component)->asset = NULL;
}

static void on_modified_shader_ref(struct lodge_property *property, const struct lodge_static_mesh_component *component)
{
	((struct lodge_static_mesh_component*)component)->shader = NULL;
}

static void on_modified_texture_ref(struct lodge_property *property, const struct lodge_static_mesh_component *component)
{
	((struct lodge_static_mesh_component*)component)->texture = NULL;
}

// FIXME(TS): separate header?
#include "lodge_gui.h"
#include <string.h>

static bool make_property_widget_static_mesh_ref(struct nk_context *ctx, struct lodge_property *property, void *object)
{
	const struct lodge_static_mesh_ref *value = lodge_property_get(property, object);
	ASSERT(value);

	struct lodge_static_mesh_ref new_value = *value;
	nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, new_value.name, sizeof_member(struct lodge_static_mesh_ref, name), NULL);

	if(!strview_equals(strview_wrap(new_value.name), strview_wrap(value->name))) {
		lodge_property_set(property, object, &new_value);
		return true;
	}

	return false;
}

static bool lodge_static_mesh_ref_from_json(const lodge_json_t node, struct lodge_static_mesh_ref *dst)
{
	strview_t tmp;
	if(lodge_json_get_string(node, &tmp)) {
		strbuf_set(strbuf_wrap(dst->name), tmp);
		return true;
	}
	return false;
}

static lodge_json_t lodge_static_mesh_ref_to_json(const struct lodge_static_mesh_ref *src)
{
	return lodge_json_new_string(strview_wrap(src->name));
}

lodge_component_type_t lodge_static_mesh_component_type_register()
{
	ASSERT(!LODGE_TYPE_STATIC_MESH_REF);
	ASSERT(!LODGE_COMPONENT_TYPE_STATIC_MESH);

	if(!LODGE_TYPE_STATIC_MESH_REF) {
		LODGE_TYPE_STATIC_MESH_REF = lodge_type_register(strview_static("static_mesh_ref"), sizeof(struct lodge_static_mesh_ref));
	
		lodge_type_set_make_property_widget_func(LODGE_TYPE_STATIC_MESH_REF, &make_property_widget_static_mesh_ref);
		lodge_json_register_type_funcs(LODGE_TYPE_STATIC_MESH_REF, &lodge_static_mesh_ref_to_json, lodge_static_mesh_ref_from_json);
	}

	if(!LODGE_COMPONENT_TYPE_STATIC_MESH) {
		LODGE_COMPONENT_TYPE_STATIC_MESH = lodge_component_type_register((struct lodge_component_desc) {
			.name = strview_static("static_mesh"),
			.description = strview_static("Adds a static mesh (without animation) that can be rendered in scene_render_system."),
			.new_inplace = lodge_static_mesh_component_new_inplace,
			.free_inplace = lodge_static_mesh_component_free_inplace,
			.size = sizeof(struct lodge_static_mesh_component),
			.properties = {
				.count = 3,
				.elements = {
					{
						.name = strview_static("static_mesh"),
						.type = LODGE_TYPE_STATIC_MESH_REF,
						.offset = offsetof(struct lodge_static_mesh_component, fbx_ref),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = on_modified_static_mesh_ref,
					},
					{
						.name = strview_static("shader"),
						.type = LODGE_TYPE_STATIC_MESH_REF,
						.offset = offsetof(struct lodge_static_mesh_component, shader_ref),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = on_modified_shader_ref,
					},
					{
						.name = strview_static("material"),
						.type = LODGE_TYPE_STATIC_MESH_REF,
						.offset = offsetof(struct lodge_static_mesh_component, texture_ref),
						.flags = LODGE_PROPERTY_FLAG_NONE,
						.on_modified = on_modified_texture_ref,
					},
				}
			}
		});
	}

	return LODGE_COMPONENT_TYPE_STATIC_MESH;
}