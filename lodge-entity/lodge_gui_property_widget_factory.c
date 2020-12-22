#include "lodge_gui_property_widget_factory.h"

#include "math4.h"
#include "color.h"
#include "lodge_gui.h"
#include "lodge_platform.h"
#include "lodge_type.h"
#include "lodge_properties.h"

#include "gruvbox.h"

size_t LODGE_TYPE_FUNC_INDEX_MAKE_PROPERTY_WIDGET = 0;

static bool make_property_widget_f32(struct nk_context *ctx, struct lodge_property *property, void *object)
{
	const float *value = lodge_property_get(property, object);
	ASSERT(value);

	//
	// FIXME(TS): recursive read-only style and input mode for widgets
	//
	if(LODGE_IS_FLAG_SET(property->flags, LODGE_PROPERTY_FLAG_READ_ONLY)) {
		nk_labelf(ctx, NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE, "%f", *value);
	} else {
		float min = -FLT_MAX;
		float max = FLT_MAX;
		float step = 0.1f;

		if(property->hints.enable) {
			min = property->hints.f32.min;
			max = property->hints.f32.max;
			step = property->hints.f32.step;
		}

		const float new_value = nk_propertyf(ctx, "#f32", min, *value, max, step, step);
		if(new_value != *value) {
			lodge_property_set(property, object, &new_value);
			return true;
		}
	}
	return false;
}

static bool make_property_widget_u32(struct nk_context *ctx, struct lodge_property *property, void *object)
{
	const uint32_t *value = lodge_property_get(property, object);
	ASSERT(value);

	if(LODGE_IS_FLAG_SET(property->flags, LODGE_PROPERTY_FLAG_READ_ONLY)) {
		nk_labelf(ctx, NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE, "%u", *value);
	} else {
		uint32_t min = 0;
		uint32_t max = INT32_MAX; // FIXME(TS): nk doesnt support UINT32_MAX
		uint32_t step = 1;

		if(property->hints.enable) {
			min = property->hints.u32.min;
			max = property->hints.u32.max;
			step = property->hints.u32.step;
		}

		int new_value = *value;
		nk_property_int(ctx, "#u32", 0, &new_value, INT32_MAX, 1, 1); // FIXME(TS): nk doesnt support UINT32_MAX
		if(new_value != *value) {
			lodge_property_set(property, object, &new_value);
			return true;
		}
	}
	
	return false;
}

static bool make_property_widget_vec3(struct nk_context *ctx, struct lodge_property *property, void *object)
{
	bool modified = false;
	const vec3 *value = lodge_property_get(property, object);
	ASSERT(value);

	nk_style_push_vec2(ctx, &ctx->style.window.group_padding, (struct nk_vec2){ 0, 0 });

	if(nk_group_begin(ctx, "vec3", NK_WINDOW_NO_SCROLLBAR)) {
		nk_layout_row_dynamic(ctx, 0, 3);

		nk_style_push_color(ctx, &ctx->style.property.label_normal, nk_color_from_vec4(GRUVBOX_BRIGHT_RED));
		nk_style_push_color(ctx, &ctx->style.property.label_hover, nk_color_from_vec4(GRUVBOX_BRIGHT_RED));
		nk_style_push_color(ctx, &ctx->style.property.label_active, nk_color_from_vec4(GRUVBOX_BRIGHT_RED));
		const float new_x = nk_propertyf(ctx, "#x", -FLT_MAX, value->x, FLT_MAX, 0.1f, 0.01f);
		nk_style_pop_color(ctx);
		nk_style_pop_color(ctx);
		nk_style_pop_color(ctx);

		nk_style_push_color(ctx, &ctx->style.property.label_normal, nk_color_from_vec4(GRUVBOX_BRIGHT_GREEN));
		nk_style_push_color(ctx, &ctx->style.property.label_hover, nk_color_from_vec4(GRUVBOX_BRIGHT_GREEN));
		nk_style_push_color(ctx, &ctx->style.property.label_active, nk_color_from_vec4(GRUVBOX_BRIGHT_GREEN));
		const float new_y = nk_propertyf(ctx, "#y", -FLT_MAX, value->y, FLT_MAX, 0.1f, 0.01f);
		nk_style_pop_color(ctx);
		nk_style_pop_color(ctx);
		nk_style_pop_color(ctx);

		nk_style_push_color(ctx, &ctx->style.property.label_normal, nk_color_from_vec4(GRUVBOX_BRIGHT_BLUE));
		nk_style_push_color(ctx, &ctx->style.property.label_hover, nk_color_from_vec4(GRUVBOX_BRIGHT_BLUE));
		nk_style_push_color(ctx, &ctx->style.property.label_active, nk_color_from_vec4(GRUVBOX_BRIGHT_BLUE));
		const float new_z = nk_propertyf(ctx, "#z", -FLT_MAX, value->z, FLT_MAX, 0.1f, 0.01f);
		nk_style_pop_color(ctx);
		nk_style_pop_color(ctx);
		nk_style_pop_color(ctx);

		if(new_x != value->x || new_y != value->y || new_z != value->z) {
			lodge_property_set(property, object, &(vec3) { new_x, new_y, new_z });
			modified = true;
		}

		nk_group_end(ctx);
	}

	nk_style_pop_vec2(ctx);
	return modified;
}

static void make_property_widget_vec4_color(struct nk_context *ctx, struct lodge_property *property, void *object, const vec4 value, bool *modified)
{
	if(nk_group_begin(ctx, "rgba", NK_WINDOW_NO_SCROLLBAR)) {
		nk_layout_row_dynamic(ctx, 0, 1);

		struct nk_colorf new_color = (struct nk_colorf) { rgba_of(value) };

		if(nk_combo_begin_color(ctx, nk_color_from_vec4(value), nk_vec2(nk_widget_width(ctx), 200))) {
			nk_layout_row_dynamic(ctx, 200, 1);
			new_color = nk_color_picker(ctx, new_color, NK_RGBA);
			nk_combo_end(ctx);
		}

		if(new_color.r != value.r || new_color.g != value.g || new_color.b != value.b || new_color.a != value.a) {
			lodge_property_set(property, object, &(vec4) { new_color.r, new_color.g, new_color.b, new_color.a });
			*modified = true;
		}

		nk_group_end(ctx);
	}
}

static void make_property_widget_vec4_components(struct nk_context *ctx, struct lodge_property *property, void *object, const vec4 *value, bool *modified)
{
	if(nk_group_begin(ctx, "vec4", NK_WINDOW_NO_SCROLLBAR)) {
		nk_layout_row_dynamic(ctx, 0, 4);

		nk_style_push_color(ctx, &ctx->style.property.label_normal, nk_color_from_vec4(GRUVBOX_BRIGHT_RED));
		nk_style_push_color(ctx, &ctx->style.property.label_hover, nk_color_from_vec4(GRUVBOX_BRIGHT_RED));
		nk_style_push_color(ctx, &ctx->style.property.label_active, nk_color_from_vec4(GRUVBOX_BRIGHT_RED));
		const float new_x = nk_propertyf(ctx, "#x", -FLT_MAX, value->x, FLT_MAX, 0.1f, 0.01f);
		nk_style_pop_color(ctx);
		nk_style_pop_color(ctx);
		nk_style_pop_color(ctx);

		nk_style_push_color(ctx, &ctx->style.property.label_normal, nk_color_from_vec4(GRUVBOX_BRIGHT_GREEN));
		nk_style_push_color(ctx, &ctx->style.property.label_hover, nk_color_from_vec4(GRUVBOX_BRIGHT_GREEN));
		nk_style_push_color(ctx, &ctx->style.property.label_active, nk_color_from_vec4(GRUVBOX_BRIGHT_GREEN));
		const float new_y = nk_propertyf(ctx, "#y", -FLT_MAX, value->y, FLT_MAX, 0.1f, 0.01f);
		nk_style_pop_color(ctx);
		nk_style_pop_color(ctx);
		nk_style_pop_color(ctx);

		nk_style_push_color(ctx, &ctx->style.property.label_normal, nk_color_from_vec4(GRUVBOX_BRIGHT_BLUE));
		nk_style_push_color(ctx, &ctx->style.property.label_hover, nk_color_from_vec4(GRUVBOX_BRIGHT_BLUE));
		nk_style_push_color(ctx, &ctx->style.property.label_active, nk_color_from_vec4(GRUVBOX_BRIGHT_BLUE));
		const float new_z = nk_propertyf(ctx, "#z", -FLT_MAX, value->z, FLT_MAX, 0.1f, 0.01f);
		nk_style_pop_color(ctx);
		nk_style_pop_color(ctx);
		nk_style_pop_color(ctx);

		nk_style_push_color(ctx, &ctx->style.property.label_normal, nk_color_from_vec4(GRUVBOX_BRIGHT_ORANGE));
		nk_style_push_color(ctx, &ctx->style.property.label_hover, nk_color_from_vec4(GRUVBOX_BRIGHT_ORANGE));
		nk_style_push_color(ctx, &ctx->style.property.label_active, nk_color_from_vec4(GRUVBOX_BRIGHT_ORANGE));
		const float new_w = nk_propertyf(ctx, "#w", -FLT_MAX, value->w, FLT_MAX, 0.1f, 0.01f);
		nk_style_pop_color(ctx);
		nk_style_pop_color(ctx);
		nk_style_pop_color(ctx);

		if(new_x != value->x || new_y != value->y || new_z != value->z || new_w != value->w) {
			lodge_property_set(property, object, &(vec4) { new_x, new_y, new_z, new_w });
			*modified = true;
		}

		nk_group_end(ctx);
	}
}

static bool make_property_widget_vec4(struct nk_context *ctx, struct lodge_property *property, void *object)
{
	bool modified = false;
	const vec4 *value = lodge_property_get(property, object);
	ASSERT(value);

	nk_style_push_vec2(ctx, &ctx->style.window.group_padding, (struct nk_vec2){ 0, 0 });

	const bool is_color = property->hints.enable && property->hints.vec4.color;

	//if(property->hints.enable) {
 	//	if(nk_checkbox_label(ctx, "Color", &property->hints.vec4.color)) {
	//		ASSERT_FAIL("Hello");
	//	}
	//}

	if(is_color) {
		make_property_widget_vec4_color(ctx, property, object, *value, &modified);
	} else {
		make_property_widget_vec4_components(ctx, property, object, value, &modified);
	}

	nk_style_pop_vec2(ctx);
	return modified;
}

static bool make_property_widget_vec2(struct nk_context *ctx, struct lodge_property *property, void *object)
{
	bool modified = false;
	const vec2 *value = lodge_property_get(property, object);
	ASSERT(value);

	nk_style_push_vec2(ctx, &ctx->style.window.group_padding, (struct nk_vec2){ 0, 0 });

	if(nk_group_begin(ctx, "vec2", NK_WINDOW_NO_SCROLLBAR)) {
		nk_layout_row_dynamic(ctx, 0, 2);

		nk_style_push_color(ctx, &ctx->style.property.label_normal, nk_color_from_vec4(GRUVBOX_BRIGHT_RED));
		nk_style_push_color(ctx, &ctx->style.property.label_hover, nk_color_from_vec4(GRUVBOX_BRIGHT_RED));
		nk_style_push_color(ctx, &ctx->style.property.label_active, nk_color_from_vec4(GRUVBOX_BRIGHT_RED));
		const float new_x = nk_propertyf(ctx, "#x", -FLT_MAX, value->x, FLT_MAX, 0.1f, 0.01f);
		nk_style_pop_color(ctx);
		nk_style_pop_color(ctx);
		nk_style_pop_color(ctx);

		nk_style_push_color(ctx, &ctx->style.property.label_normal, nk_color_from_vec4(GRUVBOX_BRIGHT_GREEN));
		nk_style_push_color(ctx, &ctx->style.property.label_hover, nk_color_from_vec4(GRUVBOX_BRIGHT_GREEN));
		nk_style_push_color(ctx, &ctx->style.property.label_active, nk_color_from_vec4(GRUVBOX_BRIGHT_GREEN));
		const float new_y = nk_propertyf(ctx, "#y", -FLT_MAX, value->y, FLT_MAX, 0.1f, 0.01f);
		nk_style_pop_color(ctx);
		nk_style_pop_color(ctx);
		nk_style_pop_color(ctx);

		if(new_x != value->x || new_y != value->y) {
			lodge_property_set(property, object, &(vec3) { new_x, new_y });
			modified = true;
		}

		nk_group_end(ctx);
	}

	nk_style_pop_vec2(ctx);
	return modified;
}

static bool make_property_widget_boolean(struct nk_context *ctx, struct lodge_property *property, void *object)
{
	const bool *value = lodge_property_get(property, object);
	ASSERT(value);

	bool new_value = *value;
	if(nk_checkbox_label(ctx, "", &new_value)) {
		lodge_property_set(property, object, &new_value);
		return true;
	}

	return false;
}

static bool make_property_widget_enum(struct nk_context *ctx, struct lodge_property *property, void *object)
{
	const int *value = lodge_property_get(property, object);
	ASSERT(value);

	const struct lodge_enum_desc *enum_desc = lodge_type_get_enum_desc(property->type);
	ASSERT(enum_desc);
	if(!enum_desc) {
		return false;
	}

	int new_value = *value;

	strview_t selected_name = enum_desc->elements[*value].name;
	if(nk_combo_begin_text(ctx, selected_name.s, selected_name.length, nk_vec2(nk_widget_width(ctx), 200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		for(size_t i = 0, count = enum_desc->count; i < count; i++) {
			strview_t name = enum_desc->elements[i].name;
			if(nk_combo_item_text(ctx, name.s, name.length, NK_TEXT_LEFT)) {
				new_value = i;
			}
		}
		nk_combo_end(ctx);
	}

	if(new_value != *value) {
		lodge_property_set(property, object, &new_value);
		return true;
	}

	return false;
}

static bool make_property_widget_properties(struct nk_context *ctx, struct lodge_property *property, void *object)
{
	const void *value = lodge_property_get(property, object);
	ASSERT(value);

	struct lodge_properties *subproperties = lodge_type_get_properties(property->type);
	ASSERT(subproperties);
	if(!subproperties) {
		return false;
	}

	// FIXME(TS): bogus ids
	int node_id = (int)object + (int)property->type;
	if(lodge_gui_property_widget_factory_make_tree(ctx, lodge_type_get_name(property->type), node_id, (char*)object + property->offset, subproperties)) {
		if(property->on_modified) {
			property->on_modified(property, object);
		}
		return true;
	}

	return false;
}

lodge_make_property_widget_func_t lodge_type_get_make_property_widget_func(lodge_type_t type)
{
	ASSERT(LODGE_TYPE_FUNC_INDEX_MAKE_PROPERTY_WIDGET);

	//
	// FIXME(TS): should attempt to get custom overload via `lodge_type_get_func` first; then default to enum/property widgets.
	//

	if(lodge_type_get_enum_desc(type)) {
		return &make_property_widget_enum;
	}

	if(lodge_type_get_properties(type)) {
		return &make_property_widget_properties;
	}

	return lodge_type_get_func(type, LODGE_TYPE_FUNC_INDEX_MAKE_PROPERTY_WIDGET);
}

void lodge_type_set_make_property_widget_func(lodge_type_t type, lodge_make_property_widget_func_t func)
{
	ASSERT(LODGE_TYPE_FUNC_INDEX_MAKE_PROPERTY_WIDGET);
	lodge_type_set_func(type, LODGE_TYPE_FUNC_INDEX_MAKE_PROPERTY_WIDGET, func);
}

void lodge_gui_property_widget_factory_init()
{
	ASSERT(!LODGE_TYPE_FUNC_INDEX_MAKE_PROPERTY_WIDGET);
	LODGE_TYPE_FUNC_INDEX_MAKE_PROPERTY_WIDGET = lodge_types_make_func_index();

	lodge_type_set_make_property_widget_func(LODGE_TYPE_F32, &make_property_widget_f32);
	lodge_type_set_make_property_widget_func(LODGE_TYPE_U32, &make_property_widget_u32);
	lodge_type_set_make_property_widget_func(LODGE_TYPE_BOOL, &make_property_widget_boolean);
	lodge_type_set_make_property_widget_func(LODGE_TYPE_VEC4, &make_property_widget_vec4);
	lodge_type_set_make_property_widget_func(LODGE_TYPE_VEC3, &make_property_widget_vec3);
	lodge_type_set_make_property_widget_func(LODGE_TYPE_VEC2, &make_property_widget_vec2);
}

#define LODGE_GUI_PROPERTY_WIDGET_FACTORY_COL1_WIDTH 150.0f

bool lodge_gui_property_widget_factory_make_tree(struct nk_context *ctx, strview_t node_label, int node_id, void *object, struct lodge_properties *properties)
{
	bool modified = false;
	if(nk_tree_push_id(ctx, NK_TREE_TAB, node_label.s, NK_MAXIMIZED, node_id)) {
		if(properties) {
			for(size_t i = 0, count = properties->count; i < count; i++) {
				struct lodge_property *property = &properties->elements[i];

				const bool is_private = LODGE_IS_FLAG_SET(property->flags, LODGE_PROPERTY_FLAG_PRIVATE);
				const bool is_transient = LODGE_IS_FLAG_SET(property->flags, LODGE_PROPERTY_FLAG_TRANSIENT);
				//const bool is_readonly = LODGE_IS_FLAG_SET(property->flags, LODGE_PROPERTY_FLAG_READ_ONLY);
				//const bool is_writeonly = LODGE_IS_FLAG_SET(property->flags, LODGE_PROPERTY_FLAG_WRITE_ONLY);

				if(is_private) {
					continue;
				}

				nk_layout_row_template_begin(ctx, 0);
				nk_layout_row_template_push_static(ctx, LODGE_GUI_PROPERTY_WIDGET_FACTORY_COL1_WIDTH);
				nk_layout_row_template_push_dynamic(ctx);
				nk_layout_row_template_end(ctx);

				if(is_transient) {
					nk_style_push_color(ctx, &ctx->style.text.color, nk_color_from_vec4(GRUVBOX_BRIGHT_YELLOW));
				}
				nk_label(ctx, property->name.s, NK_TEXT_LEFT);
				if(is_transient) {
					nk_style_pop_color(ctx);
				}

				lodge_make_property_widget_func_t make_widget = lodge_type_get_make_property_widget_func(property->type);
				if(make_widget) {
					if(make_widget(ctx, property, object)) {
						modified = true;
					}
				} else {
					nk_spacing(ctx, 1);
				}
			}
		}

		nk_tree_pop(ctx);
	}
	return modified;
}