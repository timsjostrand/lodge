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

struct lodge_sampler_properties
{
	enum lodge_sampler_min_filter	min_filter;
	enum lodge_sampler_mag_filter	mag_filter;
	enum lodge_sampler_wrap			wrap_x;
	enum lodge_sampler_wrap			wrap_y;
	enum lodge_sampler_wrap			wrap_z;
};

lodge_sampler_t		lodge_sampler_make();
lodge_sampler_t		lodge_sampler_make_properties(struct lodge_sampler_properties properties);
void				lodge_sampler_reset(lodge_sampler_t *sampler);

void				lodge_sampler_set_min_filter(lodge_sampler_t sampler, enum lodge_sampler_min_filter min_filter);
void				lodge_sampler_set_mag_filter(lodge_sampler_t sampler, enum lodge_sampler_mag_filter mag_filter);

void				lodge_sampler_set_wrap_x(lodge_sampler_t sampler, enum lodge_sampler_wrap wrap);
void				lodge_sampler_set_wrap_y(lodge_sampler_t sampler, enum lodge_sampler_wrap wrap);
void				lodge_sampler_set_wrap_z(lodge_sampler_t sampler, enum lodge_sampler_wrap wrap);

void				lodge_sampler_set_properties(lodge_sampler_t sampler, struct lodge_sampler_properties properties);

#endif
