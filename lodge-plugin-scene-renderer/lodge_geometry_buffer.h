#ifndef _LODGE_GEOMETRY_BUFFER_H
#define _LODGE_GEOMETRY_BUFFER_H

#include <stdint.h>

struct lodge_framebuffer;
typedef struct lodge_framebuffer* lodge_framebuffer_t;

struct lodge_texture_t;
typedef struct lodge_texture* lodge_texture_t;

struct lodge_geometry_buffer
{
	lodge_framebuffer_t			framebuffer;
	
	lodge_texture_t				albedo;

	lodge_texture_t				normals;

	//
	// 16 bit RGBA
	// 
	// R = entity_id
	// G = entity_selected
	// B = none
	// A = none
	//
	lodge_texture_t				editor;

	lodge_texture_t				depth;
};

struct lodge_geometry_buffer	lodge_geometry_buffer_make(uint32_t width, uint32_t height);
void							lodge_geometry_buffer_reset(struct lodge_geometry_buffer *gbuffer);
void							lodge_geometry_buffer_remake(struct lodge_geometry_buffer *gbuffer, uint32_t width, uint32_t height);

#endif