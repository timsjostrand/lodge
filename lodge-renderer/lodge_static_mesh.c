#include "lodge_static_mesh.h"

#include "lodge.h"

#include "lodge_opengl.h"

static lodge_buffer_object_t lodge_buffer_object_from_gl(const GLuint buffer_object)
{
	return (lodge_buffer_object_t)buffer_object;
}

static GLuint lodge_buffer_object_to_gl(const lodge_buffer_object_t buffer_object)
{
	return (GLuint)buffer_object;
}

lodge_buffer_object_t lodge_buffer_object_make_static(const void *data, size_t data_size)
{
	GLuint buffer_object = 0;

	/* Generate vertex buffer? */
	glCreateBuffers(1, &buffer_object);
	GL_OK_OR_GOTO(fail);

	glNamedBufferData(buffer_object, data_size, data, GL_STATIC_DRAW);
	GL_OK_OR_GOTO(fail);

	return lodge_buffer_object_from_gl(buffer_object);

fail:
	ASSERT_FAIL("Failed to make dynamic buffer object");
	if(buffer_object) {
		glDeleteBuffers(1, &buffer_object);
	}
	return 0;
}

lodge_buffer_object_t lodge_buffer_object_make_dynamic(size_t max_size)
{
	GLuint buffer_object = 0;

	/* Generate vertex buffer? */
	glCreateBuffers(1, &buffer_object);
	GL_OK_OR_GOTO(fail);

	glNamedBufferData(buffer_object, max_size, NULL, GL_DYNAMIC_DRAW);
	GL_OK_OR_GOTO(fail);

	return lodge_buffer_object_from_gl(buffer_object);

fail:
	ASSERT_FAIL("Failed to make dynamic buffer object");
	if(buffer_object) {
		glDeleteBuffers(1, &buffer_object);
	}
	return 0;
}

void lodge_buffer_object_reset(lodge_buffer_object_t buffer_object)
{
	if(buffer_object) {
		GLuint tmp = lodge_buffer_object_to_gl(buffer_object);
		glDeleteBuffers(1, &tmp);
		GL_OK_OR_ASSERT("Failed to reset buffer object");
	}
}

void lodge_buffer_object_set(lodge_buffer_object_t buffer_object, size_t offset, const void *data, size_t data_size)
{
	void* mapped_range = glMapNamedBufferRange(lodge_buffer_object_to_gl(buffer_object), offset, data_size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
	GL_OK_OR_GOTO(fail);

	if(mapped_range) {
		memcpy(mapped_range, data, data_size);
		glUnmapNamedBuffer(lodge_buffer_object_to_gl(buffer_object));
		GL_OK_OR_GOTO(fail);
	}
	
	return;

fail:
	ASSERT_FAIL("Failed to set buffer object data");
}

struct lodge_static_mesh lodge_static_mesh_make(const vec3 *vertices, size_t vertices_count, const vec3 *normals, size_t normals_count, const vec2 *tex_coords, size_t tex_coords_count, const uint32_t *indices, size_t indices_count)
{
	ASSERT(vertices_count == normals_count && vertices_count == tex_coords_count);

	return (struct lodge_static_mesh) {
		.vertices = lodge_buffer_object_make_static(vertices, vertices_count * sizeof(vec3)),
		.normals = lodge_buffer_object_make_static(normals, normals_count * sizeof(vec3)),
		.tex_coords = lodge_buffer_object_make_static(tex_coords, tex_coords_count * sizeof(vec2)),
		.indices = lodge_buffer_object_make_static(indices, indices_count * sizeof(uint32_t)),
	};
}

void lodge_static_mesh_reset(struct lodge_static_mesh *static_mesh)
{
	lodge_buffer_object_reset(static_mesh->vertices);
	lodge_buffer_object_reset(static_mesh->normals);
	lodge_buffer_object_reset(static_mesh->tex_coords);
	lodge_buffer_object_reset(static_mesh->indices);

	*static_mesh = (struct lodge_static_mesh) { 0 };
}

/////////////////////

static GLuint lodge_drawable_to_gl(const lodge_drawable_t drawable)
{
	return (GLuint)drawable;
}

static lodge_drawable_t lodge_drawable_from_gl(GLuint drawable)
{
	return (lodge_drawable_t)drawable;
}

lodge_drawable_t lodge_drawable_make(struct lodge_drawable_desc desc)
{
	GLuint drawable = 0;

	glCreateVertexArrays(1, &drawable);
	GL_OK_OR_GOTO(fail);

	for(uint32_t attrib_index = 0; attrib_index < desc.attribs_count; attrib_index++) {
		struct lodge_drawable_attrib *attrib = &desc.attribs[attrib_index];
		lodge_drawable_set_buffer_object(lodge_drawable_from_gl(drawable), attrib_index, *attrib);
	}

	// Indices
	if(desc.indices) {
		lodge_drawable_set_index_buffer(lodge_drawable_from_gl(drawable), desc.indices);
	}

	// Reset VAO binding
	glBindVertexArray(0);
	GL_OK_OR_GOTO(fail);

	return lodge_drawable_from_gl(drawable);

fail:
	ASSERT_FAIL("Failed to create drawable");
	if(drawable) {
		glDeleteVertexArrays(1, &drawable);
	}
	return 0;
}

lodge_drawable_t lodge_drawable_make_from_static_mesh(const struct lodge_static_mesh *static_mesh)
{
	return lodge_drawable_make((struct lodge_drawable_desc) {
		.indices = static_mesh->indices,
		.attribs_count = 3,
		.attribs = {
			{
				.name = strview_static("vertex"),
				.buffer_object = static_mesh->vertices,
				.float_count = 3,
				.stride = sizeof(vec3),
				.instanced = 0,
			},
			{
				.name = strview_static("normal"),
				.buffer_object = static_mesh->normals,
				.float_count = 3,
				.stride = sizeof(vec3),
				.instanced = 0,
			},
			{
				.name = strview_static("tex_coord"),
				.buffer_object = static_mesh->tex_coords,
				.float_count = 2,
				.stride = sizeof(vec2),
				.instanced = 0,
			}
		}
	});
}

void lodge_drawable_reset(lodge_drawable_t drawable)
{
	const GLuint drawable_id = lodge_drawable_to_gl(drawable);
	glDeleteVertexArrays(1, &drawable_id);
	GL_OK_OR_ASSERT("Failed to reset drawable");
}

void lodge_drawable_set_index_buffer(lodge_drawable_t drawable, lodge_buffer_object_t index_buffer)
{
	glBindVertexArray(lodge_drawable_to_gl(drawable));
	GL_OK_OR_GOTO(fail);

	glVertexArrayElementBuffer(lodge_drawable_to_gl(drawable), lodge_buffer_object_to_gl(index_buffer));
	GL_OK_OR_GOTO(fail);

	glBindVertexArray(0);
	GL_OK_OR_GOTO(fail);

	return;

fail:
	ASSERT_FAIL("Failed to set drawable index buffer");
}

void lodge_drawable_set_buffer_object(lodge_drawable_t drawable, uint32_t index, struct lodge_drawable_attrib attrib)
{
	ASSERT(!strview_empty(attrib.name));
	ASSERT(attrib.buffer_object);

	glBindVertexArray(lodge_drawable_to_gl(drawable));
	GL_OK_OR_GOTO(fail);

	glEnableVertexAttribArray(index);
	GL_OK_OR_GOTO(fail);

	if(attrib.buffer_object) {
		glBindBuffer(GL_ARRAY_BUFFER, lodge_buffer_object_to_gl(attrib.buffer_object));
		GL_OK_OR_GOTO(fail);
	}

	glVertexAttribPointer(index,
		attrib.float_count,
		GL_FLOAT,
		GL_FALSE,
		attrib.stride,
		NULL
	);
	GL_OK_OR_GOTO(fail);

	if(attrib.instanced) {
		glVertexAttribDivisor(index, attrib.instanced);
	}
	GL_OK_OR_GOTO(fail);

	glBindVertexArray(0);
	GL_OK_OR_GOTO(fail);

	return;

fail:
	ASSERT_FAIL("Failed to set drawable buffer object");
}

void lodge_drawable_render_lines(const lodge_drawable_t drawable, size_t offset, size_t count)
{
	glBindVertexArray(lodge_drawable_to_gl(drawable));
	GL_OK_OR_ASSERT("Failed to bind lines");
	
	glDrawArrays(GL_LINES, (GLint)offset, count * 2);
	GL_OK_OR_ASSERT("Failed to render lines");
	
	glBindVertexArray(0);
	GL_OK_OR_ASSERT("Failed to unbind drawable");
}

void lodge_drawable_render_indexed_instanced(const lodge_drawable_t drawable, size_t index_count, size_t instances)
{
	glBindVertexArray(lodge_drawable_to_gl(drawable));
	GL_OK_OR_ASSERT("Failed to bind drawable");
	
	glDrawElementsInstanced(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, NULL, instances);
	GL_OK_OR_ASSERT("Failed to render drawable");
	
	glBindVertexArray(0);
	GL_OK_OR_ASSERT("Failed to unbind drawable");
}