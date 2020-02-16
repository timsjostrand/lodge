#include "lodge_texture.h"

#include "lodge_opengl.h"

static texture_t lodge_texture_make(int width, int height, GLint internal_format, GLint format, GLenum type)
{
	GLuint name;

	glGenTextures(1, &name);
	GL_OK_OR_ASSERT("glGenTextures failed");

	glBindTexture(GL_TEXTURE_2D, name);
	GL_OK_OR_ASSERT("glBindTexture failed");

	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, 0);
	GL_OK_OR_ASSERT("glTextImage2D failed");

	return texture_from_gl(name);
}

texture_t lodge_texture_make_rgba(int width, int height)
{
	return lodge_texture_make(width, height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
}

texture_t lodge_texture_make_depth(int width, int height)
{
	return lodge_texture_make(width, height, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT);
}

void lodge_texture_reset(texture_t *texture)
{
	GLuint name = texture_to_gl(*texture);

	ASSERT(name > 0);
	glDeleteTextures(1, &name);
	GL_OK_OR_ASSERT("Failed to delete texture");
	texture = 0;
}