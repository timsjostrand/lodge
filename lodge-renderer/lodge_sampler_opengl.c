#include "lodge_sampler.h"

#include "lodge_opengl.h"

static GLenum lodge_sampler_min_filter_to_gl(enum lodge_sampler_min_filter min_filter)
{
	switch(min_filter)
	{
	case MIN_FILTER_NEAREST:
		return GL_NEAREST;
	case MIN_FILTER_LINEAR:
		return GL_LINEAR;
	case MIN_FILTER_NEAREST_MIPMAP_NEAREST:
		return GL_NEAREST_MIPMAP_NEAREST;
	case MIN_FILTER_LINEAR_MIPMAP_NEAREST:
		return GL_LINEAR_MIPMAP_NEAREST;
	case MIN_FILTER_NEAREST_MIPMAP_LINEAR:
		return GL_NEAREST_MIPMAP_LINEAR;
	case MIN_FILTER_LINEAR_MIPMAP_LINEAR:
		return GL_LINEAR_MIPMAP_LINEAR;
	}

	ASSERT_FAIL("Invalid lodge_sampler_min_filter");
	return GL_NEAREST_MIPMAP_LINEAR;
};

static GLenum lodge_sampler_mag_filter_to_gl(enum lodge_sampler_mag_filter mag_filter)
{
	switch(mag_filter)
	{
	case MAG_FILTER_NEAREST:
		return GL_NEAREST;
	case MAG_FILTER_LINEAR:
		return GL_LINEAR;
	}

	ASSERT_FAIL("Invalid lodge_sampler_mag_filter");
	return GL_LINEAR;
}

static GLenum lodge_sampler_wrap_to_gl(enum lodge_sampler_wrap wrap)
{
	switch(wrap)
	{
	case WRAP_CLAMP_TO_EDGE:
		return GL_CLAMP_TO_EDGE;
	case WRAP_MIRRORED_REPEAT:
		return GL_MIRRORED_REPEAT;
	case WRAP_REPEAT:
		return GL_REPEAT;
	case WRAP_MIRROR_CLAMP_TO_EDGE:
		return GL_MIRROR_CLAMP_TO_EDGE;
	}

	ASSERT_FAIL("Invalid lodge_sampler_wrap");
	return GL_REPEAT;
}


lodge_sampler_t lodge_sampler_make()
{
	GLuint name;
	glGenSamplers(1, &name);
	GL_OK_OR_RETURN(NULL);
	return lodge_sampler_from_gl(name);
}

lodge_sampler_t lodge_sampler_make_properties(struct lodge_sampler_properties properties)
{
	lodge_sampler_t sampler = lodge_sampler_make();
	lodge_sampler_set_properties(sampler, properties);
	return sampler;
}

void lodge_sampler_reset(lodge_sampler_t *sampler)
{
	ASSERT(sampler != NULL && *sampler != NULL);

	GLuint name = lodge_sampler_to_gl(*sampler);
	glDeleteSamplers(1, &name);
	GL_OK_OR_ASSERT("Failed to delete sampler");
	*sampler = NULL;
}

void lodge_sampler_set_min_filter(lodge_sampler_t sampler, enum lodge_sampler_min_filter min_filter)
{
	glSamplerParameteri(lodge_sampler_to_gl(sampler), GL_TEXTURE_MIN_FILTER, lodge_sampler_min_filter_to_gl(min_filter));
	GL_OK_OR_ASSERT("Failed lodge_sampler_set_min_filter");
}

void lodge_sampler_set_mag_filter(lodge_sampler_t sampler, enum lodge_sampler_mag_filter mag_filter)
{
	glSamplerParameteri(lodge_sampler_to_gl(sampler), GL_TEXTURE_MAG_FILTER, lodge_sampler_mag_filter_to_gl(mag_filter));
	GL_OK_OR_ASSERT("Failed lodge_sampler_set_mag_filter");
}

void lodge_sampler_set_wrap_x(lodge_sampler_t sampler, enum lodge_sampler_wrap wrap)
{
	glSamplerParameteri(lodge_sampler_to_gl(sampler), GL_TEXTURE_WRAP_S, lodge_sampler_wrap_to_gl(wrap));
	GL_OK_OR_ASSERT("Failed lodge_sampler_set_wrap_x");
}

void lodge_sampler_set_wrap_y(lodge_sampler_t sampler, enum lodge_sampler_wrap wrap)
{
	glSamplerParameteri(lodge_sampler_to_gl(sampler), GL_TEXTURE_WRAP_T, lodge_sampler_wrap_to_gl(wrap));
	GL_OK_OR_ASSERT("Failed lodge_sampler_set_wrap_y");
}

void lodge_sampler_set_wrap_z(lodge_sampler_t sampler, enum lodge_sampler_wrap wrap)
{
	glSamplerParameteri(lodge_sampler_to_gl(sampler), GL_TEXTURE_WRAP_R, lodge_sampler_wrap_to_gl(wrap));
	GL_OK_OR_ASSERT("Failed lodge_sampler_set_wrap_z");
}

void lodge_sampler_set_properties(lodge_sampler_t sampler, struct lodge_sampler_properties properties)
{
	lodge_sampler_set_min_filter(sampler, properties.min_filter);
	lodge_sampler_set_mag_filter(sampler, properties.mag_filter);
	lodge_sampler_set_wrap_x(sampler, properties.wrap_x);
	lodge_sampler_set_wrap_y(sampler, properties.wrap_y);
	lodge_sampler_set_wrap_z(sampler, properties.wrap_z);
}
