#ifndef _FBX_H
#define _FBX_H

/**
 * Usage:
 *		const char* path[] = { "Objects", "Geometry", "Vertices" };
 *		struct fbx_node *node = fbx_get_node(fbx, path, LODGE_ARRAYSIZE(path));
 *		if(node) {
 *			const size_t property_count = fbx_node_get_property_count(node);
 *			for(int i = 0; i < property_count; i++) {
 *				struct fbx_property* prop = fbx_node_get_property(node, i);
 *				const double* prop_data = fbx_node_get_property_array_double(prop);
 *				if(prop_data) {
 *					upload_vertices(prop_data);
 *					break;
 *				}
 *			}
 *		}
 */

#include <stdlib.h>
#include <stdint.h>

struct fbx;
struct fbx_node;
struct fbx_property;

struct fbx_string
{
	uint32_t		length;
	const char*		data;
};

enum fbx_property_type
{
	FBX_PROPERTY_TYPE_INT16			= 'Y',
	FBX_PROPERTY_TYPE_BOOL			= 'C',
	FBX_PROPERTY_TYPE_INT32			= 'I',
	FBX_PROPERTY_TYPE_FLOAT			= 'F',
	FBX_PROPERTY_TYPE_DOUBLE		= 'D',
	FBX_PROPERTY_TYPE_INT64			= 'L',
	FBX_PROPERTY_TYPE_BINARY		= 'R',
	FBX_PROPERTY_TYPE_STRING		= 'S',
	FBX_PROPERTY_TYPE_ARRAY_FLOAT	= 'f',
	FBX_PROPERTY_TYPE_ARRAY_INT32	= 'i',
	FBX_PROPERTY_TYPE_ARRAY_DOUBLE	= 'd',
	FBX_PROPERTY_TYPE_ARRAY_INT64	= 'l',
	FBX_PROPERTY_TYPE_ARRAY_BOOL	= 'b',
	FBX_PROPERTY_TYPE_ARRAY_CHAR	= 'c',
};

struct fbx*					fbx_new(const char *buf, size_t buf_size);
void						fbx_free(struct fbx *fbx);

struct fbx_node*			fbx_get_node(struct fbx *fbx, const char *path[], size_t path_count);
uint64_t					fbx_node_get_property_count(const struct fbx_node *node);
const struct fbx_property*	fbx_node_get_property(const struct fbx_node *node, uint64_t index);
const struct fbx_property*	fbx_node_get_property_array(const struct fbx_node *node, uint64_t index);

enum fbx_property_type		fbx_property_get_type(const struct fbx_property *prop);
int							fbx_property_is_array(const struct fbx_property *prop);
uint32_t					fbx_property_get_array_count(const struct fbx_property *prop);
const int16_t*				fbx_property_get_int16(const struct fbx_property *prop);
const char*					fbx_property_get_bool(const struct fbx_property *prop);
const int32_t*				fbx_property_get_int32(const struct fbx_property *prop);
const float*				fbx_property_get_float(const struct fbx_property *prop);
const double*				fbx_property_get_double(const struct fbx_property *prop);
const int64_t*				fbx_property_get_int64(const struct fbx_property *prop);
struct fbx_string			fbx_property_get_string(const struct fbx_property *prop);
struct fbx_string			fbx_property_get_binary(const struct fbx_property *prop);
const float*				fbx_property_get_array_float(const struct fbx_property *prop);
const double*				fbx_property_get_array_double(const struct fbx_property *prop);
const int32_t*				fbx_property_get_array_int32(const struct fbx_property *prop);
const int64_t*				fbx_property_get_array_int64(const struct fbx_property *prop);
const char*					fbx_property_get_array_bool(const struct fbx_property *prop);
const char*					fbx_property_get_array_char(const struct fbx_property *prop);

void						fbx_print(const struct fbx *fbx, int print_arrays);

#endif
