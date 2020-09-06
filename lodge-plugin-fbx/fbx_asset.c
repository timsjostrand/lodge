#include "fbx_asset.h"

#include "fbx.h"
#include "math4.h"
#include "str.h"
#include "array.h"

#include "lodge_platform.h"
#include "lodge_renderer.h"
#include "lodge_shader.h"
#include "lodge_buffer_object.h"
#include "lodge_drawable.h"

#include <string.h>

#define CONVERT_DOUBLE_ARRAY_TO_FLOAT_ARRAY

static const struct fbx_property* fbx_asset_get_array(struct fbx *fbx, const char *path[], size_t path_count)
{
	struct fbx_node *node = fbx_get_node(fbx, path, path_count);
	if(node) {
		const uint64_t property_count = fbx_node_get_property_count(node);
		for(uint64_t i = 0; i < property_count; i++) {
			const struct fbx_property *prop = fbx_node_get_property_array(node, i);
			if(prop) {
				return prop;
			}
		}
	}
	ASSERT_FAIL("fbx_asset: failed to find array property");
	return NULL;
}

static struct fbx_string fbx_asset_get_string(struct fbx *fbx, const char *path[], size_t path_count)
{
	struct fbx_node *node = fbx_get_node(fbx, path, path_count);
	if(node) {
		const uint64_t property_count = fbx_node_get_property_count(node);
		for(uint64_t i = 0; i < property_count; i++) {
			const struct fbx_property *prop = fbx_node_get_property(node, i);
			if(fbx_property_get_type(prop) == FBX_PROPERTY_TYPE_STRING) {
				return fbx_property_get_string(prop);
			}
		}
	}
	ASSERT_FAIL("fbx_asset: failed to find string property");
	static const struct fbx_string empty = { 0 };
	return empty;
}

struct float_array
{
	size_t	count;
	size_t	size;
	float	*data;
};

static struct float_array* float_array_new(size_t count)
{
	const size_t size = count * sizeof(float);
	struct float_array *tmp = (struct float_array *)malloc(sizeof(struct float_array));
	tmp->count = count;
	tmp->size = size;
	tmp->data = (float*)malloc(size);
	return tmp;
}

static void float_array_free(struct float_array *a)
{
	free(a->data);
	free(a);
}

static struct float_array* float_array_new_from_fbx_double_array(const struct fbx_property *prop)
{
	const double* double_array = fbx_property_get_array_double(prop);
	const uint32_t double_array_count = fbx_property_get_array_count(prop);
	if(!double_array || !double_array_count) {
		ASSERT_FAIL("Failed to get double array property");
		return NULL;
	}

	struct float_array *float_array = float_array_new(double_array_count);

	for(size_t i = 0; i < double_array_count; i++) {
		float_array->data[i] = (float)double_array[i];
	}

	return float_array;
}

static struct array* fbx_asset_new_layer_element(
	struct fbx *fbx,
	const int32_t vertex_indices[], size_t vertex_indices_count,
	const char *mapping_type_path[], size_t mapping_type_path_count,
	const char *ref_type_path[], size_t ref_type_path_count,
	const char *data_path[], size_t data_path_count,
	const char *indices_path[], size_t indices_path_count
)
{
	struct array *ret = NULL;

	/* Check mapping type */
	{
		struct fbx_string mapping_type_str = fbx_asset_get_string(fbx, mapping_type_path, mapping_type_path_count);
		strview_t mapping_type = strview_make(mapping_type_str.data, mapping_type_str.length);

		if(!strview_equals(mapping_type, strview_static("ByPolygonVertex"))) {
			ASSERT_FAIL("Layer element mapping != `ByPolygonVertex` not implemented");
			goto fail;
		}
	}

	/* Check reference type */
	enum ref_type {
		REF_TYPE_DIRECT,
		REF_TYPE_INDEX_TO_DIRECT
	} ref_type;
	{
		struct fbx_string ref_type_prop = fbx_asset_get_string(fbx, ref_type_path, ref_type_path_count);
		strview_t ref_type_str = strview_make(ref_type_prop.data, ref_type_prop.length);
		if(strview_equals(ref_type_str, strview_static("Direct"))) {
			ref_type = REF_TYPE_DIRECT;
		} else if(strview_equals(ref_type_str, strview_static("IndexToDirect"))) {
			ref_type = REF_TYPE_INDEX_TO_DIRECT;
		} else {
			ASSERT_FAIL("Reference type not implemented");
			goto fail;
		}
	}

	const struct fbx_property *prop_data = fbx_asset_get_array(fbx, data_path, data_path_count);
	const double* prop_data_ptr = fbx_property_get_array_double(prop_data);
	const uint32_t prop_data_count = fbx_property_get_array_count(prop_data);
	if(!prop_data || !prop_data_ptr || !prop_data_count) {
		ASSERT_FAIL("Failed to find layer element data property");
		goto fail;
	}

	const struct fbx_property *prop_indices = fbx_asset_get_array(fbx, indices_path, indices_path_count);
	const int32_t* prop_indices_ptr = fbx_property_get_array_int32(prop_indices);
	const uint32_t prop_indices_count = fbx_property_get_array_count(prop_indices);
	if(!prop_indices || !prop_indices_ptr || !prop_indices_count) {
		ASSERT_FAIL("Failed to find layer element index property");
		goto fail;
	}

	if(prop_indices_count != vertex_indices_count) {
		ASSERT_FAIL("Incorrect number of indices");
		goto fail;
	}

	ret = array_new(sizeof(vec2), prop_indices_count);

	static const vec2 invalid = { -1, -1 };
	for(uint32_t i = 0, count = array_count(ret); i < count; i++) {
		array_set(ret, i, &invalid);
	}

	int32_t prop_index_max = 0;
	int32_t ret_index_max = 0;
	for(uint32_t i = 0; i < prop_indices_count; i++) {
		if(ref_type == REF_TYPE_DIRECT) {
			array_set(ret, i, &prop_data_ptr[i]);
		} else if(ref_type == REF_TYPE_INDEX_TO_DIRECT) {
			//ASSERT(i < vertex_indices_count);
			//ASSERT(i < prop_indices_count);

			//const int32_t prop_index = prop_indices_ptr[vertex_indices[i]] * stride;
			//ASSERT(prop_index >= 0);
			//ASSERT((uint32_t)prop_index < prop_data_count);

			//const int32_t ret_index = i * stride;
			//const int32_t ret_index = vertex_indices[i] * stride;
			//ASSERT(ret_index >= 0);
			//ASSERT(ret_index < (int32_t)ret->count);

			const int32_t vertex_index = vertex_indices[i];
			const int32_t prop_index = prop_indices_ptr[i] * 2;
			const vec2 prop_data = { (float)prop_data_ptr[prop_index], (float)prop_data_ptr[prop_index + 1] };
			array_set(ret, vertex_index, &prop_data);

			//prop_index_max = max(prop_index, prop_index_max);
			//ret_index_max = max(ret_index + stride, ret_index_max);
		} else {
			ASSERT_FAIL("Reference type not implemented");
			goto fail;
		}
	}

	uint32_t fail_count = 0;
	for(uint32_t i = 0; i < array_count(ret); i++) {
		if(array_equals_at(ret, i, &invalid)) {
			fail_count++;
		}
	}
	ASSERT(fail_count == 0);

	return ret;

fail:
	array_free(ret);
	return NULL;
}

struct fbx_asset fbx_asset_make(struct fbx *fbx)
{
	struct fbx_asset asset = { 0 };

	/* Vertices */
	{
		static const char* path_vertices[] = { "Objects", "Geometry", "Vertices" };

		const struct fbx_property *vertices_prop = fbx_asset_get_array(fbx, path_vertices, LODGE_ARRAYSIZE(path_vertices));
		if(!vertices_prop) {
			goto fail;
		}

#ifndef CONVERT_DOUBLE_ARRAY_TO_FLOAT_ARRAY
		const double* vertices_prop_data = fbx_property_get_array_double(vertices_prop);
		const uint32_t vertices_prop_data_count = fbx_property_get_array_count(vertices_prop);

		glBufferData(GL_ARRAY_BUFFER, vertices_prop_data_count * sizeof(double), vertices_prop_data, GL_STATIC_DRAW);
		GL_OK_OR_GOTO(fail);

		glEnableVertexAttribArray(0);
		GL_OK_OR_GOTO(fail);

		glVertexAttribPointer(0,
			3,
			GL_FLOAT, //GL_DOUBLE,
			GL_FALSE,
			3 * sizeof(double),
			NULL
		);
		GL_OK_OR_GOTO(fail);
#else
		struct float_array *vertices_data = float_array_new_from_fbx_double_array(vertices_prop);

		asset.static_mesh.vertices = lodge_buffer_object_make_static(vertices_data->data, vertices_data->size);
		float_array_free(vertices_data);

		if(!asset.static_mesh.vertices) {
			goto fail;
		}
#endif
	}

	/* Normals */
	{
		/* Check mapping type */
		{
			static const char* path_normals_mapping_type[] = { "Objects", "Geometry", "LayerElementNormal", "MappingInformationType" };

			struct fbx_string normals_mapping_type_str = fbx_asset_get_string(fbx, path_normals_mapping_type, LODGE_ARRAYSIZE(path_normals_mapping_type));
			strview_t normals_mapping_type = strview_make(normals_mapping_type_str.data, normals_mapping_type_str.length);

#if 0
			if(!strview_equals(normals_mapping_type, strview_static("ByPolygonVertex"))) {
				ASSERT_FAIL("Normal mapping != `ByPolygonVertex` not implemented");
				goto fail;
			}
#else
			if(!strview_equals(normals_mapping_type, strview_static("ByPolygonVertex"))
				&& !strview_equals(normals_mapping_type, strview_static("ByVertex"))
				&& !strview_equals(normals_mapping_type, strview_static("ByVertice"))) {
				ASSERT_FAIL("Normal mapping != (`ByPolygonVertex`|`ByVertex`) not implemented");
				goto fail;
			}
#endif
		}
		
		/* Upload normal buffer */
		{
			static const char* path_normals[] = { "Objects", "Geometry", "LayerElementNormal", "Normals" };

			const struct fbx_property *normals_prop = fbx_asset_get_array(fbx, path_normals, LODGE_ARRAYSIZE(path_normals));
			if(!normals_prop) {
				goto fail;
			}

#ifndef CONVERT_DOUBLE_ARRAY_TO_FLOAT_ARRAY
			const double* normals_prop_data = fbx_property_get_array_double(normals_prop);
			const uint32_t normals_prop_data_count = fbx_property_get_array_count(normals_prop);

			glBufferData(GL_ARRAY_BUFFER, normals_prop_data_count * sizeof(double), normals_prop_data, GL_STATIC_DRAW);
			GL_OK_OR_GOTO(fail);

			glEnableVertexAttribArray(1);
			GL_OK_OR_GOTO(fail);

			glVertexAttribPointer(1,
				3,
				GL_FLOAT, // GL_DOUBLE,
				GL_FALSE,
				3 * sizeof(double),
				NULL
			);
#else
			struct float_array *normals_data = float_array_new_from_fbx_double_array(normals_prop);

			asset.static_mesh.normals = lodge_buffer_object_make_static(normals_data->data, normals_data->size);
			float_array_free(normals_data);

			if(!asset.static_mesh.normals) {
				goto fail;
			}
#endif
		}
	}

	static const char* path_indices[] = { "Objects", "Geometry", "PolygonVertexIndex" };
	const struct fbx_property *prop_indices = fbx_asset_get_array(fbx, path_indices, LODGE_ARRAYSIZE(path_indices));
	if(!prop_indices) {
		goto fail;
	}

	const int32_t *prop_indices_data = fbx_property_get_array_int32(prop_indices);
	const uint32_t prop_indices_data_count = fbx_property_get_array_count(prop_indices);

	if(prop_indices_data_count < 3) {
		ASSERT_FAIL("Vertex index buffer too small");
		goto fail;
	}

	enum polygon_type_ {
		POLYGON_TYPE_TRIANGLE,
		POLYGON_TYPE_QUAD
	} polygon_type;

	if(prop_indices_data[2] < 0) {
		polygon_type = POLYGON_TYPE_TRIANGLE;

		// make sure all are triangles (not needed?)
		for(uint32_t i = 2; i < prop_indices_data_count; i += 3) {
			ASSERT(prop_indices_data[i] < 0);
		}

	} else if(prop_indices_data[3] < 0) {
		polygon_type = POLYGON_TYPE_QUAD;
	} else {
		ASSERT_FAIL("Unknown polygon type");
		goto fail;
	}

	if(polygon_type != POLYGON_TYPE_TRIANGLE) {
		ASSERT_FAIL("polygon_type != triangle not implemented");
		goto fail;
	}

	/* Convert signed FBX indices to unsigned indices */
	uint32_t* indices = malloc(prop_indices_data_count * sizeof(uint32_t));
	for(uint32_t i = 0; i < prop_indices_data_count; i++) {
		const int32_t index = prop_indices_data[i];
		indices[i] = index >= 0 ? (uint32_t)index : (uint32_t)(~index);
	}

	asset.static_mesh.indices_count = prop_indices_data_count;

	/* UVs */
	{
		static const char* path_uv_mapping_type[] = { "Objects", "Geometry", "LayerElementUV", "MappingInformationType" };
		static const char* path_uvs_ref_type[] = { "Objects", "Geometry", "LayerElementUV", "ReferenceInformationType" };
		static const char* path_uvs_data[] = { "Objects", "Geometry", "LayerElementUV", "UV" };
		static const char* path_uvs_indices[] = { "Objects", "Geometry", "LayerElementUV", "UVIndex" };

		struct array *uvs = fbx_asset_new_layer_element(fbx,
			indices,				asset.static_mesh.indices_count,
			path_uv_mapping_type,	LODGE_ARRAYSIZE(path_uv_mapping_type),
			path_uvs_ref_type,		LODGE_ARRAYSIZE(path_uvs_ref_type),
			path_uvs_data,			LODGE_ARRAYSIZE(path_uvs_data),
			path_uvs_indices,		LODGE_ARRAYSIZE(path_uvs_indices)
		);

		if(!uvs) {
			goto fail;
		}

		asset.static_mesh.tex_coords = lodge_buffer_object_make_static(array_first(uvs), array_byte_size(uvs));
		array_destroy(uvs);

		if(!asset.static_mesh.tex_coords) {
			goto fail;
		}
	}

	/* Indices */
	asset.static_mesh.indices = lodge_buffer_object_make_static(indices, prop_indices_data_count * sizeof(uint32_t));
	free(indices);

	if(!asset.static_mesh.indices) {
		goto fail;
	}

	asset.drawable = lodge_drawable_make_from_static_mesh(&asset.static_mesh);

	return asset;

fail:
	ASSERT_FAIL("Failed to create fbx asset");
	free(indices);
	fbx_asset_reset(&asset);
	return asset;
}

void fbx_asset_reset(struct fbx_asset *asset)
{
	lodge_static_mesh_reset(&asset->static_mesh);
	lodge_drawable_reset(asset->drawable);
	*asset = (struct fbx_asset) { 0 };
}

void fbx_asset_render(const struct fbx_asset *asset, lodge_shader_t shader, lodge_texture_t tex, struct mvp mvp)
{
	lodge_renderer_bind_shader(shader);
	lodge_renderer_set_constant_mvp(shader, &mvp);

	// FIXME(TS): material should not be hardcoded, use texture unit instead
	lodge_renderer_bind_texture_2d(0, tex);

	lodge_drawable_render_indexed(asset->drawable, asset->static_mesh.indices_count);
}