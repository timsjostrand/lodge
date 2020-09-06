#include "lodge_buffer_object.h"

#include "lodge_opengl.h"

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
	ASSERT_FAIL("Failed to make static buffer object");
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
