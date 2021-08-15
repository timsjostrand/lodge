#ifndef _LODGE_SAMPLER_H
#define _LODGE_SAMPLER_H

struct lodge_sampler;
typedef struct lodge_sampler* lodge_sampler_t;

enum lodge_sampler_min_filter
{
	MIN_FILTER_NEAREST,
	MIN_FILTER_LINEAR,
	MIN_FILTER_NEAREST_MIPMAP_NEAREST,
	MIN_FILTER_LINEAR_MIPMAP_NEAREST,
	MIN_FILTER_NEAREST_MIPMAP_LINEAR,
	MIN_FILTER_LINEAR_MIPMAP_LINEAR
};

enum lodge_sampler_mag_filter
{
	MAG_FILTER_NEAREST,
	MAG_FILTER_LINEAR,
};

enum lodge_sampler_wrap
{
	WRAP_CLAMP_TO_EDGE,
	WRAP_MIRRORED_REPEAT,
	WRAP_REPEAT,
	WRAP_MIRROR_CLAMP_TO_EDGE
};

struct lodge_sampler_desc
{
	enum lodge_sampler_min_filter	min_filter;
	enum lodge_sampler_mag_filter	mag_filter;
	enum lodge_sampler_wrap			wrap_x;
	enum lodge_sampler_wrap			wrap_y;
	enum lodge_sampler_wrap			wrap_z;
};

lodge_sampler_t						lodge_sampler_make(struct lodge_sampler_desc desc);
void								lodge_sampler_reset(lodge_sampler_t *sampler);

#endif
