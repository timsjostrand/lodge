#include "lodge_drawable.h"

#include "lodge_static_mesh.h"
#include "lodge_opengl.h"

struct lodge_drawable_desc lodge_drawable_desc_make_from_static_mesh(const struct lodge_static_mesh *static_mesh)
{
	return (struct lodge_drawable_desc) {
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
	};
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
	return lodge_drawable_make(lodge_drawable_desc_make_from_static_mesh(static_mesh));
}

void lodge_drawable_reset(lodge_drawable_t drawable)
{
	glDeleteVertexArrays(1, &(GLuint){ lodge_drawable_to_gl(drawable) });
	GL_OK_OR_ASSERT("Failed to reset drawable");
}

void lodge_drawable_set_index_buffer(lodge_drawable_t drawable, lodge_buffer_object_t index_buffer)
{
	glVertexArrayElementBuffer(lodge_drawable_to_gl(drawable), lodge_buffer_object_to_gl(index_buffer));
	GL_OK_OR_GOTO(fail);
	return;

fail:
	ASSERT_FAIL("Failed to set drawable index buffer");
}

void lodge_drawable_set_buffer_object(lodge_drawable_t drawable, uint32_t index, struct lodge_drawable_attrib attrib)
{
	ASSERT(!strview_empty(attrib.name));
	ASSERT(attrib.buffer_object);

	GLuint vertex_array = lodge_drawable_to_gl(drawable);
	glEnableVertexArrayAttrib(vertex_array, index);
	glVertexArrayVertexBuffer(vertex_array, index, lodge_buffer_object_to_gl(attrib.buffer_object), attrib.offset, (GLsizei)attrib.stride);
	glVertexArrayAttribFormat(vertex_array, index, attrib.float_count, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vertex_array, index, index);
	glVertexArrayBindingDivisor(vertex_array, index, attrib.instanced);
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

void lodge_drawable_render_triangles(const lodge_drawable_t drawable, size_t offset, size_t count)
{
	glBindVertexArray(lodge_drawable_to_gl(drawable));
	GL_OK_OR_ASSERT("Failed to bind drawable");
	
	glDrawArrays(GL_TRIANGLES, (GLint)offset, count);
	GL_OK_OR_ASSERT("Failed to render triangles");

	glBindVertexArray(0);
	GL_OK_OR_ASSERT("Failed to unbind drawable");
}

void lodge_drawable_render_triangles_instanced(const lodge_drawable_t drawable, size_t offset, size_t count, size_t instances)
{
	glBindVertexArray(lodge_drawable_to_gl(drawable));
	GL_OK_OR_ASSERT("Failed to bind drawable");

	glDrawArraysInstanced(GL_TRIANGLES, (GLint)offset, count, instances);
	GL_OK_OR_ASSERT("Failed to render drawable");
	
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

void lodge_drawable_render_indexed(const lodge_drawable_t drawable, size_t index_count, size_t offset)
{
	glBindVertexArray(lodge_drawable_to_gl(drawable));
	GL_OK_OR_ASSERT("Failed to bind drawable");

	glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, (const void*)offset);
	GL_OK_OR_ASSERT("Failed to render drawable");
	
	glBindVertexArray(0);
	GL_OK_OR_ASSERT("Failed to unbind drawable");
}