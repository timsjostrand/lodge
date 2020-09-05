#ifndef _FBX_ASSET_H
#define _FBX_ASSET_H

#include "math4.h"
#include "lodge_renderer.h"

#include <stdint.h>

struct fbx;

struct lodge_shader;
typedef struct lodge_shader* lodge_shader_t;

struct fbx_asset
{
	uint32_t		vertex_array_object;
	uint32_t		buffer_object_vertices;
	uint32_t		buffer_object_indices;
	uint32_t		buffer_object_normals;
	uint32_t		buffer_object_uvs;
	uint32_t		indices_count;
};

struct fbx_asset	fbx_asset_make(struct fbx *fbx);
void				fbx_asset_reset(struct fbx_asset *asset);

void				fbx_asset_render(struct fbx_asset *asset, lodge_shader_t shader, lodge_texture_t tex, struct mvp mvp);

#endif