#ifndef _LODGE_SAMPLER_H
#define _LODGE_SAMPLER_H

struct lodge_sampler
{
	int name;
};

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

struct lodge_sampler_properties
{
	enum lodge_sampler_min_filter	min_filter;
	enum lodge_sampler_max_filter	mag_filter;
	enum lodge_sampler_wrap			wrap_x;
	enum lodge_sampler_wrap			wrap_y;
	enum lodge_sampler_wrap			wrap_z;
};

struct lodge_sampler	lodge_sampler_make();
struct lodge_sampler	lodge_sampler_make_properties(struct lodge_sampler_properties properties);
void					lodge_sampler_reset(struct lodge_sampler *sampler);

void					lodge_sampler_set_min_filter(struct lodge_sampler sampler, enum lodge_sampler_min_filter min_filter);
void					lodge_sampler_set_mag_filter(struct lodge_sampler sampler, enum lodge_sampler_mag_filter mag_filter);

void					lodge_sampler_set_wrap_x(struct lodge_sampler sampler, enum lodge_sampler_wrap wrap);
void					lodge_sampler_set_wrap_y(struct lodge_sampler sampler, enum lodge_sampler_wrap wrap);
void					lodge_sampler_set_wrap_z(struct lodge_sampler sampler, enum lodge_sampler_wrap wrap);

void					lodge_sampler_set_properties(struct lodge_sampler sampler, struct lodge_sampler_properties properties);

#endif