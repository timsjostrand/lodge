#include "fbx_asset.h"

#include "fbx.h"
#include "math4.h"
#include "str.h"
#include "array.h"
#include "geometry.h"

#include "lodge_platform.h"
#include "lodge_gfx.h"
#include "lodge_shader.h"
#include "lodge_buffer_object.h"
#include "lodge_drawable.h"

#include <string.h>

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

#if 0
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

static struct float_array* float_array_new_from_fbx_double_array(const struct fbx_property *prop, bool swap_y_z)
{
	const double* double_array = fbx_property_get_array_double(prop);
	const uint32_t double_array_count = fbx_property_get_array_count(prop);
	if(!double_array || !double_array_count) {
		ASSERT_FAIL("Failed to get double array property");
		return NULL;
	}

	// vec3 type
	ASSERT(double_array_count % 3 == 0);

	struct float_array *float_array = float_array_new(double_array_count);

	for(size_t i = 0; i < double_array_count; i+=3) {
		float_array->data[i + 0] = (float)double_array[i + 0];
		float_array->data[i + 1] = (float)double_array[i + (swap_y_z ? 2 : 1)];
		float_array->data[i + 2] = (float)double_array[i + (swap_y_z ? 1 : 2)];
	}

	return float_array;
}
#endif

static enum fbx_ref_type fbx_ref_type_from_strview(strview_t ref_type)
{
	if(strview_equals(ref_type, strview_static("Direct"))) {
		return FBX_REF_TYPE_DIRECT;
	} else if(strview_equals(ref_type, strview_static("IndexToDirect"))) {
		return FBX_REF_TYPE_INDEX_TO_DIRECT;
	} else {
		ASSERT_NOT_IMPLEMENTED();
		return FBX_REF_TYPE_MAX;
	}
}

static enum fbx_mapping_type fbx_mapping_type_from_strview(strview_t mapping_type)
{
	if(strview_equals(mapping_type, strview_static("ByPolygon"))) {
		return FBX_MAPPING_TYPE_BY_POLYGON;
	} else if(strview_equals(mapping_type, strview_static("ByPolygonVertex"))) {
		return FBX_MAPPING_TYPE_BY_POLYGON_VERTEX;
	} else if(strview_equals(mapping_type, strview_static("ByVertex")) || strview_equals(mapping_type, strview_static("ByVertice"))) {
		return FBX_MAPPING_TYPE_BY_VERTEX;
	} else if(strview_equals(mapping_type, strview_static("ByEdge"))) {
		return FBX_MAPPING_TYPE_BY_EDGE;
	} else if(strview_equals(mapping_type, strview_static("AllSame"))) {
		return FBX_MAPPING_TYPE_ALL_SAME;
	} else {
		ASSERT_NOT_IMPLEMENTED();
		return FBX_MAPPING_TYPE_MAX;
	}
}

static enum fbx_polygon_type fbx_polygon_type_from_index_buffer(const int32_t *indices, const uint64_t indices_count)
{
	if(indices_count < 3) {
		ASSERT_FAIL("Vertex index buffer too small");
		return FBX_POLYGON_TYPE_MAX;
	}

	if(indices[2] < 0) {
		// make sure all are triangles (not needed?)
		for(uint32_t i = 2; i < indices_count; i += 3) {
			ASSERT(indices[i] < 0);
		}
		return FBX_POLYGON_TYPE_TRIANGLE;
	} else if(indices[3] < 0) {
		return FBX_POLYGON_TYPE_QUAD;
	} else {
		ASSERT_NOT_IMPLEMENTED();
		return FBX_POLYGON_TYPE_MAX;
	}
}

//
// https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
//
static vec3 triangle_calc_surface_normal(const struct triangle *triangle)
{
	const vec3 u = vec3_sub(triangle->p1, triangle->p0);
	const vec3 v = vec3_sub(triangle->p2, triangle->p0);
	return (vec3) {
		.x = u.y * v.z - u.z * v.y,
		.y = u.z * v.x - u.x * v.z,
		.z = u.x * v.y - u.y * v.x
	};
}

struct fbx_layer_element
{
	enum fbx_mapping_type	mapping_type;
	enum fbx_ref_type		ref_type;
	
	const double			*data;
	uint32_t				data_count;

	const int32_t			*indices;
	uint32_t				indices_count;
};

struct fbx_global_settings
{
	const int32_t			*up_axis;
	const int32_t			*up_axis_sign;
	const int32_t			*front_axis;
	const int32_t			*front_axis_sign;
};

struct fbx_geometry
{
	const double				*vertices_array;
	uint32_t					vertices_array_count;

	const int32_t				*vertex_indices;
	uint32_t					vertex_indices_count;

	enum fbx_polygon_type		polygon_type;

	struct fbx_layer_element	normals;
	struct fbx_layer_element	uvs;
	//struct fbx_layer_element	tangents;
	//struct fbx_layer_element	bitangents;
};

struct fbx_doc
{
	struct fbx_global_settings	global_settings;
	struct fbx_geometry			geometry;
};

#if 0
static struct array* fbx_asset_new_layer_element(
	struct fbx *fbx,
	const uint32_t component_count, size_t vertices_count,
	const int32_t vertex_indices[], size_t vertex_indices_count,
	const char *mapping_type_path[], size_t mapping_type_path_count,
	const char *ref_type_path[], size_t ref_type_path_count,
	const char *data_path[], size_t data_path_count,
	const char *indices_path[], size_t indices_path_count)
{
	struct array *ret = NULL;

	const bool swap_y_z = true;

	// Check mapping type
	enum fbx_mapping_type mapping_type;
	{
		struct fbx_string mapping_type_prop = fbx_asset_get_string(fbx, mapping_type_path, mapping_type_path_count);
		mapping_type = fbx_mapping_type_from_strview(strview_make(mapping_type_prop.data, mapping_type_prop.length));
	}

	// Check reference type
	enum fbx_ref_type ref_type;
	{
		struct fbx_string ref_type_prop = fbx_asset_get_string(fbx, ref_type_path, ref_type_path_count);
		ref_type = fbx_ref_type_from_strview(strview_make(ref_type_prop.data, ref_type_prop.length));
	}

	const struct fbx_property *prop_data = fbx_asset_get_array(fbx, data_path, data_path_count);
	const double *prop_data_ptr = fbx_property_get_array_double(prop_data);
	const uint32_t prop_data_count = fbx_property_get_array_count(prop_data);
	if(!prop_data || !prop_data_ptr || !prop_data_count) {
		ASSERT_FAIL("Failed to find layer element data property");
		goto fail;
	}

	// The "element" represents a vec2, vec3 and so on
	const size_t element_size = sizeof(float) * component_count;
	float *element = LODGE_ALLOCA(element_size);

	if(mapping_type == FBX_MAPPING_TYPE_BY_VERTEX && ref_type == FBX_REF_TYPE_DIRECT) {
		ASSERT(prop_data_count % element_size == 0);

		//
		// Layer is in vertex order: map back to vertex index buffer.
		//
		const size_t elements_count = prop_data_count / component_count;

		if(prop_data_count != vertices_count) {
			ASSERT_FAIL("Incorrect number of vertices");
			goto fail;
		}

		ret = array_new(element_size, elements_count);
		for(uint32_t i = 0; i < prop_data_count; i+=component_count) {
			for(uint32_t c = 0; c < component_count; c++) {
				element[c] = (float)prop_data_ptr[i + c];
			}

			const int32_t vertex_index = vertex_indices[i];
			array_set(ret, vertex_index, element);
		}
	} else if(mapping_type == FBX_MAPPING_TYPE_BY_POLYGON_VERTEX && ref_type == FBX_REF_TYPE_DIRECT) {
		//
		// TODO(TS): average normals will not work, should:
		// - detect if normal differs ("a lot"), and if so, split vertices
		// - maybe, if attributes are not sharable (like for a cube), it is easier/more performant
		//   to skip the index buffer entirely (convert vertex buffer via index buffer to a flat array)
		//

		const size_t elements_count = prop_data_count / component_count;
		if(elements_count != vertex_indices_count) {
			ASSERT_FAIL("Incorrect number of indices");
			goto fail;
		}

		uint32_t *is_set = (uint32_t*)calloc(vertices_count, sizeof(uint32_t));
		ret = array_new(element_size, vertices_count);

		for(uint32_t i = 0; i < elements_count; i++) {
			const int32_t vertex_index = vertex_indices[i];
			ASSERT(vertex_index < vertices_count);
			for(uint32_t c = 0; c < component_count; c++) {
				element[c] = (float)prop_data_ptr[vertex_index * component_count + c];
			}

			array_set(ret, vertex_index, element);
			is_set[vertex_index]++;
		}

		// Average normals
		for(uint32_t i = 0; i < vertices_count; i++) {
			uint32_t count = is_set[i];

			float *elem = array_get(ret, i);

			for(uint32_t c = 0; c < component_count; c++) {
				elem[c] /= (float)count;
			}
		}

		for(uint32_t i = 0; i < vertices_count; i++) {
			const float* elem = array_get(ret, i);
			printf("%d: { %.1f %.1f %.1f }\n", i, elem[0], elem[1], component_count == 3 ? elem[2] : 0.0f);
		}

		free(is_set);
	} else if((mapping_type == FBX_MAPPING_TYPE_BY_VERTEX || mapping_type == FBX_MAPPING_TYPE_BY_POLYGON_VERTEX) && ref_type == FBX_REF_TYPE_INDEX_TO_DIRECT) {
		const struct fbx_property *prop_indices = fbx_asset_get_array(fbx, indices_path, indices_path_count);
		const int32_t *prop_indices_ptr = fbx_property_get_array_int32(prop_indices);
		const uint32_t prop_indices_count = fbx_property_get_array_count(prop_indices);
		if(!prop_indices || !prop_indices_ptr || !prop_indices_count) {
			ASSERT_FAIL("Failed to find layer element index property");
			goto fail;
		}

		if(prop_indices_count != vertex_indices_count) {
			ASSERT_FAIL("Incorrect number of indices");
			goto fail;
		}

		ret = array_new(element_size, prop_indices_count);

		for(uint32_t i = 0; i < prop_indices_count; i++) {
			const int32_t prop_index = prop_indices_ptr[i] * component_count;
			for(uint32_t c = 0; c < component_count; c++) {
				element[c] = (float)prop_data_ptr[prop_index + c];
			}
			array_set(ret, i, element);
		}
	} else {
		ASSERT_NOT_IMPLEMENTED();
		goto fail;
	}

	return ret;

fail:
	array_free(ret);
	return NULL;
}
#else
static struct fbx_layer_element fbx_layer_element_make(
	struct fbx *fbx,
	const char *mapping_type_path[], size_t mapping_type_path_count,
	const char *ref_type_path[], size_t ref_type_path_count,
	const char *data_path[], size_t data_path_count,
	const char *indices_path[], size_t indices_path_count)
{
	struct fbx_layer_element ret = { 0 };

	// Check mapping type
	{
		struct fbx_string mapping_type_prop = fbx_asset_get_string(fbx, mapping_type_path, mapping_type_path_count);
		ret.mapping_type = fbx_mapping_type_from_strview(strview_make(mapping_type_prop.data, mapping_type_prop.length));
	}

	// Check reference type
	{
		struct fbx_string ref_type_prop = fbx_asset_get_string(fbx, ref_type_path, ref_type_path_count);
		ret.ref_type = fbx_ref_type_from_strview(strview_make(ref_type_prop.data, ref_type_prop.length));
	}

	// Get data
	{
		const struct fbx_property *prop_data = fbx_asset_get_array(fbx, data_path, data_path_count);
		const double *prop_data_ptr = fbx_property_get_array_double(prop_data);
		const uint32_t prop_data_count = fbx_property_get_array_count(prop_data);
		if(!prop_data || !prop_data_ptr || !prop_data_count) {
			ASSERT_FAIL("Failed to find layer element data property");
			goto fail;
		}

		ret.data = prop_data_ptr;
		ret.data_count = prop_data_count;
	}

	// The "element" represents a vec2, vec3 and so on

	if(ret.ref_type == FBX_REF_TYPE_DIRECT) {
		// Noop
	} else if(ret.ref_type == FBX_REF_TYPE_INDEX_TO_DIRECT) {
		const struct fbx_property *prop_indices = fbx_asset_get_array(fbx, indices_path, indices_path_count);
		const int32_t *prop_indices_ptr = fbx_property_get_array_int32(prop_indices);
		const uint32_t prop_indices_count = fbx_property_get_array_count(prop_indices);
		if(!prop_indices || !prop_indices_ptr || !prop_indices_count) {
			ASSERT_FAIL("Failed to find layer element index property");
			goto fail;
		}

		ret.indices = prop_indices_ptr;
		ret.indices_count = prop_indices_count;
	} else {
		ASSERT_NOT_IMPLEMENTED();
		goto fail;
	}

	return ret;

fail:
	ASSERT_FAIL("Failed to make fbx layer element");
	return (struct fbx_layer_element) { 0 };
}
#endif

static struct fbx_global_settings fbx_global_settings_make(struct fbx *fbx)
{
	static const char* path_global_settings[] = { "GlobalSettings" };

	return (struct fbx_global_settings) {
		.up_axis = fbx_get_typed_property_int32(fbx, path_global_settings, LODGE_ARRAYSIZE(path_global_settings), "UpAxis"),
		.up_axis_sign = fbx_get_typed_property_int32(fbx, path_global_settings, LODGE_ARRAYSIZE(path_global_settings), "UpAxisSign"),
		.front_axis = fbx_get_typed_property_int32(fbx, path_global_settings, LODGE_ARRAYSIZE(path_global_settings), "FrontAxis"),
		.front_axis_sign = fbx_get_typed_property_int32(fbx, path_global_settings, LODGE_ARRAYSIZE(path_global_settings), "FrontAxisSign")
	};
}

static struct fbx_geometry fbx_geometry_make(struct fbx *fbx)
{
	static const char* path_vertices[] = { "Objects", "Geometry", "Vertices" };

	// Vertices
	const struct fbx_property *vertices_prop = fbx_asset_get_array(fbx, path_vertices, LODGE_ARRAYSIZE(path_vertices));
	if(!vertices_prop) {
		goto fail;
	}
	const double *vertices_array = fbx_property_get_array_double(vertices_prop);
	uint32_t vertices_array_count = fbx_property_get_array_count(vertices_prop);

	// Indices
	static const char* path_indices[] = { "Objects", "Geometry", "PolygonVertexIndex" };
	const struct fbx_property *prop_indices = fbx_asset_get_array(fbx, path_indices, LODGE_ARRAYSIZE(path_indices));
	if(!prop_indices) {
		goto fail;
	}

	const int32_t *vertex_indices = fbx_property_get_array_int32(prop_indices);
	const uint32_t vertex_indices_count = fbx_property_get_array_count(prop_indices);

	// Polygon type
	enum fbx_polygon_type polygon_type = fbx_polygon_type_from_index_buffer(vertex_indices, vertex_indices_count);
	if(polygon_type != FBX_POLYGON_TYPE_TRIANGLE) {
		goto fail;
	}

	// Normals
	struct fbx_layer_element normals;
	{
		static const char* path_normals_mapping_type[] = { "Objects", "Geometry", "LayerElementNormal", "MappingInformationType" };
		static const char* path_normals_ref_type[] = { "Objects", "Geometry", "LayerElementNormal", "ReferenceInformationType" };
		static const char* path_normals_data[] = { "Objects", "Geometry", "LayerElementNormal", "Normals" };
		static const char* path_normals_indices[] = { "Objects", "Geometry", "LayerElementNormal", "NormalsIndex" };

		normals = fbx_layer_element_make(fbx,
			path_normals_mapping_type,	LODGE_ARRAYSIZE(path_normals_mapping_type),
			path_normals_ref_type,		LODGE_ARRAYSIZE(path_normals_ref_type),
			path_normals_data,			LODGE_ARRAYSIZE(path_normals_data),
			path_normals_indices,		LODGE_ARRAYSIZE(path_normals_indices)
		);

		if(!normals.data) {
			goto fail;
		}
	}

	// TODO(TS): also upload bitangents (add support in static mesh)

	// UVs
	struct fbx_layer_element uvs;
	{
		static const char* path_uvs_mapping_type[] = { "Objects", "Geometry", "LayerElementUV", "MappingInformationType" };
		static const char* path_uvs_ref_type[] = { "Objects", "Geometry", "LayerElementUV", "ReferenceInformationType" };
		static const char* path_uvs_data[] = { "Objects", "Geometry", "LayerElementUV", "UV" };
		static const char* path_uvs_indices[] = { "Objects", "Geometry", "LayerElementUV", "UVIndex" };

		uvs = fbx_layer_element_make(fbx,
			path_uvs_mapping_type,	LODGE_ARRAYSIZE(path_uvs_mapping_type),
			path_uvs_ref_type,		LODGE_ARRAYSIZE(path_uvs_ref_type),
			path_uvs_data,			LODGE_ARRAYSIZE(path_uvs_data),
			path_uvs_indices,		LODGE_ARRAYSIZE(path_uvs_indices)
		);

		if(!uvs.data) {
			goto fail;
		}
	}
		
	return (struct fbx_geometry) {
		.vertices_array = vertices_array,
		.vertices_array_count = vertices_array_count,

		.vertex_indices = vertex_indices,
		.vertex_indices_count = vertex_indices_count,

		.polygon_type = polygon_type,

		.normals = normals,
		.uvs = uvs,
		//.tangents =
		//.bitangents = 
	};

fail:
	ASSERT_FAIL("Failed to make fbx geometry");
	return (struct fbx_geometry) { 0 };
}

static struct fbx_doc fbx_doc_make(struct fbx *fbx)
{
	return (struct fbx_doc) {
		.global_settings = fbx_global_settings_make(fbx),
		.geometry = fbx_geometry_make(fbx)
	};
}

//
// GPU friendly data
//
struct fbx_mesh
{
	vec3		*vertices;
	size_t		vertices_count;

	uint32_t	*indices;
	size_t		indices_count;

	vec3		*normals;
	size_t		normals_count;

	vec2		*uvs;
	size_t		uvs_count;
};

static void fbx_mesh_free_inplace(struct fbx_mesh *mesh)
{
	ASSERT(mesh);
	free(mesh->indices);
	free(mesh->vertices);
	free(mesh->normals);
	free(mesh->uvs);
}

//
// Vertices, either:
// 1) Use index buffer as-is; clone double buffer to float buffer
// 2) Split vertices; skip using index buffer and clone all vertices instead
//
// TODO(TS):
// In either case: swap X/Z if `up_axis != 2`.
//
static vec3* fbx_double3_array_to_vec3_array(size_t *dst_count, const struct fbx_layer_element *layer, const uint32_t *indices, size_t indices_count, size_t vertices_count, bool split_vertices, bool swap_y_z)
{
	ASSERT(layer->data_count % 3 == 0);

#if 1
	ASSERT(layer->mapping_type == FBX_MAPPING_TYPE_BY_POLYGON_VERTEX || layer->mapping_type == FBX_MAPPING_TYPE_BY_VERTEX);
	// Not supported?
	//ASSERT(layer->mapping_type == FBX_MAPPING_TYPE_BY_VERTEX);
	ASSERT(layer->ref_type == FBX_REF_TYPE_DIRECT || layer->ref_type == FBX_REF_TYPE_INDEX_TO_DIRECT);

	if(layer->ref_type == FBX_REF_TYPE_INDEX_TO_DIRECT) {
		ASSERT(layer->indices);
		ASSERT(layer->indices_count == indices_count);
	}

	//if(layer->ref_type == FBX_REF_TYPE_DIRECT) {
	//	ASSERT(layer->data_count/3 == vertices_count);
	//}

	*dst_count = split_vertices ? indices_count : vertices_count;
	vec3 *dst = malloc(sizeof(vec3) * (*dst_count));

	for(size_t i = 0; i < indices_count; i++) {
		const uint32_t dst_index = split_vertices ? i : indices[i];
		int32_t src_index = 0;
		
		if(layer->mapping_type == FBX_MAPPING_TYPE_BY_POLYGON_VERTEX) {
			src_index = (layer->ref_type == FBX_REF_TYPE_DIRECT ? i : layer->indices[i]);
		} else if(layer->mapping_type == FBX_MAPPING_TYPE_BY_VERTEX) {
			src_index = (layer->ref_type == FBX_REF_TYPE_DIRECT ? indices[i] : layer->indices[i]);
		}

		dst[dst_index].x = (float)layer->data[src_index * 3 + 0];
		dst[dst_index].y = (float)layer->data[src_index * 3 + (swap_y_z ? 2 : 1)];
		dst[dst_index].z = (float)layer->data[src_index * 3 + (swap_y_z ? 1 : 2)];
	}

	return dst;

#else
	vec3 *dst = NULL;

	if(split_vertices) {
		ASSERT((layer->mapping_type == FBX_MAPPING_TYPE_BY_VERTEX) || (layer->mapping_type == FBX_MAPPING_TYPE_BY_POLYGON_VERTEX));
		ASSERT(layer->ref_type == FBX_REF_TYPE_DIRECT);
		//
		// 1:1 normal and vertex.
		//
		*dst_count = indices_count;
		dst = malloc(sizeof(vec3) * indices_count);

		for(size_t i = 0, count = indices_count; i < count; i++) {
			const uint32_t vertex_index = indices[i];
			dst[i].x = (float)layer->data[vertex_index * 3 + 0];
			dst[i].y = (float)layer->data[vertex_index * 3 + (swap_y_z ? 2 : 1)];
			dst[i].z = (float)layer->data[vertex_index * 3 + (swap_y_z ? 1 : 2)];
		}
	} else {
		//
		// Vertices are shared in index buffer -- should average normals.
		//

#if 1
		*dst_count = layer->data_count / 3;
		dst = malloc(sizeof(vec3) * (*dst_count));

		for(size_t i = 0, count = (*dst_count); i < count; i++) {
			dst[i].x = (float)layer->data[i * 3 + 0];
			dst[i].y = (float)layer->data[i * 3 + (swap_y_z ? 2 : 1)];
			dst[i].z = (float)layer->data[i * 3 + (swap_y_z ? 1 : 2)];
		}
#endif
	}

	return dst;
#endif
}

static vec2* fbx_double2_array_to_vec2_array(size_t *dst_count, const struct fbx_layer_element *layer, const uint32_t *indices, size_t indices_count, size_t vertices_count, bool split_vertices)
{
	ASSERT(layer->data_count % 2 == 0);

	// NOTE(TS): not sure about case when ref_type == DIRECT

	// split_vertices: 
	//		true:  1:1 vertex, normal, tex_coord (no index buffer)
	//		false: vertices are shared in index buffer -- should average values.
	//
	// ref_type:
	//		DIRECT

	ASSERT(layer->mapping_type == FBX_MAPPING_TYPE_BY_POLYGON_VERTEX || layer->mapping_type == FBX_MAPPING_TYPE_BY_VERTEX);
	ASSERT((layer->ref_type == FBX_REF_TYPE_INDEX_TO_DIRECT) && layer->indices_count == indices_count);

	*dst_count = split_vertices ? indices_count : vertices_count;
	vec2 *dst = malloc(sizeof(vec2) * (*dst_count));

	for(size_t i = 0; i < indices_count; i++) {
		const uint32_t dst_index = split_vertices ? i : indices[i];
		const int32_t src_index = layer->ref_type == FBX_REF_TYPE_DIRECT ? i : layer->indices[i];

		dst[dst_index].x = (float)layer->data[src_index * 2 + 0];
		dst[dst_index].y = (float)layer->data[src_index * 2 + 1];
	}

#if 0
	if(split_vertices) {
		*dst_count = indices_count;
		dst = malloc(sizeof(vec2) * indices_count);

		for(size_t i = 0, count = indices_count; i < count; i++) {
			const uint32_t vertex_index = indices[i];
			dst[i].x = (float)layer->data[vertex_index * 2 + 0];
			dst[i].y = (float)layer->data[vertex_index * 2 + 1];
		}
	} else {
		*dst_count = layer->data_count / 2;
		dst = malloc(sizeof(vec2) * (*dst_count));

		for(size_t i = 0, count = (*dst_count); i < count; i++) {
			dst[i].x = (float)layer->data[i * 2 + 0];
			dst[i].y = (float)layer->data[i * 2 + 1];
		}
	}
#endif

	return dst;
}

static void fbx_mesh_new_inplace(struct fbx_mesh *mesh, struct fbx_doc *doc)
{
	ASSERT(mesh);
	ASSERT(doc);

	if(doc->geometry.polygon_type != FBX_POLYGON_TYPE_TRIANGLE) {
		goto fail;
	}

	if(!doc->global_settings.up_axis
		|| !doc->global_settings.up_axis_sign
		|| !doc->global_settings.front_axis
		|| !doc->global_settings.front_axis_sign) {
		goto fail;
	}

	// up_axis == 2 is optimal
	// up_axis == 1 can be converted into 2
	ASSERT(*doc->global_settings.up_axis == 1 || *doc->global_settings.up_axis == 2);

	const bool swap_y_z = (*doc->global_settings.up_axis == 1);

	//
	// TODO(TS): detect when vertices need to be split -- always splitting is wasteful.
	//
	const bool split_vertices = false; //(doc->geometry.normals.ref_type == FBX_REF_TYPE_DIRECT);

	// Convert signed FBX indices to unsigned indices
	{
		mesh->indices = malloc(doc->geometry.vertex_indices_count * sizeof(uint32_t));
		mesh->indices_count = doc->geometry.vertex_indices_count;
		for(uint32_t i = 0, count = doc->geometry.vertex_indices_count; i < count; i++) {
			const int32_t index = doc->geometry.vertex_indices[i];
			mesh->indices[i] = index >= 0 ? (uint32_t)index : (uint32_t)(~index);
		}
	}

	const size_t vertices_count = doc->geometry.vertices_array_count / 3;

	// Vertices
	mesh->vertices = fbx_double3_array_to_vec3_array(
		&mesh->vertices_count,
		&(struct fbx_layer_element) {
			.data = doc->geometry.vertices_array,
			.data_count = doc->geometry.vertices_array_count,
			.indices = mesh->indices,
			.indices_count = mesh->indices_count,
			.mapping_type = FBX_MAPPING_TYPE_BY_POLYGON_VERTEX,
			.ref_type = FBX_REF_TYPE_INDEX_TO_DIRECT,
		},
		mesh->indices, mesh->indices_count,
		vertices_count,
		split_vertices,
		swap_y_z
	);

	// Normals
	mesh->normals = fbx_double3_array_to_vec3_array(
		&mesh->normals_count,
		&doc->geometry.normals,
		mesh->indices, mesh->indices_count,
		vertices_count,
		split_vertices,
		swap_y_z
	);

	// Tex coords
	mesh->uvs = fbx_double2_array_to_vec2_array(
		&mesh->uvs_count,
		&doc->geometry.uvs,
		mesh->indices, mesh->indices_count,
		vertices_count,
		split_vertices
	);

	//
	// TODO(TS): Skip index buffer entirely if splitting vertices (free and set NULL), but keeping in for now for render pipeline simplicity
	//
	if(split_vertices) {
		for(uint32_t i = 0, count = doc->geometry.vertex_indices_count; i < count; i++) {
			mesh->indices[i] = i;
		}
	}

	return;

fail:
	ASSERT_FAIL("Failed to make fbx mesh");
	fbx_mesh_free_inplace(mesh);
}

static struct fbx_asset fbx_asset_make_from_mesh(struct fbx_mesh *mesh)
{
	struct fbx_asset asset = { 0 };

	asset.static_mesh.vertices = lodge_buffer_object_make_static(mesh->vertices, mesh->vertices_count * sizeof(vec3));
	if(!asset.static_mesh.vertices) {
		goto fail;
	}

	asset.static_mesh.indices = lodge_buffer_object_make_static(mesh->indices, mesh->indices_count * sizeof(uint32_t));
	asset.static_mesh.indices_count = mesh->indices_count;
	if(!asset.static_mesh.indices) {
		goto fail;
	}

	asset.static_mesh.normals = lodge_buffer_object_make_static(mesh->normals, mesh->normals_count * sizeof(vec3));
	if(!asset.static_mesh.normals) {
		goto fail;
	}

	asset.static_mesh.tex_coords = lodge_buffer_object_make_static(mesh->uvs, mesh->uvs_count * sizeof(vec2));
	if(!asset.static_mesh.tex_coords) {
		goto fail;
	}

	asset.drawable = lodge_drawable_make_from_static_mesh(&asset.static_mesh);
	if(!asset.drawable) {
		goto fail;
	}

	return asset;

fail:
	ASSERT_FAIL("Failed to make fbx asset from mesh");
	return (struct fbx_asset) { 0 };
}

//
// NOTE(TS): There is also per object transforms that needs to be taken into account.......
//
struct fbx_asset fbx_asset_make(struct fbx *fbx)
{
	// Parse FBX document from the fbx node tree
	struct fbx_doc doc = fbx_doc_make(fbx);

	struct fbx_mesh mesh = { 0 };
	fbx_mesh_new_inplace(&mesh, &doc);

	struct fbx_asset asset = fbx_asset_make_from_mesh(&mesh);
	fbx_mesh_free_inplace(&mesh);

	return asset;

//fail:
//	ASSERT_FAIL("Failed to create fbx asset");
//	fbx_mesh_free_inplace(&mesh);
//	fbx_asset_reset(&asset);
//	return (struct fbx_asset) { 0 };
}

void fbx_asset_reset(struct fbx_asset *asset)
{
	lodge_static_mesh_reset(&asset->static_mesh);
	lodge_drawable_reset(asset->drawable);
	*asset = (struct fbx_asset) { 0 };
}

void fbx_asset_render(const struct fbx_asset *asset, lodge_shader_t shader, lodge_texture_t tex, struct mvp mvp)
{
	lodge_gfx_bind_shader(shader);
	lodge_shader_set_constant_mvp(shader, &mvp);

	// FIXME(TS): material should not be hardcoded, use texture unit instead
	lodge_gfx_bind_texture_2d(0, tex);

	lodge_drawable_render_indexed(asset->drawable, asset->static_mesh.indices_count, 0);
}