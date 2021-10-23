#ifndef _LODGE_RECT_H
#define _LODGE_RECT_H

#include <stdint.h>

struct lodge_recti
{
	int32_t	x0;
	int32_t	y0;
	int32_t	x1;
	int32_t	y1;
};

static inline int32_t lodge_recti_get_width(struct lodge_recti *rect)
{
	return rect->x1 - rect->x0;
}

static inline int32_t lodge_recti_get_height(struct lodge_recti *rect)
{
	return rect->y1 - rect->y0;
}

#endif