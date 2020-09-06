#ifndef _FBX_ASSET_H
#define _FBX_ASSET_H

#include "math4.h"
#include "lodge_static_mesh.h"

#include <stdint.h>

struct fbx;

struct lodge_drawable;
typedef struct lodge_drawable* lodge_drawable_t;

struct lodge_shader;
typedef struct lodge_shader* lodge_shader_t;

struct lodge_texture;
typedef struct lodge_texture* lodge_texture_t;

struct fbx_asset
{
	lodge_drawable_t			drawable;
	struct lodge_static_mesh	static_mesh;
};

struct fbx_asset				fbx_asset_make(struct fbx *fbx);
void							fbx_asset_reset(struct fbx_asset *asset);

void							fbx_asset_render(const struct fbx_asset *asset, lodge_shader_t shader, lodge_texture_t tex, struct mvp mvp);

#endif