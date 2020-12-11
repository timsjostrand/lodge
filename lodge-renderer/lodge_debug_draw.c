#include "lodge_debug_draw.h"

#include "membuf.h"
#include "frustum.h"

#include "lodge_renderer.h"
#include "lodge_static_mesh.h"
#include "lodge_drawable.h"
#include "lodge_buffer_object.h"
#include "lodge_texture.h"
#include "lodge_sampler.h"

struct line_vertex
{
	vec3								p0;
	vec3								p1;
};

struct line_color
{
	vec4								p0;
	vec4								p1;
};

struct lodge_debug_draw_lines
{
	struct line_vertex					vertices[LODGE_DEBUG_DRAW_LINES_MAX];
	struct line_color					colors[LODGE_DEBUG_DRAW_LINES_MAX];
	float								lifetimes[LODGE_DEBUG_DRAW_LINES_MAX];
	size_t								count;

	bool								gpu_dirty;

	lodge_drawable_t					drawable;
	lodge_buffer_object_t				buffer_object_vertices;
	lodge_buffer_object_t				buffer_object_colors;
};

struct lodge_debug_draw_spheres
{
	struct lodge_static_mesh			static_mesh;
	lodge_drawable_t					drawable;
	size_t								index_count;

	vec4								pos_radius[LODGE_DEBUG_DRAW_SPHERES_MAX];
	vec4								colors[LODGE_DEBUG_DRAW_SPHERES_MAX];
	float								lifetimes[LODGE_DEBUG_DRAW_SPHERES_MAX];
	size_t								count;

	bool								gpu_dirty;

	lodge_buffer_object_t				buffer_object_pos_radius;
	lodge_buffer_object_t				buffer_object_colors;
};

struct lodge_debug_draw_textures
{
	lodge_drawable_t					drawable;
	lodge_buffer_object_t				buffer_object;
	lodge_sampler_t						sampler;

	lodge_texture_t						textures[LODGE_DEBUG_DRAW_TEXTURES_MAX];
	float								lifetimes[LODGE_DEBUG_DRAW_TEXTURES_MAX];
	size_t								count;
};

struct lodge_debug_draw
{
	struct lodge_debug_draw_lines		lines;
	struct lodge_debug_draw_spheres		spheres;
	struct lodge_debug_draw_textures	textures;
};

static size_t lodge_debug_draw_calc_sphere_vertex_count(uint32_t sector_count, uint32_t stack_count)
{
	return (stack_count + 1) * (sector_count + 1);
}

//
// Inspired by http://www.songho.ca/opengl/gl_sphere.html
//
static lodge_debug_draw_calc_sphere_vertices(const uint32_t sector_count, const uint32_t stack_count, float radius, membuf_t vertices, membuf_t normals, membuf_t tex_coords)
{
	const float length_inv = 1.0f / radius;
	const float sector_step = 2 * (float)M_PI / sector_count;
	const float stack_step = (float)M_PI / stack_count;

	size_t vertices_count = 0;
	size_t normals_count = 0;
	size_t tex_coord_count = 0;

	vec3 vertex;

	for(uint32_t i = 0; i <= stack_count; ++i) {
		const float stack_angle = (float)M_PI / 2 - i * stack_step;	// starting from pi/2 to -pi/2
		const float xy = radius * cosf(stack_angle);			// r * cos(u)
		vertex.z = radius * sinf(stack_angle);				// r * sin(u)

		// add (sector_count+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for(uint32_t j = 0; j <= sector_count; ++j) {
			const float sector_angle = j * sector_step;		// starting from 0 to 2pi

			// vertex position (x, y, z)
			vertex.x = xy * cosf(sector_angle);				// r * cos(u) * cos(v)
			vertex.y = xy * sinf(sector_angle);				// r * cos(u) * sin(v)
			membuf_append(vertices, &vertices_count, &vertex, sizeof(vec3));

			// normalized vertex normal (nx, ny, nz)
			const vec3 normal = vec3_mult_scalar(vertex, length_inv);
			membuf_append(normals, &normals_count, &normal, sizeof(vec3));

			// vertex tex coord (s, t) range between [0, 1]
			const vec2 tex_coord = vec2_make((float)j / sector_count, (float)i / stack_count);
			membuf_append( tex_coords, &tex_coord_count, &tex_coord, sizeof(vec2));
		}
	}
}

//
// generate CCW index array
//
static void lodge_debug_draw_calc_sphere_indices(const uint32_t sector_count, const uint32_t stack_count, membuf_t indices)
{
	uint32_t k1, k2;

	size_t indices_count = 0;

	for(uint32_t i = 0; i < stack_count; ++i) {
		k1 = i * (sector_count + 1);     // beginning of current stack
		k2 = k1 + sector_count + 1;      // beginning of next stack

		for(uint32_t j = 0; j < sector_count; ++j, ++k1, ++k2) {
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if(i != 0) {
				membuf_append(indices, &indices_count, &k1, sizeof(uint32_t));
				membuf_append(indices, &indices_count, &k2, sizeof(uint32_t));
				membuf_append(indices, &indices_count, &(uint32_t){ k1 + 1 }, sizeof(uint32_t));
			}

			// k1+1 => k2 => k2+1
			if(i != (stack_count - 1)) {
				membuf_append(indices, &indices_count, &(uint32_t){ k1 + 1 }, sizeof(uint32_t));
				membuf_append(indices, &indices_count, &k2, sizeof(uint32_t));
				membuf_append(indices, &indices_count, &(uint32_t){ k2 + 1 }, sizeof(uint32_t));
			}
		}
	}
}

static void lodge_debug_draw_lines_new_inplace(struct lodge_debug_draw_lines *lines)
{
	// Platform sanity checks
	ASSERT(sizeof_member(struct lodge_debug_draw_lines, vertices) == sizeof(vec3) * 2 * LODGE_DEBUG_DRAW_LINES_MAX);
	ASSERT(sizeof_member(struct lodge_debug_draw_lines, colors) == sizeof(vec4) * 2 * LODGE_DEBUG_DRAW_LINES_MAX);

	lines->buffer_object_vertices = lodge_buffer_object_make_dynamic(sizeof(lines->vertices));
	lines->buffer_object_colors = lodge_buffer_object_make_dynamic(sizeof(lines->colors));

	lines->drawable = lodge_drawable_make((struct lodge_drawable_desc) {
		.indices = NULL,
		.attribs_count = 2,
		.attribs = {
			{
				.name = strview_static("vertex"),
				.buffer_object = lines->buffer_object_vertices,
				.float_count = 3,
				.stride = sizeof(vec3),
				.instanced = 0
			},
			{
				.name = strview_static("color"),
				.buffer_object = lines->buffer_object_colors,
				.float_count = 4,
				.stride = sizeof(vec4),
				.instanced = 0
			}
		}
	});
}

static void lodge_debug_draw_spheres_new_inplace(struct lodge_debug_draw_spheres *spheres)
{
#define STACK_COUNT		16
#define SECTOR_COUNT	16
#define VERTEX_COUNT	((STACK_COUNT + 1) * (SECTOR_COUNT + 1))
#define INDEX_COUNT		(((STACK_COUNT - 2) * (SECTOR_COUNT * 2) * 3) + (2 * (SECTOR_COUNT) * 3)) // 2 triangles per sector excluding first and last stacks

	// Sanity checks
	ASSERT(sizeof_member(struct lodge_debug_draw_spheres, pos_radius) == sizeof(vec4) * LODGE_DEBUG_DRAW_SPHERES_MAX);
	ASSERT(sizeof_member(struct lodge_debug_draw_spheres, colors) == sizeof(vec4) * LODGE_DEBUG_DRAW_SPHERES_MAX);

	vec3 vertices[VERTEX_COUNT];
	vec3 normals[VERTEX_COUNT];
	vec2 tex_coords[VERTEX_COUNT];
	uint32_t indices[INDEX_COUNT];

	lodge_debug_draw_calc_sphere_vertices(SECTOR_COUNT, STACK_COUNT, 1.0f, membuf_wrap(vertices), membuf_wrap(normals), membuf_wrap(tex_coords));
	lodge_debug_draw_calc_sphere_indices(SECTOR_COUNT, STACK_COUNT, membuf_wrap(indices));

	spheres->static_mesh = lodge_static_mesh_make(vertices, VERTEX_COUNT, normals, VERTEX_COUNT, tex_coords, VERTEX_COUNT, indices, INDEX_COUNT);
	spheres->index_count = INDEX_COUNT;

	spheres->drawable = lodge_drawable_make_from_static_mesh(&spheres->static_mesh);

	spheres->buffer_object_pos_radius = lodge_buffer_object_make_dynamic(sizeof(spheres->pos_radius));
	spheres->buffer_object_colors = lodge_buffer_object_make_dynamic(sizeof(spheres->colors));
		
	lodge_drawable_set_buffer_object(spheres->drawable, 3, (struct lodge_drawable_attrib) {
		.name = strview_static("pos_radius"),
		.buffer_object = spheres->buffer_object_pos_radius,
		.offset = 0,
		.float_count = 4,
		.stride = sizeof(vec4),
		.instanced = 1
	});
	lodge_drawable_set_buffer_object(spheres->drawable, 4, (struct lodge_drawable_attrib) {
		.name = strview_static("color"),
		.buffer_object = spheres->buffer_object_colors,
		.offset = 0,
		.float_count = 4,
		.stride = sizeof(vec4),
		.instanced = 1
	});

#undef STACK_COUNT
#undef SECTOR_COUNT
#undef VERTEX_COUNT
#undef INDEX_COUNT
}

void lodge_debug_draw_textures_new_inplace(struct lodge_debug_draw_textures *textures)
{
#if 1
	const float x = 0.0f;
	const float y = 0.0f;

	const float w = 1.0f / 2.0f;
	const float h = 1.0f / 2.0f;

	const float vertex_data[] = {
		/* Top-left */
		x - w,		// x
		y + h,		// y
		0.0f,		// z
		0.0f,		// u
		0.0f,		// v
		/* Bottom-Left */
		x - w,		// x
		y - h,		// y
		0.0f,		// z
		0.0f,		// u
		1.0f,		// v
		/* Top-right */
		x + w,		// x
		y + h,		// y
		0.0f,		// z
		1.0f,		// u
		0.0f,		// v
		/* Top-right */
		x + w,		// x
		y + h,		// y
		0.0f,		// z
		1.0f,		// u
		0.0f,		// v
		/* Bottom-left */
		x - w,		// x
		y - h,		// y
		0.0f,		// z
		0.0f,		// u
		1.0f,		// v
		/* Bottom-right */
		x + w,		// x
		y - h,		// y
		0.0f,		// z
		1.0f,		// u
		1.0f,		// v
	};
#else
	const float vertex_data[] = {
		/* Top-left */
		0.0f,		// x
		1.0f,		// y
		0.0f,		// z
		0.0f,		// u
		0.0f,		// v
		/* Bottom-Left */
		0.0f,		// x
		0.0f,		// y
		0.0f,		// z
		0.0f,		// u
		1.0f,		// v
		/* Top-right */
		1.0f,		// x
		1.0f,		// y
		0.0f,		// z
		1.0f,		// u
		0.0f,		// v
		/* Top-right */
		1.0f,		// x
		1.0f,		// y
		0.0f,		// z
		1.0f,		// u
		0.0f,		// v
		/* Bottom-left */
		0.0f,		// x
		0.0f,		// y
		0.0f,		// z
		0.0f,		// u
		1.0f,		// v
		/* Bottom-right */
		1.0f,		// x
		0.0f,		// y
		0.0f,		// z
		1.0f,		// u
		1.0f,		// v
	};
#endif

	textures->buffer_object = lodge_buffer_object_make_static(vertex_data, sizeof(vertex_data));
	textures->sampler = lodge_sampler_make_properties((struct lodge_sampler_properties) {
		.min_filter = MAG_FILTER_LINEAR,
		.mag_filter = MAG_FILTER_LINEAR,
		.wrap_x = WRAP_CLAMP_TO_EDGE,
		.wrap_y = WRAP_CLAMP_TO_EDGE,
		.wrap_z = WRAP_CLAMP_TO_EDGE
	});

	textures->drawable = lodge_drawable_make((struct lodge_drawable_desc) {
		.attribs_count = 2,
		.attribs = {
			(struct lodge_drawable_attrib) {
				.name = strview_static("pos"),
				.buffer_object = textures->buffer_object,
				.offset = 0,
				.float_count = 3,
				.stride = 5 * sizeof(float),
				.instanced = 0
			},
			(struct lodge_drawable_attrib) {
				.name = strview_static("uv"),
				.buffer_object = textures->buffer_object,
				.offset = 3 * sizeof(float),
				.float_count = 2,
				.stride = 5 * sizeof(float),
				.instanced = 0
			},
		}
	});
}

void lodge_debug_draw_new_inplace(struct lodge_debug_draw *debug_draw)
{
	memset(debug_draw, 0, sizeof(struct lodge_debug_draw));

	lodge_debug_draw_lines_new_inplace(&debug_draw->lines);
	lodge_debug_draw_spheres_new_inplace(&debug_draw->spheres);
	lodge_debug_draw_textures_new_inplace(&debug_draw->textures);
}

static void lodge_debug_draw_lines_free_inplace(struct lodge_debug_draw_lines *lines)
{
	lodge_buffer_object_reset(lines->buffer_object_vertices);
	lodge_buffer_object_reset(lines->buffer_object_colors);
	lodge_drawable_reset(lines->drawable);
}

static void lodge_debug_draw_spheres_free_inplace(struct lodge_debug_draw_spheres *spheres)
{
	lodge_buffer_object_reset(spheres->buffer_object_pos_radius);
	lodge_buffer_object_reset(spheres->buffer_object_colors);
	lodge_static_mesh_reset(&spheres->static_mesh);
	lodge_drawable_reset(spheres->drawable);
}

static void lodge_debug_draw_textures_free_inplace(struct lodge_debug_draw_textures *textures)
{
	lodge_buffer_object_reset(textures->buffer_object);
	lodge_drawable_reset(textures->drawable);
}

void lodge_debug_draw_free_inplace(struct lodge_debug_draw *debug_draw)
{
	lodge_debug_draw_lines_free_inplace(&debug_draw->lines);
	lodge_debug_draw_spheres_free_inplace(&debug_draw->spheres);
	lodge_debug_draw_textures_free_inplace(&debug_draw->textures);
}

size_t lodge_debug_draw_sizeof()
{
	return sizeof(struct lodge_debug_draw);
}

static void lodge_debug_draw_lines_update(struct lodge_debug_draw_lines *lines, float dt)
{
	for(size_t i = 0; i < lines->count; i++) {
		lines->lifetimes[i] -= dt;

		if(lines->lifetimes[i] < 0.0f) {
			struct membuf_swapret ret = membuf_delete_swap_tail(membuf_wrap(lines->vertices), &lines->count, i);

			if(ret.index_a != ret.index_b) {
				membuf_swap(membuf_wrap(lines->colors), ret.index_a, ret.index_b);
				membuf_swap(membuf_wrap(lines->lifetimes), ret.index_a, ret.index_b);
			}

			i--;

			lines->gpu_dirty = true;
		}
	}
}

static void lodge_debug_draw_spheres_update(struct lodge_debug_draw_spheres *spheres, float dt)
{
	for(size_t i = 0; i < spheres->count; i++) {
		spheres->lifetimes[i] -= dt;

		if(spheres->lifetimes[i] < 0.0f) {
			struct membuf_swapret ret = membuf_delete_swap_tail(membuf_wrap(spheres->pos_radius), &spheres->count, i);

			if(ret.index_a != ret.index_b) {
				membuf_swap(membuf_wrap(spheres->colors), ret.index_a, ret.index_b);
				membuf_swap(membuf_wrap(spheres->lifetimes), ret.index_a, ret.index_b);
			}

			i--;

			spheres->gpu_dirty = true;
		}
	}
}

static void lodge_debug_draw_textures_update(struct lodge_debug_draw_textures *textures, float dt)
{
	for(size_t i = 0; i < textures->count; i++) {
		textures->lifetimes[i] -= dt;

		if(textures->lifetimes[i] < 0.0f) {
			struct membuf_swapret ret = membuf_delete_swap_tail(membuf_wrap(textures->textures), &textures->count, i);

			if(ret.index_a != ret.index_b) {
				membuf_swap(membuf_wrap(textures->lifetimes), ret.index_a, ret.index_b);
			}

			i--;

			//textures->gpu_dirty = true;
		}
	}
}


void lodge_debug_draw_update(struct lodge_debug_draw *debug_draw, float dt)
{
	lodge_debug_draw_lines_update(&debug_draw->lines, dt);
	lodge_debug_draw_spheres_update(&debug_draw->spheres, dt);
	lodge_debug_draw_textures_update(&debug_draw->textures, dt);
}

static void lodge_debug_draw_lines_render(struct lodge_debug_draw_lines *lines, lodge_shader_t shader, struct mvp mvp)
{
	lodge_renderer_bind_shader(shader);
	lodge_renderer_set_constant_mvp(shader, &mvp);

	if(lines->gpu_dirty && lines->count > 0) {
		lines->gpu_dirty = false;

		lodge_buffer_object_set(lines->buffer_object_vertices, 0, lines->vertices, sizeof(struct line_vertex) * lines->count);
		lodge_buffer_object_set(lines->buffer_object_colors, 0, lines->colors, sizeof(struct line_color) * lines->count);
	}

	lodge_drawable_render_lines(lines->drawable, 0, lines->count);
}

static void lodge_debug_draw_spheres_render(struct lodge_debug_draw_spheres *spheres, lodge_shader_t shader, struct mvp mvp)
{
	if(spheres->gpu_dirty && spheres->count > 0) {
		spheres->gpu_dirty = false;

		lodge_buffer_object_set(spheres->buffer_object_pos_radius, 0, spheres->pos_radius, sizeof(vec4) * spheres->count);
		lodge_buffer_object_set(spheres->buffer_object_colors, 0, spheres->colors, sizeof(vec4) * spheres->count);
	}

	lodge_renderer_bind_shader(shader);
	lodge_renderer_set_constant_mvp(shader, &mvp);
	lodge_drawable_render_indexed_instanced(spheres->drawable, spheres->index_count, spheres->count);
}

static void lodge_debug_draw_textures_render(struct lodge_debug_draw_textures *textures, lodge_shader_t shader)
{
	if(textures->count == 0) {
		return;
	}

	lodge_renderer_bind_shader(shader);

	const uint32_t grid_size = (uint32_t)ceil(sqrt((double)textures->count));
	const float grid_size_screen = 1.0f/(grid_size);

	for(size_t i = 0; i < textures->count; i++) {
		const uint32_t grid_x = i % grid_size;
		const uint32_t grid_y = i / grid_size;

		const float x = grid_x * grid_size_screen - grid_size_screen*(grid_size-1)/2.0f;
		const float y = grid_y * grid_size_screen - grid_size_screen*(grid_size-1)/2.0f;

		const struct mvp mvp = {
			.model = mat4_mult(mat4_translation(x, y, 0.0f), mat4_scaling(grid_size_screen, grid_size_screen, 1.0f)),
			.view = mat4_identity(),
			.projection = mat4_ortho(-0.5f, 0.5f, 0.5f, -0.5f, -1.0f, 1.0f)
		};
		lodge_renderer_set_constant_mvp(shader, &mvp);
		lodge_renderer_bind_texture_unit_2d(0, textures->textures[i], textures->sampler);
		lodge_drawable_render_triangles(textures->drawable, 0, 6);
	}
}

//
// FIXME(TS): should have nicer api than passing several shaders
//
void lodge_debug_draw_render(struct lodge_debug_draw *debug_draw, lodge_shader_t shader[3], struct mvp mvp)
{
	lodge_debug_draw_lines_render(&debug_draw->lines, shader[0], mvp);
	lodge_debug_draw_spheres_render(&debug_draw->spheres, shader[1], mvp);
	lodge_debug_draw_textures_render(&debug_draw->textures, shader[2]);
}

static void lodge_debug_draw_line_impl(struct lodge_debug_draw_lines *lines, struct line line, vec4 color, float lifetime)
{
	membuf_append(membuf_wrap(lines->vertices), &lines->count, &(struct line_vertex) {
		.p0 = line.p0,
		.p1 = line.p1,
	}, sizeof(struct line_vertex));

	membuf_set(membuf_wrap(lines->colors), lines->count - 1, &(struct line_color) {
		.p0 = color,
		.p1 = color,
	}, sizeof(struct line_color));

	membuf_set(membuf_wrap(lines->lifetimes), lines->count - 1, &lifetime, sizeof(float));

	lines->gpu_dirty = true;
}

void lodge_debug_draw_line(struct lodge_debug_draw *debug_draw, struct line line, vec4 color, float lifetime)
{
	lodge_debug_draw_line_impl(&debug_draw->lines, line, color, lifetime);
}

static void lodge_debug_draw_sphere_impl(struct lodge_debug_draw_spheres *spheres, struct sphere sphere, vec4 color, float lifetime)
{
	membuf_append(membuf_wrap(spheres->pos_radius), &spheres->count, &(vec4) { .x = sphere.pos.x, .y = sphere.pos.y, .z = sphere.pos.z, .w = sphere.r }, sizeof(vec4));
	membuf_set(membuf_wrap(spheres->colors), spheres->count - 1, &color, sizeof(vec4));
	membuf_set(membuf_wrap(spheres->lifetimes), spheres->count - 1, &lifetime, sizeof(float));

	spheres->gpu_dirty = true;
}

void lodge_debug_draw_sphere(struct lodge_debug_draw *debug_draw, struct sphere sphere, vec4 color, float lifetime)
{
	lodge_debug_draw_sphere_impl(&debug_draw->spheres, sphere, color, lifetime);
}

void lodge_debug_draw_aabb_outline(struct lodge_debug_draw *debug_draw, struct aabb aabb, vec4 color, float lifetime)
{
	ASSERT(aabb.max.x >= aabb.min.x
		&& aabb.max.y >= aabb.min.y
		&& aabb.max.z >= aabb.min.z
	);

	const vec3 dim = vec3_sub(aabb.max, aabb.min);

	const vec3 points[] = {
		vec3_add(aabb.min, vec3_make(0.0f,	dim.y,	0.0f)),
		vec3_add(aabb.min, vec3_make(dim.x,	dim.y,	0.0f)),
		vec3_add(aabb.min, vec3_make(dim.x,	0.0f,	0.0f)),

		vec3_add(aabb.min, vec3_make(0.0f,	0.0f,	dim.z)),
		vec3_add(aabb.min, vec3_make(0.0f,	dim.y,	dim.z)),
		vec3_add(aabb.min, vec3_make(dim.x,	0.0f,	dim.z)),
	};

	// "bottom" quad
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = aabb.min,	.p1 = points[0] },		color, lifetime);
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = points[0],	.p1 = points[1] },		color, lifetime);
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = points[1],	.p1 = points[2] },		color, lifetime);
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = points[2],	.p1 = aabb.min },		color, lifetime);
	
	// "top" quad
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = points[3],	.p1 = points[4] },		color, lifetime);
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = points[4],	.p1 = aabb.max },		color, lifetime);
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = aabb.max,	.p1 = points[5] },		color, lifetime);
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = points[5],	.p1 = points[3] },		color, lifetime);

	// connecting "pillars"
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = aabb.min,	.p1 = points[3] },		color, lifetime);
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = points[2],	.p1 = points[5] },		color, lifetime);
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = points[0],	.p1 = points[4] },		color, lifetime);
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = points[1],	.p1 = aabb.max },		color, lifetime);
}

void lodge_debug_draw_frustum(struct lodge_debug_draw *debug_draw, struct frustum_corners c, vec4 color, float lifetime)
{
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = *frustum_corners_get(&c, 0, 0, 0), .p1 = *frustum_corners_get(&c, 1, 0, 0) }, color, lifetime);
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = *frustum_corners_get(&c, 1, 0, 0), .p1 = *frustum_corners_get(&c, 0, 1, 0) }, color, lifetime);
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = *frustum_corners_get(&c, 0, 1, 0), .p1 = *frustum_corners_get(&c, 1, 1, 0) }, color, lifetime);
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = *frustum_corners_get(&c, 1, 1, 0), .p1 = *frustum_corners_get(&c, 0, 0, 0) }, color, lifetime);

	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = *frustum_corners_get(&c, 0, 0, 1), .p1 = *frustum_corners_get(&c, 1, 0, 1) }, color, lifetime);
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = *frustum_corners_get(&c, 1, 0, 1), .p1 = *frustum_corners_get(&c, 0, 1, 1) }, color, lifetime);
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = *frustum_corners_get(&c, 0, 1, 1), .p1 = *frustum_corners_get(&c, 1, 1, 1) }, color, lifetime);
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = *frustum_corners_get(&c, 1, 1, 1), .p1 = *frustum_corners_get(&c, 0, 0, 1) }, color, lifetime);

	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = *frustum_corners_get(&c, 0, 0, 0), .p1 = *frustum_corners_get(&c, 0, 0, 1) }, color, lifetime);
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = *frustum_corners_get(&c, 0, 1, 0), .p1 = *frustum_corners_get(&c, 0, 1, 1) }, color, lifetime);
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = *frustum_corners_get(&c, 1, 1, 0), .p1 = *frustum_corners_get(&c, 1, 1, 1) }, color, lifetime);
	lodge_debug_draw_line(debug_draw, (struct line) { .p0 = *frustum_corners_get(&c, 1, 0, 0), .p1 = *frustum_corners_get(&c, 1, 0, 1) }, color, lifetime);
}

void lodge_debug_draw_texture(struct lodge_debug_draw *debug_draw, lodge_texture_t texture, float lifetime)
{
	membuf_append(membuf_wrap(debug_draw->textures.textures), &debug_draw->textures.count, &texture, sizeof(lodge_texture_t));
	membuf_set(membuf_wrap(debug_draw->textures.lifetimes), debug_draw->textures.count - 1, &lifetime, sizeof(float));
}

size_t lodge_debug_draw_get_line_count(struct lodge_debug_draw *debug_draw)
{
	return debug_draw->lines.count;
}

size_t lodge_debug_draw_get_sphere_count(struct lodge_debug_draw *debug_draw)
{
	return debug_draw->spheres.count;
}

size_t lodge_debug_draw_get_texture_count(struct lodge_debug_draw *debug_draw)
{
	return debug_draw->textures.count;
}